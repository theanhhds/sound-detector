/* This program is just for testing */
#include "wave.h"
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include "screen.h"
#include "comm.h"

//this program will open a wave file, a display WAV header info
//this program will create a wave file

void testTone(int freq, double d)
{
	FILE *fp;
	int i;
	WAVHDR h;
	fp = fopen ("testtone2.wav","w");
	fillID("RIFF",h.ChunkID); // Chunk1size will be calculated later
	fillID("WAVE",h.Format);
	fillID("fmt ",h.Subchunk1ID);
	fillID("data", h.Subchunk2ID);
	h.Subchunk1Size = 16;
	h.AudioFormat = 1;
	h.NumChannels = 2;
	h.SampleRate = SAMPLE_RATE;
	h.BitsPerSample = 16;
	h.ByteRate = h.SampleRate * h.NumChannels * (h.BitsPerSample/8);
	h.BlockAlign = h.NumChannels * (h.BitsPerSample/8);
	h.Subchunk2Size = (int) d*h.ByteRate;
	h.ChunkSize = h.Subchunk2Size + 36;
	fwrite(&h,sizeof(h),1,fp);
	int sample;
	for (i=0;i<=d*SAMPLE_RATE;i++)
	{
		srand(time(NULL));
		sample = (i-32768-i*i/1000)*rand() * sin(2*PI*freq*i/SAMPLE_RATE);
		fwrite(&sample, sizeof(sample),1,fp);
		sample =32768*2 * sin(2*PI*freq*i/SAMPLE_RATE);
		fwrite(&sample, sizeof(sample),1,fp);
	}
	fclose(fp);
}

void fillID(const char *s, char d[])
{
	int i;
	for (i=0;i<4;i++) d[i]=*s++;
}

void displayWAVHDR(WAVHDR hdr)
{
	double Duration;
	Duration = (double)hdr.Subchunk2Size/hdr.ByteRate;
#ifdef DEBUG
	printf("Chunk ID: "); printID(hdr.ChunkID);
	printf("ChunkSize: %d\n",hdr.ChunkSize);
	printf("Chunk ID format: "); printID(hdr.Format);
	printf("Sub chunk 1 ID: "); printID(hdr.Subchunk1ID);
	printf("Sub chunk 1 size: %d\n",hdr.Subchunk1Size);
	printf("Audio Format: %d\n",hdr.AudioFormat);
	printf("Number of channels: %d\n",hdr.NumChannels);
	printf("Sample rate: %d\n",hdr.SampleRate);
	printf("Byte rate: %d\n",hdr.ByteRate);
	printf("Block Align: %d\n",hdr.BlockAlign);
	printf("Bit Per Sample: %d\n",hdr.BitsPerSample);
	printf("Sub chunk 2 ID: ");printID(hdr.Subchunk2ID);
	printf("Subchunk2Size: %d\n",hdr.Subchunk2Size);
	printf("Duration of Audio %.3f sec\n",Duration);
#else
	gotoXY(1,1); setFGcolor(RED); printf("%.2f s",Duration);
	gotoXY(1,10); setFGcolor(CYAN); printf("%d bit/s",hdr.BitsPerSample);
	gotoXY(1,20); setFGcolor(YELLOW); printf("%d sps",hdr.SampleRate);
	resetColor();
	int i;
	printf("\n");
	for (i=0;i<=206;i++) printf("-");
#endif
}

void printID(char id[])
{
	int i;
	for (i=0;i<4;i++) putchar(id[i]);
	printf("\n");
}

//function calculates 1s sample into 80 pieces of RMS values
//However, only 8 pieces of RMS data are sent to the server as Fast Mode
// of Sound Level Meter (SLM)

void displayWAVdata(short int d[])
{
//	system("clear");
	int i,j;
	double sum200, rms200, max200=0.0, min200=30000.0;	//Cal RMS200
	double Leqf[8], sum2000 = 0.0;				//Cal RMS2000 (fast Leq values)

	for (i=0;i<=80-2;i++) //Terminal has 80 columns
	{
		sum200=0.0; //Initialize the accumulator
		for (j=0;j<=SAMPLE_RATE/80;j++)
		{
			sum200 += (*d)*(*d);
			d++;			//treat d as a pointer, pointer increament
		}
		sum2000+=sum200;
		if (i%10==9)
		{
			Leqf[i/10]=sqrt(sum2000/SAMPLE_RATE/8);
			sum2000 = 0.0;	//reset sum2000
		}
		rms200 = sqrt(sum200/(SAMPLE_RATE/80));
//		rms200 = 20 * log10(rms200);		//find decibel value
		if (max200 < rms200) max200=rms200;
		if (min200 > rms200) min200=rms200;
		// find decibel value of sound using logrithm
#ifdef DEBUG	//conditional compiling
//		printf("%10.2f \n",rms200);
#else
		setFGcolor(GREEN);
		displayBar(rms200,i+2);
#endif
	}
#ifdef DEBUG
//		printf("max = %.2f min = %.2f\n",max200,min200);
#endif

#ifdef COMM	//Only in the case COMM is defined, then send data to server
	send_data_curl(Leqf);
#endif
}
