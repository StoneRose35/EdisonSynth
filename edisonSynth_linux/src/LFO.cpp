/*
 * LFO.cpp
 *
 *  Created on: Dec 29, 2015
 *      Author: philipp
 */

#include "LFO.h"
#include <math.h>

double LFO::get_frequency()
{
return frequency;
}

void LFO::set_frequency(double f)
{
	frequency = f;
}

void LFO::reset()
{
	current_phase=0;
}

double LFO::get_nextval(double delta_t)
{
	double nphase;
	double res;
	nphase=current_phase+delta_t;
	if(nphase> 1.0/frequency)
	{
		nphase=nphase - 1.0/frequency;
	}
	current_phase=nphase;
	if(waveform==1)
	{
		res = sin(2.0*M_PI*frequency*current_phase);
	}
	else if(waveform==2)
	{
		if(current_phase*frequency<0.5)
		{
			res = 0.0;
		}
		else
		{
			res= 1.0;
		}
	}
	else if (waveform==3)
	{
		res = frequency*current_phase;
	}
	else
	{
		res=1.0-frequency*current_phase;
	}
	return res;
}

LFO::LFO() {
	frequency=2.45;
	waveform=1;
	current_phase=0;

}

LFO::~LFO() {
	// TODO Auto-generated destructor stub
}

