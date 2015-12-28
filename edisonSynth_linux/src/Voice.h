/*
 * Voice.h
 *
 *  Created on: Dec 11, 2015
 *      Author: philipp
 */

#include "Oscillator.h"
#include "Envelope.h"

#ifndef VOICE_H_
#define VOICE_H_

class Voice
{
private:
	int current_note;
	char is_on; // 0: off, 1: on
	short osc1_divider; // goes from 2 to 32767
	short osc2_divider;
	short osc1_semitones;
	short osc2_semitones;
	short env_divider;
public:
	Oscillator* o1;
	Oscillator* o2;
	Envelope* env_vol;
	Voice();
	void set_note(int note);
	short get_nextval();
	void update(double delta_t);
	void set_on_off(char on_off);
	void set_osc1_level(double level);
	void set_osc2_level(double level);
private:
	double get_frequency(double noteval);

};




#endif /* VOICE_H_ */
