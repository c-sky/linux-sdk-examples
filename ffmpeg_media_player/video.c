/*
 * video module
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

#include "csky_common.h"
#include "video.h"

int prepare_video(PlayerState * ps)
{
	double fps;

	ps->pvideoStream = ps->pFormatCtx->streams[ps->videoStream];

	/* find decoder for the stream */
	ps->pvideoCodec =
	    avcodec_find_decoder(ps->pvideoStream->codecpar->codec_id);
	if (ps->pvideoCodec == NULL) {
		LOG_E("Couldn't find video decoder!!!!!");
		return -1;
	}

	/* Allocate a codec context for the decoder */
	ps->pvideoCodecCtx = avcodec_alloc_context3(ps->pvideoCodec);
	if (!ps->pvideoCodecCtx) {
		LOG_E("Failed to allocate codec context!!!!!");
		return -1;
	}

	/* Copy codec parameters from input stream to output codec context */
	if ((avcodec_parameters_to_context
	     (ps->pvideoCodecCtx, ps->pvideoStream->codecpar)) < 0) {
		LOG_E("Failed to copy codec parameters to decoder context");
		return -1;
	}

	if (avcodec_open2(ps->pvideoCodecCtx, ps->pvideoCodec, NULL) < 0) {
		LOG_E("Couldn't open video decoder");
		return -1;
	}

	if (ps->pvideoStream->avg_frame_rate.den
	    && ps->pvideoStream->avg_frame_rate.num) {
		fps =
		    (double)ps->pvideoStream->avg_frame_rate.num /
		    ps->pvideoStream->avg_frame_rate.den;
		ps->delay = (int)((1000 / fps) * 1000);
		LOG_I("video will delay for %d us for each frame", ps->delay);
	}

	return 0;
}

int get_one_avpacket(PlayerState * ps, AVPacket * packet)
{
	int ret;

	while (ps->videoPacketQueue.nbPkts == 0) {
		if (ps->quit)
			return -1;

		usleep(100 * 1000);
	}

	ret = packet_queue_get(&ps->videoPacketQueue, packet, 1);
	if (ret < 0) {
		LOG_E("Get video packet error");
		return -1;
	}
	LOG_D("packet sz is :%d", packet->size);

	return 0;
}
