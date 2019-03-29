#include "wav.h"
#include <windows.h>

#define WAV_LINEFEED        "\x0a"

static int   wav_show_help = 0;
static int   wav_show_version = 0;
static char  *wav_input_file = NULL;
static char  *wav_terminalid = NULL;
static char  *wav_cseqid = NULL;
static int   wav_print = 0;
static char  *wav_outdir = NULL;

void show_version_info(void)
{
	if (wav_show_help){
		printf(
			"Usage: kuangshiface [-?hv] [-i input] [-t terminalid] [-c cseq] "
			"[-o outdir]" WAV_LINEFEED
			WAV_LINEFEED
			"Options:" WAV_LINEFEED
			"  -?,-h              : this help" WAV_LINEFEED
			"  -v                 : show version and exit" WAV_LINEFEED
			"  -i input           : input wav file" WAV_LINEFEED
			"  -t terminalid      : terminal id " WAV_LINEFEED
			"  -c cseq            : cseq id " WAV_LINEFEED
			"  -p                 : print out log to console " WAV_LINEFEED
			"  -o outdir          : output dir" WAV_LINEFEED WAV_LINEFEED
			);
	}
}

int wav_get_options(int argc, char *const *argv)
{
	char  *p;
	int   i;

	for (i = 1; i < argc; i++) {
		p = argv[i];

		if (*p++ != '-') {
			printf("invalid option: \"%s\"\n", argv[i]);
			return -1;
		}

		while (*p) {

			switch (*p++) {

			case '?':
			case 'h':
				wav_show_version = 1;
				wav_show_help = 1;
				break;

			case 'p':
				wav_print = 1;
				break;

			case 'v':
				wav_show_version = 1;
				break;

			case 'i':
				if (*p) {
					wav_input_file = p;
					goto next;
				}

				if (argv[++i]) {
					wav_input_file = argv[i];
					goto next;
				}

				printf("option \"-i\" requires input file\n");
				return -1;

			case 't':
				if (*p) {
					wav_terminalid = p;
					goto next;
				}

				if (argv[++i]) {
					wav_terminalid = argv[i];
					goto next;
				}

				printf("option \"-t\" requires terminalid\n");
				return -1;

			case 'c':
				if (*p) {
					wav_cseqid = p;
					goto next;
				}

				if (argv[++i]) {
					wav_cseqid = argv[i];
					goto next;
				}

				printf("option \"-c\" requires cseq id\n");
				return -1;

			case 'o':
				if (*p) {
					wav_outdir = p;
					goto next;
				}

				if (argv[++i]) {
					wav_outdir = argv[i];
					goto next;
				}

				printf("option \"-t\" requires output dir\n");
				return -1;

			default:
				printf("invalid option: \"%c\"\n", *(p - 1));
				return -1;
			}
		}

	next:
		continue;
	}

	return 0;
}

int main(int argc, char **argv)
{
	if (wav_get_options(argc, argv) != 0){
		printf("wav_get_options() failed.\nend...\n");
		return -1;
	}

	if (wav_show_version){
		show_version_info();
		return 0;
	}

	if (wav_input_file == NULL || wav_terminalid == NULL || wav_cseqid == NULL || wav_outdir == NULL)
	{
		printf("please setup -i(input) or -t(terminalid) or -c(cseq) or -o(outdir).\n");
		return -1;
	}

	wav_print_log = wav_print;

	//wav_t *wav = (wav_t *)malloc(sizeof(wav_t));
	wav_t *wav = NULL;
	//wav_open(wav_input_file, wav);
	wav = wav_open(wav_input_file);
	if (wav == NULL)
	{
		printf_t("open wav file failed.\n");
		return -1;
	}

	wav_dump(wav);

	FILE *fd1 = NULL;
	FILE *fd2 = NULL;

	if (wav->format.channels == 1)
	{
		wav_close(&wav);
		char newfile[256] = { 0 };
		sprintf(newfile, "%s/%s_%s_1.wav", wav_outdir, wav_terminalid, wav_cseqid);
		CopyFile(wav_input_file, newfile, true);
		return 0;
	}
	else if (wav->format.channels == 2)
	{
		wav_head whead;
		memset(&whead, 0, sizeof(whead));

		//handle RIFF WAVE chunk
		memcpy(whead.riff_id, wav->riff.id, 4);
		whead.riff_size = wav->data.size / 2 + sizeof(whead) - 8;
		memcpy(whead.riff_fmt, wav->riff.type, 4);
		//FMT
		//memcpy(whead.fmt_id, "fmt ", 4);
		memcpy(whead.fmt_id, wav->format.id, 4);
		whead.fmt_size = wav->format.size;
		whead.fmt_audiofmt = wav->format.compression_code;
		whead.fmt_channel = 1;
		whead.fmt_samplerate = wav->format.samples_per_sec;
		whead.fmt_bitspersample = wav->format.bits_per_sample;
		whead.fmt_blockalign = (whead.fmt_bitspersample * whead.fmt_channel) >> 3;
		whead.fmt_byterate = whead.fmt_blockalign * whead.fmt_samplerate;
		//DATA HEADER
		memcpy(whead.data_id, wav->data.id, 4);
		whead.data_size = wav->data.size / 2;

		char filename1[256] = { 0 };
		char filename2[256] = { 0 };

		sprintf(filename1, "%s\\%s_%s_1.wav", wav_outdir, wav_terminalid, wav_cseqid);
		sprintf(filename2, "%s\\%s_%s_2.wav", wav_outdir, wav_terminalid, wav_cseqid);

		fd1 = fopen(filename1, "wb");
		if (fd1 == NULL)
		{
			printf_t("fopen faild(%s).\n", filename1);
			goto done;
		}

		fd2 = fopen(filename2, "wb");
		if (fd2 == NULL)
		{
			printf_t("fopen faild(%s).\n", filename2);
			goto done;
		}

		fwrite(&whead, 1, sizeof(whead), fd1);
		fwrite(&whead, 1, sizeof(whead), fd2);

		int read_size;
		char buffer[12] = { 0 };
		wav_rewind(wav);
		int count = 0;
		int persample = wav->format.bits_per_sample / 8;
		while (!wav_over(wav))
		{
			read_size = wav_read_data(wav, buffer, persample * 2);
			if (read_size <= 0 || read_size != 4)
			{
				printf_t("read_size(%d) != 4, count = %d\n", read_size, count);
				continue;
			}
			count += read_size;
			fwrite(buffer, 1, persample, fd1);
			fwrite(buffer + persample, 1, persample, fd2);
		}
	}
	else
	{
		printf_t("channels(%d) of wav is error, bits_per_sample(%d) of wav is error.\n", wav->format.channels, wav->format.bits_per_sample);
		return -1;
	}

done:
	if (fd1 != NULL) fclose(fd1);
	if (fd2 != NULL) fclose(fd2);
	wav_close(&wav);

	printf_t("wav closed.\n");

	return -1;
}