#include "Voice.h"
#include "constants.h"
#include <ctime>
#include <cmath>
#include <iostream>
#include <fstream>

#define N_BUFFERLOADS 2000

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
	wavetable.open("wavess.tab",ios::binary|ios::out);

	short*** res;
	short*** res2;
	Voice* o=new Voice();


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
		o->set_note(a);
		for(int b=0;b<256;b++)
		{
			o->o1->set_symm((double)b/255.0);
			o->update(1);
			for(int c=0;c<2048;c++)
			{
				res[a][b][c]=static_cast<unsigned short>(o->o1->get_nextval()*32767);
				wavetable.write(reinterpret_cast<char*>( &res[a][b][c] ) ,sizeof(res[a][b][c]));
			}
		}
	}
	wavetable.close();

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
				if(res2[a][b][c]!=res[a][b][c])
				{
					cout << "ERROR! Wavetable values are not equal" << endl;
				}
			}
		}
	}


}



#ifdef TESTING
int main()
{
	/*
	Voice* voc=new Voice();


	char note=14;
  voc->set_note((int)note);
  voc->set_on_off(1);
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

	//voc->update(FRAMES_BUFFER/SAMPLING_RATE);

	double samplingtimes[N_BUFFERLOADS];
	double updatetimes[N_BUFFERLOADS];
	int cnt1;
	cout << "Starting Performance Test, running " <<  N_BUFFERLOADS << " updates " << endl;
	for(cnt1=0;cnt1 < N_BUFFERLOADS;cnt1++)
	{
		int start=clock();
		voc->update(FRAMES_BUFFER/SAMPLING_RATE);
		int stop1=clock();
		updatetimes[cnt1]=(stop1-start)/double(CLOCKS_PER_SEC)*1000.0;
		for(int cnt2=0;cnt2<FRAMES_BUFFER;cnt2++)
		{
			voc->get_nextval();
		}
		int stop2=clock();
		samplingtimes[cnt1]=(stop2-stop1)/double(CLOCKS_PER_SEC)*1000.0;
	}
	cout << " Performance Test finished, computing statistics " << endl;
	double mean_sampling=0;
	double stddev_sampling=0;
	double mean_update=0;
	double stddev_update=0;
	for(int c3=0;c3 < N_BUFFERLOADS ; c3++)
	{
		mean_sampling+=samplingtimes[c3]/N_BUFFERLOADS;
		mean_update+=updatetimes[c3]/N_BUFFERLOADS;
	}
	for (int c4=0;c4<N_BUFFERLOADS;c4++)
	{
		stddev_sampling+=((samplingtimes[c4]-mean_sampling)*(samplingtimes[c4]-mean_sampling))/(N_BUFFERLOADS-1);
		stddev_update+=((updatetimes[c4]-mean_update)*(updatetimes[c4]-mean_update))/(N_BUFFERLOADS-1);
	}
	stddev_sampling=sqrt(stddev_sampling);
	stddev_update = sqrt(stddev_update);
	cout << "Results for sampling: Average value: " << mean_sampling << " ms, standard deviation: " << stddev_sampling << " ms" << endl;
	cout << "Results for sampling: Avg is: " << mean_sampling*(SAMPLING_RATE/1000)/FRAMES_BUFFER*100 << "% of time available " << endl;
	cout << "Results for update: Average value: " << mean_update << " ms, standard deviation: " << stddev_update << " ms" << endl;
	cout << "Results for update: Avg is: " << mean_update*(SAMPLING_RATE/1000)/FRAMES_BUFFER*100 << "% of time available " << endl;
	*/
	generate_wavetable();
}
#endif
