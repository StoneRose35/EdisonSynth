/*
 * Gui.cpp
 *
 *  Created on: Jun 24, 2016
 *      Author: philipp
 */

#include "Gui.h"
#include <alsa/asoundlib.h>
#include <iostream>

using namespace std;

/*
 * Pinout for display is as follows:
 * Arduino Pin0: RS
 *         Pin1: RW
 *         Pin2: E
 *         Pin4: DB7
 *         Pin7: DB6
 *         Pin8: DB5
 *         Pin12: DB4
 * */
Gui::Gui() {
	// TODO Auto-generated constructor stub

}

void Gui::init_gui(char * is_running)
{
	int status;
	should_be_running=is_running;
	display=new LCDisplay(128,130,131,42,49,48,129);
	status=pthread_create(&gui_thread,NULL,Gui::static_thread_method,this);
	if(status ==-1)
	{
		cout << "Error creating gui controller thread" << endl;
	}
}

void* Gui::static_thread_method(void * args)
{
	static_cast<Gui*>(args)->thread_function();
		return NULL;
}

void Gui::thread_function()
{
	while(*should_be_running==1)
	{
		char* displaystring="Hello Synthie";
		display->writeString(displaystring);
		usleep(50000);
	}
}
Gui::~Gui() {
	// TODO Auto-generated destructor stub
}

