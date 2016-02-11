/*
 * Envelope.cpp
 *
 *  Created on: Dec 23, 2015
 *      Author: philipp
 */

#include "Envelope.h"
#include <iostream>

using namespace std;

Envelope::Envelope() {
	phase=0;
	current_value=0;
	attack =0;
	decay =0;
	sustain =1.0;
	release=0;
	time_in_phase=0;
}


double Envelope::nextval(double delta_t)
{
double currval=1;

	if (phase==0)
	{
		current_value=0;
		return current_value;
	}
	if (phase==1)
	{
		if(time_in_phase+delta_t > attack)
		{
			time_in_phase = time_in_phase+delta_t - attack;
			phase = 2;
			int time_full_vol = clock();
			cout << "envelope at full volume at: " << time_full_vol/CLOCKS_PER_SEC*1000.0 << endl;
		}
		else
		{
			time_in_phase+=delta_t;
			current_value = time_in_phase/attack;
			return current_value;
		}
	}
	if(phase==2)
	{
		if(time_in_phase+delta_t > decay)
		{
			time_in_phase = time_in_phase+delta_t - decay;
			phase = 3;
		}
		else
		{
			time_in_phase+=delta_t;
			currval=1.0-time_in_phase/decay;
			if(currval < sustain)
			{
				phase=3;
				current_value = sustain;
				return current_value;
			}
			else
			{
				current_value =currval;
				return current_value;
			}
		}
	}
	if(phase==3)
	{
		current_value = sustain;
		return current_value;
	}
	if(phase==4)
	{
		if(time_in_phase+delta_t > release)
		{
			current_value =0;
			phase = 0;
			return current_value;
		}
		else if (current_value - (delta_t/release) < 0)
		{
			current_value =0;
			phase = 0;
			return current_value;
		}
		else
		{
			time_in_phase+=delta_t;
			current_value -= delta_t/release;
			return current_value;
		}
	}

	return 0;
}

void Envelope::setAttack(double a)
{
	attack =a/1000.0;
}

void Envelope::setDecay(double d)
{
	decay =d/1000.0;
}

void Envelope::setSustain(double s)
{
	sustain = s;
}

void Envelope::setRelease(double r)
{
	release = r/1000.0;
}

double Envelope::getAttack()
{
	return attack;
}

double Envelope::getDecay()
{
	return decay;
}

double Envelope::getSustain()
{
	return sustain;
}

double Envelope::getRelease()
{
	return release;
}

bool Envelope::isOn()
{
	return phase!=0;
}

void Envelope::switchOn()
{
	time_in_phase =0;
	current_value=0;
	phase = 1;
}

void Envelope::switchOff()
{
	time_in_phase=0;
	if(phase!=0)
	{
		phase = 4;
	}
}

Envelope::~Envelope() {
	// TODO Auto-generated destructor stub
}

