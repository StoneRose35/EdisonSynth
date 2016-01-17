#include "constants.h"
#include <iostream>
#include <math.h>
#include <fstream>
#include <alsa/asoundlib.h>
using namespace std;


/**
 * the generated wavetable is three dimensional,
 * the first dimension is note number with 0 being note 0 (27.5Hz) and 88 being note 88 (4434.92 Hz)
 * the second dimension is symmetry with linear interpolation between 0.0 and 1.0 over 256 values
 * the third dimension is time, each waveform contains 1024 samples.
 * */


short*** read_wavetable()
{
	short*** res2;
	ifstream wt_in;
	wt_in.open("wavess.tab",ios::binary|ios::in);
	// initialize in wavetable
	res2=new short**[88];
	for(int k=0;k<88;k++)
	{
		res2[k]=new short*[256];
		for(int l=0;l<256;l++)
		{
			res2[k][l]=new short[2048];
		}
	}
	short bfrval;
	// read in wavetable
	for(int a=0;a<88;a++)
	{
		for(int b=0;b<256;b++)
		{
			for(int c=0;c<2048;c++)
			{
				wt_in.read(reinterpret_cast<char*>(&bfrval),sizeof(short));
				res2[a][b][c]=bfrval;
			}
		}
	}
	wt_in.close();
	return res2;
}
