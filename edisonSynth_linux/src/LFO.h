/*
 * LFO.h
 *
 *  Created on: Dec 29, 2015
 *      Author: philipp
 */

#ifndef LFO_H_
#define LFO_H_

class LFO {
private:
	char waveform; // 1: sine, 2: square, 3: sawtooth rising, 4: sawtooth falling
	double frequency;
	double current_phase;
public:
	double get_nextval(double delta_t);
	void reset();
	void set_frequency(double f);
	double get_frequency();
	LFO();
	virtual ~LFO();
};

#endif /* LFO_H_ */
