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

Voice::Voice(short*** wt)
{
	// initialize subcomponents
	o1=new Oscillator(wt);
	o2=new Oscillator(wt);
	lfo1=new LFO();
	lfo2=new LFO();
	env_vol = new Envelope();
	env_div=new Envelope();
	filter=new Filter();


	o1->set_waveform(0);
	o2->set_waveform(1);
	o2->set_symm(0.01);
	osc1_amt = 0.5;
	osc2_amt = 0.5;
	osc1_semitones=0;
	osc2_semitones=12;
	current_note=22;
	env_vol->setAttack(0);
	env_vol->setRelease(0);

	env_div->setAttack(20);
	env_div->setDecay(100);
	env_div->setSustain(0);


	// switch off and reset envelopes
	is_on=0;
	env_value1=0.0;
	env_value2=0.0;
	param_set_active=1;
	samples_to_interpolate=FRAMES_BUFFER;
	interp_cntr=0;

	 lfo1_to_o1_frequency=0.0;
	 lfo1_to_o2_frequency=0;
	 lfo1_to_filter_cutoff=0;
	 lfo1_to_filter_resonance=0;
	 lfo1_to_o1_symm=0;
	 lfo1_to_o2_symm=0;

	 lfo2_to_o1_frequency=0;
	 lfo2_to_o2_frequency=0;
	 lfo2_to_filter_cutoff=0;
	 lfo2_to_filter_resonance=0;
	 lfo2_to_o1_symm=0.0;
	 lfo2_to_o2_symm=0;

	 envd_to_o1_frequency=0;
	 envd_to_o2_frequency=0;
	 envd_to_filter_cutoff=0.5;
	 envd_to_filter_resonance=0;
	 envd_to_o1_symm=0;
	 envd_to_o2_symm=0;

	 o1_symm_basis=0.1;
	 o2_symm_basis=0.1;
	 filter_cutoff_base=600;
	 filter_reso_base=0.0;

	 pitchbend_value=0;
	 pitchbend_amount=12;
}


/**
 * sets the note of the the Voice, the fourier coefficient are recalculated casing the interpolation counter of the oscillator to be reset
 * */
void Voice::set_note(int note)
{
	current_note=note;
	double f = get_frequency((double)(current_note - 69 + osc1_semitones));
	o1->update(f);
	f = get_frequency((double)(current_note - 69 + osc2_semitones));
	o2->update(f);
}

int Voice::get_note()
{
	return current_note;
}

bool Voice::is_voice_on()
{
	return env_vol->isOn();
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
		double delta = (interp_cntr/samples_to_interpolate);
		if(param_set_active == 2 )
		{
			env_amt =  env_value1 + (env_value2-env_value1)*delta;
		}
		else
		{
			env_amt =  env_value2 + (env_value1-env_value2)*delta;
		}
		if(interp_cntr < samples_to_interpolate - 1)
		{
			interp_cntr++;
		}
		result=o1->get_nextval()*osc1_amt;
		result+= o2->get_nextval()*osc2_amt;
		result=filter->calc(result/32767.0)*32767.0;
		result*= env_amt;
		//cout << result << endl;
		return (short)result;
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
	double lfo1val;
	double lfo2val;

	double o_symm;
	double o_freqoffset;
	double f_f;
	double f_r;
	// update all modulators (Evelopes, LFO's)
	eval=env_vol->nextval(delta_t);
	eval2=env_div->nextval(delta_t);
	lfo1val=lfo1->get_nextval(delta_t);
	lfo2val=lfo2->get_nextval(delta_t);

	// set modulation matrix (which source goes to which target)
	o_symm=o1_symm_basis + lfo1_to_o1_symm*lfo1val + lfo2_to_o1_symm*lfo2val + envd_to_o1_symm*eval2;
	o_freqoffset=lfo1_to_o1_frequency*lfo1val + lfo2_to_o1_frequency*lfo1val + envd_to_o1_frequency*eval2;
	// calculator new oscillator coefficients
	//int n_samples;
	//n_samples=SAMPLING_RATE*delta_t;
	o1->update(o_symm,get_frequency((double)(current_note - 69 + osc1_semitones+o_freqoffset+pitchbend_value+0.05)));
	o_symm=o2_symm_basis + lfo1_to_o2_symm*lfo1val + lfo2_to_o2_symm*lfo2val + envd_to_o2_symm*eval2;
	o_freqoffset=lfo1_to_o2_frequency*lfo1val + lfo2_to_o2_frequency*lfo1val + envd_to_o2_frequency*eval2;
	o2->update(o_symm,get_frequency((double)(current_note - 69 + osc2_semitones+o_freqoffset+pitchbend_value)));

	f_f=get_frequency(filter_cutoff_base + lfo1_to_filter_cutoff*lfo1val + lfo2_to_filter_cutoff*lfo2val + envd_to_filter_cutoff*eval2);
	f_r=filter_reso_base + lfo1_to_filter_resonance*lfo1val + lfo2_to_filter_resonance*lfo2val + envd_to_filter_resonance*eval2;
	filter->update(f_f,f_r);


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
	interp_cntr=0;
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


void Voice::set_pitchbend_value(int pb_midi)
{
	pitchbend_value=((double)(pb_midi-8192))/8192*pitchbend_amount;
}
