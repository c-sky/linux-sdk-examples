/*
 * V4L2 Coenc encoding example application
 * Cai Huoqing <huoqing_cai@c-sky.com>
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
#include "fileops.h"
#include "mfc.h"
#include "v4l2_common.h"

char g_scan_char;
#define DEBUG_SCAN_STEP		//dbg("press any key to continue:");scanf("%c", &g_scan_char);

/* The number of compress4ed stream buffers */
#define STREAM_BUFFER_CNT	6

/* The number of extra buffers for the encoded output.
 * This is the number of buffers that the application can keep
 * used and still enable MFC to encode with the hardware. */
#define RESULT_BUFFER_CNT 2

static int output_init(struct instance *i)
{
	if (i->misc.output_filename == NULL) {
		return 0;
	}
	i->misc.output_fd = fopen(i->misc.output_filename, "w+b");
	if (i->misc.output_fd == NULL) {
		printf("create output file '%s' failed\n",
		       i->misc.output_filename);
		return -1;
	}
	printf("create ouptut file '%s' OK\n", i->misc.output_filename);
	return 0;
}

static int output_deinit(struct instance *i)
{
	if (i->misc.output_fd) {
		fclose(i->misc.output_fd);
		i->misc.output_fd = NULL;
	}
	return 0;
}

void cleanup(struct instance *i)
{
	if (i->mfc.fd)
		mfc_close(i);
	if (i->in.fd)
		input_close(i);
	output_deinit(i);
}

int dequeue_output(struct instance *i, int *n)
{
	struct v4l2_buffer qbuf;

	memzero(qbuf);
	qbuf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	qbuf.memory = V4L2_MEMORY_MMAP;

	if (mfc_enc_dequeue_buf(i, &qbuf))
		return -1;

	*n = qbuf.index;

	return 0;
}

int dequeue_capture(struct instance *i, int *n, unsigned char *disp_paddr,
		    int *finished, int *used)
{
	struct v4l2_buffer qbuf;

	memzero(qbuf);
	qbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	qbuf.memory = V4L2_MEMORY_MMAP;

	if (mfc_enc_dequeue_buf(i, &qbuf)) {
		dbg("dequeue_capture failed");
		return -1;
	}

	*n = qbuf.index;
	*disp_paddr = *((unsigned char *)qbuf.timecode.userbits);
	*finished = qbuf.bytesused == 0;
	*used = qbuf.bytesused;

	return 0;
}

static pthread_t mfc_thread;
static pthread_t parser_thread;
static pthread_t daemon_thread;

/* This threads is responsible for parsing the stream and
 * feeding MFC with consecutive frames to encode */
static int s_output_stream_on = 0;
void *encode_thread_func(void *args)
{
	struct instance *i = (struct instance *)args;
	int ret;
	int used, n;

	while (!i->error && !i->finish && !i->parser.finished) {
		n = 0;
		while (n < i->mfc.out_buf_cnt && i->mfc.out_buf_flag[n])
			n++;

		if (n < i->mfc.out_buf_cnt && !i->parser.finished) {
			used =
			    STREAM_BUFFER_WIDTH * STREAM_BUFFER_HEIGHT * 3 / 2;
			if (i->in.offs == i->in.size) {
				dbg("Parser has extracted all frames");
				i->parser.finished = 1;
				break;
			}
			memcpy(i->mfc.out_buf_addr[n], i->in.p + i->in.offs,
			       used);
			DEBUG_SCAN_STEP;

			dbg("Compress frame %d of size %d: %02X %02X %02X %02X %02X %02X %02X %02X ...", n, used, i->mfc.out_buf_addr[n][0], i->mfc.out_buf_addr[n][1], i->mfc.out_buf_addr[n][2], i->mfc.out_buf_addr[n][3], i->mfc.out_buf_addr[n][4], i->mfc.out_buf_addr[n][5], i->mfc.out_buf_addr[n][6], i->mfc.out_buf_addr[n][7]);

			if (s_output_stream_on == 0 && i->in.offs > 0) {
				int ret;
				ret = mfc_stream(i, V4L2_BUF_TYPE_VIDEO_OUTPUT,
						 VIDIOC_STREAMON);
				if (ret) {
					err("Failed");
					return (void *)(-1);
				}
				s_output_stream_on = 1;
			}
			ret = mfc_enc_queue_buf_out(i, n, used);

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
	return 0;
}

/* This thread handles the CAPTURE side of MFC. it receives
 * encoded frames and queues empty buffers back to MFC.
 * Also it passes the encoded frames to FIMC, so they
 * can be processed and displayed. */
static int s_last_dequeue = -1;

void *mfc_thread_func(void *args)
{
	struct instance *i = (struct instance *)args;
	int finished, used;
	int n, size, rate;
#ifdef DEBUG
	int frame_count = 0;
#endif
	/* The thread is cancelable. */
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	/* The thread can be canceled at any time. */
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	while (!i->error && !i->finish) {
		dbg("dequeue start");
		/* Can dequeue a processed buffer */
		unsigned char disp_paddr = 0;
		if (dequeue_capture(i, &n, &disp_paddr, &finished, &used)) {
			err("Error when dequeueing CAPTURE buffer");
			i->error = 1;
			break;
		}
		dbg("dequeue end");
		i->encoded_cnt++;
		if (finished) {
			dbg("Finished extracting last frames");
			i->finish = 1;
			break;
		}
		dbg("****** encode Frames Count=%d ******", ++frame_count);
		/* print encoding progress rate */
		rate = i->in.offs / (i->in.size / 100);
		printf("\rencoding progresss [%d%%]", rate);
		fflush(stdout);

		if (i->mfc.cap_buf_addr[n][0] != MAP_FAILED) {
			char *cap_data = i->mfc.cap_buf_addr[n][0];
			size =
			    fwrite(cap_data, sizeof(char), used,
				   i->misc.output_fd);
			if (size > 0)
				dbg("write to es file success size %d", size);
		}
		if (s_last_dequeue >= 0) {
			dbg("Current display cap_buf_addr capture");
			mfc_enc_queue_buf_cap(i, s_last_dequeue);
		}
		s_last_dequeue = n;

		continue;
	}

	printf("MFC thread finished\n");
	return 0;
}

void *daemon_thread_func(void *args)
{
	const int interval = 2;
	const int run_limit = -1;	/* unit: second, -1 means run until no frame */

	int run_time = 0;
	struct instance *i = (struct instance *)args;
	static int last_encoded_cnt = 0;

	while (1) {
		sleep(interval);
		if (last_encoded_cnt == i->encoded_cnt) {
			printf("No frame encoded in last %d seconds\n",
			       interval);
			break;
		}
		last_encoded_cnt = i->encoded_cnt;

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

	printf("V4L2 Coenc encoding example application\n");

	if (parse_args(&inst, argc, argv)) {
		print_usage(argv[0]);
		return 1;
	}
	printf("input name %s inst.mfc.name %s\n", inst.in.name, inst.mfc.name);
	printf("V4L2 Coenc encoding example application1\n");
	if (input_open(&inst, inst.in.name)) {
		cleanup(&inst);
		return 1;
	}
	printf("V4L2 Coenc encoding example application2\n");
	if (mfc_open(&inst, inst.mfc.name)) {
		cleanup(&inst);
		return 1;
	}

	dbg("Successfully opened all necessary files and devices");

	if (mfc_enc_setup_output(&inst, V4L2_PIX_FMT_YUV420,
				 STREAM_BUFFER_CNT, STREAM_BUFFER_WIDTH,
				 STREAM_BUFFER_HEIGHT)) {
		cleanup(&inst);
		return 1;
	}

	if (mfc_enc_setup_capture(&inst, RESULT_BUFFER_CNT)) {
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
		if (mfc_enc_queue_buf_cap(&inst, n)) {
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

	if (output_init(&inst) != 0) {
		cleanup(&inst);
		return 1;
	}

	/* Now we're safe to run the threads */
	dbg("Launching threads");

	if (pthread_create(&parser_thread, NULL, encode_thread_func, &inst)) {
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
