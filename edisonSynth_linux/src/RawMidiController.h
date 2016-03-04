/*
 * midi_controller_raw.h
 *
 *  Created on: Feb 11, 2016
 *      Author: philipp
 */

#ifndef RAWMIDICONTROLLER_H_
#define RAWMIDICONTROLLER_H_

#define NOTEON 1
#define NOTEOFF 2
#define PITCHBEND 3
#define MIDICONTROLLER 4
#define CMD_UNKNOWN 64
#include <alsa/asoundlib.h>
#include "Voice.h"

class RawMidiController
{
public:
	void init(char **,Voice **,char *);
	snd_rawmidi_t* midiin = NULL;
	void midiinfunction();
private:

	static void *static_thread_method(void *arg);

	Voice ** voices_midiraw;
	pthread_t midiraw_controller_thread;
	char * should_be_running;
	unsigned char last_cmd;
};





#endif /* RAWMIDICONTROLLER_H_ */
