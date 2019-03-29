#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../VedioProcessing/vedio_processing.h"


/**
* Convert PCM16LE raw data to WAVE format
* @param pcmpath      Input PCM file.
* @param channels     Channel number of PCM file.
* @param sample_rate  Sample rate of PCM file.
* @param wavepath     Output WAVE file.
*/
int simplest_pcm16le_to_wave(const char *pcmpath, int channels, int sample_rate, const char *wavepath)
{

	typedef struct WAVE_HEADER {
		char         fccID[4];
		unsigned   long    dwSize;
		char         fccType[4];
	}WAVE_HEADER;

	typedef struct WAVE_FMT {
		char         fccID[4];
		unsigned   long       dwSize;
		unsigned   short     wFormatTag;
		unsigned   short     wChannels;
		unsigned   long       dwSamplesPerSec;
		unsigned   long       dwAvgBytesPerSec;
		unsigned   short     wBlockAlign;
		unsigned   short     uiBitsPerSample;
	}WAVE_FMT;

	typedef struct WAVE_DATA {
		char       fccID[4];
		unsigned long dwSize;
	}WAVE_DATA;


	if (channels == 0 || sample_rate == 0) {
		channels = 2;
		sample_rate = 44100;
	}
	int bits = 16;

	WAVE_HEADER   pcmHEADER;
	WAVE_FMT   pcmFMT;
	WAVE_DATA   pcmDATA;

	unsigned   short   m_pcmData;
	FILE   *fp, *fpout;

	fp = fopen(pcmpath, "rb");
	if (fp == NULL) {
		printf("open pcm file error\n");
		return -1;
	}
	fpout = fopen(wavepath, "wb+");
	if (fpout == NULL) {
		printf("create wav file error\n");
		return -1;
	}
	//WAVE_HEADER
	memcpy(pcmHEADER.fccID, "RIFF", strlen("RIFF"));
	memcpy(pcmHEADER.fccType, "WAVE", strlen("WAVE"));
	fseek(fpout, sizeof(WAVE_HEADER), 1);
	//WAVE_FMT
	pcmFMT.dwSamplesPerSec = sample_rate;
	pcmFMT.dwAvgBytesPerSec = pcmFMT.dwSamplesPerSec * sizeof(m_pcmData);
	pcmFMT.uiBitsPerSample = bits;
	memcpy(pcmFMT.fccID, "fmt ", strlen("fmt "));
	pcmFMT.dwSize = 16;
	pcmFMT.wBlockAlign = 2;
	pcmFMT.wChannels = channels;
	pcmFMT.wFormatTag = 1;

	fwrite(&pcmFMT, sizeof(WAVE_FMT), 1, fpout);

	//WAVE_DATA;
	memcpy(pcmDATA.fccID, "data", strlen("data"));
	pcmDATA.dwSize = 0;
	fseek(fpout, sizeof(WAVE_DATA), SEEK_CUR);

	fread(&m_pcmData, sizeof(unsigned short), 1, fp);
	while (!feof(fp)) {
		pcmDATA.dwSize += 2;
		fwrite(&m_pcmData, sizeof(unsigned short), 1, fpout);
		fread(&m_pcmData, sizeof(unsigned short), 1, fp);
	}

	pcmHEADER.dwSize = 44 + pcmDATA.dwSize;

	rewind(fpout);
	fwrite(&pcmHEADER, sizeof(WAVE_HEADER), 1, fpout);
	fseek(fpout, sizeof(WAVE_FMT), SEEK_CUR);
	fwrite(&pcmDATA, sizeof(WAVE_DATA), 1, fpout);

	fclose(fp);
	fclose(fpout);

	return 0;
}

//立体声拆分成左右两个声道
void pcm_lr_separate(const char *inputpath, const char *leftpath, const char *rightpath)
{
	char buf[16] = { 0 };
	static int leftFlag = FALSE;
	int size = 0;

	FILE *fp = fopen(inputpath, "rb+");
	FILE *fp_l = fopen(leftpath, "wb+");
	FILE *fp_r = fopen(rightpath, "wb+");

	while (!feof(fp))
	{
		size = fread(buf, 1, 2, fp); // 此处是读取16bit（一个声道），一个字节8位，所以count为2

		if ((size>0) && (leftFlag == FALSE))
		{
			leftFlag = TRUE;
			fwrite(buf, 1, size, fp_l);
		}
		else if ((size>0) && (leftFlag == TRUE))
		{
			leftFlag = FALSE;
			fwrite(buf, 1, size, fp_r);
		}
	}
	printf("立体声拆分左右声道完成...\n");
	fclose(fp);
	fclose(fp_l);
	fclose(fp_r);
}

//左右声道合成为立体声
void lr_pcm_separate(const char *outputpath, const char *leftpath, const char *rightpath)
{
	char buf[16] = { 0 };
	static int leftFlag = FALSE;
	int size = 0;

	FILE *fp = fopen(outputpath, "wb+");
	FILE *fp_l = fopen(leftpath, "rb+");
	FILE *fp_r = fopen(rightpath, "rb+");

	while (1)
	{
		size = fread(buf, 1, 2, fp_r);
		fwrite(buf, 1, size, fp);
		size = fread(buf, 1, 2, fp_l);
		fwrite(buf, 1, size, fp);
		if (feof(fp_l) != 0 && feof(fp_r) != 0)
		{
			break;
		}
	}
	printf("左右声道合成立体声完成...\n");
	fclose(fp);
	fclose(fp_l);
	fclose(fp_r);
}

void wav_to_pcm(const char *inputpath, const char *outputpath)	//输入路径，输出路径，
{
	char buf[16] = { 0 };
	char FormatData[440] = { 0 };

	//char ChunkID[4] = { 0 };
	//char ChunkSize[4] = { 0 };
	//char Format[4] = { 0 };
	//char Subchunk1ID[4] = { 0 };
	//char Subchunk1Size[4] = { 0 };
	//char AudioFormat[2] = { 0 };
	//char NumChannels[2] = { 0 };
	//char SampleRate[4] = { 0 };
	//char ByteRate[4] = { 0 };
	//char BlockAlign[2] = { 0 };
	//char BitsPerSample[2] = { 0 };
	//char Subchunk2ID[4] = { 0 };
	//char Subchunk2Size[4] = { 0 };

	int size = 0;

	FILE *fp_out = fopen(outputpath, "wb+");
	FILE *fp_in = fopen(inputpath, "rb+");
	
	if (NULL == fp_in) {
		printf("open input file error...\n");
		return;
	}
	size = fread(FormatData, 1, 44, fp_in);
	while (1) {
		
		size = fread(buf, 1, 2, fp_in);
		fwrite(buf, 1, size, fp_out);

		if (feof(fp_in) != 0)
		{
			break;
		}
	}
	printf("WAV文件转PCM文件完成...\n");
	
	fclose(fp_in);
	fclose(fp_out);
}

void pcm_cut(const char *inputpath, const char *cutpath) 
{
	char buf[160] = { 0 };
	int size = 0;
	long int tmp = 0;
	long int run_time = 0;//准备截取的长度
	long int FileSize = 0;

	FILE *fp = fopen(inputpath, "rb+");//用于计算文件长度
	FILE *fp_in = fopen(inputpath, "rb+");
	FILE *fp_cut = fopen(cutpath, "wb+");//新建文件，存放截取的音频文件

	if (NULL == fp) {
		perror("Error opening file");
		return;
	}
	else {
		fseek(fp, 0, SEEK_END);
		FileSize = ftell(fp);
	}

	run_time = FileSize - 19200;

	while (1)
	{

		fread(buf, 16, 10, fp_in);
		fwrite(buf, 16, 10, fp_cut);
		tmp = tmp + 160;
		if (tmp >= run_time) {
			break;
		}
	
	}
	fclose(fp);
	fclose(fp_in);
	fclose(fp_cut);
}