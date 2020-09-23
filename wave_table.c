/*
 * AutoZen
 * Steven James <pyro@linuxlabs.com>
 * Linux Labs http://www.linuxlabs.com
 * http://www.linuxlabs.com/software/AutoZen.html
 *
 * This is Free software, licensed under the GNU Public License V2.
 *
 * Version: 2.1
 */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "wave_table.h"

//////////////////////////////////
//
//	Waveform generation
//
/////////////////////////////////

#define MAX_HARMONICS 6
#define BEAT_MAX 40
#define SAMPLE_SIZE 2

static int *WaveTable;
static int *PhaseTable;

static double harmonic_curtimeL[MAX_HARMONICS];
static double harmonic_curtimeR[MAX_HARMONICS];
static double phase_curtime;
static double phase_curtime2;

static unsigned int sample_rate=0;

#define SCALE_FACTOR 32000

// WaveTable and PhaseTable are each 1 second long at the set sample rate. If simply iterated, they will produce valuse for a 1 Hz signal. 
// Convieniantly, the increment value for the index = desired frequency of the output in Hz

static int InitWaveTable(unsigned int SampleRate)
{
	unsigned int i;
	double increment = (2*M_PI) / SampleRate;
	double Current=0;

	WaveTable = (int *) calloc(SampleRate,sizeof(int));

	for(i=0;i<SampleRate;i++,Current += increment) {
		WaveTable[i]= (int) floor( sin(Current) * SCALE_FACTOR);
	}

	return(1);
}	// end InitWaveTable

static int InitPhaseTable(unsigned int SampleRate)
{
	unsigned int i;
	double increment = (2*M_PI) / SampleRate;
	double Current=0;

	PhaseTable = (int *) calloc(SampleRate,sizeof(int));
	
	for(i=0; i<SampleRate; i++)	// Phase is termed in samples ahead That is, a wave 180 degrees out of phase will be 1/2 of the wave table ahead.
		PhaseTable[i]=i;

	return(1);
}

static void IncrementCurtimes(double timeval[], int harmonics, double increment, double detune)
{
	int i;

	for(i=0;i<harmonics; i++) {
		timeval[i] += increment * pow(2,i) + detune;
		timeval[i] = fmod(timeval[i],sample_rate);
	}
}

static int ComputeSummation(double timeval[], int harmonics, double Volume, int phase_shift)
{
	int i;
	int sigma=0;

	if(harmonics > MAX_HARMONICS) {
		fprintf(stderr, "Programming error, harmonics (%d) > MAX_HARMONICS (%d)\n", harmonics, MAX_HARMONICS);
		exit(-1);
	}

	for(i=0; i<harmonics; i++) {
		int index = floor(timeval[i])+phase_shift;
		while(index<0)
			index+=sample_rate;
		index %=sample_rate;
		sigma += (int) (WaveTable[ index] /(1<<i));
	}

	sigma /=harmonics;

	return( floor( (Volume*sigma)/100));
}

static int ComputeMonauralSummation(double timeval[], double Volume, double fade, int phase_shift)
{
	int ret;
	double sigma;

	sigma = WaveTable[ ((int) floor(timeval[0]) + phase_shift)%sample_rate ] * fade;
	sigma += WaveTable[ ((int) floor(timeval[1]) + phase_shift)%sample_rate] * (1.0-fade);
	
	return ( floor(Volume*sigma/100));
}


int TimeStep(int sample[2], double base, double beat, int harmonics, double Volume, int mode)
{
	int i;

	phase_curtime= fmod( phase_curtime + beat, sample_rate);

	switch(mode) {
		case TS_MODE_CLASSIC:
			for(i=0; i<MAX_HARMONICS; i++) {
				if(i&1)
					harmonic_curtimeL[i] = fmod(harmonic_curtimeL[i] + base * pow(2,i), sample_rate);
				else
					harmonic_curtimeL[i] = fmod(harmonic_curtimeL[i] + base * pow(2,i) +beat, sample_rate);
			}
			sample[0] = ComputeSummation(harmonic_curtimeL, harmonics, Volume,0);
			sample[1] = ComputeSummation(harmonic_curtimeL, harmonics, Volume, PhaseTable[ (int) floor(phase_curtime)]);
			break;
		case TS_MODE_OLD:
			for(i=0; i<MAX_HARMONICS; i++) {
				if(i&1) {
					harmonic_curtimeL[i] = fmod(harmonic_curtimeL[i] + base * pow(2,i), sample_rate);
					harmonic_curtimeR[i] = fmod( harmonic_curtimeR[i] + base * pow(2,i) + beat, sample_rate);
				} else {
					harmonic_curtimeL[i] = fmod(harmonic_curtimeL[i] + base * pow(2,i) +beat, sample_rate);
					harmonic_curtimeR[i] = fmod( harmonic_curtimeR[i] + base * pow(2,i), sample_rate);
				}
			}
			sample[0] = ComputeSummation(harmonic_curtimeL, harmonics, Volume,0);
			sample[1] = ComputeSummation(harmonic_curtimeR, harmonics, Volume,0);
			break;

		case TS_MODE_BITONAL: {
			double fade;

			// currently, harmonics = 2, a base + 2nd harmonic.
			phase_curtime2= fmod( phase_curtime2 + beat/2, sample_rate);

			harmonic_curtimeL[0] = fmod(harmonic_curtimeL[0] + base, sample_rate);
			harmonic_curtimeL[1] = fmod(harmonic_curtimeL[1] + base*4, sample_rate);
			fade = (sin(2*M_PI*phase_curtime2/sample_rate) +1.0)/2.0;
			sample[0] = ComputeMonauralSummation( harmonic_curtimeL, Volume, fade, 0);
			sample[1] = ComputeMonauralSummation( harmonic_curtimeL, Volume, fade, PhaseTable[ (int) floor(phase_curtime)]);
			break;
		}
	}
}

int InitWavegen(unsigned int SampleRate)
{
	int i;

	InitWaveTable(SampleRate);
	InitPhaseTable(SampleRate);

	for(i=0;i<MAX_HARMONICS; i++)
		harmonic_curtimeL[i] = harmonic_curtimeR[i] = 0;

	phase_curtime=0;
	sample_rate = SampleRate;
}

void FreeWavegen(void) {
	free(PhaseTable);
	free(WaveTable);
	sample_rate=0;	
}

