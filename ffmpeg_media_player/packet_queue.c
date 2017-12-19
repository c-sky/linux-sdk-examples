/*
 * packet module
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

#define __STDC_CONSTANT_MACROS

#ifdef __cplusplus
extern "C" {
#endif

#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
#include "csky_common.h"
#include <pthread.h>

#ifdef __cplusplus
}
#endif
#include "packet_queue.h"
void packet_queue_init(PacketQueue * queue)
{
	queue->firstPkt = NULL;
	queue->lastPkt = NULL;
	queue->nbPkts = 0;
	pthread_mutex_init(&queue->mutex, NULL);
	pthread_cond_init(&queue->cond, NULL);
}

int packet_queue_put(PacketQueue * queue, AVPacket * packet)
{
	AVPacketList *pkt_list;
	AVPacket tmp_pkt = { 0 };

	if (av_packet_ref(&tmp_pkt, packet) < 0) {
		return -1;
	}

	pkt_list = (AVPacketList *) av_malloc(sizeof(AVPacketList));
	if (pkt_list == NULL) {
		return -1;
	}

	pkt_list->pkt = tmp_pkt;
	pkt_list->next = NULL;

	pthread_mutex_lock(&queue->mutex);

	if (queue->lastPkt == NULL) {
		queue->firstPkt = pkt_list;
	} else {
		queue->lastPkt->next = pkt_list;
	}

	queue->lastPkt = pkt_list;
	queue->nbPkts++;
	queue->size += tmp_pkt.size;

	pthread_cond_signal(&queue->cond);
	pthread_mutex_unlock(&queue->mutex);

	return 0;
}

int packet_queue_get(PacketQueue * queue, AVPacket * pkt, int block)
{
	AVPacketList *pkt_list = NULL;
	int ret = 0;

	pthread_mutex_lock(&queue->mutex);
	av_packet_unref(pkt);
	while (1) {
		pkt_list = queue->firstPkt;
		if (pkt_list != NULL) {
			queue->firstPkt = queue->firstPkt->next;
			if (queue->firstPkt == NULL) {
				queue->lastPkt = NULL;
			}

			queue->nbPkts--;
			queue->size -= pkt_list->pkt.size;
			*pkt = pkt_list->pkt;
			av_free(pkt_list);
			ret = 1;
			break;
		} else if (block == 0) {
			ret = 0;
			break;
		} else {
			pthread_cond_wait(&queue->cond, &queue->mutex);
		}
	}

	pthread_mutex_unlock(&queue->mutex);

	return ret;
}
