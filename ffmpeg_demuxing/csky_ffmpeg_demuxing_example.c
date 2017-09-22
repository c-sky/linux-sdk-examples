/*
 * demuxing with libavcodec API example
 *
 * Copyright (c) 2001 Fabrice Bellard
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdio.h>
#include <libavformat/avformat.h>
#include "csky_common.h"

#define CSKY_FFMPEG_DEMUX_MAJOR_NUM (1)
#define CSKY_FFMPEG_DEMUX_MINOR_NUM (0)

static const char *const prog_name = "csky_ffmpeg_demuxing_example";

static AVFormatContext *ifmt_ctx = NULL;
static AVFormatContext *ofmt_ctx_a = NULL, *ofmt_ctx_v = NULL;
static AVOutputFormat *ofmt_a = NULL, *ofmt_v = NULL;
static const char *src_filename = NULL;
static const char *video_dst_filename = NULL;
static const char *audio_dst_filename = NULL;
static int video_stream_idx = -1;
static int audio_stream_idx = -1;

static void prepare_common(void)
{
	/* register all formats */
	av_register_all();

	/* open input file, and allocate format context */
	if (avformat_open_input(&ifmt_ctx, src_filename, NULL, NULL) < 0) {
		fprintf(stderr, "Could not open source file %s\n",
			src_filename);
		exit(1);
	}

	if (avformat_find_stream_info(ifmt_ctx, 0) < 0) {
		fprintf(stderr, "Could not find stream information\n");
		exit(1);
	}

	/* open video output file, and allocate format context */
	avformat_alloc_output_context2(&ofmt_ctx_v, NULL, NULL,
				       video_dst_filename);
	if (!ofmt_ctx_v) {
		fprintf(stderr, "Could not create output context\n");
		exit(1);
	}
	ofmt_v = ofmt_ctx_v->oformat;

	/* open audio output file, and allocate format context */
	avformat_alloc_output_context2(&ofmt_ctx_a, NULL, NULL,
				       audio_dst_filename);
	if (!ofmt_ctx_a) {
		fprintf(stderr, "Could not create output context\n");
		exit(1);
	}
	ofmt_a = ofmt_ctx_a->oformat;
}

static void add_stream(void)
{
	int i;

	for (i = 0; i < ifmt_ctx->nb_streams; i++) {
		/* Create output AVStream according to input AVStream */
		AVStream *in_stream = ifmt_ctx->streams[i];
		AVStream *out_stream = NULL;

		if (in_stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			video_stream_idx = i;
			out_stream = avformat_new_stream(ofmt_ctx_v, NULL);
		} else if (in_stream->codecpar->codec_type ==
			   AVMEDIA_TYPE_AUDIO) {
			audio_stream_idx = i;
			out_stream = avformat_new_stream(ofmt_ctx_a, NULL);
		} else {
			break;
		}

		if (!out_stream) {
			fprintf(stderr, "Failed allocating output stream\n");
			exit(1);
		}

		if (avcodec_parameters_copy
		    (out_stream->codecpar, in_stream->codecpar) < 0) {
			fprintf(stderr,
				"Failed to copy codec parameter from input to output stream\n");
			exit(1);
		}
		out_stream->codecpar->codec_tag = 0;

	}
}

static void dump_format(void)
{
	printf("==============Input Video=============\n");
	av_dump_format(ifmt_ctx, 0, src_filename, 0);

	printf("\n==============Output Video============\n");
	av_dump_format(ofmt_ctx_v, 0, video_dst_filename, 1);

	printf("\n==============Output Audio============\n");
	av_dump_format(ofmt_ctx_a, 0, audio_dst_filename, 1);
}

static int write_output_file(void)
{
	int ret = 0;

	AVPacket pkt;

	/* Open output file */
	if (!(ofmt_v->flags & AVFMT_NOFILE)) {
		if (avio_open
		    (&ofmt_ctx_v->pb, video_dst_filename, AVIO_FLAG_WRITE)
		    < 0) {
			fprintf(stderr, "Could not open output file '%s'",
				video_dst_filename);
			exit(1);
		}
	}

	if (!(ofmt_a->flags & AVFMT_NOFILE)) {
		if (avio_open
		    (&ofmt_ctx_a->pb, audio_dst_filename, AVIO_FLAG_WRITE)
		    < 0) {
			fprintf(stderr, "Could not open output file '%s'",
				audio_dst_filename);
			exit(1);
		}
	}

	/* Write file header */
	if (avformat_write_header(ofmt_ctx_v, NULL) < 0) {
		fprintf(stderr,
			"Error occurred when opening video output file\n");
		exit(1);
	}

	if (avformat_write_header(ofmt_ctx_a, NULL) < 0) {
		fprintf(stderr,
			"Error occurred when opening audio output file\n");
		exit(1);
	}

	while (1) {
		AVFormatContext *ofmt_ctx;
		AVStream *in_stream, *out_stream;

		/* Get an AVPacket */
		if (av_read_frame(ifmt_ctx, &pkt) < 0)
			break;

		in_stream = ifmt_ctx->streams[pkt.stream_index];

		if (pkt.stream_index == video_stream_idx) {
			out_stream = ofmt_ctx_v->streams[0];
			ofmt_ctx = ofmt_ctx_v;
			LOG_D("Write Video Packet. size:%d\tpts:%" PRId64 "\n",
			      pkt.size, pkt.pts);
		} else if (pkt.stream_index == audio_stream_idx) {
			out_stream = ofmt_ctx_a->streams[0];
			ofmt_ctx = ofmt_ctx_a;
			LOG_D("Write Audio Packet. size:%d\tpts:%" PRId64 "\n",
			      pkt.size, pkt.pts);
		} else {
			continue;
		}

		/* Convert PTS/DTS */
		pkt.pts =
		    av_rescale_q_rnd(pkt.pts, in_stream->time_base,
				     out_stream->time_base,
				     (AV_ROUND_NEAR_INF |
				      AV_ROUND_PASS_MINMAX));
		pkt.dts =
		    av_rescale_q_rnd(pkt.dts, in_stream->time_base,
				     out_stream->time_base,
				     (AV_ROUND_NEAR_INF |
				      AV_ROUND_PASS_MINMAX));
		pkt.duration =
		    av_rescale_q(pkt.duration, in_stream->time_base,
				 out_stream->time_base);
		pkt.pos = -1;
		pkt.stream_index = 0;

		/* Write a packet to output media file */
		if ((ret = av_interleaved_write_frame(ofmt_ctx, &pkt)) < 0) {
			fprintf(stderr, "Error muxing packet\n");
			break;
		}

		av_packet_unref(&pkt);
	}

	/* Write file trailer */
	av_write_trailer(ofmt_ctx_a);
	av_write_trailer(ofmt_ctx_v);

	return ret;
}

static void cleanup(void)
{
	/* close input */
	avformat_close_input(&ifmt_ctx);

	/* close output */
	if (ofmt_ctx_a && !(ofmt_a->flags & AVFMT_NOFILE)) {
		if (avio_close(ofmt_ctx_a->pb) < 0) {
			fprintf(stderr, "Error close audio avio\n");
			exit(1);
		}
	}

	if (ofmt_ctx_v && !(ofmt_v->flags & AVFMT_NOFILE)) {
		if (avio_close(ofmt_ctx_v->pb) < 0) {
			fprintf(stderr, "Error close video avio\n");
			exit(1);
		}
	}

	avformat_free_context(ofmt_ctx_a);
	avformat_free_context(ofmt_ctx_v);
}

int main(int argc, char *argv[])
{
	int ret;

	if (argc < 4) {
		fprintf(stderr, "%s %d.%d\n", prog_name,
			CSKY_FFMPEG_DEMUX_MAJOR_NUM,
			CSKY_FFMPEG_DEMUX_MINOR_NUM);
		fprintf(stderr,
			"usage: %s input_file video_output_file audio_output_file\n"
			"API example program to show how to read frames from an input file.\n"
			"This program reads frames from a file, demux them and output\n"
			"the uncoded video frame to video file named video_output_file,\n"
			"the uncoded audio frame to audio file named video_output_file.\n"
			"\n", argv[0]);
		exit(1);
	}

	src_filename = argv[1];
	video_dst_filename = argv[2];
	audio_dst_filename = argv[3];

	prepare_common();
	add_stream();
	dump_format();

	ret = write_output_file();
	if (ret < 0)
		return -1;

	cleanup();

	return 0;
}
