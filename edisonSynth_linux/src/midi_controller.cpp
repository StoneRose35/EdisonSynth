/*
 * midi_controller.cpp
 *
 *  Created on: Jan 23, 2016
 *      Author: philipp
 */

#include <pthread.h>

#include <iostream>
#include <alsa/asoundlib.h>
#include "midi_controller.h"
#include "constants.h"
#include "Voice.h"
#include "edisonSynth.h"

using namespace std;

Voice ** voices_for_midi;
snd_seq_t *seq_handle;
pthread_t midi_controller_thread;

snd_seq_t *open_seq(char* midi_dev ) {

  int portid;

  if (snd_seq_open(&seq_handle, midi_dev, SND_SEQ_OPEN_DUPLEX, 0) < 0) {
    cout << "Error opening ALSA sequencer." << endl;
    exit(1);
  }
  snd_seq_set_client_name(seq_handle, "EdisonSynth");
  if ((portid = snd_seq_create_simple_port(seq_handle, "EdisonSynth",
            SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
            SND_SEQ_PORT_TYPE_APPLICATION)) < 0) {
    fprintf(stderr, "Error creating sequencer port.\n");
    exit(1);
  }
  return(seq_handle);
}

int midi_action(snd_seq_t *seq_handle) {

  snd_seq_event_t *ev;
  int noteval;
  do {
    snd_seq_event_input(seq_handle, &ev);
    switch (ev->type) {
      case SND_SEQ_EVENT_CONTROLLER:
    	  //cout << "received controller: " << ev->data.control.param << " with value: " << ev->data.control.value << endl;
        if(ev->data.control.param == 3)
        {
        	 for(int h=0;h<N_VOICES;h++)
			  {
        		 voices_for_midi[h]->o1_symm_basis=ev->data.control.value/127.0;
        		 voices_for_midi[h]->o2_symm_basis=ev->data.control.value/127.0;
			  }
        }
        if(ev->data.control.param==9)
        {
       	 for(int h=0;h<N_VOICES;h++)
			  {
       		 voices_for_midi[h]->filter_cutoff_base=ev->data.control.value*0.8982-48;
			  }
        }
        if(ev->data.control.param==14)
        {
       	 for(int h=0;h<N_VOICES;h++)
			  {
       		 voices_for_midi[h]->filter_reso_base=ev->data.control.value/127.0;
			  }
        }
        if(ev->data.control.param==16)
        {
       	 for(int h=0;h<N_VOICES;h++)
			  {
       		 voices_for_midi[h]->env_vol->setAttack(ev->data.control.value*15.74); // zero to two seconds
			  }
        }
        if(ev->data.control.param==17)
        {
       	 for(int h=0;h<N_VOICES;h++)
			  {
       		 voices_for_midi[h]->env_vol->setDecay(ev->data.control.value*15.74);// zero to two seconds
			  }
        }
        if(ev->data.control.param==18)
        {
       	 for(int h=0;h<N_VOICES;h++)
			  {
       		 voices_for_midi[h]->env_vol->setSustain(ev->data.control.value/127.0);
			  }
        }
        if(ev->data.control.param==19)
        {
       	 for(int h=0;h<N_VOICES;h++)
			  {
       		 voices_for_midi[h]->env_vol->setRelease(ev->data.control.value*15.74);// zero to two seconds
			  }
        }
        if(ev->data.control.param==1)
        {
       	 for(int h=0;h<N_VOICES;h++)
	     {
       		 voices_for_midi[h]->envd_to_filter_cutoff=ev->data.control.value*0.3;
		 }
        }
        if(ev->data.control.param==20)
        {
       	 for(int h=0;h<N_VOICES;h++)
			  {
       		 voices_for_midi[h]->env_div->setAttack(ev->data.control.value*15.74); // zero to two seconds
			  }
        }
        if(ev->data.control.param==21)
        {
       	 for(int h=0;h<N_VOICES;h++)
			  {
       		 voices_for_midi[h]->env_div->setDecay(ev->data.control.value*15.74);// zero to two seconds
			  }
        }
        if(ev->data.control.param==22)
        {
       	 for(int h=0;h<N_VOICES;h++)
			  {
       		 voices_for_midi[h]->env_div->setSustain(ev->data.control.value/127.0);
			  }
        }
        if(ev->data.control.param==23)
        {
       	 for(int h=0;h<N_VOICES;h++)
			  {
       		 voices_for_midi[h]->env_div->setRelease(ev->data.control.value*15.74);// zero to two seconds
			  }
        }
    	  break;
      case SND_SEQ_EVENT_PITCHBEND:
    	  //cout << "received pitch bend " << endl;
    	  for(int h=0;h<N_VOICES;h++)
    	  {
    		  voices_for_midi[h]->set_pitchbend_value(ev->data.control.value);
    	  }
        break;
      case SND_SEQ_EVENT_NOTEON:
    	  noteval=ev->data.note.note;
    	  int idx_free_voice;
    	  idx_free_voice=-1;
    	  cout << "received note on: " << noteval << endl;
    	  for(int h=0;h<N_VOICES;h++)
    	  {
    		  if(!voices_for_midi[h]->is_voice_on())
    		  {
    			  idx_free_voice=h;
    			  break;
    		  }
    	  }
    	  if(idx_free_voice>-1)
    	  {
    		  voices_for_midi[idx_free_voice]->set_note(noteval);
    		  voices_for_midi[idx_free_voice]->set_on_off(1);
    	  }
        break;
      case SND_SEQ_EVENT_NOTEOFF:
    	  noteval=ev->data.note.note;
    	  int idx_switch_off;
    	  idx_switch_off=-1;
    	  int switchoff_array[N_VOICES];
    	  int switchoff_cntr;
    	  switchoff_cntr=0;
    	  //cout << "received note off: " << noteval << endl;
    	  for(int h=0;h<N_VOICES;h++)
    	  {
    		  if(voices_for_midi[h]->is_voice_on())
    		  {
    			  if(voices_for_midi[h]->get_note()==noteval) // voice is taken
    			  {
    				  idx_switch_off=h;
    				  switchoff_array[switchoff_cntr++]=h;
    			  }
    		  }
    	  }
    	  if(switchoff_cntr>0)
    	  {
    		  for(int q=0;q<switchoff_cntr;q++)
    		  {
    			  voices_for_midi[switchoff_array[q]]->set_on_off(0);
    		  }
    	  }
        break;
    }
    snd_seq_free_event(ev);
  } while (snd_seq_event_input_pending(seq_handle, 0) > 0);
  return(0);
}

void* midi_thread_worker(void* args) {


  int npfd;
  struct pollfd *pfd;
  cout << "midi thread started " << endl;
  npfd = snd_seq_poll_descriptors_count(seq_handle, POLLIN);
  pfd = (struct pollfd *)alloca(npfd * sizeof(struct pollfd));
  snd_seq_poll_descriptors(seq_handle, pfd, npfd, POLLIN);
  cout << "polling " << endl;
  while (1) {
    if (poll(pfd, npfd, -1) > 0) {
      midi_action(seq_handle);
    }
  }
  return NULL;
}

snd_seq_t * init_midi_controller(Voice** vocs_addr,char** midi_cfg)
	{
	int err;
	cout << "initializing midi" << endl;
	voices_for_midi=vocs_addr;
	open_seq(midi_cfg[1]);
	//err=pthread_create(&midi_controller_thread,NULL,&midi_thread_worker,NULL);
	//if(err!=0)
	//{
	//	cout << "midi controller init error: thread creation not possible" << endl;
	//}


	/* connect keyboard to port just created*/
    snd_seq_addr_t sender, dest;
    snd_seq_port_subscribe_t *subs;
    sender.client =  (unsigned char)atoi(midi_cfg[2]);
    sender.port = 0;
    dest.client = (unsigned char)atoi(midi_cfg[3]);
    dest.port = 0;
    snd_seq_port_subscribe_alloca(&subs);
    snd_seq_port_subscribe_set_sender(subs, &sender);
    snd_seq_port_subscribe_set_dest(subs, &dest);
    snd_seq_port_subscribe_set_queue(subs, 1);
    snd_seq_port_subscribe_set_time_update(subs, 1);
    snd_seq_port_subscribe_set_time_real(subs, 1);
    snd_seq_subscribe_port(seq_handle, subs);

	return seq_handle;

}
