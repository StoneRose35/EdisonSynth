#include "constants.h"
#include <iostream>
#include <math.h>
#include <fstream>
#include <alsa/asoundlib.h>
#include "Oscillator.h"
using namespace std;


/**
 * the generated wavetable is three dimensional,
 * the first dimension is note number with 0 being note 0 (27.5Hz) and 88 being note 88 (4434.92 Hz)
 * the second dimension is symmetry with linear interpolation between 0.0 and 1.0 over 256 values
 * the third dimension is time, each waveform contains 1024 samples.
 * */
void generate_wavetable()
{
	ofstream wavetable;
	ofstream textout;
	double current_sampleval;
	double waveform_max;
	wavetable.open(WAVETABLE_FILENAME,ios::binary|ios::out);
	double waveform_bfr[2048];
	short*** res;

	Oscillator* o=new Oscillator();


	// initialize the wavetable
	res=new short**[256];
	for(int k=0;k<256;k++)
	{
		res[k]=new short*[256];
		for(int l=0;l<256;l++)
		{
			res[k][l]=new short[2048];
		}
	}
// fill it
	for(int a=0;a<256;a++)
	{
		cout << "a: " << a << endl;
		for(int b=0;b<256;b++)
		{
			// call update twice to set frequency1
			o->update((double)b/255,20.0+20.0*a);
			o->update((double)b/255,20.0+20.0*a);
			o->recalc_coeffs();
			waveform_max=0;
			for(int u=0;u<2048;u++)
			{
				current_sampleval=o->compute_nextval();
				waveform_bfr[u]=current_sampleval;
				if(fabs(current_sampleval)>waveform_max)
				{
					waveform_max=fabs(current_sampleval);
				}
			}
			for(int c=0;c<2048;c++)
			{
				current_sampleval=waveform_bfr[c]/waveform_max;
				current_sampleval = current_sampleval*32767.0;
				res[a][b][c]=(short)(current_sampleval);
				wavetable.write(reinterpret_cast<char*>( &res[a][b][c] ) ,sizeof(res[a][b][c]));
			}
		}
	}

	wavetable.close();





}



short*** read_wavetable()
{
	short*** res2;
	ifstream wt_in;
	short * arraybfr=new short[2048];
	wt_in.open(WAVETABLE_FILENAME,ios::binary|ios::in);
	// initialize in wavetable
	res2=new short**[256];
	for(int k=0;k<256;k++)
	{
		res2[k]=new short*[256];
		/*for(int l=0;l<256;l++)
		{
			res2[k][l]=new short[2048];
		}*/
	}
	short bfrval;
	// read in wavetable

	for(int a=0;a<256;a++)
	{
		for(int b=0;b<256;b++)
		{
			arraybfr=new short[2048];
			wt_in.read(reinterpret_cast<char*>(arraybfr),2048*sizeof(short));
			res2[a][b]=arraybfr;
			/*if(a==48 && b==25)
			{
				for(int c=0;c<2048;c++)
				{
					cout << "example waveform: " << arraybfr[c] << endl;
				}
			}*/
		}
	}

	//wt_in.read(reinterpret_cast<char*>(res2[0]),256*256*2048*sizeof(short));
	//delete arraybfr;
	wt_in.close();
	return res2;
}
