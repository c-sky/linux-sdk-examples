/*
 * audio module
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

#define __STDC_CONSTANT_MACROS	//FFmpeg

#include "csky_common.h"
#include "player.h"
#include "audio.h"
#include <pthread.h>
#include <alsa/asoundlib.h>

static SwrContext *swr_ctx = NULL;
static snd_pcm_t *pcm = NULL;

static void ffmpeg_fmt_to_alsa_fmt(AVCodecContext * c, snd_pcm_t * pcm,
				   snd_pcm_hw_params_t * params)
{
	int ret = 0;

	switch (c->sample_fmt) {
	case AV_SAMPLE_FMT_U8:	/* unsigned 8 bits */
		LOG_I("AV_SAMPLE_FMT_U8");
		ret =
		    snd_pcm_hw_params_set_format(pcm, params,
						 SND_PCM_FORMAT_S8);
		break;

	case AV_SAMPLE_FMT_S16:	/* signed 16 bits */
		LOG_I("AV_SAMPLE_FMT_S16");
		/* SND_PCM_FORMAT_S16 is ok, not care SND_PCM_FORMAT_S16_LE or SND_PCM_FORMAT_S16_BE */
		ret =
		    snd_pcm_hw_params_set_format(pcm, params,
						 SND_PCM_FORMAT_S16);
		break;

	case AV_SAMPLE_FMT_S16P:	/* signed 16 bits, planar */
		LOG_I("AV_SAMPLE_FMT_S16P");
		/* SND_PCM_FORMAT_S16 is ok, not care SND_PCM_FORMAT_S16_LE or SND_PCM_FORMAT_S16_BE */
		ret =
		    snd_pcm_hw_params_set_format(pcm, params,
						 SND_PCM_FORMAT_S16);
		break;

	case AV_SAMPLE_FMT_FLT:	/* float */
		LOG_I("AV_SAMPLE_FMT_FLT");
		ret =
		    snd_pcm_hw_params_set_format(pcm, params,
						 SND_PCM_FORMAT_FLOAT);
		break;

	case AV_SAMPLE_FMT_FLTP:	/* float, planar */
		LOG_I("AV_SAMPLE_FMT_FLTP");
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
	LOG_I("channels=%d", c->channels);

	val = c->sample_rate;
	LOG_I("sample_rate=%u", val);
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
	LOG_I("snd_pcm_get_params, buffer_size=%ld, period_size=%ld",
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
	LOG_I("snd_pcm_sw_params_get_avail_min, val=%ld", frm_val);

	ret =
	    snd_pcm_sw_params_get_start_threshold((const snd_pcm_sw_params_t *)
						  swparams, &frm_val);
	if (ret < 0) {
		LOG_E("snd_pcm_sw_params_get_start_threshold failed, ret=%d",
		      ret);
		exit(1);
	}
	LOG_I("snd_pcm_sw_params_get_start_threshold, val=%ld", frm_val);

	frm_val = (buffer_size / period_size) * period_size;
	ret = snd_pcm_sw_params_set_start_threshold(pcm, swparams, frm_val);
	if (ret < 0) {
		LOG_E("snd_pcm_sw_params_set_start_threshold failed, ret=%d",
		      ret);
		exit(1);
	}
	LOG_I("snd_pcm_sw_params_set_start_threshold, val=%ld", frm_val);

	/* write the parameters to the playback device */
	ret = snd_pcm_sw_params(pcm, swparams);
	if (ret < 0) {
		LOG_E("Unable to set sw params for %s", snd_strerror(ret));
		exit(1);
	}
}

static int prepare_audio(PlayerState * ps)
{
	ps->paudioStream = ps->pFormatCtx->streams[ps->audioStream];

	/* find decoder for the stream */
	ps->paudioCodec =
	    avcodec_find_decoder(ps->paudioStream->codecpar->codec_id);
	if (ps->paudioCodec == NULL) {
		LOG_E("Couldn't find audio decoder!!!!!");
		return -1;
	}

	/* Allocate a codec context for the decoder */
	ps->paudioCodecCtx = avcodec_alloc_context3(ps->paudioCodec);
	if (!ps->paudioCodecCtx) {
		LOG_E("Failed to allocate codec context!!!!!");
		return -1;
	}

	/* Copy codec parameters from input stream to output codec context */
	if ((avcodec_parameters_to_context
	     (ps->paudioCodecCtx, ps->paudioStream->codecpar)) < 0) {
		fprintf(ERR_STREAM,
			"Failed to copy codec parameters to decoder context");
		return -1;
	}

	avcodec_open2(ps->paudioCodecCtx, ps->paudioCodec, NULL);

	init_pcm(ps->paudioCodecCtx);

	return 0;
}

void *play_audio(void *ps_arg)
{
	PlayerState *ps = (PlayerState *) ps_arg;

	if (prepare_audio(ps) < 0) {
		LOG_E("Failed to prepare audio!!!!!");
		if (ps->paudioCodecCtx)
			avcodec_free_context(&ps->paudioCodecCtx);
		pthread_exit(NULL);
	}

	AVPacket *pkt = av_packet_alloc();
	AVFrame *pframe = av_frame_alloc();

	while (1) {
		if (ps->audioPacketQueue.nbPkts == 0) {
			if (ps->quit)
				break;
			usleep(100 * 1000);
			continue;
		}

		if (audio_decode_frame(ps, pkt, pframe) == -2) {
			LOG_E("Failed to decode audio frame!!!!!");
			break;
		}
		av_packet_unref(pkt);
		av_frame_unref(pframe);
	}

	snd_pcm_drain(pcm);
	snd_pcm_close(pcm);

	avcodec_free_context(&ps->paudioCodecCtx);
	av_packet_free(&pkt);
	av_frame_free(&pframe);
	swr_free(&swr_ctx);

	pthread_exit(NULL);
}

static void init_swr_ctx(AVFrame * frame)
{
	int ret;
	enum AVSampleFormat out_sample_fmt = (enum AVSampleFormat)frame->format;

	if (swr_ctx)
		return;

	LOG_I
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
			       (frame->channels),
			       (enum AVSampleFormat)frame->format,
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

// one packet may contain several frame
int audio_decode_frame(PlayerState * ps, AVPacket *pkt, AVFrame *pframe)
{
	LOG_D("decode_audio~~~");
	uint8_t *audio_buf = ps->audio_buf;
	int ret = 0;

	if (ps->quit == 1) {
		LOG_I("ps->quit is 1");
		return -1;
	}

	if (ps->audioPacketQueue.nbPkts == 0) {
		LOG_I("ps->audioPacketQueue.nbPktsi is 0");
		return -1;
	}

	if (packet_queue_get(&ps->audioPacketQueue, pkt, 1) < 0) {
		LOG_E("Get queue packet error");
		return -1;
	}

	if (pkt->stream_index != ps->audioStream) {
		LOG_E("packet stream %d is not audio stream!",
		      pkt->stream_index);
		return -1;
	}
	//decode audio packet, receive a decoded frame
	ret = avcodec_send_packet(ps->paudioCodecCtx, pkt);
	if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
		LOG_E("send decode packet error");
		return -1;
	}

	while (1) {
		ret = avcodec_receive_frame(ps->paudioCodecCtx, pframe);

		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
			break;
		} else if (ret < 0) {
			LOG_E("Error during decoding");
			return -2;
		}

		init_swr_ctx(pframe);

		swr_convert(swr_ctx, &audio_buf, pframe->sample_rate,
			    (const uint8_t **)pframe->extended_data,
			    pframe->nb_samples);

		LOG_D
		    ("frame->channel=%d, frame->nb_samples=%d, frame->format=%d",
		     pframe->channels, pframe->nb_samples, pframe->format);

		if ((ret =
		     snd_pcm_writei(pcm, audio_buf, pframe->nb_samples)) < 0) {
			//ALSA lib pcm.c:7843:(snd_pcm_recover) underrun occurred
			if ((ret = snd_pcm_recover(pcm, ret, 0)) < 0) {
				LOG_E("snd_pcm_writei failed, ret=%d", ret);
				continue;
			}
		}
	}

	return ret;
}
