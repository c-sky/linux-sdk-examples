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
#include <stdlib.h>
#include <string.h>
#include <linux/videodev2.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <signal.h>

#include "args.h"
#include "common.h"
#include "fb.h"
#include "fileops.h"
#include "mfc.h"
#include "parser.h"
#include "v4l2_common.h"

char g_scan_char;
#define DEBUG_SCAN_STEP		//dbg("press any key to continue:");scanf("%c", &g_scan_char);

/* This is the size of the buffer for the compressed stream.
 * It limits the maximum compressed frame size. */
#define STREAM_BUFFER_SIZE	(1024 * 1024)
/* The number of compress4ed stream buffers */
#define STREAM_BUFFER_CNT	2

/* The number of extra buffers for the decoded output.
 * This is the number of buffers that the application can keep
 * used and still enable MFC to decode with the hardware. */
#define RESULT_EXTRA_BUFFER_CNT 2

static int record_init(struct instance *i)
{
	if (i->misc.record_filename == NULL) {
		return 0;
	}

	i->misc.record_fd = fopen(i->misc.record_filename, "w+b");
	if (i->misc.record_fd == NULL) {
		printf("create record file '%s' failed\n",
		       i->misc.record_filename);
		return -1;
	}
	printf("create record file '%s' OK\n", i->misc.record_filename);
	return 0;
}

static int record_deinit(struct instance *i)
{
	if (i->misc.record_fd) {
		fclose(i->misc.record_fd);
		i->misc.record_fd = NULL;
	}
	return 0;
}

static int record_yuv_pic(struct instance *i,
			  char *y, char *u, char *v, int pixels)
{
	if (i->misc.record_fd == NULL) {
		return 0;
	}

	fwrite(y, pixels, 1, i->misc.record_fd);
	fwrite(u, pixels / 4, 1, i->misc.record_fd);
	fwrite(v, pixels / 4, 1, i->misc.record_fd);

	return 0;
}

void cleanup(struct instance *i)
{
	if (i->mfc.fd)
		mfc_close(i);
	if (i->fb.fd)
		fb_close(i);
	if (i->in.fd)
		input_close(i);
	record_deinit(i);
}

int dequeue_output(struct instance *i, int *n)
{
	struct v4l2_buffer qbuf;

	memzero(qbuf);
	qbuf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	qbuf.memory = V4L2_MEMORY_MMAP;

	if (mfc_dec_dequeue_buf(i, &qbuf))
		return -1;

	*n = qbuf.index;

	return 0;
}

int dequeue_capture(struct instance *i, int *n, unsigned int *disp_paddr,
		    int *finished)
{
	struct v4l2_buffer qbuf;

	memzero(qbuf);
	qbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	qbuf.memory = V4L2_MEMORY_MMAP;

	if (mfc_dec_dequeue_buf(i, &qbuf)) {
		dbg("dequeue_capture failed");
		return -1;
	}
	//v4l_print_buffer(&qbuf);

	*n = qbuf.index;
	*disp_paddr = *((unsigned int *)qbuf.timecode.userbits);
	*finished = qbuf.bytesused == 0;

	dbg("n=%d, qbuf.bytesused=%d, finished=%d, user_addr=%p, disp_paddr=0x%08x",
		*n, qbuf.bytesused, *finished, i->mfc.cap_buf_addr[*n][0], *disp_paddr);

	return 0;
}

static pthread_t mfc_thread;
static pthread_t parser_thread;
static pthread_t daemon_thread;

/* This threads is responsible for parsing the stream and
 * feeding MFC with consecutive frames to decode */
static int s_output_stream_on = 0;
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
			ret = i->parser.func(&i->parser.ctx,
					     i->in.p + i->in.offs,
					     i->in.size - i->in.offs,
					     i->mfc.out_buf_addr[n],
					     i->mfc.out_buf_size, &used, &fs,
					     0);

			if (ret == 0 && i->in.offs == i->in.size) {
				dbg("Parser has extracted all frames");
				i->parser.finished = 1;
				fs = 0;
				break;
			}

			DEBUG_SCAN_STEP;
#if 0
			dbg("Extracted frame of size %d: %02X %02X %02X %02X %02X %02X %02X %02X ...",
				fs, i->mfc.out_buf_addr[n][0], i->mfc.out_buf_addr[n][1],
				i->mfc.out_buf_addr[n][2], i->mfc.out_buf_addr[n][3],
				i->mfc.out_buf_addr[n][4], i->mfc.out_buf_addr[n][5],
				i->mfc.out_buf_addr[n][6], i->mfc.out_buf_addr[n][7]);
#endif

			if (s_output_stream_on == 0 && i->in.offs > 512) {
				int ret;
				ret =
				    mfc_stream(i, V4L2_BUF_TYPE_VIDEO_OUTPUT,
					       VIDIOC_STREAMON);
				if (ret) {
					err("Failed");
					return (void *)(-1);
				}
				s_output_stream_on = 1;
			}

			ret = mfc_dec_queue_buf_out(i, n, fs);
			DEBUG_SCAN_STEP;

			i->mfc.out_buf_flag[n] = 1;
			i->in.offs += used;
		} else {
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

	printf("Parser thread finished\n");
	return 0;
}

/* This thread handles the CAPTURE side of MFC. it receives
 * decoded frames and queues empty buffers back to MFC.
 * Also it passes the decoded frames to FIMC, so they
 * can be processed and displayed. */
static int s_last_dequeue = -1;

void *mfc_thread_func(void *args)
{
	struct instance *i = (struct instance *)args;
	int finished;
	int n;
#ifdef DEBUG
	int frame_count = 0;
#endif
	dbg("mfc_thread_func+");

	/* The thread is cancelable. */
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	/* The thread can be canceled at any time. */
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	while (!i->error && !i->finish) {
		/* Can dequeue a processed buffer */
		unsigned int disp_paddr;
		if (dequeue_capture(i, &n, &disp_paddr, &finished)) {
			err("Error when dequeueing CAPTURE buffer");
			i->error = 1;
			break;
		}

		i->decoded_cnt++;
		if (finished) {
			dbg("Finished extracting last frames");
			i->finish = 1;
			break;
		}
		dbg("****** Decode Frames Count=%d ******", ++frame_count);

		/* Display this cap buf on LCD */
		struct csky_fb_lcd_pbase_yuv base_yuv;
		base_yuv.y = disp_paddr;
		base_yuv.u = base_yuv.y + i->mfc.cap_w * i->mfc.cap_h;
		base_yuv.v = base_yuv.u + (i->mfc.cap_w * i->mfc.cap_h) / 4;
		ioctl(i->fb.fd, CSKY_FBIO_SET_PBASE_YUV, &base_yuv);
		fb_power_on(i);
		fb_wait_for_vsync(i);

		if (i->mfc.cap_buf_addr[n][0] != MAP_FAILED) {
			char *cap_data = i->mfc.cap_buf_addr[n][0];
			record_yuv_pic(i,
				       &(cap_data[0]),
				       &(cap_data[1920 * 1088]),
				       &(cap_data
					 [1920 * 1088 + 1920 * 1088 / 4]),
				       (i->fb.width * i->fb.height));
#if 0
			int loop;

			dbg("cap[%d] (vaddr=%p, disp_paddr=0x%08x)",
			    n, i->mfc.cap_buf_addr[n][0], disp_paddr);
			printf("\t Y data is:");
			for (loop = 0; loop < 16; ++loop) {
				printf("%02x ", cap_data[loop]);
			}
			printf("\n");

			printf("\t U data is:");
			for (loop = 0; loop < 16; ++loop) {
				printf("%02x ", cap_data[loop + 1920 * 1088]);
			}
			printf("\n");

			printf("\t V data is:");
			for (loop = 0; loop < 16; ++loop) {
				printf("%02x ",
				       cap_data[loop + 1920 * 1088 +
						1920 * 1088 / 4]);
			}
			printf("\n");
#endif
		}
		//DEBUG_SCAN_STEP;

		if (s_last_dequeue >= 0) {
			mfc_dec_queue_buf_cap(i, s_last_dequeue);
			//i->mfc.cap_buf_flag[s_last_dequeue] = BUF_FREE;
		}
		s_last_dequeue = n;
		dbg("Current display cap_buf_addr[%d]:0x%08x", n, disp_paddr);
		continue;
	}

	printf("MFC thread finished\n");
	return 0;
}

void *daemon_thread_func(void *args)
{
	const int interval = 2;
	const int run_limit = -1;	// unit: second, -1 means run until no frame

	int run_time = 0;
	struct instance *i = (struct instance *)args;
	static int last_decoded_cnt = 0;

	while (1) {
		sleep(interval);
		if (last_decoded_cnt == i->decoded_cnt) {
			printf("No frame decoded in last %d seconds\n",
			       interval);
			break;
		}
		last_decoded_cnt = i->decoded_cnt;

		if (run_limit != -1 && (run_time += interval) > run_limit) {
			printf("Run time = %d seconds\n", run_limit);
			break;
		}
	}

	i->finish = 1;
	printf("Daemon thread finished\n");
	pthread_cancel(mfc_thread);
	return 0;
}

void *timeout_thread_func(void *args)
{
	struct instance *i = (struct instance *)args;
	sleep(i->misc.timeout);
	exit(0);
	return 0;
}

int main(int argc, char **argv)
{
	struct instance inst;
	int n;

	printf("V4L2 Codec decoding example application\n");

	if (parse_args(&inst, argc, argv)) {
		print_usage(argv[0]);
		return 1;
	}

	if (input_open(&inst, inst.in.name)) {
		cleanup(&inst);
		return 1;
	}

	if (fb_open(&inst, inst.fb.name)) {
		cleanup(&inst);
		return 1;
	}

	if (mfc_open(&inst, inst.mfc.name)) {
		cleanup(&inst);
		return 1;
	}

	dbg("Successfully opened all necessary files and devices");

	if (mfc_dec_setup_output(&inst, inst.parser.codec,
				 STREAM_BUFFER_SIZE, STREAM_BUFFER_CNT)) {
		cleanup(&inst);
		return 1;
	}

	parse_stream_init(&inst.parser.ctx);

	if (mfc_dec_setup_capture(&inst, RESULT_EXTRA_BUFFER_CNT)) {
		cleanup(&inst);
		return 1;
	}

	DEBUG_SCAN_STEP;
	dbg("I for one welcome our succesfully setup environment.");

	/* Since our fabulous V4L2 framework enforces that at least one buffer
	 * is queued before switching streaming on then we need to add the
	 * following code. Otherwise it could be ommited and it all would be
	 * handled by the mfc_thread.*/

	for (n = 0; n < inst.mfc.cap_buf_cnt; n++) {
		if (mfc_dec_queue_buf_cap(&inst, n)) {
			cleanup(&inst);
			return 1;
		}

		inst.mfc.cap_buf_flag[n] = BUF_MFC;
		inst.mfc.cap_buf_queued++;
	}

	if (mfc_stream(&inst, V4L2_BUF_TYPE_VIDEO_CAPTURE, VIDIOC_STREAMON)) {
		cleanup(&inst);
		return 1;
	}

	if (record_init(&inst) != 0) {
		cleanup(&inst);
		return 1;
	}

	/* Now we're safe to run the threads */
	dbg("Launching threads");

	if (pthread_create(&parser_thread, NULL, parser_thread_func, &inst)) {
		cleanup(&inst);
		return 1;
	}

	if (pthread_create(&mfc_thread, NULL, mfc_thread_func, &inst)) {
		cleanup(&inst);
		return 1;
	}

	if (pthread_create(&daemon_thread, NULL, daemon_thread_func, &inst)) {
		cleanup(&inst);
		return 1;
	}

	if (inst.misc.timeout > 0) {
		pthread_t timeout_thread;
		printf("*** To run timeout thread, timeout: %d seconds ***\n",
		       inst.misc.timeout);
		pthread_create(&timeout_thread, NULL, timeout_thread_func,
			       &inst);
	}

	pthread_join(parser_thread, 0);
	pthread_join(mfc_thread, 0);
	pthread_join(daemon_thread, 0);

	printf("Threads have finished\n");

	cleanup(&inst);
	return 0;
}
