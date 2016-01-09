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
	osc1_amt = 0.0;
	osc2_amt = 1.0;
	osc1_semitones=0;
	osc2_semitones=0;
	current_note=22;
	// fill both coefficients buffers
	o1->recalc_coeffs(FRAMES_BUFFER,get_frequency((double)(current_note - 48 + osc1_semitones)));
	o2->recalc_coeffs(FRAMES_BUFFER,get_frequency((double)(current_note - 48 + osc2_semitones)));
	o1->recalc_coeffs(FRAMES_BUFFER,get_frequency((double)(current_note - 48 + osc1_semitones)));
	o2->recalc_coeffs(FRAMES_BUFFER,get_frequency((double)(current_note - 48 + osc2_semitones)));
	is_on=0;
	env_value1=0.0;
	env_value2=0.0;
	param_set_active=1;
	samples_to_interpolate=FRAMES_BUFFER;
	interp_cntr=0;
}


/**
 * sets the note of the the Voice, the fourier coefficient are recalculated casing the interpolation counter of the oscillator to be reset
 * */
void Voice::set_note(int note)
{
	current_note=note;
	double f = get_frequency((double)(current_note - 48 + osc1_semitones));
	o1->recalc_coeffs(FRAMES_BUFFER,f);
	f = get_frequency((double)(current_note - 48 + osc2_semitones));
	o2->recalc_coeffs(FRAMES_BUFFER,f);
}

/*
 * gets the next sample value of the voice, (almsot) no computation is done the the voice is off
 * */
short Voice::get_nextval()
{
	double result=0;
	double env_amt;


	if(!env_vol->isOn())
	{
		return 0;
	}
	else
	{
		if(param_set_active == 1 )
		{
			env_amt = env_value1 + (env_value2-env_value1)*((double)interp_cntr/(double)samples_to_interpolate);
		}
		else
		{
			env_amt = env_value2 + (env_value1-env_value2)*((double)interp_cntr/(double)samples_to_interpolate);
		}
		if(interp_cntr < samples_to_interpolate - 1)
		{
			interp_cntr++;
		}
		result=o1->get_nextval()*osc1_amt;
		result+= o2->get_nextval()*osc2_amt;
		result*=env_amt;
		return (short)(result*32767);
	}

}

double Voice::get_frequency(double notenumber)
{
	return 440.0*pow(TWO_TWROOT,notenumber);
}

/**
 * updates the fourier coefficients and the volume envelope values,
 * first applies all modulators
 */
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
    o1->set_fcutoff(240+get_frequency(eval2*24));
	o2->set_fcutoff(140+get_frequency(eval2*24));
	//o2->set_symm(lfo1val*0.1+0.5);

	// calculator new oscillator coefficients
	int n_samples;
	n_samples=SAMPLING_RATE*delta_t;
	o1->recalc_coeffs(n_samples,get_frequency((double)(current_note - 48 + osc1_semitones)));
	o2->recalc_coeffs(n_samples,get_frequency((double)(current_note - 48 + osc2_semitones)));


	if(param_set_active==1)
	{
		env_value2=eval;
		param_set_active=2;
	}
	else
	{
		env_value1=eval;
		param_set_active=1;
	}
}

/**
 * sets the level of oscillator 1, range goes from 0 to 1
 * automatically adjusts the level of oscillator 2 to avoid clipping
 */
void Voice::set_osc1_level(double level)
{
	osc1_amt = level;
	if(osc1_amt + osc2_amt > 1)
	{
		osc2_amt = 1- (osc1_amt + osc2_amt);
	}
}

/**
 * sets the level of oscillator 2, range goes from 0 to 1
 * automatically adjusts the level of oscillator 1 to avoid clipping
 */
void Voice::set_osc2_level(double level)
{
	osc2_amt = level;
	if(osc1_amt + osc2_amt > 1)
	{
		osc1_amt = 1- (osc1_amt + osc2_amt);
	}
}

/**
 * switches the voice on or off, after switching off the voice the voice might still be "active" depending on the release value of the
 * volume envelope
 */
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



