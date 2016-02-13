#include "Voice.h"
#include "constants.h"
#include <ctime>
#include <cmath>
#include <iostream>
#include <fstream>

#include "EdisonSynthesizer.h"
#include "WavetableAccessor.h"

#define N_BUFFERLOADS 50000

using namespace std;

EdisonSynthesizer* es_test;
char* buffer_test;
struct sb_with_idx
{
	int index_start;
	int index_end;
	char* soundbuffer;
};


void performance_test()
{

		char is_running;
		is_running=1;
		es_test=new EdisonSynthesizer(&is_running);
		buffer_test = new char[FRAMES_BUFFER*4];

		char note=14;
		es_test->init_voices();
		es_test->vocs[0]->set_note(note);
		es_test->vocs[0]->set_on_off(1);

		es_test->vocs[0]->update(FRAMES_BUFFER/SAMPLING_RATE);

		double samplingtimes[N_BUFFERLOADS];
		int cnt1;
		cout << "Starting Performance Test, running " <<  N_BUFFERLOADS << " updates " << endl;
		for(cnt1=0;cnt1 < N_BUFFERLOADS;cnt1++)
		{
			int start;
			start=clock();
			es_test->playback_callback(NULL);
			int stop2=clock();
			samplingtimes[cnt1]=(stop2-start)/double(CLOCKS_PER_SEC)*1000.0;
		}
		cout << " Performance Test finished, computing statistics " << endl;
		double mean_sampling=0;
		double stddev_sampling=0;
		double stddev_update=0;
		for(int c3=0;c3 < N_BUFFERLOADS ; c3++)
		{
			mean_sampling+=samplingtimes[c3]/N_BUFFERLOADS;
		}
		for (int c4=0;c4<N_BUFFERLOADS;c4++)
		{
			stddev_sampling+=((samplingtimes[c4]-mean_sampling)*(samplingtimes[c4]-mean_sampling))/(N_BUFFERLOADS-1);
		}
		stddev_sampling=sqrt(stddev_sampling);
		stddev_update = sqrt(stddev_update);
		cout << "Results for sampling: Average value: " << mean_sampling << " ms, standard deviation: " << stddev_sampling << " ms" << endl;
		cout << "Results for sampling: Avg is: " << mean_sampling*(SAMPLING_RATE/1000)/FRAMES_BUFFER*100 << "% of time available " << endl;
}

#ifdef TESTING
int main()
{
/*
	for(double z=0;z<128;z++)
	{
		cout << "Frequency of: " << z << " is: " << EdisonSynthesizer::getFrequency(z) << endl;
	}*/
	performance_test();
}
#endif
