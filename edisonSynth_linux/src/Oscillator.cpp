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
	double raw_val;
	double sum_coeffs=0;
	if(harm_coeffs1)
	{
		delete harm_coeffs1;
	}
	harm_coeffs1=new double[512];
	for(int ii=0;ii<512;ii++)
	{
		if(waveform==0) // square wave
		{
			if(ii==0)
			{
				raw_val = (((symm1/2.0-0.5)*2.0));
			}
			else
			{
				raw_val = ((4.0/M_PI*sin(M_PI*(double)ii*symm1/2.0)/(double)ii));
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
				raw_val = ((-2.0*(pow(-1.0,(double)ii)*pow(2.0/symm1,2.0))/((double)ii*(double)ii*(2.0/symm1-1.0)*M_PI*M_PI)*sin(((double)ii*(2.0/symm1-1.0)*M_PI)/(2.0/symm1))));
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

short Oscillator::get_nextval()
{
	double sample_val=0;
	double delta = (interp_cntr/samples_to_interpolate);



	if(coeffs_active==2)
	{
		current_symm =  symm1 + (symm2-symm1)*delta;
		current_frequency = frequency_1 + (frequency_2-frequency_1)*delta;
	}
	else
	{
		current_symm =  symm2 + (symm1-symm2)*delta;
		current_frequency = frequency_2 + (frequency_1-frequency_2)*delta;
	}
	d_phase = SINE_SAMPLES*current_frequency/SAMPLING_RATE;

	int symmInt=(int)(current_symm*255);

	current_phase+=d_phase;

	if(current_phase >= SINE_SAMPLES)
	{
		current_phase -= SINE_SAMPLES;
	}

	int phaseInt=current_phase;

	int fInt=current_frequency/20.0-1.0;

	if(fInt < 0)
	{
		fInt=0;
	}
	if(fInt>255)
	{
		fInt=255;
	}

	sample_val=wavetable[fInt][symmInt][phaseInt];

//	cout << "sample at [" << fInt << "][" << symmInt << "][" << phaseInt << "]: " << sample_val << endl;

	if(interp_cntr < samples_to_interpolate - 1)
	{
		interp_cntr++;
	}
	return sample_val;
}

/**
 * used only for the generation of the wavetable
 */
double Oscillator::compute_nextval()
{
	int harm_cntr=1;

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
	return sample_val;
}

void Oscillator::update(double symmetry,double frequency)
{
	if(symmetry > 1)
	{
		symmetry=1;
	}
	if(symmetry<0.01)
	{
		symmetry=0.01;
	}
	if(frequency < 20)
	{
		frequency=20;
	}
	if(frequency > 5120)
	{
		frequency=5120;
	}
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
