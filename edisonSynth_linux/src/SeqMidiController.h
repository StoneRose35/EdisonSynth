/*
 * midi_controller.h
 *
 *  Created on: Jan 24, 2016
 *      Author: philipp
 */

#ifndef SEQMIDICONTROLLER_H_
#define SEQMIDICONTROLLER_H_
#include "Voice.h"

class SeqMidiController
{
public:
	void init_midi_controller(Voice** vocs_addr,char* midi_dev,char* clientPort,char* instrumentPort);
	int midi_action();
	snd_seq_t* mseq;
private:

	void open_seq(char* midi_dev);
	Voice **voices;
};



#endif /* SEQMIDICONTROLLER_H_ */
