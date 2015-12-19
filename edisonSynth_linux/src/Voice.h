/*
 * Voice.h
 *
 *  Created on: Dec 11, 2015
 *      Author: philipp
 */

#include "Oscillator.h"

#ifndef VOICE_H_
#define VOICE_H_

class Voice
{

	int current_note;
	char is_on; // 0: off, 1: on
	short osc1_divider; // goes from 2 to 32767
	short osc2_divider;
	short osc1_semitones;
	short osc2_semitones;
	double osc1_symm;
	double osc2_symm;

public:
	Oscillator* o1;
	Oscillator* o2;
	Voice();
	void set_note(int note);
	short get_nextval();
	void update(double delta_t);
	void set_on_off(char on_off);
private:
	double get_frequency(double noteval);

};




#endif /* VOICE_H_ */
