/*
 * seq2wav, part of AutoZen
 * Steven James <pyro@linuxlabs.com>
 * Linux Labs http://www.linuxlabs.com
 * http://www.linuxlabs.com/software/AutoZen.html
 *
 * This is Free software, licensed under the GNU Public License V2.
 *
 * Version: 1.3
 */



#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>

#include "wave_table.h"

//////////////////////////////////
//
//	.wav header
//
//////////////////////////////////

typedef struct {
	char 			riff[4]; 			// = "RIFF"
	unsigned int 	total_length;
	char 			id[8]; 				//  = "WAVEfmt_"
	int				fmtlen; 			// = 0x10
	short int		format;				// = 01
	short int 		channels;
	unsigned int 	sample_rate;
	unsigned int 	bytes_per_second;
	short int		bytes_per_sample;
	short int		bits_per_sample;
	char			filler[4]; 			//  = "DATA"
	unsigned int 	sample_bytes;
} 	__attribute__ ((packed)) wav_header;

wav_header wh;
	

//////////////////////////////////
//
//	Waveform generation
//
/////////////////////////////////

#define SAMPLE_RATE 44100

#define DEFAULT_HARMONICS 3

#define ZERO_CROSSING 32768

int nHarmonics = DEFAULT_HARMONICS;

double increment=300;
double detune=10.0;
double volume = 50.0;

int mode = TS_MODE_BITONAL;

/* sequencer variables */
FILE *fSequence=NULL;
char szInstruction[1024];
double target;
double dBeatIncrement;

int count=0;
volatile int seconds=0;

// functions 
			
				
long int GenerateSamples(double frequency, double beat, int tenth_seconds, FILE *Output)
{
	int iCur;
	int iCur2;
	int iCharIn;
	char quit=0;
	int i,j,count=0;
	int samples[2];


	for(i = tenth_seconds; i ; i--) {
		for(j=0;j<SAMPLE_RATE/10;j++) {
			TimeStep(samples, frequency, beat, nHarmonics, volume, mode);

			fwrite(&(samples[0]),1,2,Output); // left
			fwrite(&(samples[1]),1,2,Output); // right

			count+=4;	// bump the sample counter!
		}
	}	// end for i		

return(count);
}

unsigned long int RunSequence(FILE *Sequence, FILE *Out)
	{
	char *token;
	int i;
	unsigned long int count =0;
	static int first=0;


	while( fgets(szInstruction,sizeof(szInstruction),Sequence) ) {	// fetch it in
		token = strtok(szInstruction," ,\n");
	
		if(!strcmp(token,"VOLUME")) {
			token = strtok(NULL," ,\n");
			/* token is the volume */
			volume = atof(token);

			if(volume > 100)
				volume = 100;
			else if (volume <0) 
				volume = 0;
		
		}

		if(!strcmp(token, "MODE")) {
			token = strtok(NULL," ,\n");
			if(!strcmp(token, "BITONAL"))
				mode = TS_MODE_BITONAL;
			if(!strcmp(token, "OLD"))
				mode = TS_MODE_OLD;
			if(!strcmp(token, "CLASSIC"))
				mode = TS_MODE_CLASSIC;
		}

		if(!strcmp(token,"FADE")) {
			token = strtok(NULL," ,\n");
			target = atof(token);
			token = strtok(NULL," ,\n");
			i = atoi(token);
	
			i*=10;
			dBeatIncrement = target - volume;
			dBeatIncrement/= (i);
	
			for(; i ; i--) {
				count += GenerateSamples(increment, detune, 1, Out);
				volume += dBeatIncrement;
			}
				
		}
			
		if(!strcmp(token,"BASE")) { /* set the base frequency */
			token = strtok(NULL," ,\n");
			increment = atof(token);
		}
			
		if(!strcmp(token,"HARMONICS")) { /* set the base frequency */
			token = strtok(NULL," ,\n");
			i = atoi(token);
			if(i<MAX_HARMONICS)
				nHarmonics = i;
		}
			
		if(!strcmp(token,"SET")) {
			/* we are to set the frequency */
			token = strtok(NULL," ,\n");
			/* token is the frequency */
			detune = atof(token);
		}
	
		if(!strcmp(token,"SLIDE")) {
			token = strtok(NULL," ,\n");
			target = atof(token);
			token = strtok(NULL," ,\n");
			i = atoi(token);
	
			dBeatIncrement = target - detune;
			dBeatIncrement/= i;
	
			for(; i ; i--) {
				count += GenerateSamples(increment, detune, 10, Out);
				detune += dBeatIncrement;
			}
				
		}
	
		if(!strcmp(token,"HOLD")) {
			token = strtok(NULL," ,\n");
			i = atoi(token);
			count += GenerateSamples(increment, detune, i*10, Out);
		}
	
	
		if(!strcmp(token,"EXIT")) {
			return(count);
		}
	
		if(!strcmp(token,"END")) {
			return(count);
		}
	}	// end while

	return(count);
}	/* end RunSequence */

void InitHeaderConstants(void)
{
	memcpy(wh.riff, "RIFF", 4);
	memcpy(wh.id, "WAVEfmt ", 8);
	wh.format = 0x01;
	wh.fmtlen = 0x10;
	wh.channels = 2;
	wh.sample_rate = SAMPLE_RATE;
	wh.bytes_per_second = SAMPLE_RATE *4; // (2 bytes/sample * 2 channels)
	wh.bytes_per_sample = 4;				// 2 channels * 2 bytes/sample
	wh.bits_per_sample = 16;				// 2 bytes/sample * 8
	memcpy(wh.filler, "data", 4);
}

int main (int argc, char *argv[])
{
	struct stat statbuf;
	FILE *fOut;

	if(argc <3) {
		printf("Create a .wav file ready to burn to CD from an AutoZen sequence file:\n");
		printf("%s <sequence> <output>\n",argv[0]);
		printf("\t<sequence> = sequence file to read\n");
		printf("\t<output> the .wav file to create\n");
		return(-1);
	}

	if((fSequence = fopen(argv[1],"r")) == NULL) {
		fprintf(stderr,"ERROR: cannot open %s for reading\n",argv[1]);
		return(-2);
	}

	if((fOut = fopen(argv[2],"w")) == NULL) {
		fprintf(stderr,"ERROR: cannot open %s for writing\n",argv[2]);
		return(-3);
	}

	InitWavegen(SAMPLE_RATE);

	InitHeaderConstants();

	fwrite(&wh,sizeof(wav_header), 1, fOut);

	RunSequence(fSequence, fOut);

	fclose(fSequence);

	fclose(fOut);

	// now, apply fixup to .wav header
	stat(argv[2], &statbuf);

	wh.total_length = statbuf.st_size - 8;
	wh.sample_bytes = statbuf.st_size - sizeof(wav_header);

	fOut = fopen(argv[2], "r+");

	fwrite(&wh,sizeof(wav_header), 1, fOut);

	fclose (fOut);

	return(0);
}


