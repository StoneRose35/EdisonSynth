/*
 * Filter.cpp
 *
 *  Created on: Dec 29, 2015
 *      Author: philipp
 */

#include "Filter.h"
#include "constants.h"
#include <math.h>

Filter::Filter() {
	fcutoff=20000;
	res=0;
	y1=y2=y3=oldx=oldy1=oldy2=oldy3=0;
	calc(0.0);
}

double Filter::calc(double in)
{
	double x;
	x=in - r*y4;

	y1=x*p + oldx*p - k*y1;
	y2=y1*p+oldy1*p - k*y2;
	y3=y2*p+oldy2*p - k*y3;
	y4=y3*p+oldy3*p - k*y4;

	y4 = y4 - (y4*y4*y4)/6.0;
	oldx = x;
	oldy1 = y1;
	oldy2 = y2;
	oldy3 = y3;
	return y4;
}

void Filter::set_fcutoff(double fc)
{
	fcutoff=fc;
	f=(fcutoff+fcutoff)/SAMPLING_RATE;
	k=3.6*f-1.6*f*f -1;
	p=(k+1)*0.5;
	scale=exp((1.0-p)*1.386249);
	r=res*scale;
}

void Filter::set_res(double r)
{
	res = r;
	set_fcutoff(fcutoff);
}

Filter::~Filter() {
	// TODO Auto-generated destructor stub
}

