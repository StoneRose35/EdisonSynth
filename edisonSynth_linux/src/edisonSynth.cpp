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
#include "Oscillator.h"
#include "Voice.h"

using namespace std;


short *sinewave;
snd_pcm_t *handle;
snd_pcm_hw_params_t *params;
snd_pcm_sw_params_t *sw_params;
Voice* voc;


struct sb_with_idx
{
	int index_start;
	int index_end;
	char* soundbuffer;
};

/**
 * NOT USED, used the read the sine table
 * */
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


/*
 * reads the config file containing the alsa device string as the only content
 * */
char * read_config()
{
	char* result;

	ifstream cfg_stream;
	streampos fsize;
	string str_res;
	cfg_stream.open(CONFIG_FILE);
	getline(cfg_stream,str_res);
	cfg_stream.close();
	int sz = str_res.size();
	result=new char[sz];
	str_res.copy(result,sz,0);
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


/**
 * the callback function doing the sample calculation, basically mixes all active voices sound output together
 * the places the values as short integers into the write buffer the passes them to also, which in turn does the rest of the
 * magic...
 * */
int playback_callback(snd_pcm_t* handle,snd_pcm_sframes_t nframes)
{
	int rc;
	char *buffer;

	struct sb_with_idx thread_data[2];
	void * threadstatus;
	int size = nframes * 2 * N_CHANNELS;
	buffer = (char *) malloc(size);

	short sample_val;



	  	  for( int j=0;j<size;j+=4)
	  	  {
	    	sample_val=voc->get_nextval();
	    	for(int nc=0;nc<N_CHANNELS*2;nc+=2)
	    	{
	    		*(buffer + j + nc) = sample_val & 0xff;
	    		*(buffer + j + nc + 1) = (sample_val >> 8) & 0xff;
	    	}

	  	  }


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

/**
 * initializes the alsa device and the synth classes
 * then starts the synth engine, playback is interrupt-driven with playback_callback being the sampling routine
 * modulators update is done in the loop within this function, using Voice->update
 *  * */
void start_audio(snd_pcm_t *handle,snd_pcm_hw_params_t *params,snd_pcm_sw_params_t *sw_params)
{
	  int rc;
	  short*** wavetable;
	  unsigned int val=SAMPLING_RATE;
	  snd_pcm_sframes_t frames_to_deliver;
	  double delta_t;
	  double t_total=0.0;
	  double t_total_old=0.0;

	  cout << "reading wavetable" << endl;
	  wavetable=read_wavetable();
	  cout << "done" << endl;
	  voc=new Voice(wavetable);
	  const char* snd_dev=read_config();
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

	  	  char note=14;
	  	  voc->set_note((int)note);
	  	  voc->set_on_off(0);
	  	  voc->o2->set_waveform(0);
	  	  voc->o2->set_symm(0.5);
	  	  voc->env_vol->setAttack(1);
	  	  voc->env_vol->setDecay(1);
	  	  voc->env_vol->setSustain(1.0);
	  	  voc->env_vol->setRelease(420);
	  	  voc->env_div->setAttack(1);
	  	  voc->env_div->setDecay(154);
	  	  voc->env_div->setSustain(0.0);
	  	  voc->env_div->setRelease(420);
	  	  voc->lfo1->set_frequency(2.78);
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
			//clock_t time=clock();
			delta_t=(double)frames_to_deliver / (double)SAMPLING_RATE;


			t_total+=delta_t;

			// toggle note every 0.7 secs, increasing the note by one semitone every 1.4s
			if(t_total-t_total_old > 0.7)
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
			//time =clock() - time;
			//cout << double(time)/CLOCKS_PER_SEC*1000 << " of a max of " << (double)FRAMES_BUFFER/(double)SAMPLING_RATE*1000.0 << " ms elapsed" << endl;
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
	/*
	ofstream wavetable;
	wavetable.open("wave.txt",ios::out);
	sine_wavetable_reader();
*/

	// THIS STARTS THE SOUND!!
	start_audio(handle,params,sw_params);

	/*
	voc = new Voice();
  char note=14;
  voc->set_note((int)note);
  voc->set_on_off(0);
  voc->set_osc1_level(0.8);
  voc->set_osc2_level(0.0);
  voc->o1->set_waveform(0);
  voc->o1->set_symm(0.45);
  voc->o1->set_fcutoff(850);
  voc->o1->set_resonance(-0.0);

  voc->o2->set_waveform(1);
  voc->o2->set_fcutoff(2600);
  voc->o2->set_resonance(-1.98);
  voc->o2->set_symm(0.01);
  voc->env_vol->setAttack(1);
  voc->env_vol->setDecay(1);
  voc->env_vol->setSustain(1.0);
  voc->env_vol->setRelease(420);
  voc->env_div->setAttack(220);
  voc->env_div->setDecay(45);
  voc->env_div->setSustain(0.8);
  voc->env_div->setRelease(420);
  voc->lfo1->set_frequency(1.23);
  voc->set_on_off(1);
  voc->update(5);
  voc->update(5);
*/



	/*Oscillator o1;
	o1.set_f(getFrequency(14-48));
	o1.set_waveform(1);
	o1.set_symm(0.01);
	o1.recalc_coeffs(256);
	o1.recalc_coeffs(256);
*/
/*
	for(int k=0;k< 2048 ;k++)
	{
		wavetable << voc->get_nextval() << endl;
	}
	wavetable.close();
*/
	return 0;
}
#endif
