//============================================================================
// Name        : edisonSynth.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include "EdisonSynthesizer.h"
#include "constants.h"
#include <time.h>
#include <iostream>
#include <fstream>
#include <math.h>

#include <limits>
#include <alsa/asoundlib.h>
#include <string.h>
#include <sched.h>

#include "SeqMidiController.h"
#include "WavetableAccessor.h"


using namespace std;


EdisonSynthesizer::EdisonSynthesizer(char * flag_addr)
{
	engine_running=flag_addr;
	buffer = new char[UPDATE_SIZE];
}
/*
 * reads the config file containing the alsa audio device name in the first line
 * and the alsa midi device in te second line
 * */
void EdisonSynthesizer::read_config()
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
	config=result;
}

/**
 * print information about the sound driver
 */
void EdisonSynthesizer::show_alsa_info()
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

void EdisonSynthesizer::printIfError(int rc)
{
	if(rc < 0)
  {
	  cout << "unable to open device " << snd_strerror(rc) << endl;
  }
}


void EdisonSynthesizer::init_voices()
{
	 WavetableAccessor* wta;
	  cout << "reading wavetable" << endl;
	  wta=new WavetableAccessor();
	  if(!wta->is_wavetable_available())
	  {
		  cout << "wavetable hasn't been found, generating it" << endl;
		  cout << "this takes a few minutes, so please be patient" << endl;
		  wta->generate_wavetable();
	  }
	  int start=clock();
	  wt = wta->read_wavetable();
	  int stop=clock();
	  cout << "done, took " << ((double)(stop-start)/1000000l)*1000 << " ms " << endl;
	  Voice * voc;
	  for(int h=0;h<N_VOICES;h++)
	  {
		voc=new Voice(wt);
		vocs[h]=voc;
	  }
	  delta_t=(double)FRAMES_BUFFER/(double)SAMPLING_RATE;
}

void EdisonSynthesizer::init_midi()
{
	//rmc=new RawMidiController();
	//rmc->init(config,vocs,engine_running);
	smc=new SeqMidiController();
	smc->init_midi_controller(vocs,config[1],config[2],config[3]);
}


void EdisonSynthesizer::init_i2c()
{
	i2ccontroller = new I2CController();
	i2ccontroller->init_i2c(vocs,engine_running);
}
/**
 * the callback function doing the sample calculation, basically mixes all active voices sound output together
 * the places the values as short integers into the write buffer the passes them to also, which in turn does the rest of the
 * magic...
 * */
int EdisonSynthesizer::playback_callback(snd_pcm_t* handle)
{
#ifndef TESTING
	int rc;
#endif

	short sample_val=0;



	  	  for( int j=0;j<UPDATE_SIZE;j+=4)
	  	  {
	  		sample_val=0;

	  		for (int h=0;h<N_VOICES;h++)
	  		{
	  			sample_val+=vocs[h]->get_nextval()/N_VOICES;
	  		}

	    	for(int nc=0;nc<N_CHANNELS*2;nc+=2)
	    	{
	    		*(buffer + j + nc) = sample_val & 0xff;
	    		*(buffer + j + nc + 1) = (sample_val >> 8) & 0xff;
	    	}

	  	  }
#ifndef TESTING
	rc = snd_pcm_writei(handle, buffer, FRAMES_BUFFER);
#endif
	for(int z=0;z<N_VOICES;z++)
	{
		vocs[z]->update(delta_t);
	}
#ifndef TESTING
	return rc;
#endif
#ifdef TESTING
	return 0;
#endif
}



/**
 * initializes the alsa device and the synth classes
 * then starts the synth engine, playback is interrupt-driven with playback_callback being the sampling routine
 * modulators update is done in the loop within this function, using Voice->update
 *  * */
void EdisonSynthesizer::start_audio(snd_pcm_t *handle,snd_pcm_hw_params_t *params)
{
	  int rc;
	  unsigned int val=SAMPLING_RATE;

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

	  unsigned long periodsize;
	  periodsize=FRAMES_BUFFER;
	  rc = snd_pcm_hw_params_set_buffer_size_near(handle, params, &periodsize);
	  printIfError(rc);

	  periodsize/=2;
      rc = snd_pcm_hw_params_set_period_size_near(handle, params, &periodsize, 0);
	  printIfError(rc);

	rc = snd_pcm_hw_params(handle, params);
	if(rc < 0)
	{
		cout << "unable to write parameters to device " << endl;
	}


	/* prepare the interface for use*/
    if ((rc= snd_pcm_prepare (handle)) < 0)
    {
    	cout << "Init: cannot prepare audio interface for use (" << snd_strerror (rc) << ")" << endl;
    }

    buffer = (char *) malloc(FRAMES_BUFFER*N_CHANNELS*2);

    *engine_running = 1;


	sched_param schparams;
	sched_getparam(0, &schparams);
	schparams.sched_priority = sched_get_priority_max(SCHED_FIFO);
	sched_setscheduler(0, SCHED_FIFO,&schparams);




    //prepare poll descriptors
	int seq_nfds;
	int nfds;
	pollfd* pfds;

	seq_nfds = snd_seq_poll_descriptors_count(smc->mseq, POLLIN);
	//seq_nfds = snd_rawmidi_poll_descriptors_count(rmc->midiin);
	nfds = snd_pcm_poll_descriptors_count (handle);
	pfds = (struct pollfd *)alloca(sizeof(struct pollfd) * (seq_nfds + nfds));
	snd_seq_poll_descriptors(smc->mseq, pfds, seq_nfds, POLLIN);
	//snd_rawmidi_poll_descriptors(rmc->midiin, pfds, seq_nfds);
	snd_pcm_poll_descriptors (handle, pfds+seq_nfds, nfds);

	cout << "Synth Engine running!" << endl;
	int l1;
	while(*engine_running==1)
	{

  		if (poll (pfds, seq_nfds + nfds, 1000) > 0) {
			for (l1 = 0; l1 < seq_nfds; l1++) {
			   if (pfds[l1].revents > 0) smc->midi_action();
			}
			for (l1 = seq_nfds; l1 < seq_nfds + nfds; l1++) {
				if (pfds[l1].revents > 0) {
					if (playback_callback(handle) < FRAMES_BUFFER) {
						fprintf (stderr, "buffer underrun, try increasing the buffer size !\n");
						snd_pcm_prepare(handle);
					}
				}
			}

		}

	 }

	  snd_pcm_close (handle);
	  free(buffer);

}

void EdisonSynthesizer::start_synth()
{
	start_audio(handle,params);
}



/**
 * computes the frequency given the note number, note number 0 has 440Hz,
 * note range goes from -48 up to 40
 */
double EdisonSynthesizer::getFrequency(double notenumber)
{
	return 440.0*pow(TWO_TWROOT,notenumber);
}




