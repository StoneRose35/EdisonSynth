/*
 * Filter.h
 *
 *  Created on: Dec 29, 2015
 *      Author: philipp
 */

#ifndef FILTER_H_
#define FILTER_H_

class Filter {
private:
	double fcutoff; // cutoff frequency, goes from 0 to 20000
	double res; // the resonance, goes from 0 to 1
	double f;

	// values needed for computations
	double k1;
	double k2;
	double p1;
	double p2;
	double r1;
	double r2;

	//interpolation stuff
	double interp_cntr;
	double samples_to_interpolate;
	char coeffs_active;

	double scale;
	double y1;
	double y2;
	double y3;
	double y4;
	double oldx;
	double oldy1;
	double oldy2;
	double oldy3;

public:
	Filter();
	void init();
	void set_fcutoff(double fc);
	double get_fcutoff();
	void set_res(double r);
	void update(double cutoff,double reson);
	double get_res();
	virtual ~Filter();
	double calc(double);
};

#endif /* FILTER_H_ */
