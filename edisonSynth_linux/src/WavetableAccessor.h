/*
 * generators.h
 *
 *  Created on: Dec 5, 2015
 *      Author: philipp
 */

#ifndef WAVETABLEACCESSOR_H_
#define WAVETABLEACCESSOR_H_

class WavetableAccessor
{
public:
	short*** read_wavetable();
	void generate_wavetable();
	bool is_wavetable_available();
};

#endif /* WAVETABLEACCESSOR_H_ */
