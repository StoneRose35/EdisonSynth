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
private:
	char coeffs_active; //1 or 2
	double samples_to_interpolate; // for how many samples the coefficients should be interpolated;
	double interp_cntr; // the interpolation counter, increases with each get_nextval(), is reset when recalc_coeffs() is called
	double current_phase;
	double current_frequency;
	double frequency_1;
	double frequency_2;
	double symm1;
	double symm2;
	double d_phase;
	double current_symm; // symm goes from 1 (symmetric) to 0.01 (asymmetric)
	int waveform; //0: square, 1: triangle/sawtooth
	short*** wavetable;

	double* harm_coeffs1;



public:
	Oscillator();
	Oscillator(short*** wavetable);
	short get_nextval();
	double compute_nextval();
	int get_waveform();
	void set_waveform(int wf);
	void reset();
	void set_symm(double s);
	double getNote(double f);
	void update(double symmetry,double frequency);
	void update(double frequency);
	void recalc_coeffs();
};


#endif /* OSCILLATOR_H_ */
