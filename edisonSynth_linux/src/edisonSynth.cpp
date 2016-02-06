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
#include <string.h>
#include <ctime>
#include "Oscillator.h"
#include "Voice.h"
#include "edisonSynth.h"
#include "midi_controller.h"

using namespace std;


short *sinewave;
snd_pcm_t *handle;
snd_pcm_hw_params_t *params;
snd_pcm_sw_params_t *sw_params;
Voice * vocs[N_VOICES];
short*** wt;
char ** config;
snd_seq_t * seq_handle1;

int test_cntr;


/*
 * reads the config file containing the alsa audio device name in the first line
 * and the alsa midi device in te second line
 * */
char ** read_config()
{
	char** result;

	result=new char*[4];

	ifstream cfg_stream;
	streampos fsize;
	string str_res;
	cfg_stream.open(CONFIG_FILE);

	getline(cfg_stream,str_res);
	while(str_res.substr(0,1)=="#")
	{
		getline(cfg_stream,str_res);
	}

	int sz = str_res.size();
	result[0]=new char[sz+1];
	str_res.copy(result[0],sz,0);
	result[0][sz]='\0';

	getline(cfg_stream,str_res);
	while(str_res.substr(0,1)=="#")
	{
		getline(cfg_stream,str_res);
	}

	sz = str_res.size();
	result[1]=new char[sz+1];
	str_res.copy(result[1],sz,0);
	result[1][sz]='\0';

	getline(cfg_stream,str_res);
	while(str_res.substr(0,1)=="#")
	{
		getline(cfg_stream,str_res);
	}

	sz = str_res.size();
	result[2]=new char[sz+1];
	str_res.copy(result[2],sz,0);
	result[2][sz]='\0';

	getline(cfg_stream,str_res);
	while(str_res.substr(0,1)=="#")
	{
		getline(cfg_stream,str_res);
	}

	sz = str_res.size();
	result[3]=new char[sz+1];
	str_res.copy(result[3],sz,0);
	result[3][sz]='\0';

	cfg_stream.close();
	return result;
}

/**
 * print information about the sound driver
 */
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


void init_voices()
{
	  cout << "reading wavetable" << endl;
	  int start=clock();
	  wt=read_wavetable();
	  int stop=clock();
	  cout << "done, took " << ((double)(stop-start)/CLOCKS_PER_SEC)*1000 << " ms " << endl;
	  cout << "initializing voices" << endl;
	  Voice * voc;
	  for(int h=0;h<N_VOICES;h++)
	  {
		voc=new Voice(wt);
		vocs[h]=voc;
	  }
	  cout << "done!" << endl;
}

/**
 * the callback function doing the sample calculation, basically mixes all active voices sound output together
 * the places the values as short integers into the write buffer the passes them to also, which in turn does the rest of the
 * magic...
 * */
int playback_callback(snd_pcm_t* handle,snd_pcm_sframes_t nframes)
{
	int rc;
	char *buffer;

	int size = nframes * 2 * N_CHANNELS;
	buffer = (char *) malloc(size);

	short sample_val=0;



	  	  for( int j=0;j<size;j+=4)
	  	  {
	  		sample_val=0;
	  		for (int h=0;h<N_VOICES;h++)
	  		{
	  			sample_val+=vocs[h]->get_nextval()/N_VOICES;
	  		}

	  		//sample_val=32767*sin(230/(double)SAMPLING_RATE*2*M_PI*test_cntr++);
	    	for(int nc=0;nc<N_CHANNELS*2;nc+=2)
	    	{
	    		*(buffer + j + nc) = sample_val & 0xff;
	    		*(buffer + j + nc + 1) = (sample_val >> 8) & 0xff;
	    	}

	  	  }


	  	  rc = snd_pcm_writei(handle, buffer, nframes);
	return rc;
}

/**
 * initializes the alsa device and the synth classes
 * then starts the synth engine, playback is interrupt-driven with playback_callback being the sampling routine
 * modulators update is done in the loop within this function, using Voice->update
 *  * */
void start_audio(snd_pcm_t *handle,snd_pcm_hw_params_t *params,snd_pcm_sw_params_t *sw_params)
{
	  int rc;
	  unsigned int val=SAMPLING_RATE;
	  double delta_t;
	  double t_total=0.0;
	  int perfclock_start;
	  int perfclock_stop;
	  double cpu_percentage;

	  int nfds;
	  int seq_nfds;
	  pollfd* pfds;
	  int l1;

	  const char* snd_dev=config[0];
	  rc=snd_pcm_open(&handle,snd_dev,SND_PCM_STREAM_PLAYBACK,0);
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

	  rc = snd_pcm_hw_params_set_channels(handle, params, N_CHANNELS);
	  printIfError(rc);

	  int dir;

	  rc = snd_pcm_hw_params_set_rate_near(handle,
	  	                                   params, &val, &dir);
	  printIfError(rc);

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

	  	  if ((rc= snd_pcm_prepare (handle)) < 0)
	  	  {
	  	      cout << "Init: cannot prepare audio interface for use (" << snd_strerror (rc) << ")" << endl;
	  	  }


	  	  //prepare poll descriptors
	  	   seq_nfds = snd_seq_poll_descriptors_count(seq_handle1, POLLIN);
	  	   nfds = snd_pcm_poll_descriptors_count (handle);
	  	   pfds = (struct pollfd *)alloca(sizeof(struct pollfd) * (seq_nfds + nfds));
	  	   snd_seq_poll_descriptors(seq_handle1, pfds, seq_nfds, POLLIN);
	  	   snd_pcm_poll_descriptors (handle, pfds+seq_nfds, nfds);

	  	  cout << "Synth Engine running!" << endl;
	  	  while(1)
	  	  {




	  		if (poll (pfds, seq_nfds + nfds, 1000) > 0) {
				for (l1 = 0; l1 < seq_nfds; l1++) {
				   if (pfds[l1].revents > 0) midi_action(seq_handle1);
				}
				for (l1 = seq_nfds; l1 < seq_nfds + nfds; l1++) {
					if (pfds[l1].revents > 0) {
						if (playback_callback(handle,FRAMES_BUFFER) < FRAMES_BUFFER) {
							fprintf (stderr, "buffer underrun, try increasing the buffer size !\n");
							snd_pcm_prepare(handle);
						}
					}
				}

				delta_t=(double)FRAMES_BUFFER / (double)SAMPLING_RATE;
				t_total+=delta_t;

				for(int z=0;z<N_VOICES;z++)
				{
					vocs[z]->update(delta_t);
				}

			}

			//perfclock_stop=clock();
			//cpu_percentage = (perfclock_stop-perfclock_start)/CLOCKS_PER_SEC*SAMPLING_RATE/FRAMES_BUFFER*100;
			//cout << "CPU load: " << cpu_percentage << "%" << endl;
	  	  }



}




/**
 * computes the frequency given the note number, note number 0 has 440Hz,
 * note range goes from -48 up to 40
 */
double getFrequency(double notenumber)
{
	return 440.0*pow(TWO_TWROOT,notenumber);
}

#ifndef TESTING
int main() {

	test_cntr=0;
	// read configuration file
	config=read_config();

	//initialize the voices
	init_voices();

	// initialize thread handling midi
	seq_handle1 = init_midi_controller(vocs,config);

	// THIS STARTS THE SOUND!!
	start_audio(handle,params,sw_params);

	return 0;
}
#endif
