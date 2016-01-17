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



void Oscillator::set_waveform(int wf)
{
	waveform=wf;
}

Oscillator::Oscillator(short*** wt)
{
	samples_to_interpolate=FRAMES_BUFFER;
	interp_cntr=0;
	current_phase=0;
	current_frequency=225;
	current_symm=1.0;
	frequency_1=225;
	frequency_2=225;
	symm1=1;
	symm2=1;
	coeffs_active=1;
	waveform=1;
	d_phase=(double)SINE_SAMPLES*current_frequency/(double)SAMPLING_RATE;
	wavetable=wt;
}

Oscillator::Oscillator()
{
	samples_to_interpolate=FRAMES_BUFFER;
	interp_cntr=0;
	current_phase=0;
	current_frequency=225;
	current_symm=1.0;
	frequency_1=225;
	frequency_2=225;
	symm1=1;
	symm2=1;
	coeffs_active=1;
	waveform=1;
	d_phase=(double)SINE_SAMPLES*current_frequency/(double)SAMPLING_RATE;
}



void Oscillator::recalc_coeffs()
{
	double* coeffs2;
	double* harm_killer;
	double raw_val;
	double f_harm;
	double filter_coeff;
	double sum_coeffs=0;
	if(harm_coeffs1)
	{
		delete harm_coeffs1;
	}
	harm_coeffs1=new double[512];
	for(int ii=0;ii<512;ii++)
	{
		f_harm=current_frequency*ii;
		//q=(1.0+(f_harm/f_cutoff)*(f_harm/f_cutoff));
		filter_coeff = 1.0;// 1.0/sqrt(q*q*q*q + resonance*resonance -2*resonance*q*q*cos(4*atan(f_harm/f_cutoff)));
		if(waveform==0) // square wave
		{
			if(ii==0)
			{
				raw_val = (((current_symm/2.0-0.5)*2.0)*filter_coeff);
			}
			else
			{
				raw_val = ((4.0/M_PI*sin(M_PI*(double)ii*current_symm/2.0)/(double)ii))*filter_coeff;
			}
		}
		else
		{
			if(ii==0)
			{
				raw_val = 0.0;
			}
			else
			{
				raw_val = ((-2.0*(pow(-1.0,(double)ii)*pow(2.0/current_symm,2.0))/((double)ii*(double)ii*(2.0/current_symm-1.0)*M_PI*M_PI)*sin(((double)ii*(2.0/current_symm-1.0)*M_PI)/(2.0/current_symm)))*filter_coeff);
			}
		}
		sum_coeffs+=raw_val*raw_val;
		*(harm_coeffs1+ii)=raw_val;

	}
}
void Oscillator::set_symm(double s)
{
	current_symm=s;
}

double Oscillator::getNote(double f)
{
	double res;
	res = TWELVE_DIV_LOG2*(log(f) - LOG440);
	return res;
}

double Oscillator::get_nextval()
{

	double sample_val=0;


	if(coeffs_active==1)
	{
		current_symm =  symm1 + (symm2-symm1)*((double)interp_cntr/(double)samples_to_interpolate);
		current_frequency = frequency_1 - (frequency_2-frequency_1)*((double)interp_cntr/(double)samples_to_interpolate);
	}
	else
	{
		current_symm =  symm2 + (symm1-symm2)*((double)interp_cntr/(double)samples_to_interpolate);
		current_frequency = frequency_2 - (frequency_1-frequency_2)*((double)interp_cntr/(double)samples_to_interpolate);
	}
	d_phase = (double)SINE_SAMPLES*current_frequency/(double)SAMPLING_RATE;

	int symmInt=(int)(current_symm*255);

	current_phase+=d_phase;

	if(current_phase >= SINE_SAMPLES)
	{
		current_phase -= (double)SINE_SAMPLES;
	}

	int phaseInt=(int)current_phase;

	int fInt=(int)getNote(current_frequency)+48;

	sample_val=wavetable[fInt][symmInt][phaseInt];


	if(interp_cntr < samples_to_interpolate - 1)
	{
		interp_cntr++;
	}
	return sample_val;
}

/**
 * used only for the generation of the wavetable
 */
short Oscillator::compute_nextval()
{
	int harm_cntr=1;

	int intphase;
	double harm_phase;
	double sample_val=0;
	double harm_coeff;

	harm_phase=current_phase;


		harm_coeff=harm_coeffs1[0];
		current_frequency = frequency_1;


	d_phase = 1;
	sample_val+=harm_coeff;

	while(current_frequency*harm_cntr<F_LIMIT)
	{

			harm_coeff=harm_coeffs1[harm_cntr];
		intphase=(int)harm_phase;

		if(waveform==1)
		{
			sample_val+=sin(harm_phase*2.0*M_PI/SINE_SAMPLES)*harm_coeff;
		}
		else
		{
			sample_val+=cos(harm_phase*2.0*M_PI/SINE_SAMPLES)*harm_coeff;
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
	if(interp_cntr < samples_to_interpolate - 1)
	{
		interp_cntr++;
	}
	return (short)(sample_val*32767);
}

void Oscillator::update(double symmetry,double frequency)
{
	if(coeffs_active==1)
	{
		symm2=symmetry;
		frequency_2=frequency;
		coeffs_active=2;
	}
	else
	{
		symm1=symmetry;
		frequency_1=frequency;
		coeffs_active=1;
	}
	interp_cntr=0;
}

void Oscillator::update(double frequency)
{
	if(coeffs_active==1)
	{
		symm2=symm1;
		frequency_2=frequency;
		coeffs_active=2;
	}
	else
	{
		symm1=symm2;
		frequency_1=frequency;
		coeffs_active=1;
	}
	interp_cntr=0;
}
