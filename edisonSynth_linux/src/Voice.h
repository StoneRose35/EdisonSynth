/*
 * Voice.h
 *
 *  Created on: Dec 11, 2015
 *      Author: philipp
 */

#include "Oscillator.h"
#include "Envelope.h"
#include "LFO.h"
#include "Filter.h"

#ifndef VOICE_H_
#define VOICE_H_


class Voice
{
private:
	int current_note;
	char is_on; // 0: off, 1: on
	double osc1_amt; // goes from 0 to  1
	double osc2_amt;
	short osc1_semitones;
	short osc2_semitones;
	double env_value1;
	double env_value2;
	char param_set_active; // either 1 or 2
	double interp_cntr;
	double samples_to_interpolate;
public:
	Oscillator* o1;
	Oscillator* o2;
	Envelope* env_vol;
	Envelope* env_div;
	LFO* lfo1;
	LFO* lfo2;
	Filter* filter;
	Voice(short*** wt);
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
