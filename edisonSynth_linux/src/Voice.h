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

	double lfo1_to_o1_frequency;
	double lfo1_to_o2_frequency;
	double lfo1_to_filter_cutoff;
	double lfo1_to_filter_resonance;
	double lfo1_to_o1_symm;
	double lfo1_to_o2_symm;

	double lfo2_to_o1_frequency;
	double lfo2_to_o2_frequency;
	double lfo2_to_filter_cutoff;
	double lfo2_to_filter_resonance;
	double lfo2_to_o1_symm;
	double lfo2_to_o2_symm;

	double envd_to_o1_frequency;
	double envd_to_o2_frequency;
	double envd_to_filter_cutoff;
	double envd_to_filter_resonance;
	double envd_to_o1_symm;
	double envd_to_o2_symm;

	double o1_symm_basis;
	double o2_symm_basis;
	double filter_cutoff_base;
	double filter_reso_base;

	double pitchbend_value;
	double pitchbend_amount;

public:
	Oscillator* o1;
	Oscillator* o2;
	Envelope* env_vol;
	Envelope* env_div;
	LFO* lfo1;
	LFO* lfo2;
	Filter* filter;
	Voice(short*** wt);


	short get_nextval();
	void update(double delta_t);

	void set_note(int note);
	int get_note();
	bool is_voice_on();
	void set_on_off(char on_off);
	void set_osc1_level(double level);
	void set_osc2_level(double level);
	void set_pitchbend_value(int pb_midi); // pitch bend goin from 0 to 2^14 -1
private:
	double get_frequency(double noteval);

};




#endif /* VOICE_H_ */
