/*
 * V4L2 Codec decoding example application
 * Lu Chongzhi <chongzhi_lu@c-sky.com>
 *
 * Main file of the application
 *
 * Copyright 2017 C-SKY Microsystems Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <stdio.h>
#include <string.h>
#include <linux/videodev2.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include "args.h"
#include "common.h"
#include "csky_fb_test.h"
#include "fileops.h"
#include "mfc.h"
#include "parser.h"

//FFmpeg related head files
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"


char g_scan_char;
#define DEBUG_SCAN_STEP //dbg("press any key to continue:");scanf("%c", &g_scan_char);

/* This is the size of the buffer for the compressed stream.
 * It limits the maximum compressed frame size. */
#define STREAM_BUFFER_SIZE	(1024 * 1024)
/* The number of compress4ed stream buffers */
#define STREAM_BUFFER_CNT	2

/* The number of extra buffers for the decoded output.
 * This is the number of buffers that the application can keep
 * used and still enable MFC to decode with the hardware. */
#define RESULT_EXTRA_BUFFER_CNT 2


void cleanup(struct instance *i)
{
	if (i->mfc.fd)
		mfc_close(i);
	if (i->fb.fd)
		fb_close(i);
	if (i->in.fd)
		input_close(i);
}

int extract_and_process_header(struct instance *i, AVCodecContext *pCodecCtx)
{
	int used = 0;	/* All bytes consumed from media file head */
	int fs = 0;	/* Frame output size in bytes */
	int ret;
#if 0
	ret = i->parser.func(&i->parser.ctx, i->in.p + i->in.offs,
		i->in.size - i->in.offs, i->mfc.out_buf_addr[0],
		i->mfc.out_buf_size, &used, &fs, 1);
	if (ret == 0) {
		err("Failed to extract header from stream");
		return -1;
	}
	//fwrite(pCodecCtx->extradata, pCodecCtx->extradata_size, 1, fp_es);
#endif
	memcpy(i->mfc.out_buf_addr[0], pCodecCtx->extradata, pCodecCtx->extradata_size);
	fs = pCodecCtx->extradata_size;
#if 0
	printf("OK?");
	uint8_t sps[100];
	uint8_t pps[100];
	int spsLength=0;
	int ppsLength=0;   // i->ffmpeg.pFormatCtx
	spsLength=i->ffmpeg.pFormatCtx->streams[0]->codec->extradata[6]*0xFF+
		i->ffmpeg.pFormatCtx->streams[0]->codec->extradata[7];

	ppsLength=i->ffmpeg.pFormatCtx->streams[0]->codec->extradata[8+spsLength+1]*0xFF
		+i->ffmpeg.pFormatCtx->streams[0]->codec->extradata[8+spsLength+2];

	int j = 0;
	for (j=0;j<spsLength;j++)
	{
		sps[j]=i->ffmpeg.pFormatCtx->streams[0]->codec->extradata[j+8];
	}

	for (j=0;j<ppsLength;j++)
	{
		pps[j]=i->ffmpeg.pFormatCtx->streams[0]->codec->extradata[j+8+2+1+spsLength];
	}

	char nal_start[]={0,0,0,1};
	memcpy(i->mfc.out_buf_addr[fs], nal_start, 4);
	fs = fs + 4;
	memcpy(i->mfc.out_buf_addr[fs], sps, spsLength);
	fs = fs + spsLength;
	memcpy(i->mfc.out_buf_addr[fs], nal_start, 4);
	fs = fs + 4;
	memcpy(i->mfc.out_buf_addr[fs], pps, ppsLength);
	fs = fs + ppsLength;
#endif
	i->mfc.out_buf_addr[0][fs+0] = 0x00;
	i->mfc.out_buf_addr[0][fs+1] = 0x00;
	i->mfc.out_buf_addr[0][fs+2] = 0x01; //01
	i->mfc.out_buf_addr[0][fs+3] = 0xff;
	fs = 512;
	printf("OK!!!");
	int n;
	for (n = 0; n < fs; ++n) {
		printf("%02x ", i->mfc.out_buf_addr[0][n]);
		if (n % 16 == 15)
			printf("\n");
	}

	/* For H263 the header is passed with the first frame, so we should
	 * pass it again */
	if (i->parser.codec != V4L2_PIX_FMT_H263)
		i->in.offs += used;
	else
	/* To do this we shall reset the stream parser to the initial
	 * configuration */
		parse_stream_init(&i->parser.ctx);

	dbg("Extracted header of size %d", fs);

	ret = mfc_dec_queue_buf_out(i, 0, fs);
	if (ret) {
		dbg("mfc_dec_queue_buf_out failed, ret=%d", ret);
		return -1;
	}

	ret = mfc_stream(i, V4L2_BUF_TYPE_VIDEO_OUTPUT, VIDIOC_STREAMON);
	if (ret) {
		err("Failed");
		return -1;
	}

	return 0;
}

int dequeue_output(struct instance *i, int *n)
{
	struct v4l2_buffer qbuf;
	struct v4l2_plane planes[MFC_OUT_PLANES];

	memzero(qbuf);
	qbuf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	qbuf.memory = V4L2_MEMORY_MMAP;
	qbuf.m.planes = planes;
	qbuf.length = 1;

	if (mfc_dec_dequeue_buf(i, &qbuf))
		return -1;

	*n = qbuf.index;

	return 0;
}

int dequeue_capture(struct instance *i, int *n, unsigned int *paddr, int *finished)
{
	struct v4l2_buffer qbuf;

	memzero(qbuf);
	qbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	qbuf.memory = V4L2_MEMORY_MMAP;
	qbuf.length = 2;

	if (mfc_dec_dequeue_buf(i, &qbuf)) {
		dbg("dequeue_capture failed");
		return -1;
	}
	*paddr = *((unsigned int*)qbuf.timecode.userbits);
	dbg("qbuf.bytesused=%d, paddr=0x%08x", qbuf.bytesused, *paddr);
	*finished = qbuf.bytesused == 0;
	*n = qbuf.index;

	return 0;
}

/* This threads is responsible for parsing the stream and
 * feeding MFC with consecutive frames to decode */
void *parser_thread_func(void *args)
{
	struct instance *i = (struct instance *)args;
	int ret;
	int used, fs, n;

	while (!i->error && !i->finish && !i->parser.finished) {
		n = 0;
		while (n < i->mfc.out_buf_cnt && i->mfc.out_buf_flag[n])
			n++;

		if (n < i->mfc.out_buf_cnt && !i->parser.finished) {
			//dbg("parser.func = %p", i->parser.func);
			while(av_read_frame(i->ffmpeg.pFormatCtx, &i->ffmpeg.packet) >= 0) {
				if(i->ffmpeg.packet.stream_index != i->ffmpeg.videoStream) {
					av_free_packet(&i->ffmpeg.packet);
					continue;
				}
				else {
					break;
				}
			}
			if(i->ffmpeg.packet.data != NULL) {
				memcpy(i->mfc.out_buf_addr[n], i->ffmpeg.packet.data, i->ffmpeg.packet.size);
				fs = i->ffmpeg.packet.size;
				av_free_packet(&i->ffmpeg.packet);
				printf("get 1 frame...\n");
			} else {
				dbg("Parser has extracted all frames");
				i->parser.finished = 1;
				fs = 0;
			}

			/*
			ret = i->parser.func(&i->parser.ctx,
				i->in.p + i->in.offs, i->in.size - i->in.offs,
				i->mfc.out_buf_addr[n], i->mfc.out_buf_size,
				&used, &fs, 0);


			if (ret == 0 && i->in.offs == i->in.size) {
				dbg("Parser has extracted all frames");
				i->parser.finished = 1;
				fs = 0;
			}
			*/
			DEBUG_SCAN_STEP;

			if (fs <= 0) {
				dbg("No more frame");
				break;
			}

			dbg("Extracted frame of size %d: %02X %02X %02X %02X %02X %02X %02X %02X ...",
				fs,
				i->mfc.out_buf_addr[n][0], i->mfc.out_buf_addr[n][1],
				i->mfc.out_buf_addr[n][2], i->mfc.out_buf_addr[n][3],
				i->mfc.out_buf_addr[n][4], i->mfc.out_buf_addr[n][5],
				i->mfc.out_buf_addr[n][6], i->mfc.out_buf_addr[n][7]);
			ret = mfc_dec_queue_buf_out(i, n, fs);
			DEBUG_SCAN_STEP;

			i->mfc.out_buf_flag[n] = 1;
			// i->in.offs += used;
		}else {
			DEBUG_SCAN_STEP;
			ret = dequeue_output(i, &n);
			DEBUG_SCAN_STEP;
			i->mfc.out_buf_flag[n] = 0;
			if (ret && !i->parser.finished) {
				err("Failed to dequeue a buffer in parser_thread");
				i->error = 1;
			}
		}
	}

	dbg("Parser thread finished");
	avformat_close_input(&i->ffmpeg.pFormatCtx);
	return 0;
}

/* This thread handles the CAPTURE side of MFC. it receives
 * decoded frames and queues empty buffers back to MFC.
 * Also it passes the decoded frames to FIMC, so they
 * can be processed and displayed. */
void *mfc_thread_func(void *args)
{
	struct instance *i = (struct instance *)args;
	int finished;
	int n;
	int frame_count = 0;
	dbg("mfc_thread_func+");
	while (!i->error && !i->finish) {
		if (i->mfc.cap_buf_queued < i->mfc.cap_buf_cnt_min) {

			n = 0;
			while (n < i->mfc.cap_buf_cnt &&
				i->mfc.cap_buf_flag[n] != BUF_FREE)
				n++;

			if (n < i->mfc.cap_buf_cnt) {
				/* Can queue a buffer */
				mfc_dec_queue_buf_cap(i, n);
				i->mfc.cap_buf_flag[n] = BUF_MFC;
				i->mfc.cap_buf_queued++;
			} else {
				err("Something went seriously wrong. There should be a buffer");
				int j;
				for (j = 0; j < i->mfc.cap_buf_cnt; ++j) {
					printf("cap_buf_flag[%d]=%d\n", j, i->mfc.cap_buf_flag[j]);
				}
				i->error = 1;
				continue;
			}

			continue;
		}

		if (i->mfc.cap_buf_queued < i->mfc.cap_buf_cnt) {
			n = 0;
			while (n < i->mfc.cap_buf_cnt &&
				i->mfc.cap_buf_flag[n] != BUF_FREE)
				n++;

			if (n < i->mfc.cap_buf_cnt) {
				/* Can queue a buffer */
				mfc_dec_queue_buf_cap(i, n);
				i->mfc.cap_buf_flag[n] = BUF_MFC;
				i->mfc.cap_buf_queued++;
				continue;
			}
		}

		if (i->mfc.cap_buf_queued >= i->mfc.cap_buf_cnt_min) {
			/* Can dequeue a processed buffer */
			unsigned int paddr;
			if (dequeue_capture(i, &n, &paddr, &finished)) {
				err("Error when dequeueing CAPTURE buffer");
				i->error = 1;
				break;
			}
			if (finished) {
				dbg("Finished extracting last frames");
				i->finish = 1;
				break;
			}
			dbg("****** Decode Frames Count=%d ******", ++frame_count);

#if 0
			if (i->mfc.cap_buf_addr[n][0] != MAP_FAILED) {
				int loop;
				char *cap_data = i->mfc.cap_buf_addr[n][0];

				dbg("cap[%d] (vaddr=%p, paddr=0x%08x)",
					n, i->mfc.cap_buf_addr[n][0], paddr);
				printf("\t Y data is:");
				for (loop = 0; loop < 16; ++loop) {
					printf("%02x ", cap_data[loop]);
				}
				printf("\n");

				printf("\t U data is:");
				for (loop = 0; loop < 16; ++loop) {
					printf("%02x ", cap_data[loop + 1920*1088]);
				}
				printf("\n");

				printf("\t V data is:");
				for (loop = 0; loop < 16; ++loop) {
					printf("%02x ", cap_data[loop + 1920*1088 + 1920*1088/4]);
				}
				printf("\n");
			}
#endif

			/* Display this cap buf on LCD */
			fb_power_on(i);
			fb_wait_for_vsync(i);
			struct csky_fb_lcd_pbase_yuv base_yuv;
			base_yuv.y = paddr;
			base_yuv.u = base_yuv.y + 1920*1088;
			base_yuv.v = base_yuv.u + 1920*1088/4;
			ioctl(i->fb.fd, CSKY_FBIO_SET_PBASE_YUV, &base_yuv);

			/* Free this cap buf */
			i->mfc.cap_buf_flag[n] = BUF_FREE;
			i->mfc.cap_buf_queued--;

			continue;
		}
	}

	dbg("MFC thread finished");
	return 0;
}

int main(int argc, char **argv)
{
	struct instance inst;
	pthread_t mfc_thread;
	pthread_t parser_thread;
	int n;

	printf("V4L2 Codec decoding example application\n");

	if (parse_args(&inst, argc, argv)) {
		print_usage(argv[0]);
		return 1;
	}
#if 0
	if (input_open(&inst, inst.in.name)) {
		cleanup(&inst);
		return 1;
	}
#endif
	if (fb_open(&inst, inst.fb.name)) {
		cleanup(&inst);
		return 1;
	}

	if (mfc_open(&inst, inst.mfc.name)) {
		cleanup(&inst);
		return 1;
	}

	dbg("Successfully opened all necessary files and devices");


// *****************FFmpeg相关**********************
	av_register_all();
	avformat_network_init();

	//AVFormatContext *pFormatCtx;
	inst.ffmpeg.pFormatCtx = avformat_alloc_context();

	// Open video file
	if(avformat_open_input(&inst.ffmpeg.pFormatCtx, inst.in.name, NULL, NULL) != 0)
	{
		printf("Couldn't open input stream.\n");
  		return -1; // Couldn't open file
	}
	//Dump information about file onto standard error
	printf("--------------- File Information ----------------\n");
	av_dump_format(inst.ffmpeg.pFormatCtx, 0, inst.in.name, 0);

	if(avformat_find_stream_info(inst.ffmpeg.pFormatCtx,NULL)<0)
	{
		printf("Couldn't find stream information.\n");
		return -1;
	}

	// Find the first video stream
	// int i,videoStream;
	int i;
	inst.ffmpeg.videoStream = -1;
	for(i=0; i < inst.ffmpeg.pFormatCtx->nb_streams; i++)
	{
		if(inst.ffmpeg.pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			inst.ffmpeg.videoStream = i;
			break;
		}
	}
	if(inst.ffmpeg.videoStream == -1)
	{
		printf("Didn't find a video stream.\n");
		return -1;
	}

	AVCodecContext *pCodecCtx;
	pCodecCtx = inst.ffmpeg.pFormatCtx->streams[inst.ffmpeg.videoStream]->codec;
	printf("pCodecCtx->extradata_size = %d\n",
			pCodecCtx->extradata_size);

// ****************************************************


	if (mfc_dec_setup_output(&inst, inst.parser.codec,
		STREAM_BUFFER_SIZE, STREAM_BUFFER_CNT)) {
		cleanup(&inst);
		return 1;
	}

	parse_stream_init(&inst.parser.ctx);

	if (extract_and_process_header(&inst, pCodecCtx)) {
		cleanup(&inst);
		return 1;
	}

	DEBUG_SCAN_STEP;

	if (mfc_dec_setup_capture(&inst, RESULT_EXTRA_BUFFER_CNT)) {
		cleanup(&inst);
		return 1;
	}

	DEBUG_SCAN_STEP;

	if (dequeue_output(&inst, &n)) {
		cleanup(&inst);
		return 1;
	}

	DEBUG_SCAN_STEP;
	dbg("I for one welcome our succesfully setup environment.");

	/* Since our fabulous V4L2 framework enforces that at least one buffer
	 * is queued before switching streaming on then we need to add the
	 * following code. Otherwise it could be ommited and it all would be
	 * handled by the mfc_thread.*/

	for (n = 0 ; n < inst.mfc.cap_buf_cnt; n++) {

		if (mfc_dec_queue_buf_cap(&inst, n)) {
			cleanup(&inst);
			return 1;
		}

		inst.mfc.cap_buf_flag[n] = BUF_MFC;
		inst.mfc.cap_buf_queued++;
	}

	DEBUG_SCAN_STEP;

	if (mfc_stream(&inst, V4L2_BUF_TYPE_VIDEO_CAPTURE, VIDIOC_STREAMON)) {
		cleanup(&inst);
		return 1;
	}

	/* Now we're safe to run the threads */
	dbg("Launching threads");

	DEBUG_SCAN_STEP;

	if (pthread_create(&parser_thread, NULL, parser_thread_func, &inst)) {
		cleanup(&inst);
		return 1;
	}

	if (pthread_create(&mfc_thread, NULL, mfc_thread_func, &inst)) {
		cleanup(&inst);
		return 1;
	}

	pthread_join(parser_thread, 0);
	pthread_join(mfc_thread, 0);

	dbg("Threads have finished");

	cleanup(&inst);
	return 0;
}

