/*
 * dump_code.cpp
 *
 *  Created on: Dec 5, 2015
 *      Author: philipp
 */

#include "constants.h"
#include "edisonSynth.h"
#include <iostream>
#include <fstream>
#include <alsa/asoundlib.h>
#include <math.h>

using namespace std;

unsigned short ** wavetables;

unsigned short interpolate_wavetable(int note,double pos)
{
	int basepos;
	unsigned short val;
	basepos=(int)floor(pos);
	if(basepos >= SAMPLE_SIZE)
	{
		cout << "position is not within sample" << endl;
	}
	else if (basepos == SAMPLE_SIZE-1)
	{
		val= wavetables[note][basepos] + (pos - floor(pos))*(wavetables[note][0]-wavetables[note][basepos]);
	}
	else
	{
		val= wavetables[note][basepos] + (pos - floor(pos))*(wavetables[note][basepos + 1]-wavetables[note][basepos]);
		//cout << " left value is: " <<  wavetables[note][basepos] << " right value is: " << wavetables[note][basepos + 1] << " interpolation factor: " << (pos - floor(pos)) << " interpolated value: " << val << endl;
	}
	return val;
}

// lookuptabelle für gesampelte bandlimitierte Signale, das ging nicht so guet
void generate_sound(snd_pcm_hw_params_t *params,snd_pcm_t *handle)
{
	int rc;
	snd_pcm_uframes_t frames;

    char *buffer;
    int size = frames * 4; /* 2 bytes/sample, 2 channels */
      buffer = (char *) malloc(size);
	int note =22;
		      double f=223; //getFrequency(note-48);
		      double d_phase = (double)SAMPLE_SIZE*f/(double)SAMPLING_RATE;
		      cout << " frequency is: " << f << " delta phase is: " << d_phase << endl;
		      double phase_d=0;
		      unsigned short sample_val;
		      for(int w=0;w<15000;w++)
		      {
		    	  for (int j=0;j<size;j++)
		    	  {
		    	  // filling buffer from wavetable
		    		  phase_d+=d_phase;
		    		  while(phase_d >=SAMPLE_SIZE)
		    		  {
		    			  phase_d -= (double)SAMPLE_SIZE;
		    		  }
		    		  sample_val=interpolate_wavetable(note,phase_d);
		    		  // des gleiche links und rechts, darum zwoa mol befuit
		    		  *(buffer + j + 1) = sample_val & 0xff;
		    		  *(buffer + j + 0) = (sample_val >> 8) & 0xff;
		    		  *(buffer + j + 3) = sample_val & 0xff;
		    		  *(buffer + j + 2) = (sample_val >> 8) & 0xff;
		    	  }

		    	  rc = snd_pcm_writei(handle, buffer, frames);
		    	  if(rc==-EPIPE)
		    	  {
		    		  cout << "der Puffer ist leergelaufen" << endl;
		    	  }
		    	  else if (rc < 0)
		    	  {
		    		  cout << " error from writei " << snd_strerror(rc) << endl;
		    	  }
		    	  else if(rc !=(int)frames)
		    	  {
		    		  cout << "not enough samples written" << endl;
		    	  }

		      }
		      snd_pcm_drain(handle);
		      snd_pcm_close(handle);
		      free(buffer);
}


void sawtooth_wavetable_reader()
{
	ifstream wt_in;
	wt_in.open("saw.tab",ios::binary|ios::in);
	unsigned short bfrval;
	wavetables=new unsigned short*[N_NOTES];
	for (int k=0;k<N_NOTES;k++)
	{
		cout << " reading note: " << k << endl;
		wavetables[k]=new unsigned short[SAMPLE_SIZE];
		for(int l=0;l<SAMPLE_SIZE;l++)
		{
			wt_in.read(reinterpret_cast<char*>(&bfrval),sizeof(unsigned short));
			*(wavetables[k]+l)=bfrval;
		}
	}
	wt_in.close();
}
