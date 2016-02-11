/*
 * constants.h
 *
 *  Created on: Dec 5, 2015
 *      Author: philipp
 */

#ifndef CONSTANTS_H_
#define CONSTANTS_H_


/*
 * Configuration constants for the Synth itself
 * */
#define N_WAVEFORMS 5
#define SAMPLE_SIZE 1746

#define SAMPLING_RATE 48000
#define F_LIMIT 20000
#define N_NOTES 88
#define SINE_SAMPLES 2048 //16384
#define SINE_SAMPLES_4 4096
#define FRAMES_BUFFER 256
#define N_CHANNELS 2
#define CONFIG_FILE "sound_dev.cfg"
#define WAVETABLE_FILENAME 	"wavess.tab"
#define N_VOICES 8

/**
 * precomputed numbers
 */
#define TWO_TWROOT 1.059463094359295310 // twelve'th root of two
#define LOG440 6.0867747269 // the natural logarithm of 440
#define TWELVE_DIV_LOG2 17.3123404906676000 // 12/ln(2)



/*
 * Configuration constants for the build toolchain
 * */
//#define TESTING


#endif /* CONSTANTS_H_ */
