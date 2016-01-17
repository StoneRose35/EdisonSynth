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
	wavetable.open(WAVETABLE_FILENAME,ios::binary|ios::out);
	textout.open("onewave.txt");

	short*** res;

	Oscillator* o=new Oscillator();


	// initialize the wavetable
	res=new short**[88];
	for(int k=0;k<88;k++)
	{
		res[k]=new short*[256];
		for(int l=0;l<256;l++)
		{
			res[k][l]=new short[2048];
		}
	}
// fill it
	for(int a=0;a<88;a++)
	{
		for(int b=0;b<256;b++)
		{
			o->update((double)b/255,440.0*pow(TWO_TWROOT,a));
			o->recalc_coeffs();
			for(int c=0;c<2048;c++)
			{
				res[a][b][c]=static_cast<unsigned short>(o->compute_nextval());
				wavetable.write(reinterpret_cast<char*>( &res[a][b][c] ) ,sizeof(res[a][b][c]));
				if(a==0 && b==128)
				{
					textout << res[a][b][c] << endl;
				}
			}
		}
	}
	textout.close();
	wavetable.close();





}



short*** read_wavetable()
{
	short*** res2;
	ifstream wt_in;
	wt_in.open(WAVETABLE_FILENAME,ios::binary|ios::in);
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
