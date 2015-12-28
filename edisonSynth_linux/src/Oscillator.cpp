/*
 * Oscillator.cpp
 *
 *  Created on: Dec 11, 2015
 *      Author: philipp
 */


#include "constants.h"
#include <iostream>
#include <fstream>
#include <math.h>
#include "Oscillator.h"
using namespace std;


void Oscillator::set_f(double f)
{
	current_frequency = f;
	d_phase = (double)SINE_SAMPLES*current_frequency/(double)SAMPLING_RATE;
}

void Oscillator::set_waveform(int wf)
{
	waveform=wf;
}

Oscillator::Oscillator()
{
	sinewaves = new short[SINE_SAMPLES];
	harm_coeffs=new short[512];
	current_phase=0;
	current_frequency=0;
	current_symm=1.0;
	f_cutoff=20000;
	resonance=0;

	ifstream wt_in;
	wt_in.open("sine.tab",ios::binary|ios::in);
	short bfrval;
	for(int z=0;z<SINE_SAMPLES;z++)
	{
		wt_in.read(reinterpret_cast<char*>(&bfrval),sizeof(short));
		*(sinewaves + z)=bfrval;
	}
	wt_in.close();

}

void Oscillator::set_fcutoff(double fc)
{
	f_cutoff = fc;
}

void Oscillator::set_resonance(double k)
{
	resonance=k;
}

void Oscillator::recalc_coeffs()
{
	short* coeffs2;
	short* harm_killer;
	double raw_val;
	double f_harm;
	double q;
	double filter_coeff;
	coeffs2=new short[512];
	for(int ii=0;ii<512;ii++)
	{
		f_harm=current_frequency*ii;
		q=(1.0+(f_harm/f_cutoff)*(f_harm/f_cutoff));
		filter_coeff = 1.0/sqrt(q*q*q*q + resonance*resonance -2*resonance*q*q*cos(4*atan(f_harm/f_cutoff)));
		if(waveform==0) // square wave
		{
			if(ii==0)
			{
				raw_val = (1.0/(((current_symm/2.0-0.5)*2.0)*filter_coeff));
			}
			else
			{
				raw_val = (1.0/((4.0/M_PI*sin(M_PI*(double)ii*current_symm/2.0)/(double)ii))*filter_coeff);
			}
		}
		else
		{
			if(ii==0)
			{
				raw_val = 1000000.0;
			}
			else
			{
				raw_val = (1.0/((-2.0*(pow(-1.0,(double)ii)*pow(2.0/current_symm,2.0))/((double)ii*(double)ii*(2.0/current_symm-1.0)*M_PI*M_PI)*sin(((double)ii*(2.0/current_symm-1.0)*M_PI)/(2.0/current_symm)))*filter_coeff));
			}
		}
		if(isinf(raw_val))
		{
			*(coeffs2+ii)=32767;
		}
		else if (fabs(raw_val) > 32767.0)
		{
			*(coeffs2+ii)=(short)copysign(32767.0,raw_val);
		}
		else
		{
			*(coeffs2+ii)=(short)round(raw_val);
		}
		//cout << "raw val is: " << raw_val << endl;
		//cout << "abs(raw val) is: " << fabs(raw_val) << endl;
		//cout << " coefficient #" << ii << " :" << *(coeffs2+ii) << endl;
	}
	harm_killer=harm_coeffs;
	harm_coeffs=coeffs2;
	delete harm_killer;
}
void Oscillator::set_symm(double s)
{
	current_symm=s;
}

short Oscillator::get_nextval()
{
	int harm_cntr=1;

	int intphase;
	double harm_phase;
	short sample_val=0;
	if(waveform==1)
	{
		harm_phase=current_phase;
	}
	else
	{
		harm_phase=current_phase + SINE_SAMPLES_4;
	}
	if(harm_phase >= SINE_SAMPLES)
	{
		harm_phase -= (double)SINE_SAMPLES;
	}
	sample_val+=32767/harm_coeffs[0];
	while(current_frequency*harm_cntr<F_LIMIT)
	{
		intphase=(int)harm_phase;
		if(harm_coeffs[harm_cntr]>-32767 && harm_coeffs[harm_cntr] < 32767)
		{
			sample_val+=(*(sinewaves + intphase))/harm_coeffs[harm_cntr];
		}
		//sample_val=sample_val << 2;
		harm_phase+=current_phase;
		if (harm_phase > SINE_SAMPLES)
		{
			harm_phase -= SINE_SAMPLES;
		}
		harm_cntr++;
	}
	//cout << " harmonics used: " << harm_cntr << endl;
	current_phase+=d_phase;
	if(current_phase >= SINE_SAMPLES)
	{
		current_phase -= (double)SINE_SAMPLES;
	}
	return sample_val;
}
