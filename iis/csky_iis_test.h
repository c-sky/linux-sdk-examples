/*
 * Network test include file
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

/****************************************************************************
 * Included Files
 ****************************************************************************/
#include <alsa/asoundlib.h>
#include <stdio.h>
#include <sndfile.h>

/**************************************************************************
 * Macro Defination
 * **********************************************************************/

#define CSKY_IIS_MAJOR_NUM  1
#define CSKY_IIS_MINOR_NUM  0

/**************************************************************************
 * Structure Defination
 * **********************************************************************/
struct pcm_params {
	long long frames;
	int channels;
	int samplerate;
	snd_pcm_format_t snd_format;
	int bytespersamp;
};

typedef struct pcm_params pcm_params_t;

/****************************************************************************
 * Public Functions
 ****************************************************************************/
SNDFILE *get_file_info(const char *file, pcm_params_t * wav_param);
void play_wav_file(const char *file, pcm_params_t * wav_param, int volume);
void tune_volume(int volume);
