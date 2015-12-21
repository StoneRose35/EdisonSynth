/*
 * Voice.cpp
 *
 *  Created on: Dec 11, 2015
 *      Author: philipp
 */
#include "Voice.h"
#include <math.h>

Voice::Voice()
{
	o1=new Oscillator();
	o2=new Oscillator();

	o1->set_waveform(0);
	o2->set_waveform(1);
	osc1_divider = 1000;
	osc2_divider = 2;
	osc1_semitones=0;
	osc2_semitones=12;
	o1->recalc_coeffs();
	o2->recalc_coeffs();
	is_on=0;
}

void Voice::set_note(int note)
{
	current_note=note;
	o1->set_f(get_frequency((double)(current_note - 48 + osc1_semitones)));
	o2->set_f(get_frequency((double)(current_note - 48 + osc2_semitones)));
}

short Voice::get_nextval()
{
	short result=0;
	if(is_on==0)
	{
		return result;
	}
	else
	{
		result=o1->get_nextval()/ osc1_divider;
		result+= o2->get_nextval() / osc2_divider;
		return result;
	}

}

double Voice::get_frequency(double notenumber)
{
	return 440.0*pow(pow(2.0,1.0/12.0),notenumber);
}

void Voice::update(double delta_t)
{
	o1->recalc_coeffs();
	o2->recalc_coeffs();
}

void Voice::set_on_off(char on_off)
{
	is_on=on_off;
}



