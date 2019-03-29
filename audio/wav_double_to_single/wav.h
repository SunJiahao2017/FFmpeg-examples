#ifndef __WAV__H__
#define __WAV__H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct _wav_riff_t{
	char id[5]; //ID:"RIFF"
	int size; //file_len - 8
	char type[5]; //type:"WAVE"
}wav_riff_t;


typedef struct _wav_format_t{
	char id[5]; //ID:"fmt"
	int size;
	short compression_code;
	short channels;
	int samples_per_sec;
	int avg_bytes_per_sec;
	short block_align;
	short bits_per_sample;
}wav_format_t;


typedef struct _wav_fact_t{
	char id[5];
	int size;
}wav_fact_t;


typedef struct _wav_data_t{
	char id[5];
	int size;
}wav_data_t;


typedef struct _wav_t{
	FILE *fp;
	wav_riff_t riff;
	wav_format_t format;
	wav_fact_t fact;
	wav_data_t data;
	int file_size;
	int data_offset;
	int data_size;
}wav_t;

typedef struct _wav_head{
	char    riff_id[4];        //0-3
	int     riff_size;         //4-7
	char    riff_fmt[4];       //8-11
	char    fmt_id[4];         //12-15
	int     fmt_size;          //16-19
	short   fmt_audiofmt;      //20-21
	short   fmt_channel;       //22-23
	int     fmt_samplerate;    //24-27
	int     fmt_byterate;      //28-31
	short   fmt_blockalign;    //32-33
	short   fmt_bitspersample; //34-35
	char    data_id[4];        //36-39
	int     data_size;         //40-43
}wav_head;


//wav_t *wav_open(char *file_name, wav_t *wav);
wav_t *wav_open(char *file_name);
int  strncasecmp(char *s1, char *s2, register int n);
void wav_close(wav_t **wav);
void wav_rewind(wav_t *wav);
int  wav_over(wav_t *wav);
int  wav_read_data(wav_t *wav, char *buffer, int buffer_size);
void wav_dump(wav_t *wav);
void printf_t(const char *fmt, ...);

extern int wav_print_log;

#endif