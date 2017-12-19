/*
 * main module
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>

#define __STDC_CONSTANT_MACROS

#include "csky_common.h"
#include "player.h"
#include "packet_queue.h"
#include "audio.h"
#include "video.h"

static pthread_t demuxing_thread_id;
static pthread_t audio_thread_id;
static struct instance inst;

int main(int argc, char *argv[])
{
	int ret;

	PlayerState *ps = NULL;

	if (parse_args(&inst, argc, argv)) {
		print_usage(argv[0]);
		exit(-1);
	}

	ps = (PlayerState *) av_malloc(sizeof(PlayerState));
	if (ps == NULL) {
		LOG_E("malloc ps error");
	}

	player_state_init(ps);

	ps->filename = inst.in.name;

	if (prepare_common(ps) != 0) {
		LOG_E("prepare common error");
		exit(-1);
	}

	av_dump_format(ps->pFormatCtx, 0, ps->filename, 0);

	if (ps->audioStream != -1) {
		packet_queue_init(&ps->audioPacketQueue);
	}

	if (ps->videoStream != -1) {
		packet_queue_init(&ps->videoPacketQueue);
	}

	pthread_create(&demuxing_thread_id, NULL, demuxing_thread, ps);

	if (ps->audioStream != -1) {
		pthread_create(&audio_thread_id, NULL, play_audio, ps);
	}

	if (ps->videoStream != -1) {
		if (prepare_video(ps) == 0) {
		/* prepare video ok , ready to open fb...*/
			inst.ps = ps;

			if (video_init(&inst) != 0) {
				LOG_E("video init failed");
				exit(-1);
			}
		}
		else {
		/* prepare video failed*/
			ps->videoStream = -1;
		}
	}

	ret = pthread_join(demuxing_thread_id, NULL);
	if (ret < 0) {
		LOG_E("pthread_join return %d", ret);
		exit(-1);
	}

	if (ps->audioStream != -1) {
		ret = pthread_join(audio_thread_id, NULL);
		if (ret < 0) {
			LOG_E("pthread_join return %d", ret);
			exit(-1);
		}
	}

	if (ps->videoStream != -1)
		video_deinit(&inst);

	av_freep(ps);

	return 0;
}

void *demuxing_thread(void *arg)
{
	PlayerState *ps = (PlayerState *) arg;
	AVPacket *packet = av_packet_alloc();

	while (1) {
		if (ps->audioPacketQueue.nbPkts >= MAX_AUDIO_QUEUE_SIZE ||
		    ps->videoPacketQueue.nbPkts >= MAX_VIDEO_QUEUE_SIZE) {
			//LOG_D("too much data, delay...");
			usleep(100 * 1000);
			continue;
		}

		if (av_read_frame(ps->pFormatCtx, packet) < 0) {
			if ((ps->pFormatCtx->pb->error) == 0) {
				LOG_I("finish reading.delay...");
				sleep(4);
				LOG_I("Play finished!");
				ps->quit = 1;
				break;
			} else {
				LOG_E("error, delay...");
				continue;
			}
		}

		if (packet->stream_index == ps->videoStream)
			packet_queue_put(&ps->videoPacketQueue, packet);
		else if (packet->stream_index == ps->audioStream)
			packet_queue_put(&ps->audioPacketQueue, packet);

		av_packet_unref(packet);
	}

	av_packet_free(&packet);
	pthread_exit(NULL);
}

int prepare_common(PlayerState * ps)
{
	avcodec_register_all();

	av_register_all();

	//open file
	if (avformat_open_input(&ps->pFormatCtx, ps->filename, NULL, NULL) != 0) {
		LOG_E("open input file %s error", ps->filename);
		return -1;
	}

	if (avformat_find_stream_info(ps->pFormatCtx, NULL) < 0) {
		LOG_E("Couldn't find stream info");
		return -1;
	}

	ps->videoStream = -1;
	ps->audioStream = -1;
	if (find_stream_index
	    (ps->pFormatCtx, &ps->videoStream, &ps->audioStream) == -2) {
		LOG_E("Couldn't find any stream index");
		return -1;
	}
	return 0;
}

void player_state_init(PlayerState * ps)
{
	ps->pFormatCtx = NULL;
	ps->quit = 0;

	ps->audioStream = -1;
	ps->paudioStream = NULL;
	ps->paudioCodecCtx = NULL;
	ps->paudioCodec = NULL;
	ps->audioBufSize = 0;
	ps->audioBufIndex = 0;

	ps->videoStream = -1;
	ps->pvideoStream = NULL;
	ps->pvideoCodecCtx = NULL;
	ps->pvideoCodec = NULL;
	ps->video_buf = NULL;
	ps->videoBufSize = 0;
	ps->videoBufIndex = 0;
	ps->pswsCtx = NULL;

	ps->pixfmt = AV_PIX_FMT_YUV420P;

	ps->audio_clock = 0.0;
	ps->video_clock = 0.0;
	ps->pre_frame_pts = 0.0;
	ps->pre_cur_frame_delay = 40e-3;
	ps->cur_frame_pts = 0.0;
	ps->delay = 0;
}

int find_stream_index(AVFormatContext * pFormatCtx, int *videoStream,
		      int *audioStream)
{
	assert(videoStream != NULL || audioStream != NULL);
	if (videoStream == NULL && audioStream == NULL) {
		return -2;
	}

	unsigned int i = 0;
	for (i = 0; i < pFormatCtx->nb_streams; i++) {
		if (pFormatCtx->streams[i]->codecpar->codec_type ==
		    AVMEDIA_TYPE_VIDEO) {
			*videoStream = i;
		}
		if (pFormatCtx->streams[i]->codecpar->codec_type ==
		    AVMEDIA_TYPE_AUDIO) {
			*audioStream = i;
		}

	}
	if ((*videoStream) == -1 || (*audioStream) == -1)
		return -1;
	else
		return 0;
}
