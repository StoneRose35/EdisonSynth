/*
 * Filter.cpp
 *
 *  Created on: Dec 29, 2015
 *      Author: philipp
 */

#include "Filter.h"
#include "constants.h"
#include <iostream>
#include <math.h>

using namespace std;

Filter::Filter() {
	fcutoff=20000;
	res=0;
	y1=y2=y3=oldx=oldy1=oldy2=oldy3=0;
	calc(0.0);
}

double Filter::calc(double in)
{
	double x;
	double k,p,r;
	double delta = (interp_cntr/samples_to_interpolate);
	if(coeffs_active==1)
	{
		k=k2 + (k1-k2)*delta;
		p=p2 + (p1-p2)*delta;
		r=r2 + (r1-r2)*delta;
	}
	else
	{
		k=k1 + (k2-k1)*delta;
		p=p1 + (p2-p1)*delta;
		r=r1 + (r2-r1)*delta;
	}
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
	if(interp_cntr < samples_to_interpolate-1)
	{
		interp_cntr++;
	}
	//cout << "in: " << in << ", y1: " << y1 << ", y2: " << y2 << ", y3: " << y3 << ", y4: " << y4 << endl;
	return y4;
}

void Filter::set_fcutoff(double fc)
{
	if(fc<20)
	{
		fc=20;
	}
	if(fc>20000)
	{
		fc=20000;
	}
	if(coeffs_active==1)
	{
		fcutoff=fc;
		f=(fcutoff+fcutoff)/SAMPLING_RATE;
		k1=3.6*f-1.6*f*f -1;
		p1=(k1+1)*0.5;
		scale=exp((1.0-p1)*1.386249);
		r1=res*scale;
	}
	else
	{
		fcutoff=fc;
		f=(fcutoff+fcutoff)/SAMPLING_RATE;
		k2=3.6*f-1.6*f*f -1;
		p2=(k2+1)*0.5;
		scale=exp((1.0-p2)*1.386249);
		r2=res*scale;
	}
}

void Filter::update(double cutoff,double reson)
{
	if(coeffs_active==1)
	{
		coeffs_active=2;
		set_res(reson);
		set_fcutoff(cutoff);
	}
	else
	{
		coeffs_active=1;
		set_res(reson);
		set_fcutoff(cutoff);
	}
	interp_cntr=0;
}
void Filter::set_res(double r)
{
	if(r>1)
	{
		r=1;
	}
	if(r < 0)
	{
		r=0;
	}
	res = r;
	set_fcutoff(fcutoff);
}

double Filter::get_res()
{
	return res;
}

double Filter::get_fcutoff()
{
return fcutoff;
}
Filter::~Filter() {
	// TODO Auto-generated destructor stub
}

