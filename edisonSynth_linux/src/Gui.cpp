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
static void buttonDownRisingHandler(void * args);
static void buttonUpRisingHandler(void * args);
static void buttonOkRisingHandler(void * args);

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

void Gui::init_gui(char * is_running,SeqMidiController * smcIn,I2CController * i2ccontr)
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
	i2cc=i2ccontr;

	buttonUp = new Gpio(3);
	buttonUp->dir(mraa::DIR_IN);
	buttonUp->mode(mraa::MODE_PULLUP);

	buttonDown = new Gpio(10);
	buttonDown->dir(mraa::DIR_IN);
	buttonDown->mode(mraa::MODE_PULLUP);

	buttonOk = new Gpio(11);
	buttonOk->dir(mraa::DIR_IN);
	buttonOk->mode(mraa::MODE_PULLUP);

	// initial display, show the first midi client available

	idDisplayed=0;
	idAttached=0;

	smc->getAllMidiClients(&midiclients);
	smc->reattach_midi_client(midiclients[0]);
	displayCurrentMidiClient(&midiclients);
	//sleep(2);

	status=pthread_create(&gui_thread,NULL,Gui::static_thread_method,this);
	if(status ==-1)
	{
		cout << "Error creating gui controller thread" << endl;
	}
	sleep(4);


	buttonUp->isr(EDGE_FALLING,buttonUpHandler,this);
	buttonDown->isr(EDGE_FALLING,buttonDownHandler,this);
	buttonOk->isr(EDGE_FALLING,buttonOkHandler,this);



	//displayCurrentMidiClient(&midiclients);

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

		n_clients = smc->getAllMidiClients(&midiclients);
		sleep(2);

	}
}
Gui::~Gui() {
	// TODO Auto-generated destructor stub
}

void Gui::displayCurrentMidiClient(char *** clientlist)
{
	string dstring;
	i2cc->request_pause();
	dstring=string("Midi Client ") + to_string(idDisplayed) + ":\n" + string((*clientlist)[idDisplayed]);
	if(idDisplayed==idAttached)
	{
		dstring+="\nSelected";
	}
	char * strptr=(char*)dstring.c_str();
	display->writeString(strptr);
	i2cc->restart();
}

static void buttonUpHandler(void* args)
{
	string dstring;
	Gui* thisGui=(Gui*)args;
	thisGui->buttonUp->isrExit();
	thisGui->buttonDown->isrExit();
	thisGui->buttonOk->isrExit();
	if(thisGui->idDisplayed==thisGui->n_clients-1)
	{
		thisGui->idDisplayed=0;
	}
	else
	{
		thisGui->idDisplayed++;
	}
	thisGui->displayCurrentMidiClient(&(thisGui->midiclients));
	usleep(30000);
	thisGui->buttonUp->isr(EDGE_RISING,buttonUpRisingHandler,thisGui);
	thisGui->buttonDown->isr(EDGE_FALLING,buttonDownHandler,thisGui);
	thisGui->buttonOk->isr(EDGE_FALLING,buttonOkHandler,thisGui);
}

static void buttonDownHandler(void* args)
{
	string dstring;
	Gui* thisGui=(Gui*)args;
	thisGui->buttonUp->isrExit();
	thisGui->buttonDown->isrExit();
	thisGui->buttonOk->isrExit();
	if(thisGui->idDisplayed==0)
	{
		thisGui->idDisplayed=thisGui->n_clients-1;
	}
	else
	{
		thisGui->idDisplayed--;
	}
	thisGui->displayCurrentMidiClient(&(thisGui->midiclients));
	usleep(10000);
	thisGui->buttonDown->isr(EDGE_RISING,buttonDownRisingHandler,thisGui);
	thisGui->buttonUp->isr(EDGE_FALLING,buttonUpHandler,thisGui);
	thisGui->buttonOk->isr(EDGE_FALLING,buttonOkHandler,thisGui);
}

static void buttonOkHandler(void* args)
{
	string dstring;
	int idAttachedOld;
	Gui* thisGui=(Gui*)args;
	thisGui->buttonUp->isrExit();
	thisGui->buttonDown->isrExit();
	thisGui->buttonOk->isrExit();
	idAttachedOld = thisGui->idAttached;
	thisGui->idAttached=thisGui->idDisplayed;

	if(idAttachedOld !=thisGui->idAttached )
	{
		thisGui->smc->reattach_midi_client(thisGui->midiclients[thisGui->idAttached]);
		thisGui->displayCurrentMidiClient(&(thisGui->midiclients));
	}
	usleep(30000);
	thisGui->buttonOk->isr(EDGE_RISING,buttonOkRisingHandler,thisGui);
	thisGui->buttonUp->isr(EDGE_FALLING,buttonUpHandler,thisGui);
	thisGui->buttonDown->isr(EDGE_FALLING,buttonDownHandler,thisGui);
}

static void buttonDownRisingHandler(void * args)
{
	Gui* thisGui=(Gui*)args;
	thisGui->buttonDown->isrExit();
	usleep(10000);
	thisGui->buttonDown->isr(EDGE_FALLING,buttonDownHandler,thisGui);
}

static void buttonUpRisingHandler(void * args)
{
	Gui* thisGui=(Gui*)args;
	thisGui->buttonUp->isrExit();
	usleep(10000);
	thisGui->buttonUp->isr(EDGE_FALLING,buttonUpHandler,thisGui);
}

static void buttonOkRisingHandler(void * args)
{
	Gui* thisGui=(Gui*)args;
	thisGui->buttonOk->isrExit();
	usleep(10000);
	thisGui->buttonOk->isr(EDGE_FALLING,buttonOkHandler,thisGui);
}
