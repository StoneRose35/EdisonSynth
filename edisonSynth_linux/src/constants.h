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
#define SAMPLING_RATE 48000 // The Sampling rate in Hz
#define F_LIMIT 20000 // The upper frequency limit for the waveform generation using additive synthetisation
#define N_NOTES 88 // the number of notes available
#define SINE_SAMPLES 16384 // number of samples in the precomputed sine function
#define SINE_SAMPLES_4 SINE_SAMPLES/4 // a quarter of the above ;-)
#define FRAMES_BUFFER 256 // the size of the frame buffer in samples, defines the latency


/*
 * Configuration constants for the build toolchain
 * */
#define TESTING


#endif /* CONSTANTS_H_ */
