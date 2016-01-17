#include "Voice.h"
#include "constants.h"
#include <ctime>
#include <cmath>
#include <iostream>
#include <fstream>
#include "generators.h"

#define N_BUFFERLOADS 2000

using namespace std;



#ifdef TESTING
int main()
{

	generate_wavetable();

	short*** wt;
	wt=read_wavetable();

	Voice* voc=new Voice(wt);


	char note=14;
  voc->set_note((int)note);
  voc->set_on_off(1);
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

}
#endif
