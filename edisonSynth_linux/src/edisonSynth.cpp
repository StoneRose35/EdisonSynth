//============================================================================
// Name        : edisonSynth.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include "constants.h"
#include "generators.h"
#include <iostream>
#include <math.h>
#include <fstream>
#include <limits>
#include <alsa/asoundlib.h>
#include "Oscillator.h"
#include "Voice.h"
using namespace std;



short *sinewave;
snd_pcm_t *handle;
snd_pcm_hw_params_t *params;
snd_pcm_sw_params_t *sw_params;
Voice* voc;





void sine_wavetable_reader()
{
	ifstream wt_in;
	wt_in.open("sine.tab",ios::binary|ios::in);
	short bfrval;
	sinewave=(short*)malloc(SINE_SAMPLES*sizeof(short));
	for(int z=0;z<SINE_SAMPLES;z++)
	{
		wt_in.read(reinterpret_cast<char*>(&bfrval),sizeof(short));
		*(sinewave + z)=bfrval;
	}
	wt_in.close();
}



void show_alsa_info()
{
	cout << "Alsa library version: " << SND_LIB_VERSION_STR << endl;

		cout << "PCM stream types: " << endl;
		 for (int val = 0; val <= SND_PCM_STREAM_LAST; val++)
		 {
			 cout << snd_pcm_stream_name((snd_pcm_stream_t)val) << endl;
		 }
		 cout << "PCM access types: " << endl;
		 for(int val=0; val < SND_PCM_ACCESS_LAST;val++)
		 {
			 cout << snd_pcm_access_name((snd_pcm_access_t)val) << endl;
		 }

		 cout << "PCM formats: " << endl;
		 for(int val=0;val < SND_PCM_FORMAT_LAST;val++)
		 {
			 if(snd_pcm_format_name((snd_pcm_format_t)val)!=NULL)
			 {
				 cout << snd_pcm_format_name((snd_pcm_format_t)val) << " (" <<snd_pcm_format_description((snd_pcm_format_t)val) << ")" << endl;
			 }
		 }

		 cout << "PCM subformats: " << endl;
		 for(int val=0; val < SND_PCM_SUBFORMAT_LAST;val++)
		 {
			 cout << snd_pcm_subformat_name((snd_pcm_subformat_t)val) << " (" << snd_pcm_subformat_description((snd_pcm_subformat_t)val) << ") " << endl;
		 }

		 cout << "PCM states: " << endl;
		 for(int val =0;val<SND_PCM_STATE_LAST;val++)
		 {
			 cout << "    " << snd_pcm_state_name((snd_pcm_state_t)val) << endl;
		 }

}

void printIfError(int rc)
{
	if(rc < 0)
  {
	  cout << "unable to open device " << snd_strerror(rc) << endl;
  }
}

int playback_callback(snd_pcm_t* handle,snd_pcm_sframes_t nframes)
{
	int rc;
	char *buffer;
	    int size = nframes * 4; /* 2 bytes/sample, 2 channels */
	      buffer = (char *) malloc(size);

	    short sample_val;


	  	  for( int j=0;j<size;j+=4)
	  	  {
	    	sample_val=voc->get_nextval();
			*(buffer + j + 0) = sample_val & 0xff;
			*(buffer + j + 1) = (sample_val >> 8) & 0xff;
			*(buffer + j + 2) = sample_val & 0xff;
			*(buffer + j + 3) = (sample_val >> 8) & 0xff;
	  	  }
	  	  //cout << " integer phase: " << intphase << ", table value: " << *(sinewave + intphase) << endl;
		  //cout << " s val: " << sample_val << endl;
	  	  //wave << sample_val << endl;
	  	  rc = snd_pcm_writei(handle, buffer, nframes);
		  if(rc==-EPIPE)
		  {
			  cout << "der Puffer ist leergelaufen" << endl;
		  }
		  else if (rc < 0)
		  {
			  cout << " error from writei " << snd_strerror(rc) << endl;
		  }
	return rc;
}

void init_alsa_device(snd_pcm_t *handle,snd_pcm_hw_params_t *params,snd_pcm_sw_params_t *sw_params)
{
	  int rc;
	  unsigned int val=SAMPLING_RATE;
	  snd_pcm_sframes_t frames_to_deliver;
	  double delta_t;
	  double t_total=0.0;
	  double t_total_old=0.0;
	  voc=new Voice();
	  rc=snd_pcm_open(&handle,"plughw:0,0",SND_PCM_STREAM_PLAYBACK,0); // plughw:0,0 would be the internal sound card
	  printIfError(rc);

	  snd_pcm_hw_params_alloca(&params);

	  rc = snd_pcm_hw_params_any(handle, params);
	  printIfError(rc);

	  rc = snd_pcm_hw_params_set_access(handle, params,
	                        SND_PCM_ACCESS_RW_INTERLEAVED);
	  printIfError(rc);

	  rc = snd_pcm_hw_params_set_format(handle, params,
	                                SND_PCM_FORMAT_S16_LE);
	  printIfError(rc);

	  rc = snd_pcm_hw_params_set_channels(handle, params, 2);
	  printIfError(rc);

	  int dir;

	  rc = snd_pcm_hw_params_set_rate_near(handle,
	  	                                   params, &val, &dir);
	  printIfError(rc);

	  //snd_pcm_uframes_t frames = FRAMES_BUFFER;
	  //snd_pcm_hw_params_set_period_size_near(handle,
	  //	                                params, &frames, &dir);
	  //printIfError(rc);

	rc = snd_pcm_hw_params(handle, params);
	if(rc < 0)
	{
		cout << "unable to write parameters to device " << endl;
	}

	//snd_pcm_hw_params_free(params);

	rc = snd_pcm_sw_params_malloc (&sw_params);
	printIfError(rc);

	rc = snd_pcm_sw_params_current(handle,sw_params);
	printIfError(rc);

	rc= snd_pcm_sw_params_set_avail_min(handle,sw_params,FRAMES_BUFFER);
	printIfError(rc);

	rc =snd_pcm_sw_params_set_start_threshold (handle, sw_params, 0U);
	printIfError(rc);

	rc = snd_pcm_sw_params (handle, sw_params);
	if(rc < 0)
	{
		cout << "cannot set software parameters" << endl;
	}
	  	    //unsigned long bufferSize;

	  	  //snd_pcm_hw_params_get_buffer_size( params, &bufferSize );

	  	  //cout << "Init: Buffer size = " << bufferSize << " frames." << endl;


	  	  if ((rc= snd_pcm_prepare (handle)) < 0)
	  	  {
	  	      cout << "Init: cannot prepare audio interface for use (" << snd_strerror (rc) << ")" << endl;
	  	  }

	  	  char note=14;
	  	  voc->set_note((int)note);
	  	  voc->set_on_off(0);
	  	  voc->o2->set_fcutoff(2600);
	  	  voc->o2->set_resonance(-1.98);
	  	  voc->o2->set_symm(0.01);
	  	  voc->env_vol->setAttack(420);
	  	  voc->env_vol->setDecay(1);
	  	  voc->env_vol->setSustain(1.0);
	  	  voc->env_vol->setRelease(420);
	  	  voc->env_div->setAttack(220);
	  	  voc->env_div->setDecay(45);
	  	  voc->env_div->setSustain(0.8);
	  	  voc->env_div->setRelease(420);
	  	  voc->lfo1->set_frequency(1.23);
	  	  char note_toggle=0;
	  	  while(1)
	  	  {
	  		rc = snd_pcm_wait (handle, 1000);
	  		if(rc < 0)
	  		{
	  			cout << "poll failed: " << snd_strerror(rc) << endl;
	  			break; // kills the audio
	  		}

	  		if ((frames_to_deliver = snd_pcm_avail_update (handle)) < 0) {
				if (frames_to_deliver == -EPIPE) {
					cout << "an xrun occured\n" << endl;
					break;
				} else {
					cout << "unknown ALSA avail update return value \n" << frames_to_deliver << endl;
					break;
				}
			}

			frames_to_deliver = frames_to_deliver > FRAMES_BUFFER ? FRAMES_BUFFER : frames_to_deliver;

			/* deliver the data */

			if (playback_callback (handle,frames_to_deliver) != frames_to_deliver) {
					fprintf (stderr, "playback callback failed\n");
				break;
			}
			delta_t=(double)frames_to_deliver / (double)SAMPLING_RATE;
			//
			// compute new value of envelopes and LFO's

			// set random notes every second
			t_total+=delta_t;
			//cout << "t_total is: " << t_total << endl;
			/*if(t_total > 15.0 && t_total - t_total_old > 0.25)
			{
				//voc.set_note(rand()%30+4);
				voc->o1->set_symm(0.45*sin(t_total/3.0)+0.5);
				voc->o2->set_symm(0.45*cos(t_total/3.0)+0.5);
				t_total_old=t_total;
				cout << "should update note " << endl;
			}*/
			if(t_total-t_total_old > 0.67)
			{
				t_total_old=t_total;
				if(note_toggle==0)
				{
					note_toggle=1;
					note++;
					voc->set_note((int)note);
					cout << "switching on " << endl;
				}
				else
				{
					note_toggle=0;
					cout  << "switching off " << endl;
				}
				voc->set_on_off(note_toggle);
				//voc->o2->set_fcutoff(5000+sin(t_total/0.2)*4900);
			}

			//voc->o1->set_symm(0.45*sin(t_total/0.6)+0.5);
			//voc->o2->set_symm(0.45*cos(t_total/0.8)+0.5);
			voc->update(delta_t);
	  	  }

	  	  //generate_sound2(params,handle);


}

#ifndef TESTING
int main() {
	//ofstream wavetable;
	//wavetable.open("wave.txt",ios::out);
	sine_wavetable_reader();
	init_alsa_device(handle,params,sw_params);
	/*
	Oscillator o1;
	o1.set_f(getFrequency(22-48));
	o1.set_waveform(1);
	o1.recalc_coeffs(1.0);
	for(int k=0;k< 2048 ;k++)
	{
		wavetable << o1.get_nextval() << endl;
	}
	wavetable.close();
	*/
	return 0;
}
#endif
