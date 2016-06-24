/*
 * LCDisplay.cpp
 *
 *  Created on: Jun 23, 2016
 *      Author: philipp
 */

#include "LCDisplay.h"
#include "gpio.hpp"
#include <unistd.h>

using namespace std;

LCDisplay::LCDisplay() {
	// TODO Auto-generated constructor stub

}

LCDisplay::LCDisplay(int e_pn,int rs_pn,int rw_pn,int d0_pn,int d1_pn,int d2_pn,int d3_pn)
{
	// setup pins
	e_pin=new Gpio(e_pn);
	e_pin->dir(DIR_OUT);
	rs_pin=new Gpio(rs_pn);
	rs_pin->dir(DIR_OUT);
	rw_pin=new Gpio(rw_pn);
	rw_pin->dir(DIR_OUT);
	d4_pin=new Gpio(d0_pn);
	d4_pin->dir(DIR_OUT);
	d5_pin=new Gpio(d1_pn);
	d5_pin->dir(DIR_OUT);
	d6_pin=new Gpio(d2_pn);
	d6_pin->dir(DIR_OUT);
	d7_pin=new Gpio(d3_pn);
	d7_pin->dir(DIR_OUT);

	Set4BitOperation();
	CmdIn(0x28,0); // Function set: 2line display, 4bit operation
	CmdIn(12,0); // Display on, no cursor
	CmdIn(6,0); // Entry set: increment


}

void LCDisplay::writeString(char * string)
{
	int cntr=0;
	char linenumber=0;
	CmdIn(2,0); // return home
	// clear entire display (write empty characters
	for(int cnt2=0;cnt2<20;cnt2++)
	{
		CmdIn(0x20,1);
	}
	CmdIn(0xC0,0); // jump to second line
	for(int cnt2=0;cnt2<20;cnt2++)
	{
		CmdIn(0x20,1);
	}
	CmdIn(0x14,0); // jump to third line
	for(int cnt2=0;cnt2<20;cnt2++)
	{
		CmdIn(0x20,1);
	}
	CmdIn(0x54,0); // jump to fourth line
	for(int cnt2=0;cnt2<20;cnt2++)
	{
		CmdIn(0x20,1);
	}
	CmdIn(2,0); // return home
	while(*(string+cntr)!=0)
	{
		CmdIn(*(string+cntr),1);
		cntr++;
		if(cntr>19 && linenumber==0)
		{
			CmdIn(0xC0,0); // jump to second line
			linenumber++;
		}
		else if (cntr>39 && linenumber==1)
		{
			CmdIn(0x14,0); // jump to third line
			linenumber++;
		}
		else if (cntr>59 && linenumber==2)
		{
			CmdIn(0x54,0); // jump to fourth line
			linenumber++;
		}
	}
}

LCDisplay::~LCDisplay() {
	// TODO Auto-generated destructor stub
}

void LCDisplay::CmdIn(char cmd,int reg)
{
	int busyflag;

	d4_pin->dir(DIR_OUT);
	d5_pin->dir(DIR_OUT);
	d6_pin->dir(DIR_OUT);
	d7_pin->dir(DIR_OUT);

	rs_pin->write(reg);
	rw_pin->write(0);
	usleep(WAIT_TIME);
	e_pin->write(1);
	// write msn
	d4_pin->write(cmd & 0x10);
	d5_pin->write(cmd & 0x20);
	d6_pin->write(cmd & 0x40);
	d7_pin->write(cmd & 0x80);
	usleep(WAIT_TIME);
	e_pin->write(0);
	usleep(WAIT_TIME);
	e_pin->write(1);
	d4_pin->write(cmd & 0x01);
	d5_pin->write(cmd & 0x02);
	d6_pin->write(cmd & 0x04);
	d7_pin->write(cmd & 0x08);
	usleep(WAIT_TIME);
	e_pin->write(0);

	// check busy flag
	d4_pin->dir(DIR_IN);
	d5_pin->dir(DIR_IN);
	d6_pin->dir(DIR_IN);
	d7_pin->dir(DIR_IN);
	usleep(WAIT_TIME);
	rw_pin->write(1);
	usleep(WAIT_TIME);
	busyflag = d7_pin->read();
	e_pin->write(0);


	// read address counter (not used)
	usleep(WAIT_TIME);
	e_pin->write(1);
	usleep(WAIT_TIME);
	e_pin->write(0);
	while (busyflag > 0)
	{
		usleep(WAIT_TIME);
		e_pin->write(1);
		usleep(WAIT_TIME);
		busyflag = d7_pin->read();
		e_pin->write(0);

		// read address counter (not used)
		usleep(WAIT_TIME);
		e_pin->write(1);
		usleep(WAIT_TIME);
		e_pin->write(0);
	}
}

void LCDisplay::Set4BitOperation()
{
	int busyflag;
	d4_pin->dir(DIR_OUT);
	d5_pin->dir(DIR_OUT);
	d6_pin->dir(DIR_OUT);
	d7_pin->dir(DIR_OUT);
	int cmd = 0x20;
	rs_pin->write(0);
	rw_pin->write(0);
	usleep(WAIT_TIME);
	e_pin->write(1);
	// write msn
	d4_pin->write(cmd & 0x10);
	d5_pin->write(cmd & 0x20);
	d6_pin->write(cmd & 0x40);
	d7_pin->write(cmd & 0x80);
	usleep(WAIT_TIME);
	e_pin->write(0);
	usleep(WAIT_TIME);

	// check busy flag
	d4_pin->dir(DIR_IN);
	d5_pin->dir(DIR_IN);
	d6_pin->dir(DIR_IN);
	d7_pin->dir(DIR_IN);
	usleep(WAIT_TIME);
	rw_pin->write(1);
	usleep(WAIT_TIME);
	busyflag = d7_pin->read();
	e_pin->write(0);


	// read address counter (not used)
	usleep(WAIT_TIME);
	e_pin->write(1);
	usleep(WAIT_TIME);
	e_pin->write(0);
	while (busyflag > 0)
	{
		usleep(WAIT_TIME);
		e_pin->write(1);
		usleep(WAIT_TIME);
		busyflag = d7_pin->read();
		e_pin->write(0);

		// read address counter (not used)
		usleep(WAIT_TIME);
		e_pin->write(1);
		usleep(WAIT_TIME);
		e_pin->write(0);
	}
}

