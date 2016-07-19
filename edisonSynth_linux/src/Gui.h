/*
 * Gui.h
 *
 *  Created on: Jun 24, 2016
 *      Author: philipp
 */

#ifndef GUI_H_
#define GUI_H_
#include "LCDisplay.h"
#include "SeqMidiController.h"

class Gui {
public:
	Gui();
	virtual ~Gui();
	void init_gui(char * is_running,SeqMidiController * smcIn);
	int idDisplayed;
	int n_clients;
	int idAttached;
	Gpio* buttonUp;
	Gpio* buttonDown;
	Gpio* buttonOk;
private:
	static void *static_thread_method(void *arg);
	void thread_function();
	pthread_t gui_thread;
	char* should_be_running;
	LCDisplay* display;
	char ** midiclients;


	SeqMidiController * smc;


};
static void buttonUpHandler(void* args);
static void buttonDownHandler(void* args);
static void buttonOkHandler(void* args);
#endif /* GUI_H_ */
