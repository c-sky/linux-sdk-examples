/*
 * packet_queue.h
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

#ifndef PACKET_QUEUE_H_
#define PACKET_QUEUE_H_

#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"

typedef struct PacketQueue {
	AVPacketList *firstPkt;
	AVPacketList *lastPkt;
	int nbPkts;
	int size;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
} PacketQueue;

void packet_queue_init(PacketQueue * queue);

int packet_queue_put(PacketQueue * queue, AVPacket * packet);

int packet_queue_get(PacketQueue * queue, AVPacket * packet, int block);

#endif
