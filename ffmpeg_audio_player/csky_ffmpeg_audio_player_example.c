/*
 * A simple audio player example
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
#include <stdlib.h>
#include <string.h>

#include <libavutil/frame.h>
#include <libavutil/mem.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include <alsa/asoundlib.h>

//#define CONFIG_LOG_LEVEL  4
#include "csky_common.h"

#define CSKY_AUDIO_PLAYER_MAJOR_NUM (1)
#define CSKY_AUDIO_PLAYER_MINOR_NUM (0)

static const char *const prog_name = "csky_ffmpeg_audio_player_example";
static snd_pcm_t *pcm = NULL;
static SwrContext *swr_ctx = NULL;
static AVFormatContext *ifmt_ctx = NULL;
static AVStream *audio_stream = NULL;
static int audio_stream_idx = -1;
static const char *src_filename = NULL;
static int pcm_writei_failed_cnt = 0;
static int pcm_writei_total_cnt = 0;

#define MAX_AUDIO_FRAME_SIZE  192000	// 1 second of 48khz 32bit audio
static uint8_t audio_buf[MAX_AUDIO_FRAME_SIZE * 4];

static int open_codec_context(int *stream_idx,
			      AVCodecContext ** dec_ctx,
			      AVFormatContext * fmt_ctx, enum AVMediaType type)
{
	int ret;

	ret = av_find_best_stream(fmt_ctx, type, -1, -1, NULL, 0);
	if (ret < 0) {
		LOG_E("Could not find %s stream in input file",
		      av_get_media_type_string(type));
		return ret;
	} else {
		int stream_index;
		AVStream *st;
		AVCodec *dec = NULL;

		stream_index = ret;
		st = fmt_ctx->streams[stream_index];

		/* find decoder for the stream */
		dec = avcodec_find_decoder(st->codecpar->codec_id);
		if (!dec) {
			LOG_E("Failed to find %s codec",
			      av_get_media_type_string(type));
			return AVERROR(EINVAL);
		}

		/* Allocate a codec context for the decoder */
		*dec_ctx = avcodec_alloc_context3(dec);
		if (!*dec_ctx) {
			LOG_E("Failed to allocate the %s codec context",
			      av_get_media_type_string(type));
			return AVERROR(ENOMEM);
		}

		/* Copy codec parameters from input stream to output codec context */
		if ((ret =
		     avcodec_parameters_to_context(*dec_ctx,
						   st->codecpar)) < 0) {
			LOG_E
			    ("Failed to copy %s codec parameters to decoder context",
			     av_get_media_type_string(type));
			return ret;
		}

		if ((ret = avcodec_open2(*dec_ctx, dec, NULL)) < 0) {
			LOG_E("Failed to open %s codec",
			      av_get_media_type_string(type));
			return ret;
		}
		*stream_idx = stream_index;
	}

	return 0;
}

static void ffmpeg_fmt_to_alsa_fmt(AVCodecContext * c, snd_pcm_t * pcm,
				   snd_pcm_hw_params_t * params)
{
	int ret = 0;

	switch (c->sample_fmt) {
	case AV_SAMPLE_FMT_U8:	/* unsigned 8 bits */
		LOG_D("AV_SAMPLE_FMT_U8");
		ret =
		    snd_pcm_hw_params_set_format(pcm, params,
						 SND_PCM_FORMAT_S8);
		break;

	case AV_SAMPLE_FMT_S16:	/* signed 16 bits */
		LOG_D("AV_SAMPLE_FMT_S16");
		/* SND_PCM_FORMAT_S16 is ok, not care SND_PCM_FORMAT_S16_LE or SND_PCM_FORMAT_S16_BE */
		ret =
		    snd_pcm_hw_params_set_format(pcm, params,
						 SND_PCM_FORMAT_S16);
		break;

	case AV_SAMPLE_FMT_S16P:	/* signed 16 bits, planar */
		LOG_D("AV_SAMPLE_FMT_S16P");
		/* SND_PCM_FORMAT_S16 is ok, not care SND_PCM_FORMAT_S16_LE or SND_PCM_FORMAT_S16_BE */
		ret =
		    snd_pcm_hw_params_set_format(pcm, params,
						 SND_PCM_FORMAT_S16);
		break;

	case AV_SAMPLE_FMT_FLT:	/* float */
		LOG_D("AV_SAMPLE_FMT_FLT");
		ret =
		    snd_pcm_hw_params_set_format(pcm, params,
						 SND_PCM_FORMAT_FLOAT);
		break;

	case AV_SAMPLE_FMT_FLTP:	/* float, planar */
		LOG_D("AV_SAMPLE_FMT_FLTP");
		ret =
		    snd_pcm_hw_params_set_format(pcm, params,
						 SND_PCM_FORMAT_FLOAT);
		break;
	default:
		LOG_E("Format %d not support!", c->sample_fmt);
		exit(1);
	}

	if (ret < 0) {
		LOG_E("snd_pcm_hw_params_set_format failed, ret=%d", ret);
		exit(1);
	}
}

static void prepare_common(AVCodecContext ** c)
{
	/* register all the codecs */
	avcodec_register_all();

	/* register all formats */
	av_register_all();

	/* open input file, and allocate format context */
	if (avformat_open_input(&ifmt_ctx, src_filename, NULL, NULL) < 0) {
		LOG_E("Could not open source file %s!", src_filename);
		exit(1);
	}

	if (avformat_find_stream_info(ifmt_ctx, 0) < 0) {
		LOG_E("Could not find stream information!");
		exit(1);
	}

	if (open_codec_context
	    (&audio_stream_idx, c, ifmt_ctx, AVMEDIA_TYPE_AUDIO) < 0) {
		LOG_E("open_codec_context failed!");
		exit(1);
	} else {
		audio_stream = ifmt_ctx->streams[audio_stream_idx];
	}

}

static void init_pcm(AVCodecContext * c)
{
	int ret;
	unsigned int val;
	snd_pcm_hw_params_t *params = NULL;

	ret = snd_pcm_open(&pcm, "default", SND_PCM_STREAM_PLAYBACK, 0);
	if (ret < 0) {
		LOG_E("snd_pcm_open failed, ret=%d", ret);
		exit(1);
	}

	snd_pcm_hw_params_alloca(&params);

	ret = snd_pcm_hw_params_any(pcm, params);
	if (ret < 0) {
		LOG_E("snd_pcm_hw_params_any failed, ret=%d", ret);
		exit(1);
	}

	ret = snd_pcm_hw_params_set_access(pcm, params,
					   SND_PCM_ACCESS_RW_INTERLEAVED);
	if (ret < 0) {
		LOG_E("snd_pcm_hw_params_set_access failed, ret=%d", ret);
		exit(1);
	}

	ffmpeg_fmt_to_alsa_fmt(c, pcm, params);

	ret = snd_pcm_hw_params_set_channels(pcm, params, c->channels);
	if (ret < 0) {
		LOG_E("snd_pcm_hw_params_set_channels failed, ret=%d", ret);
		exit(1);
	}
	LOG_D("channels=%d", c->channels);

	val = c->sample_rate;
	LOG_D("sample_rate=%u", val);
	ret = snd_pcm_hw_params_set_rate_near(pcm, params, &val, 0);
	if (ret < 0) {
		LOG_E("snd_pcm_hw_params_set_rate_near failed, ret=%d", ret);
		exit(1);
	}

	ret = snd_pcm_hw_params(pcm, params);
	if (ret < 0) {
		LOG_E("snd_pcm_hw_params failed, ret=%d", ret);
		exit(1);
	}

	snd_pcm_uframes_t period_size;
	snd_pcm_uframes_t buffer_size;

	ret = snd_pcm_get_params(pcm, &buffer_size, &period_size);
	if (ret < 0) {
		LOG_E("snd_pcm_get_params failed, ret=%d", ret);
		exit(1);
	}
	LOG_D("snd_pcm_get_params, buffer_size=%ld, period_size=%ld",
	      buffer_size, period_size);

	snd_pcm_uframes_t frm_val;

	/* get the current swparams */
	snd_pcm_sw_params_t *swparams;
	snd_pcm_sw_params_alloca(&swparams);

	ret = snd_pcm_sw_params_current(pcm, swparams);
	if (ret < 0) {
		LOG_E("Unable to determine current swparams: %s",
		      snd_strerror(ret));
		exit(1);
	}

	ret = snd_pcm_sw_params_get_avail_min(swparams, &frm_val);
	if (ret < 0) {
		LOG_E("snd_pcm_sw_params_get_avail_min failed, ret=%d", ret);
		exit(1);
	}
	LOG_D("snd_pcm_sw_params_get_avail_min, val=%ld", frm_val);

	ret =
	    snd_pcm_sw_params_get_start_threshold((const snd_pcm_sw_params_t *)
						  swparams, &frm_val);
	if (ret < 0) {
		LOG_E("snd_pcm_sw_params_get_start_threshold failed, ret=%d",
		      ret);
		exit(1);
	}
	LOG_D("snd_pcm_sw_params_get_start_threshold, val=%ld", frm_val);

	frm_val = (buffer_size / period_size) * period_size;
	ret = snd_pcm_sw_params_set_start_threshold(pcm, swparams, frm_val);
	if (ret < 0) {
		LOG_E("snd_pcm_sw_params_set_start_threshold failed, ret=%d",
		      ret);
		exit(1);
	}
	LOG_D("snd_pcm_sw_params_set_start_threshold, val=%ld", frm_val);

	/* write the parameters to the playback device */
	ret = snd_pcm_sw_params(pcm, swparams);
	if (ret < 0) {
		LOG_E("Unable to set sw params for %s", snd_strerror(ret));
		exit(1);
	}
}

static void init_swr_ctx(AVFrame * frame)
{
	int ret;
	enum AVSampleFormat out_sample_fmt = frame->format;

	if (swr_ctx)
		return;

	LOG_D
	    ("frame: channnels=%d, default_layout=%" PRId64
	     ", format=%d, sample_rate=%d", frame->channels,
	     av_get_default_channel_layout(frame->channels), frame->format,
	     frame->sample_rate);

	if (frame->format == AV_SAMPLE_FMT_FLTP) {
		out_sample_fmt = AV_SAMPLE_FMT_FLT;
	} else if (frame->format == AV_SAMPLE_FMT_S16P) {
		out_sample_fmt = AV_SAMPLE_FMT_S16;
	}

	swr_ctx =
	    swr_alloc_set_opts(NULL,
			       av_get_default_channel_layout
			       (frame->channels),
			       out_sample_fmt,
			       frame->sample_rate,
			       av_get_default_channel_layout
			       (frame->channels), frame->format,
			       frame->sample_rate, 0, NULL);

	if (swr_ctx == NULL) {
		LOG_E("swr_ctx == NULL!");
		exit(1);
	}

	ret = swr_init(swr_ctx);
	if (ret < 0) {
		LOG_E("swr_init failed, ret=%d", ret);
		exit(1);
	}

}

static void decode(AVCodecContext * dec_ctx, AVPacket * pkt, AVFrame * frame)
{
	int ret;
	uint8_t *out = audio_buf;

	/* send the packet with the compressed data to the decoder */
	ret = avcodec_send_packet(dec_ctx, pkt);
	if (ret < 0) {
		LOG_E("Error submitting the packet to the decoder");
		exit(1);
	}

	/* read all the output frames (in general there may be any number of them */
	while (1) {
		ret = avcodec_receive_frame(dec_ctx, frame);

		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
			return;
		} else if (ret < 0) {
			LOG_E("Error during decoding");
			exit(1);
		}

		init_swr_ctx(frame);

		swr_convert(swr_ctx, &out, frame->sample_rate,
			    (const uint8_t **)frame->extended_data,
			    frame->nb_samples);

		pcm_writei_total_cnt++;

		if ((ret = snd_pcm_writei(pcm, out, frame->nb_samples)) < 0) {
			pcm_writei_failed_cnt++;

			//ALSA lib pcm.c:7843:(snd_pcm_recover) underrun occurred
			if ((ret = snd_pcm_recover(pcm, ret, 0)) < 0) {
				LOG_E("snd_pcm_writei failed, ret=%d", ret);
				continue;
			}
		}
	}
}

int main(int argc, char **argv)
{
	AVPacket *pkt;
	AVCodecContext *c = NULL;
	AVFrame *decoded_frame = NULL;

	if (argc <= 1) {
		fprintf(stderr, "%s %d.%d\n", prog_name,
			CSKY_AUDIO_PLAYER_MAJOR_NUM,
			CSKY_AUDIO_PLAYER_MINOR_NUM);
		fprintf(stderr, "Usage: %s <input file>\n", argv[0]);
		exit(1);
	}

	src_filename = argv[1];

	prepare_common(&c);
	init_pcm(c);
	av_dump_format(ifmt_ctx, audio_stream_idx, src_filename, 0);

	pkt = av_packet_alloc();

	decoded_frame = av_frame_alloc();
	if (decoded_frame == NULL) {
		LOG_E("Could not allocate audio frame!");
		exit(1);
	}

	while ((av_read_frame(ifmt_ctx, pkt) >= 0)) {
		if (pkt->stream_index != audio_stream_idx)
			continue;

		if (pkt->size)
			decode(c, pkt, decoded_frame);
	}

	snd_pcm_drain(pcm);
	snd_pcm_close(pcm);

	avcodec_free_context(&c);
	av_frame_free(&decoded_frame);
	av_packet_free(&pkt);

	swr_free(&swr_ctx);

	LOG_I("audio player play successfully!");
	LOG_D("snd_pcm_writei total cnt: %d", pcm_writei_total_cnt);
	LOG_D("snd_pcm_writei failed cnt: %d", pcm_writei_failed_cnt);
	return 0;
}
