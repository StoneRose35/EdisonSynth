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
	double fcutoff;
	double res;
	double f;
	double k;
	double p;
	double scale;
	double y1;
	double y2;
	double y3;
	double y4;
	double oldx;
	double oldy1;
	double oldy2;
	double oldy3;
	double r;
public:
	Filter();
	void init();
	void set_fcutoff(double fc);
	double get_fcutoff();
	void set_res(double r);
	double get_res();
	virtual ~Filter();
	double calc(double);
};

#endif /* FILTER_H_ */
