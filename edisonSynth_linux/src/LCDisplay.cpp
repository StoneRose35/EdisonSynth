/*
 * LCDisplay.cpp
 *
 *  Created on: Jun 23, 2016
 *      Author: philipp
 */

#include "LCDisplay.h"
#include "gpio.hpp"
#include <unistd.h>
#include <iostream>
#include "common.hpp"

using namespace std;

LCDisplay::LCDisplay() {
	// TODO Auto-generated constructor stub

}

LCDisplay::LCDisplay(int e_pn,int rs_pn,int rw_pn,int d0_pn,int d1_pn,int d2_pn,int d3_pn)
{
	mraa::init();
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

   //initialize by instructions
	WriteSingleCmd(0x30);
	usleep(5000);
	WriteSingleCmd(0x30);
	usleep(200);
	WriteSingleCmd(0x30);
	usleep(60);

	// set 4-bit interface
    WriteSingleCmd(0x20);
    usleep(60);
	CmdIn(0x28,0); // Function set: 1line display, 4bit operation
	CmdIn(12,0); // Display on, no cursor
	CmdIn(1,0); // Display clear
	CmdIn(6,0); // Entry set: increment


}

void LCDisplay::writeString(char * string)
{
	int cntr=0;
	int linecntr=0;
	char linenumber=0;
	CmdIn(1,0); // clear display

	while(*(string+cntr)!=0)
	{
		if(*(string+cntr)==10 || *(string+cntr)==13)
		{
			if(linenumber==0)
			{
				linecntr=20;
				CmdIn(0x40+0x80,0); // jump to second line
				linenumber++;
			}
			else if(linenumber==1)
			{
				linecntr=40;
				CmdIn(0x14+0x80,0); // jump to third line
				linenumber++;
			}
			else if(linenumber==2)
			{
				linecntr=60;
				CmdIn(0x54+0x80,0); // jump to fourth line
				linenumber++;
			}
			cntr++;
		}
		else
		{
			CmdIn(*(string+cntr),1);
			cntr++;
			linecntr++;
			if(linecntr>19 && linenumber==0)
			{
				CmdIn(0x40+0x80,0); // jump to second line
				linenumber++;
			}
			else if (linecntr>39 && linenumber==1)
			{
				CmdIn(0x14+0x80,0); // jump to third line
				linenumber++;
			}
			else if (linecntr>59 && linenumber==2)
			{
				CmdIn(0x54+0x80,0); // jump to fourth line
				linenumber++;
			}
		}
	}
	rs_pin->write(0);
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

	// write msn
	d4_pin->write(greaterZeroToInt(cmd & 0x10));
	d5_pin->write(greaterZeroToInt(cmd & 0x20));
	d6_pin->write(greaterZeroToInt(cmd & 0x40));
	d7_pin->write(greaterZeroToInt(cmd & 0x80));
	ToggleEPin();
	d4_pin->write(greaterZeroToInt(cmd & 0x01));
	d5_pin->write(greaterZeroToInt(cmd & 0x02));
	d6_pin->write(greaterZeroToInt(cmd & 0x04));
	d7_pin->write(greaterZeroToInt(cmd & 0x08));
	ToggleEPin();

	// check busy flag
	d4_pin->dir(DIR_IN);
	d5_pin->dir(DIR_IN);
	d6_pin->dir(DIR_IN);
	d7_pin->dir(DIR_IN);
	rs_pin->write(0);
	rw_pin->write(1);
	busyflag = ToggleEPin();


	// read address counter (not used)
	ToggleEPin();
	while (busyflag > 0)
	{
		busyflag = ToggleEPin();
		// read address counter (not used)
		ToggleEPin();
	}
}

int LCDisplay::ToggleEPin()
{
	int busyflag;
	usleep(WAIT_TIME);
	e_pin->mode(mraa::MODE_STRONG);
	e_pin->write(1);
	usleep(WAIT_TIME);
	busyflag = d7_pin->read();
	e_pin->write(0);
	usleep(WAIT_TIME);
	return busyflag;
}
int LCDisplay::greaterZeroToInt(int nrin)
{
	if(nrin>0)
		return 1;
	else
		return 0;
}


void LCDisplay::WriteSingleCmd(int cmd)
{
	d4_pin->dir(DIR_OUT);
	d5_pin->dir(DIR_OUT);
	d6_pin->dir(DIR_OUT);
	d7_pin->dir(DIR_OUT);
	rs_pin->write(0);
	rw_pin->write(0);
	usleep(WAIT_TIME);
	// write msn
	d4_pin->write(greaterZeroToInt(cmd & 0x10));
	d5_pin->write(greaterZeroToInt(cmd & 0x20));
	d6_pin->write(greaterZeroToInt(cmd & 0x40));
	d7_pin->write(greaterZeroToInt(cmd & 0x80));
	ToggleEPin();
}

