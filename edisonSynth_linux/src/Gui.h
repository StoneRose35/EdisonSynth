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
#include "I2CController.h"

class Gui {
public:
	Gui();
	virtual ~Gui();
	void init_gui(char * is_running,SeqMidiController * smcIn,I2CController *i2ccontr);
	int idDisplayed;
	int n_clients;
	int idAttached;
	Gpio* buttonUp;
	Gpio* buttonDown;
	Gpio* buttonOk;
	char ** midiclients;
	SeqMidiController * smc;
	I2CController * i2cc;
	void displayCurrentMidiClient(char *** clientlist);
private:
	static void *static_thread_method(void *arg);
	void thread_function();

	pthread_t gui_thread;
	char* should_be_running;
	LCDisplay* display;





};

#endif /* GUI_H_ */
