/*
 * video.h
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

#ifndef VIDEO_H_
#define VIDEO_H_

#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
#include "libavutil/time.h"
#include "libavutil/imgutils.h"

#include "player.h"
#include "packet_queue.h"
#include "video/args.h"

int video_init(struct instance *inst);
void video_deinit(struct instance *inst);
int prepare_video(PlayerState * ps);
int get_one_avpacket(PlayerState * ps, AVPacket * packet);

#endif
