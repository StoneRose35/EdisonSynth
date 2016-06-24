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

Gui::Gui() {
	// TODO Auto-generated constructor stub

}

void Gui::init_gui(char * is_running)
{
	int status;
	should_be_running=is_running;
	display=new LCDisplay(1,2,3,4,5,6,7);
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
	char* displaystring;
	while(*should_be_running==1)
	{
		displaystring="Hello Synthie";
		display->writeString(displaystring);
		usleep(50000);
	}
}
Gui::~Gui() {
	// TODO Auto-generated destructor stub
}

