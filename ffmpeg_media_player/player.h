/*
 * player.h
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

#ifndef PLAYER_H_
#define PLAYER_H_

#include <unistd.h>

#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
#include "libavutil/time.h"

#include "packet_queue.h"

#define MAX_AUDIO_FRAME_SIZE  192000	// 1 second of 48khz 32bit audio

#define ERR_STREAM            stderr
#define OUT_SAMPLE_RATE       44100
#define OUT_STREAM            stdout
#define WINDOW_W              640
#define WINDOW_H              320

#define MAX_AUDIO_QUEUE_SIZE  32	//128
#define MAX_VIDEO_QUEUE_SIZE  16	//64

typedef struct PlayerState {
	AVFormatContext *pFormatCtx;
	char *filename;
	int quit;

	//audio
	int audioStream;
	AVStream *paudioStream;
	AVCodecContext *paudioCodecCtx;
	AVCodec *paudioCodec;
	PacketQueue audioPacketQueue;
	uint8_t audio_buf[MAX_AUDIO_FRAME_SIZE * 4];
	unsigned int audioBufSize;
	unsigned int audioBufIndex;

	//video
	int videoStream;
	AVStream *pvideoStream;
	AVCodecContext *pvideoCodecCtx;
	AVCodec *pvideoCodec;
	PacketQueue videoPacketQueue;
	uint8_t *video_buf;
	unsigned int videoBufSize;
	unsigned int videoBufIndex;

	struct SwsContext *pswsCtx;

	int pixel_w;
	int pixel_h;
	int window_w;
	int window_h;

	enum AVPixelFormat pixfmt;
	AVFrame outFrame;

	//synchronize
	double audio_clock;
	double video_clock;
	double pre_frame_pts;
	double cur_frame_pts;
	double pre_cur_frame_delay;
	uint32_t delay;
	double frame_timer;
} PlayerState;

void player_state_init(PlayerState * ps);

void *demuxing_thread(void *arg);

int prepare_common(PlayerState * ps);

int find_stream_index(AVFormatContext * pFormatCtx, int *videoStream,
		      int *audioStream);

#endif
