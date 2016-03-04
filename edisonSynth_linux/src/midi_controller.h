/*
 * midi_controller.h
 *
 *  Created on: Jan 24, 2016
 *      Author: philipp
 */

#ifndef MIDI_CONTROLLER_H_
#define MIDI_CONTROLLER_H_
#include "Voice.h"
snd_seq_t *open_seq(char* midi_dev);
int midi_action(snd_seq_t *seq_handle);
snd_seq_t * init_midi_controller(Voice** vocs_addr,char* midi_dev);



#endif /* MIDI_CONTROLLER_H_ */
