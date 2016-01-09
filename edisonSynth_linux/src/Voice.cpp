/*
 * Voice.cpp
 *
 *  Created on: Dec 11, 2015
 *      Author: philipp
 */
#include "Voice.h"
#include <math.h>
#include <iostream>
#include "constants.h"

using namespace std;

Voice::Voice()
{
	o1=new Oscillator();
	o2=new Oscillator();
	lfo1=new LFO();
	lfo2=new LFO();
	env_vol = new Envelope();
	env_div=new Envelope();
	o1->set_waveform(0);
	o2->set_waveform(1);
	osc1_divider = 1000;
	osc2_divider = 2;
	osc1_semitones=0;
	osc2_semitones=12;
	current_note=22;
	// fill both coefficients buffers
	o1->recalc_coeffs(FRAMES_BUFFER);
	o2->recalc_coeffs(FRAMES_BUFFER);
	o1->recalc_coeffs(FRAMES_BUFFER);
	o2->recalc_coeffs(FRAMES_BUFFER);
	is_on=0;
	env_divider1=32767;
	env_divider2=32767;
	param_set_active=1;
	samples_to_interpolate=FRAMES_BUFFER;
	interp_cntr=0;
}

void Voice::set_note(int note)
{
	current_note=note;
	double f = get_frequency((double)(current_note - 48 + osc1_semitones));
	o1->set_f(f);
	o2->set_f(f);
}

short Voice::get_nextval()
{
	short result=0;
	short env_divider;


	if(!env_vol->isOn())
	{
		return result;
	}
	else
	{
		if(param_set_active == 1 )
		{
			env_divider = env_divider1 + (env_divider2-env_divider1)*((double)interp_cntr/(double)samples_to_interpolate);
		}
		else
		{
			env_divider = env_divider2 + (env_divider1-env_divider2)*((double)interp_cntr/(double)samples_to_interpolate);
		}
		if(interp_cntr < samples_to_interpolate - 1)
		{
			interp_cntr++;
		}
		result=o1->get_nextval()/ osc1_divider;
		result+= o2->get_nextval() / osc2_divider;
		result/=env_divider;
		return result;
	}

}

double Voice::get_frequency(double notenumber)
{
	return 440.0*pow(TWO_TWROOT,notenumber);
}

void Voice::update(double delta_t)
{
	double eval;
	double eval2;
	//double lfo1val;
	//double lfo2val;

	// update all modulators (Evelopes, LFO's)
	eval=env_vol->nextval(delta_t);
	eval2=env_div->nextval(delta_t);
	//lfo1val=lfo1->get_nextval(delta_t);
	//lfo2val=lfo2->get_nextval(delta_t);

	// set modulation matrix (which source goes to which target)

	// set new oscillator properties (frequency, symmetry, filter q, filter f)
	o1->set_fcutoff(get_frequency(current_note*0.5 + eval2*24));
	o2->set_fcutoff(get_frequency(current_note*0.5 + eval2*24));

	// calculator new oscillator coefficients
	int n_samples;
	n_samples=SAMPLING_RATE*delta_t;
	o1->recalc_coeffs(n_samples);
	o2->recalc_coeffs(n_samples);

	if(eval == 0)
	{
		if(param_set_active==1)
		{
			env_divider2=32767;
			param_set_active=2;
		}
		else
		{
			env_divider1=32767;
			param_set_active=1;
		}
	}
	else
	{
		if(param_set_active==1)
		{
			env_divider2=(short)(1.0/eval);
			param_set_active=2;
		}
		else
		{
			env_divider1=(short)(1.0/eval);
			param_set_active=1;
		}
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
		env_div->switchOff();
	}
	else
	{
		env_vol->switchOn();
		env_div->switchOn();
	}
	is_on=on_off;
}



