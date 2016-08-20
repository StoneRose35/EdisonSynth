/*
 * midi_controller.cpp
 *
 *  Created on: Jan 23, 2016
 *      Author: philipp
 */

#include <pthread.h>

#include <iostream>
#include <alsa/asoundlib.h>
#include "constants.h"
#include "SeqMidiController.h"
#include "Voice.h"

using namespace std;

Voice ** voices_for_midi;
pthread_t midi_controller_thread;

void SeqMidiController::open_seq(char* midi_dev ) {

  int portid;

  if (snd_seq_open(&mseq, midi_dev, SND_SEQ_OPEN_DUPLEX, 0) < 0) {
    cout << "Error opening ALSA sequencer." << endl;
    exit(1);
  }
  snd_seq_set_client_name(mseq, SEQUENCER_NAME);
  if ((portid = snd_seq_create_simple_port(mseq, SEQUENCER_NAME,
            SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
            SND_SEQ_PORT_TYPE_APPLICATION)) < 0) {
    fprintf(stderr, "Error creating sequencer port.\n");
    exit(1);
  }

}

int SeqMidiController::midi_action() {

  snd_seq_event_t *ev;
  int noteval;
  do {
    snd_seq_event_input(mseq, &ev);
    switch (ev->type) {
      case SND_SEQ_EVENT_CONTROLLER:
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
    	  //cout << "received note on: " << noteval << endl;
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
  } while (snd_seq_event_input_pending(mseq, 0) > 0);
  return(0);
}


void SeqMidiController::init_midi_controller(Voice** vocs_addr,char* midi_dev,char* clientName)
{
	snd_seq_addr sender_addr;
	snd_seq_addr receiver_addr;
	cout << "initializing midi" << endl;
	voices_for_midi=vocs_addr;
	open_seq(midi_dev);
	snd_seq_port_subscribe_malloc(&portsubs);
	sender_addr.client=searchMidiClient(clientName);
	sender_addr.port=0;
	receiver_addr.client=searchMidiClient(SEQUENCER_NAME);
	receiver_addr.port=0;
	snd_seq_port_subscribe_set_sender(portsubs,&sender_addr);
	snd_seq_port_subscribe_set_dest(portsubs,&receiver_addr);
	snd_seq_port_subscribe_set_queue(portsubs,1);
	snd_seq_port_subscribe_set_time_real(portsubs,1);
	snd_seq_port_subscribe_set_time_update(portsubs,1);
	snd_seq_subscribe_port(mseq,portsubs);
}

int SeqMidiController::searchMidiClient(const char* clientName)
{
	int client_detected;
	int client_id;
	std::string clientnamestring;
	snd_seq_client_info_t * info;
	snd_seq_client_info_malloc(&info);
	client_detected = snd_seq_query_next_client(mseq,info);
	while(client_detected > -1)
	{
		const char * clientname = snd_seq_client_info_get_name(info);
		clientnamestring=string(clientname);
		if (clientnamestring.find(clientName)==0)
		{
			client_id = snd_seq_client_info_get_client(info);
		}
		client_detected = snd_seq_query_next_client(mseq,info);
	}
	return client_id;
}

void SeqMidiController::reattach_midi_client(char * new_midi_dev)
{
	snd_seq_addr sender_addr;
	snd_seq_addr receiver_addr;
	if(portsubs!=NULL && mseq!=NULL)
	{
		snd_seq_unsubscribe_port(mseq,portsubs);
		snd_seq_port_subscribe_free(portsubs);
	}
	snd_seq_port_subscribe_malloc(&portsubs);
	sender_addr.client=searchMidiClient(new_midi_dev);
	sender_addr.port=0;
	receiver_addr.client=searchMidiClient(SEQUENCER_NAME);
	receiver_addr.port=0;
	snd_seq_port_subscribe_set_sender(portsubs,&sender_addr);
	snd_seq_port_subscribe_set_dest(portsubs,&receiver_addr);
	snd_seq_port_subscribe_set_queue(portsubs,1);
	snd_seq_port_subscribe_set_time_real(portsubs,1);
	snd_seq_port_subscribe_set_time_update(portsubs,1);
	snd_seq_subscribe_port(mseq,portsubs);
}

int SeqMidiController::getAllMidiClients(char *** clientarray)
{
	int client_detected;
	int client_cntr;
	int u;
	std::string clientnamestring;
	snd_seq_client_info_t * info;
	snd_seq_client_info_malloc(&info);
	client_detected = snd_seq_query_next_client(mseq,info);
	client_cntr=0;
	while(client_detected > -1)
	{
		const char * clientname = snd_seq_client_info_get_name(info);
		if(strcmp(clientname,SEQUENCER_NAME)!=0)
		{
			u=0;
			while(clientname[u]!=0)
			{
				(*clientarray)[client_cntr][u]=clientname[u];
				u++;
			}
			client_cntr++;
		}
		client_detected = snd_seq_query_next_client(mseq,info);
	}
	snd_seq_client_info_free(info);
	return client_cntr;
}


