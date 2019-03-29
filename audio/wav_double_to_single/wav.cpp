#include "wav.h"
#include <ctype.h>
#include <stdarg.h>

int wav_print_log = 0;


int strncasecmp(char *s1, char *s2, register int n)
{
	while (--n >= 0 && toupper((unsigned char)*s1) == toupper((unsigned char)*s2++))
	    if (*s1++ == ' ') return 0;
	return(n < 0 ? 0 : toupper((unsigned char)*s1) - toupper((unsigned char)*--s2));
}

//wav_t *wav_open(char *file_name, wav_t *wav)
wav_t *wav_open(char *file_name)
{
	wav_t *wav = NULL;
	char buffer[256];
	int read_len = 0;
	int offset = 0;

	if (NULL == file_name)
	{
		printf_t("file_name is NULL\n");
		return NULL;
	}
	
	wav = (wav_t *)malloc(sizeof(wav_t));
	if (NULL == wav)
	{
		printf_t("malloc wav failedly\n");
		return NULL;
	}
	
	memset(wav, 0, sizeof(wav_t));

	wav->fp = fopen(file_name, "rb+");
	if (NULL == wav->fp)
	{
		printf_t("fopen %s failedly\n", file_name);
		free(wav);
		return NULL;
	}

	//handle RIFF WAVE chunk 
	read_len = fread(buffer, 1, 12, wav->fp);
	if (read_len < 12)
	{
		printf_t("error wav file\n");
		wav_close(&wav);
		return NULL;
	}

	if (strncasecmp("RIFF", buffer, 4))
	{
		printf_t("error wav file\n");
		wav_close(&wav);
		return NULL;
	}
	memcpy(wav->riff.id, buffer, 4);
	wav->riff.size = *(int *)(buffer + 4);
	if (strncasecmp("WAVE", buffer + 8, 4))
	{
		printf_t("error wav file\n");
		wav_close(&wav);
		return NULL;
	}
	memcpy(wav->riff.type, buffer + 8, 4);
	wav->file_size = wav->riff.size + 8;
	offset += 12;
	while (true)
	{
		char id_buffer[5] = { 0 };
		int tmp_size = 0;

		read_len = fread(buffer, 1, 8, wav->fp);
		if (read_len < 8)
		{
			printf_t("error wav file\n");
			wav_close(&wav);
			return NULL;
		}
		memcpy(id_buffer, buffer, 4);
		tmp_size = *(int *)(buffer + 4);

		if (0 == strncasecmp("FMT", id_buffer, 3))
		{
			memcpy(wav->format.id, id_buffer, 4);
			wav->format.size = tmp_size;
			read_len = fread(buffer, 1, tmp_size, wav->fp);
			if (read_len < tmp_size)
			{
				printf_t("error wav file\n");
				wav_close(&wav);
				return NULL;
			}
			wav->format.compression_code = *(short *)buffer;
			wav->format.channels = *(short *)(buffer + 2);
			wav->format.samples_per_sec = *(int *)(buffer + 4);
			wav->format.avg_bytes_per_sec = *(int *)(buffer + 8);
			wav->format.block_align = *(short *)(buffer + 12);
			wav->format.bits_per_sample = *(short *)(buffer + 14);
		}
		else if (0 == strncasecmp("DATA", id_buffer, 4))
		{
			memcpy(wav->data.id, id_buffer, 4);
			wav->data.size = tmp_size;
			offset += 8;
			wav->data_offset = offset;
			wav->data_size = wav->data.size;
			break;
		}
		else{
			printf_t("unhandled chunk: %s, size: %d\n", id_buffer, tmp_size);
			fseek(wav->fp, tmp_size, SEEK_CUR);
		}
		offset += 8 + tmp_size;
	}

	return wav;
}


void wav_close(wav_t **wav){
	wav_t *tmp_wav;

	if (NULL == wav)
	{
		return;
	}

	tmp_wav = *wav;
	if (NULL == tmp_wav)
	{
		return;
	}

	if (NULL != tmp_wav->fp)
	{
		fclose(tmp_wav->fp);
	}

	free(tmp_wav);
	*wav = NULL;
}

void wav_rewind(wav_t *wav){
	if (fseek(wav->fp, wav->data_offset, SEEK_SET) < 0)
	{
		printf_t("wav rewind failedly\n");
	}
}

int wav_over(wav_t *wav)
{
	return feof(wav->fp);
}


int wav_read_data(wav_t *wav, char *buffer, int buffer_size)
{
	return fread(buffer, 1, buffer_size, wav->fp);
}


void wav_dump(wav_t *wav){
	printf_t("file length: %d\n", wav->file_size);
	printf_t("\nRIFF WAVE Chunk\n");
	printf_t("id: %s\n", wav->riff.id);
	printf_t("size: %d\n", wav->riff.size);
	printf_t("type: %s\n", wav->riff.type);

	printf_t("\nFORMAT Chunk\n");
	printf_t("id: %s\n", wav->format.id);
	printf_t("size: %d\n", wav->format.size);
	if (wav->format.compression_code == 0)
	{
		printf_t("compression: Unknown\n");
	}
	else if (wav->format.compression_code == 1)
	{
		printf_t("compression: PCM/uncompressed\n");
	}
	else if (wav->format.compression_code == 2)
	{
		printf_t("compression: Microsoft ADPCM\n");
	}
	else if (wav->format.compression_code == 6)
	{
		printf_t("compression: ITU G.711 a-law\n");
	}
	else if (wav->format.compression_code == 7)
	{
		printf_t("compression: ITU G.711 ?¦Ì-law\n");
	}
	else if (wav->format.compression_code == 17)
	{
		printf_t("compression: IMA ADPCM\n");
	}
	else if (wav->format.compression_code == 20)
	{
		printf_t("compression: ITU G.723 ADPCM (Yamaha)\n");
	}
	else if (wav->format.compression_code == 49)
	{
		printf_t("compression: GSM 6.10\n");
	}
	else if (wav->format.compression_code == 64)
	{
		printf_t("compression: ITU G.721 ADPCM\n");
	}
	else if (wav->format.compression_code == 80)
	{
		printf_t("compression: MPEG\n");
	}
	else
	{
		printf_t("compression: Unknown\n");
	}
	printf_t("channels: %d\n", wav->format.channels);
	printf_t("samples: %d\n", wav->format.samples_per_sec);
	printf_t("avg_bytes_per_sec: %d\n", wav->format.avg_bytes_per_sec);
	printf_t("block_align: %d\n", wav->format.block_align);
	printf_t("bits_per_sample: %d\n", wav->format.bits_per_sample);

	printf_t("\nDATA Chunk\n");
	printf_t("id: %s\n", wav->data.id);
	printf_t("size: %d\n", wav->data.size);
	printf_t("data offset: %d\n", wav->data_offset);
}

void printf_t(const char *fmt, ...)
{
	static char buffer[2048] = { 0 };
	int len;
	if (wav_print_log)
	{
		va_list    args;
		va_start(args, fmt);
		len = vsnprintf(NULL, 0, fmt, args);
		va_start(args, fmt);
		vsnprintf(buffer, len + 1, fmt, args);
		printf(buffer);
		va_end(args);
	}
}