/*
 * Envelope.h
 *
 *  Created on: Dec 23, 2015
 *      Author: philipp
 */

#ifndef ENVELOPE_H_
#define ENVELOPE_H_

class Envelope {
	double attack,decay,sustain,release;
	unsigned char phase; // 0: off, 1: attack, 2: decay, 3: sustain, 4: release
	double time_in_phase;
	double current_value;
public:
	Envelope();
	virtual ~Envelope();
	double nextval(double delta_t); // delta_t is in ms
	void setAttack(double a); // in ms
	void setDecay(double d); // in ms
	void setSustain(double s); // from 0 to 1
	void setRelease(double r); // in ms
	double getAttack();
	double getDecay();
	double getSustain();
	double getRelease();
	bool isOn();
	void switchOn();
	void switchOff();
};

#endif /* ENVELOPE_H_ */
