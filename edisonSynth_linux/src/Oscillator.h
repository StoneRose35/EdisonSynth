/*
 * Oscillator.h
 *
 *  Created on: Dec 11, 2015
 *      Author: philipp
 */

#ifndef OSCILLATOR_H_
#define OSCILLATOR_H_


class Oscillator
{
	short* sinewaves;
	double* harm_coeffs1;
	double* harm_coeffs2;
	short* harm_coeffs_swap;
	char coeffs_active; //1 or 2
	int samples_to_interpolate; // for how many samples the coefficients should be interpolated;
	int interp_cntr; // the interpolation counter, increases with each get_nextval(), is reset when recalc_coeffs() is called
	double current_phase;
	double current_frequency;
	double frequency_1;
	double frequency_2;
	double d_phase;
	double current_symm; // symm goes from 1 (symmetric) to 0.01 (asymmetric)
	int waveform; //0: square, 1: triangle/sawtooth
	double f_cutoff;
	double resonance; // goes from 0 to -2

public:
	Oscillator();
	double get_nextval();
	int get_waveform();
	void set_waveform(int wf);
	void reset();
	void set_symm(double s);
	void recalc_coeffs(int nsamples,double nfreq);
	void set_fcutoff(double fc);
	void set_resonance(double k);
};


#endif /* OSCILLATOR_H_ */
