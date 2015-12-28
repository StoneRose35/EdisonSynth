/*
 * Voice.cpp
 *
 *  Created on: Dec 11, 2015
 *      Author: philipp
 */
#include "Voice.h"
#include <math.h>
#include <iostream>

using namespace std;

Voice::Voice()
{
	o1=new Oscillator();
	o2=new Oscillator();
	env_vol = new Envelope();
	o1->set_waveform(0);
	o2->set_waveform(1);
	osc1_divider = 1000;
	osc2_divider = 2;
	osc1_semitones=0;
	osc2_semitones=12;
	current_note=22;
	o1->recalc_coeffs();
	o2->recalc_coeffs();
	is_on=0;
	env_divider=32767;
}

void Voice::set_note(int note)
{
	current_note=note;
	cout << "current note is: " << current_note << endl;
	double nn = (double)(current_note - 48 + osc1_semitones);
	double f = get_frequency((double)(current_note - 48 + osc1_semitones));
	o1->set_f(f);
	o2->set_f(f);
}

short Voice::get_nextval()
{
	short result=0;
	if(!env_vol->isOn())
	{
		return result;
	}
	else
	{
		result=o1->get_nextval()/ osc1_divider;
		result+= o2->get_nextval() / osc2_divider;
		result/=env_divider;
		return result;
	}

}

double Voice::get_frequency(double notenumber)
{
	return 440.0*pow(pow(2.0,1.0/12.0),notenumber);
}

void Voice::update(double delta_t)
{
	double eval;
	o1->recalc_coeffs();
	o2->recalc_coeffs();
	eval=env_vol->nextval(delta_t);
	if(eval == 0)
	{
		env_divider=32767;
	}
	else
	{
		env_divider = (short)(1.0/eval);
	}
}

void Voice::set_osc1_level(double level)
{
	osc1_divider = (int)((1.0-level)*32765.0+2.0);
}

void Voice::set_osc2_level(double level)
{
	osc2_divider = (int)((1.0-level)*32765.0+2.0);
}
void Voice::set_on_off(char on_off)
{
	if(on_off ==0)
	{
		env_vol->switchOff();
	}
	else
	{
		env_vol->switchOn();
	}
	is_on=on_off;
}



