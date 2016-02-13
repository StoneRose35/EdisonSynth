/*
 * edisonSynth.h
 *
 *  Created on: Dec 5, 2015
 *      Author: philipp
 */

#ifndef EDISONSYNTHESIZER_H_
#define EDISONSYNTHESIZER_H_
#include "Oscillator.h"
#include "Voice.h"
#include <alsa/asoundlib.h>
#include "constants.h"
#include "RawMidiController.h"

class EdisonSynthesizer
{
public:
	EdisonSynthesizer(char * flag_running);
	void read_config();
	static double getFrequency(double notenumber);
	void init_voices();
	void init_midi();
	void start_synth();
	// midi controller object
	RawMidiController * rmc;
private:
	void start_audio(snd_pcm_t *handle,snd_pcm_hw_params_t *params);
	int playback_callback(snd_pcm_t* handle);
	void printIfError(int rc);
	void show_alsa_info();

	// handle to the audio playback device
	snd_pcm_t *handle;

	// structure holding the hardware parameters (sampling rate, access mode, bits etc.)
	snd_pcm_hw_params_t *params;

	// the array of voice objects for the synthesizer
	Voice * vocs[N_VOICES];

	// wavetable, this is a three-dim array
	short*** wt;

	// the string array containing the dynamically configurable items
	char ** config;

	// audio ring buffer
	char* buffer;

	// the precomputed time elapsed between two ring buffer writes, used of updating the modulation sources
	double delta_t;

	char * engine_running;

};

#define UPDATE_SIZE 2*N_CHANNELS*FRAMES_BUFFER
#define DELTA_T FRAMES_BUFFER/SAMPLING_RATE

#endif /* EDISONSYNTHESIZER_H_ */
