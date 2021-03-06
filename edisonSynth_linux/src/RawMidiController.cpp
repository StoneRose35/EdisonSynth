/*
 * midi_controller_raw.c
 *
 *  Created on: Feb 11, 2016
 *      Author: philipp
 */

#include "RawMidiController.h"

#include <pthread.h>
#include <iostream>
#include <alsa/asoundlib.h>
#include "constants.h"
#include "Voice.h"
#include "EdisonSynthesizer.h"





using namespace std;

void RawMidiController::init(char ** config,Voice ** vocs_addr,char * flag_running)
{
	int status;
	*flag_running=1;
	should_be_running=flag_running;
	   if ((status = snd_rawmidi_open(&midiin, NULL, config[1], SND_RAWMIDI_SYNC)) < 0) {
	      cout << "Problem opening MIDI input: " << snd_strerror(status) << endl;
	   }
	voices_midiraw=vocs_addr;

	status=pthread_create(&midiraw_controller_thread,NULL,RawMidiController::static_thread_method,this);
	if(status ==-1)
	{
		cout << "Error creating midi thread" << endl;
	}

}

void* RawMidiController::static_thread_method(void* args)
{
	static_cast<RawMidiController*>(args)->midiinfunction();
	return NULL;
}

void RawMidiController::midiinfunction() {
   // this is the parameter passed via last argument of pthread_create():
   char buffer[1];
   int status;
   int noteval;
   unsigned char byte1;
   unsigned char byte2;
   unsigned char current_val;
   char processed;
   while (*should_be_running > 0) {
      if (midiin == NULL) {
         break;
      }
      if ((status = snd_rawmidi_read(midiin, buffer, 1)) < 0) {
         cout << "Problem reading MIDI input: " << snd_strerror(status) << endl;
      }
      processed=0;
      current_val=(unsigned char)buffer[0];
      if ((current_val&0xF0) == 0x80) {  // note off
    	  last_cmd=NOTEOFF;
    	  byte1=0x80;
    	  byte2=0x80;
    	  processed=1;
      }
      else if ((current_val&0xF0) == 0x90) // note on
      {
    	  last_cmd=NOTEON;
    	  byte1=0x80;
    	  byte2=0x80;
    	  processed=1;
      }
      else if ((current_val&0xF0) == 0xB0) // control change
      {
    	  last_cmd=MIDICONTROLLER;
    	  byte1=0x80;
    	  byte2=0x80;
    	  processed=1;
      }
      else if ((current_val&0xF0) == 0xE0) // pitch bend
      {
    	  last_cmd=PITCHBEND;
    	  byte1=0x80;
    	  byte2=0x80;
    	  processed=1;
      }
      else if((current_val&0x80)>0) // other command
      {
    	  last_cmd=CMD_UNKNOWN;
    	  byte1=0x80;
    	  byte2=0x80;
    	  processed=1;
      }
      else if ((current_val&0x80)==0x00 && byte1==0x80)
      {
    	  byte1 = (unsigned char)buffer[0];
    	  processed=1;
      }
      else if ((current_val&0x80)==0x00 && (byte1&0x80)==0x00 && byte2 ==0x80 && processed==0) // second data byte received, time for action!
      {
    	  byte2 = (unsigned char)buffer[0];
    	  if(last_cmd==NOTEON && byte2>0)
    	  {
    		  noteval = byte1;
    		  int idx_free_voice;
			  idx_free_voice=-1;
			  //cout << "received note on: " << noteval << endl;
			  for(int h=0;h<N_VOICES;h++)
			  {
				  if(!voices_midiraw[h]->is_voice_on())
				  {
					  idx_free_voice=h;
					  break;
				  }
			  }
			  if(idx_free_voice>-1)
			  {
				  voices_midiraw[idx_free_voice]->set_note(noteval);
				  voices_midiraw[idx_free_voice]->set_on_off(1);
			  }
    	  }
    	  else if(last_cmd==NOTEOFF || (last_cmd==NOTEON && byte2==0))
    	  {
			  int switchoff_array[N_VOICES];
			  int switchoff_cntr;
			  switchoff_cntr=0;
    		  noteval = byte1;
    		  //cout << "received note off: " << noteval << endl;
			  for(int h=0;h<N_VOICES;h++)
			  {
				  if(voices_midiraw[h]->is_voice_on())
				  {
					  if(voices_midiraw[h]->get_note()==noteval) // voice is taken
					  {
						  switchoff_array[switchoff_cntr++]=h;
					  }
				  }
			  }
			  if(switchoff_cntr>0)
			  {
				  for(int q=0;q<switchoff_cntr;q++)
				  {
					 voices_midiraw[switchoff_array[q]]->set_on_off(0);
				  }
			  }
    	  }
    	  else if(last_cmd==MIDICONTROLLER)
    	  {

    		    if(byte1 == 3)
    		        {
    		        	 for(int h=0;h<N_VOICES;h++)
    					  {
    		        		 voices_midiraw[h]->o1_symm_basis=byte2/127.0;
    		        		 voices_midiraw[h]->o2_symm_basis=byte2/127.0;
    					  }
    		        }
    		        if(byte1==9)
    		        {
    		       	 for(int h=0;h<N_VOICES;h++)
    					  {
    		       		voices_midiraw[h]->filter_cutoff_base=byte2*0.8982-48;
    					  }
    		        }
    		        if(byte1==14)
    		        {
    		       	 for(int h=0;h<N_VOICES;h++)
    					  {
    		       		voices_midiraw[h]->filter_reso_base=byte2/127.0;
    					  }
    		        }
    		        if(byte1==16)
    		        {
    		       	 for(int h=0;h<N_VOICES;h++)
    					  {
    		       		voices_midiraw[h]->env_vol->setAttack(byte2*15.74); // zero to two seconds
    					  }
    		        }
    		        if(byte1==17)
    		        {
    		       	 for(int h=0;h<N_VOICES;h++)
    					  {
    		       		voices_midiraw[h]->env_vol->setDecay(byte2*15.74);// zero to two seconds
    					  }
    		        }
    		        if(byte1==18)
    		        {
    		       	 for(int h=0;h<N_VOICES;h++)
    					  {
    		       		voices_midiraw[h]->env_vol->setSustain(byte2/127.0);
    					  }
    		        }
    		        if(byte1==19)
    		        {
    		       	 for(int h=0;h<N_VOICES;h++)
    					  {
    		       		voices_midiraw[h]->env_vol->setRelease(byte2*15.74);// zero to two seconds
    					  }
    		        }
    		        if(byte1==1)
    		        {
    		       	 for(int h=0;h<N_VOICES;h++)
    			     {
    		       		voices_midiraw[h]->envd_to_filter_cutoff=byte2*0.3;
    				 }
    		        }
    		        if(byte1==20)
    		        {
    		       	 for(int h=0;h<N_VOICES;h++)
    					  {
    		       		voices_midiraw[h]->env_div->setAttack(byte2*15.74); // zero to two seconds
    					  }
    		        }
    		        if(byte1==21)
    		        {
    		       	 for(int h=0;h<N_VOICES;h++)
    					  {
    		       		voices_midiraw[h]->env_div->setDecay(byte2*15.74);// zero to two seconds
    					  }
    		        }
    		        if(byte1==22)
    		        {
    		       	 for(int h=0;h<N_VOICES;h++)
    					  {
    		       		voices_midiraw[h]->env_div->setSustain(byte2/127.0);
    					  }
    		        }
    		        if(byte1==23)
    		        {
    		       	 for(int h=0;h<N_VOICES;h++)
    					  {
    		       		voices_midiraw[h]->env_div->setRelease(byte2*15.74);// zero to two seconds
    					  }
    		        }
    	  }
    	  else if(last_cmd==PITCHBEND)
    	  {
    		  unsigned short pb_val;
    		  pb_val=0;
    		  pb_val=(unsigned short)((byte2 << 7) | byte1);
        	  for(int h=0;h<N_VOICES;h++)
        	  {
        		  voices_midiraw[h]->set_pitchbend_value((int)pb_val);
        	  }
    	  }
    	  byte1=0x80;
    	  byte2=0x80;
      }


   }
   snd_rawmidi_close(midiin);
   midiin  = NULL;
}
