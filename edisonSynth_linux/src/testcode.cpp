#include "Voice.h"
#include "constants.h"
#include <ctime>
#include <cmath>
#include <iostream>
#include <fstream>
#include "generators.h"

#define N_BUFFERLOADS 50000

using namespace std;

Voice* voc_test;
char* buffer_test;
struct sb_with_idx
{
	int index_start;
	int index_end;
	char* soundbuffer;
};


/**
 * the callback function doing the sample calculation, basically mixes all active voices sound output together
 * the places the values as short integers into the write buffer the passes them to also, which in turn does the rest of the
 * magic...
 * */
int playback_callback()
{



	int size = FRAMES_BUFFER * 2 * N_CHANNELS;


	// THREADED

		short sample_val;
		for( int j=0;j<size;j+=4)
	  {
		sample_val=voc_test->get_nextval();

		for(int nc=0;nc<4;nc+=2)
		{
			*(buffer_test + j + nc) = sample_val & 0xff;
			*(buffer_test + j + nc + 1) = (sample_val >> 8) & 0xff;
		}

	  }

	return 0;
}

void performance_test()
{
	//cout << "generating wavetable" << endl;
		//generate_wavetable();
		cout << "reading wavetable" << endl;
		short*** wt;
		wt=read_wavetable();

		voc_test=new Voice(wt);
		buffer_test = new char[FRAMES_BUFFER*4];

		char note=14;
		voc_test->set_note((int)note);
		voc_test->set_on_off(1);
		voc_test->o2->set_symm(0.01);
		voc_test->env_vol->setAttack(420);
		voc_test->env_vol->setDecay(1);
		voc_test->env_vol->setSustain(1.0);
		voc_test->env_vol->setRelease(420);
		voc_test->env_div->setAttack(220);
		voc_test->env_div->setDecay(45);
		voc_test->env_div->setSustain(0.8);
		voc_test->env_div->setRelease(420);
		voc_test->lfo1->set_frequency(1.23);
		voc_test->filter->set_fcutoff(270.0);
		voc_test->filter->set_res(0.67);

		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
		//voc->update(FRAMES_BUFFER/SAMPLING_RATE);

		double samplingtimes[N_BUFFERLOADS];
		double updatetimes[N_BUFFERLOADS];
		int cnt1;
		cout << "Starting Performance Test, running " <<  N_BUFFERLOADS << " updates " << endl;
		for(cnt1=0;cnt1 < N_BUFFERLOADS;cnt1++)
		{
			int start=clock();
			voc_test->update(FRAMES_BUFFER/SAMPLING_RATE);
			int stop1=clock();
			updatetimes[cnt1]=(stop1-start)/double(CLOCKS_PER_SEC)*1000.0;
			start=clock();
			/*for(int cnt2=0;cnt2<FRAMES_BUFFER;cnt2++)
			{
				voc->get_nextval();
			}*/
			playback_callback();
			int stop2=clock();
			samplingtimes[cnt1]=(stop2-start)/double(CLOCKS_PER_SEC)*1000.0;
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
}

#ifdef TESTING
int main()
{
	ofstream waveout;

	short*** wt;
	generate_wavetable();
	cout << "reading wavetable" << endl;
	wt=read_wavetable();
	waveout.open("waveout.txt");
	cout << "done" << endl;
	voc_test=new Voice(wt);
	buffer_test = new char[FRAMES_BUFFER*4];

	int cnt2=0;
	char note=30;
	voc_test->set_note((int)note);
	voc_test->set_on_off(1);
	voc_test->o2->set_symm(0.1);
	voc_test->env_vol->setAttack(42);
	voc_test->env_vol->setDecay(1);
	voc_test->env_vol->setSustain(1.0);
	voc_test->env_vol->setRelease(420);
	voc_test->env_div->setAttack(220);
	voc_test->env_div->setDecay(45);
	voc_test->env_div->setSustain(0.8);
	voc_test->env_div->setRelease(420);
	voc_test->lfo1->set_frequency(1.23);
	voc_test->filter->set_fcutoff(5000.0);
	voc_test->filter->set_res(0.6);
	voc_test->update(0.00533333);
	for(int r=0;r<2048;r++)
	{
		waveout << voc_test->get_nextval() << endl;
		cnt2++;
		if(cnt2 > FRAMES_BUFFER-1)
		{
			voc_test->update(0.00533333);
			cnt2=0;

		}
	}
	waveout.close();
}
#endif
