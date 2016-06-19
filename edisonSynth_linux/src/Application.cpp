/*
 * Application.c
 *
 *  Created on: Feb 13, 2016
 *      Author: philipp
 */
#include "EdisonSynthesizer.h"
#include <iostream>


using namespace std;


EdisonSynthesizer*  es;

// a flag stating whether the synth is running, 1 is running,0 is off, setting it 0 causes all the threads to end properly
char engine_running;

// a console thread allowing to shut down the synth properly
#ifdef USE_MONITORING
pthread_t shutdown_thread;
void* stop_synth(void* args)
{
	char answer[16];
	while(1)
	{
		cout << "enter \"q\" to stop the synth " << endl;
		cin >> answer;
		if(answer[0]=='q')
		{
			engine_running=0;
			break;
		}
	}
		return NULL;
}
#endif

#ifndef TESTING
int main() {



	engine_running=1;
	es=new EdisonSynthesizer(&engine_running);
	// read configuration file
	es->read_config();

	//initialize the voices
	es->init_voices();

	// initialize thread handling midi
	es->init_midi();

	// initialize the i2c controller
	es->init_i2c();

	// start the thread used to shut down the engine properly
#ifdef USE_MONITORING
	int err;
	err = pthread_create(&shutdown_thread,NULL,&stop_synth,NULL);
	if(err!=0)
	{
		cout << " error creating monitoring thread " << endl;
	}
#endif
	// THIS STARTS THE SOUND!!
	es->start_synth();



	return 0;
}
#endif


