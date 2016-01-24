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

Voice*** voices_for_midi;
snd_seq_t *seq_handle;

snd_seq_t *open_seq(char* midi_dev ) {

  int portid;

  if (snd_seq_open(&seq_handle, midi_dev, SND_SEQ_OPEN_DUPLEX, 0) < 0) {
    cout << "Error opening ALSA sequencer." << endl;
    exit(1);
  }
  snd_seq_set_client_name(seq_handle, "ALSA Sequencer Demo");
  if ((portid = snd_seq_create_simple_port(seq_handle, "ALSA Sequencer Demo",
            SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
            SND_SEQ_PORT_TYPE_APPLICATION)) < 0) {
    fprintf(stderr, "Error creating sequencer port.\n");
    exit(1);
  }
  return(seq_handle);
}

void midi_action(snd_seq_t *seq_handle) {

  snd_seq_event_t *ev;
  int noteval;
  do {
    snd_seq_event_input(seq_handle, &ev);
    switch (ev->type) {
      case SND_SEQ_EVENT_CONTROLLER:
        break;
      case SND_SEQ_EVENT_PITCHBEND:
    	  for(int h=0;h<N_VOICES;h++)
    	  {
    		  (*voices_for_midi)[h]->set_pitchbend_value(ev->data.control.value);
    	  }
        break;
      case SND_SEQ_EVENT_NOTEON:
    	  noteval=ev->data.note.note;
    	  int idx_free_voice;
    	  idx_free_voice=-1;
    	  for(int h=0;h<N_VOICES;h++)
    	  {
    		  if((*voices_for_midi)[h]->is_voice_on())
    		  {
    			  if((*voices_for_midi)[h]->get_note()==noteval) // voice is taken
    			  {
    				  idx_free_voice=-1;
    				  break;
    			  }
    		  }
    		  else
    		  {
    			  idx_free_voice=h;
    		  }
    	  }
    	  if(idx_free_voice>-1)
    	  {
    		  (*voices_for_midi)[idx_free_voice]->set_note(noteval);
    		  (*voices_for_midi)[idx_free_voice]->set_on_off(1);
    	  }
        break;
      case SND_SEQ_EVENT_NOTEOFF:
    	  noteval=ev->data.note.note;
    	  int idx_switch_off;
    	  idx_switch_off=-1;
    	  for(int h=0;h<N_VOICES;h++)
    	  {
    		  if((*voices_for_midi)[h]->is_voice_on())
    		  {
    			  if((*voices_for_midi)[h]->get_note()==noteval) // voice is taken
    			  {
    				  idx_switch_off=h;
    				  break;
    			  }
    		  }
    	  }
    	  if(idx_switch_off>-1)
    	  {
    		  (*voices_for_midi)[idx_switch_off]->set_on_off(0);
    	  }
        break;
    }
    snd_seq_free_event(ev);
  } while (snd_seq_event_input_pending(seq_handle, 0) > 0);
}

void* midi_thread_worker(void* args) {


  int npfd;
  struct pollfd *pfd;
  npfd = snd_seq_poll_descriptors_count(seq_handle, POLLIN);
  pfd = (struct pollfd *)alloca(npfd * sizeof(struct pollfd));
  snd_seq_poll_descriptors(seq_handle, pfd, npfd, POLLIN);
  while (1) {
    if (poll(pfd, npfd, 100000) > 0) {
      midi_action(seq_handle);
    }
  }
  return NULL;
}

void init_midi_controller(Voice*** vocs_addr,char* midi_dev)
{
	pthread_t midi_controller_thread;
	int err;
	voices_for_midi=vocs_addr;
	open_seq(midi_dev);
	err=pthread_create(&midi_controller_thread,NULL,&midi_thread_worker,NULL);
	if(err!=0)
	{
		cout << "midi controller init error: thread creation not possible" << endl;
	}


}
