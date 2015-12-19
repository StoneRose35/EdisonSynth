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
	short* harm_coeffs;
	double current_phase;
	double current_frequency;
	double d_phase;
	double current_symm; // symm goes from 1 (symmetric) to 0.01 (asymmetric)
	int waveform; //0: square, 1: triangle/sawtooth
public:
	Oscillator();
	short get_nextval();
	int get_waveform();
	void set_waveform(int wf);
	void reset();
	void set_f(double f);
	void set_symm(double s);
	void recalc_coeffs();
};


#endif /* OSCILLATOR_H_ */
