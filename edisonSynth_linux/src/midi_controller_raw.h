/*
 * midi_controller_raw.h
 *
 *  Created on: Feb 11, 2016
 *      Author: philipp
 */

#ifndef MIDI_CONTROLLER_RAW_H_
#define MIDI_CONTROLLER_RAW_H_


void *midiinfunction(void *arg);
void init_rawmidi(char ** config,Voice ** vocs_addr,char * flag_running);


#endif /* MIDI_CONTROLLER_RAW_H_ */
