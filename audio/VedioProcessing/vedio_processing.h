#ifndef VEDIO_PROCESSING_H_
#define VEDIO_PROCESSING_H_

#include <stdio.h>
#include <string.h>

#define FALSE 0
#define TRUE 1

int simplest_pcm16le_to_wave(const char *pcmpath, int channels, int sample_rate, const char *wavepath);
void pcm_lr_separate(const char *inputpath, const char *leftpath, const char *rightpath);
void lr_pcm_separate(const char *outputpath, const char *leftpath, const char *rightpath);
void wav_to_pcm(const char *inputpath, const char *outputpath);

void pcm_cut(const char *inputpath, const char *cutpath);

#endif