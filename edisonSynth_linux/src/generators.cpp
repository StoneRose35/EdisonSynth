#include "constants.h"
#include <iostream>
#include <math.h>
#include <fstream>
#include <alsa/asoundlib.h>
using namespace std;


void sawtooth_wavetable_generator()
{
	double frequency;
	double * waveform;
	int harm_cnt;
	ofstream wavetable;
	double maximum;
	double minimum;
	unsigned short sample;
	wavetable.open("saw.tab",ios::binary|ios::out);
	for (int n=-48;n<40;n++)
	{
		frequency = 440.0*pow(pow(2.0,1.0/12.0),(double)n);
		cout << "processing note: " << n << " having f: " << frequency << endl;
		waveform = new double[SAMPLE_SIZE];
		maximum=0.0;
		minimum=0.0;
		for(int k=0;k<SAMPLE_SIZE;k++)
		{
			waveform[k]=0;
			harm_cnt=0;
			while(frequency*(harm_cnt+1) < F_LIMIT)
			{
				waveform[k]+=1.0/(double)(harm_cnt+1)*sin((double)k*(double)(harm_cnt+1)/(double)SAMPLE_SIZE*2.0*M_PI);
				harm_cnt++;
			}
			if(waveform[k]>maximum)
			{
				maximum = waveform[k];
			}
			if(waveform[k]<minimum)
			{
				minimum = waveform[k];
			}
		}
		for(int u=0;u<SAMPLE_SIZE;u++)
		{
			waveform[u]=((waveform[u]-minimum)/(maximum - minimum))*65536.0;
			sample = static_cast<unsigned short>(waveform[u]);
			wavetable.write(reinterpret_cast<char*>( &sample ) ,sizeof(sample));
		}

	}
	wavetable.close();
}

void sine_wavetable_generator()
{
	ofstream wavetable;
	wavetable.open("sine.tab",ios::binary|ios::out);
	short sample;
	double minimum=0;
	double maximum=0;
	double *waveform;

	waveform=(double*)malloc(SINE_SAMPLES*sizeof(double));
	for (int z=0;z<SINE_SAMPLES; z++)
	{
		*(waveform + z)=sin(2.0*M_PI/(double)SINE_SAMPLES*(double)z);
		if(*(waveform + z) < minimum)
		{
			minimum=*(waveform + z);
		}

		if(*(waveform + z) > maximum)
		{
			maximum=*(waveform + z);
		}
	}

	for(int u=0;u<SINE_SAMPLES;u++)
	{
		sample = static_cast<short>(waveform[u]*32767.0);
		wavetable.write(reinterpret_cast<char*>( &sample ) ,sizeof(sample));
	}
	free(waveform);
	wavetable.close();
}
