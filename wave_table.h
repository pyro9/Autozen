#ifndef WAVE_TABLE_H
#define WAVE_TABLE_H
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

#define MAX_HARMONICS 6
#define BEAT_MAX 40
#define SAMPLE_SIZE 2

#define TS_MODE_CLASSIC 0
#define TS_MODE_BITONAL 1
#define TS_MODE_OLD 2

int TimeStep(int sample[2], double base, double beat, int harmonics, double Volume, int mode);
int InitWavegen(unsigned int SampleRate);
void FreeWavegen(void);
#endif
