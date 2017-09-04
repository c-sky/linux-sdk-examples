/*
 * IIS test module
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "csky_iis_test.h"

static const char *prog_name = "csky_iis_example";

static void help_info(void);

/**
  \brief       Get the information of a wave file
  \param[in]   file  Location of wave file
  \param[out]  wav_param  a struct to store wavefile information
  \return wave file handle; NULL invalid wave file
  */
SNDFILE *get_file_info(const char *file, pcm_params_t * wav_param)
{

	SF_INFO sfinfo;
	int format;
	SNDFILE *infile;

	infile = sf_open(file, SFM_READ, &sfinfo);
	if (infile != NULL) {
		if (sfinfo.format / 0x10000 == 1) {
			printf("Wave File informations:\n");
			printf("Frames: %ld\n", sfinfo.frames);
			printf("Channels: %d\n", sfinfo.channels);
			printf("Sample rate: %d\n", sfinfo.samplerate);

		} else {
			printf("It is not a wave file\n");
			return NULL;
		}

		/* Set the information got by libsndfile into wav_param */
		wav_param->frames = sfinfo.frames;
		wav_param->channels = sfinfo.channels;
		wav_param->samplerate = sfinfo.samplerate;

		/* Get subtype value */
		format = sfinfo.format % 0x10000;

		/* Set PCM format */
		if (format == SF_FORMAT_PCM_S8) {
			wav_param->snd_format = SND_PCM_FORMAT_S8;
			wav_param->bytespersamp = 1;
			printf("Bits: Signed 8 bit\n");

		} else if (format == SF_FORMAT_PCM_16) {
			wav_param->snd_format = SND_PCM_FORMAT_S16_LE;
			wav_param->bytespersamp = 2;
			printf("Bits: Signed 16 bit\n");

		} else if (format == SF_FORMAT_PCM_24) {
			wav_param->snd_format = SND_PCM_FORMAT_S24_LE;
			wav_param->bytespersamp = 3;
			printf("Bits: Signed 24 bit\n");

		} else if (format == SF_FORMAT_PCM_32) {
			wav_param->snd_format = SND_PCM_FORMAT_S32_LE;
			wav_param->bytespersamp = 4;
			printf("Bits: Signed 32 bit\n");

		} else if (format == SF_FORMAT_PCM_U8) {
			wav_param->snd_format = SND_PCM_FORMAT_U8;
			wav_param->bytespersamp = 1;
			printf("Bits: Unsigned 8 bit\n");

		} else if (format == SF_FORMAT_FLOAT) {
			wav_param->snd_format = SND_PCM_FORMAT_FLOAT_LE;
			wav_param->bytespersamp = 4;
			printf("Bits: Float 32 bit\n");

		} else if (format == SF_FORMAT_DOUBLE) {
			wav_param->snd_format = SND_PCM_FORMAT_FLOAT64_LE;
			wav_param->bytespersamp = 8;
			printf("Bits: Float 64 bit\n");
		}

	} else {
		printf("Cannot open this wav file %s\n", file);
		return NULL;
	}
	return infile;
}

/**
  \brief       Play a wave file using default pcm device
  \param[in]   file        Location of wave file
  \param[in]   wav_param   A struct to store wave file informations
  \param[in]   volume      Volume value
  */
void play_wav_file(const char *file, pcm_params_t * wav_param, int volume)
{

	int dir, size;
	int ret, rc;
	snd_pcm_t *handle;
	snd_pcm_uframes_t frames;
	long long duration;

	char *buffer;

	int wavfile;

	int val, loops;

	snd_pcm_hw_params_t *params;

	/* Open PCM Device */
	if (snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0) {
		printf("Unable to open pcm device!\n");
		return;
	}

	/* Allocate a hardware parameters object. */
	snd_pcm_hw_params_alloca(&params);

	/* Fill it in with default values. */
	snd_pcm_hw_params_any(handle, params);

	/* Set the desired hardware parameters. */
	/* Interleaved mode */
	snd_pcm_hw_params_set_access(handle, params,
				     SND_PCM_ACCESS_RW_INTERLEAVED);

	snd_pcm_hw_params_set_format(handle, params, wav_param->snd_format);

	/* Set channels */
	snd_pcm_hw_params_set_channels(handle, params, wav_param->channels);

	/* Set samplerate */
	val = wav_param->samplerate;
	snd_pcm_hw_params_set_rate_near(handle, params, &val, &dir);

	/* Set period size to 32 frames. */
	frames = 32;
	snd_pcm_hw_params_set_period_size_near(handle, params, &frames, &dir);

	/* Write the parameters to the driver */
	if (snd_pcm_hw_params(handle, params) < 0) {
		printf("Unable to set hardware parameters.\n");
		return;
	}

	/* Use a buffer large enough to hold one period */
	snd_pcm_hw_params_get_period_size(params, &frames, &dir);
	size = frames * wav_param->channels * wav_param->bytespersamp;
	buffer = malloc(size);

	duration =
	    (long long)wav_param->frames * 1000000 / wav_param->samplerate;
	snd_pcm_hw_params_get_period_time(params, &val, &dir);

	/* If volume needs to be tuned */
	if (volume > 0)
		tune_volume(volume);

	loops = duration / val;

	/* Open wave file */
	wavfile = open(file, O_RDONLY);

	/* Play wave file */
	while (loops > 0) {
		loops--;
		/* Read data from wave file */
		ret = read(wavfile, buffer, size);
		if (ret == 0) {
			printf("end of file on input\n");
			break;
		} else if (ret != size) {
			printf("short read: read %d bytes\n", ret);
		}

		/* Write data to pcm device */
		ret = snd_pcm_writei(handle, buffer, frames);
		if (ret == -EPIPE) {
			/* EPIPE means underrun */
			printf("underrun occurred\n");
			snd_pcm_prepare(handle);
		} else if (ret < 0) {
			printf("error from writei: %s\n", snd_strerror(ret));
		} else if (ret != (int)frames) {
			printf("short write, write %d frames\n", ret);
		}
	}
	printf("End read.\n");

	close(wavfile);
	snd_pcm_drain(handle);
	snd_pcm_close(handle);
	free(buffer);
}

/**
  \brief       Tune mixer volume
  \param[in]   volume   Volume value range(0-100]
  */
void tune_volume(int volume)
{
	long min, max;
	snd_mixer_t *handle;
	snd_mixer_selem_id_t *sid;
	const char *card = "default";
	const char *selem_name = "Master";

	/*  Open alsa mixer */
	snd_mixer_open(&handle, 0);

	/*  Attach with default sound card */
	snd_mixer_attach(handle, card);

	/* Register mixer simple element */
	snd_mixer_selem_register(handle, NULL, NULL);

	/* Load mixer */
	snd_mixer_load(handle);

	snd_mixer_selem_id_alloca(&sid);
	snd_mixer_selem_id_set_index(sid, 0);
	snd_mixer_selem_id_set_name(sid, selem_name);
	snd_mixer_elem_t *elem = snd_mixer_find_selem(handle, sid);

	/* Find default volume range */
	snd_mixer_selem_get_playback_volume_range(elem, &min, &max);

	/*Set Volume */
	snd_mixer_selem_set_playback_volume_all(elem, volume * max / 100);

	snd_mixer_close(handle);
}

int main(int argc, char **argv)
{

	int volume = 0;
	SNDFILE *infile = NULL;
	if (argc < 3) {
		help_info();
		return -1;
	}

	pcm_params_t wav_param;

	/* Only show wave file info */
	if (strcmp(argv[1], "show") == 0)
		get_file_info(argv[2], &wav_param);

	/* Play wave file */
	else if (strcmp(argv[1], "play") == 0) {
		/* Volume value given */
		if (argc >= 4) {
			volume = atoi(argv[3]);
			if (volume < 0 || volume > 100) {
				printf("Volume is not a valid value.\n");
				help_info();
				return -1;
			}
		}
		infile = get_file_info(argv[2], &wav_param);
		if (infile)
			play_wav_file(argv[2], &wav_param, volume);
	} else {
		help_info();
		return -1;
	}

	return 0;
}

static void help_info(void)
{
	printf("%s %d.%d\n", prog_name, CSKY_IIS_MAJOR_NUM, CSKY_IIS_MINOR_NUM);
	printf("Usage: %s show/play file [volume] \n", prog_name);
	printf("show: it will show the wave file information\n");
	printf("play: it will play the wave file as a media player\n");
	printf("file: {string} A media file in wave format.\n");
	printf("volume: {integer} volume value, range(0-100]\n");
}
