/*
 * midi_controller.h
 *
 *  Created on: Jan 24, 2016
 *      Author: philipp
 */

#ifndef SEQMIDICONTROLLER_H_
#define SEQMIDICONTROLLER_H_
#define SEQUENCER_NAME "EdisonSynth Midi Sequencer"
#include "Voice.h"
#include <alsa/asoundlib.h>

class SeqMidiController
{
public:
	void init_midi_controller(Voice** vocs_addr,char* midi_dev,char* clientPort);
	void reattach_midi_client(char* new_midi_dev);
	int getAllMidiClients(char *** clientarray);
	int midi_action();
	snd_seq_t* mseq;
private:

	void open_seq(char* midi_dev);
	int searchMidiClient(const char * clientName);
	Voice **voices;
	snd_seq_port_subscribe_t *portsubs;
};



#endif /* SEQMIDICONTROLLER_H_ */
