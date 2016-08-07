/*
 * Gui.cpp
 *
 *  Created on: Jun 24, 2016
 *      Author: philipp
 */

#include "Gui.h"
#include "gpio.hpp"
#include <unistd.h>
#include <iostream>

static void buttonUpHandler(void* args);
static void buttonDownHandler(void* args);
static void buttonOkHandler(void* args);

using namespace std;

/*
 * Pinout for display is as follows:
 * Arduino Pin0: RS
 *         Pin1: RW
 *         Pin2: E
 *         Pin4: DB7
 *         Pin6: DB6
 *         Pin8: DB5
 *         Pin12: DB4
 * */
Gui::Gui() {
	// TODO Auto-generated constructor stub

}

void Gui::init_gui(char * is_running,SeqMidiController * smcIn)
{
	int status;
	string dstring;
	should_be_running=is_running;
	display=new LCDisplay(2,0,1,12,8,6,4);
	midiclients=new char*[8];
	for(int k=0;k<8;k++)
	{
		midiclients[k]=new char[24];
		for (int l=0;l<24;l++)
		{
			midiclients[k][l]=0;
		}
	}
	smc=smcIn;
	buttonUp = new Gpio(14);
	buttonUp->dir(mraa::DIR_IN);
	buttonUp->mode(mraa::MODE_PULLUP);

	buttonDown = new Gpio(15);
	buttonDown->dir(mraa::DIR_IN);
	buttonDown->mode(mraa::MODE_PULLUP);

	buttonOk = new Gpio(16);
	buttonOk->dir(mraa::DIR_IN);
	buttonOk->mode(mraa::MODE_PULLUP);

	status=pthread_create(&gui_thread,NULL,Gui::static_thread_method,this);
	if(status ==-1)
	{
		cout << "Error creating gui controller thread" << endl;
	}
	sleep(4);

	buttonUp->isr(EDGE_RISING,buttonUpHandler,this);
	buttonDown->isr(EDGE_RISING,buttonDownHandler,this);
	buttonOk->isr(EDGE_RISING,buttonOkHandler,this);

	// initial display, show the first midi client available

	idDisplayed=0;
	idAttached=0;
	displayCurrentMidiClient();

}

void* Gui::static_thread_method(void * args)
{
	static_cast<Gui*>(args)->thread_function();
		return NULL;
}

void Gui::thread_function()
{
	//string dstring;
	while(*should_be_running==1)
	{
		n_clients = smc->getAllMidiClients(&midiclients);
		sleep(5);
	}
}
Gui::~Gui() {
	// TODO Auto-generated destructor stub
}

void Gui::displayCurrentMidiClient()
{
	string dstring;
	dstring=string("Midi Client:\n") + string(midiclients[idDisplayed]);
	if(idDisplayed==idAttached)
	{
		dstring+="\nSelected";
	}
	display->writeString((char*)dstring.c_str());

}

static void buttonUpHandler(void* args)
{
	string dstring;
	Gui* thisGui=(Gui*)args;
	if(thisGui->idDisplayed==thisGui->n_clients-1)
	{
		thisGui->idDisplayed=0;
	}
	else
	{
		thisGui->idDisplayed++;
	}

	thisGui->displayCurrentMidiClient();
	thisGui->buttonUp->isr(EDGE_NONE,NULL,NULL);
	usleep(30000);
	thisGui->buttonUp->isr(EDGE_RISING,buttonUpHandler,thisGui);
}

static void buttonDownHandler(void* args)
{
	string dstring;
	Gui* thisGui=(Gui*)args;
	if(thisGui->idDisplayed==0)
	{
		thisGui->idDisplayed=thisGui->n_clients-1;
	}
	else
	{
		thisGui->idDisplayed--;
	}

	thisGui->displayCurrentMidiClient();

	thisGui->buttonDown->isr(EDGE_NONE,NULL,NULL);
	usleep(30000);
	thisGui->buttonDown->isr(EDGE_RISING,buttonDownHandler,thisGui);
}

static void buttonOkHandler(void* args)
{
	string dstring;
	Gui* thisGui=(Gui*)args;
	thisGui->idAttached=thisGui->idDisplayed;

	thisGui->displayCurrentMidiClient();

	thisGui->buttonOk->isr(EDGE_NONE,NULL,NULL);
	usleep(30000);
	thisGui->buttonOk->isr(EDGE_RISING,buttonOkHandler,thisGui);
}

