/*
 * LCDisplay.h
 *
 *  Created on: Jun 23, 2016
 *      Author: philipp
 */

#ifndef LCDISPLAY_H_
#define LCDISPLAY_H_


#include "gpio.hpp"

#define WAIT_TIME 3

using namespace mraa;

class LCDisplay {
public:
	LCDisplay();
	LCDisplay(int e_pn,int rs_pn,int rw_pn,int d0_pn,int d1_pn,int d2_pn,int d3_pn);
	void writeString(char* string);
	virtual ~LCDisplay();
private:
	Gpio* e_pin;
	Gpio* rs_pin;
	Gpio* rw_pin;
	Gpio* d4_pin;
	Gpio* d5_pin;
	Gpio* d6_pin;
	Gpio* d7_pin;
	void CmdIn(char cmd,int reg);// 0 instruction register, 1: data register
	void Set4BitOperation();
};



#endif /* LCDISPLAY_H_ */
