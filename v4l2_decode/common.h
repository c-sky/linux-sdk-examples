/*
 * V4L2 Codec decoding example application
 * Lu Chongzhi <chongzhi_lu@c-sky.com>
 *
 * Common stuff header file
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

#ifndef INCLUDE_COMMON_H
#define INCLUDE_COMMON_H

#define IN
#define OUT
#define INOUT

#include <stdio.h>
#include <errno.h>
#include <semaphore.h>

#include "parser.h"
#include <linux/videodev2.h>

/* When ADD_DETAILS is defined every debug and error message contains
 * information about the file, function and line of code where it has
 * been called */
#define ADD_DETAILS
/* When DEBUG is defined debug messages are printed on the screen.
 * Otherwise only error messages are displayed. */
//#define DEBUG

#ifdef ADD_DETAILS
#define err(msg, ...) \
	fprintf(stderr, "Error (%s:%s:%d): " msg "\n", __FILE__, \
		__func__, __LINE__, ##__VA_ARGS__)
#else
#define err(msg, ...) \
	fprintf(stderr, "Error: " msg "\n", __FILE__, ##__VA_ARGS__)
#endif /* ADD_DETAILS */

#ifdef DEBUG
#ifdef ADD_DETAILS
#define dbg(msg, ...) \
	fprintf(stdout, "(%s:%s:%d): " msg "\n", __FILE__, \
		__func__, __LINE__, ##__VA_ARGS__)
#else
#define dbg(msg, ...) \
	fprintf(stdout, msg "\n", ##__VA_ARGS__)
#endif /* ADD_DETAILS */
#else /* DEBUG */
#define dbg(...) {}
#endif /* DEBUG */

#define memzero(x)\
        memset(&(x), 0, sizeof (x));

/* Maximum number of output buffers */
#define MFC_MAX_OUT_BUF 16
/* Maximum number of capture buffers (32 is the limit imposed by MFC */
#define MFC_MAX_CAP_BUF 32
/* Number of output planes */
#define MFC_OUT_PLANES 1
/* Number of capture planes */
#define MFC_CAP_PLANES 1
/* Maximum number of planes used in the application */
#define MFC_MAX_PLANES MFC_CAP_PLANES
/* Maximum number of frame buffers - used for double buffering and
 * vsyns synchronisation */
#define FB_MAX_BUFS 2

/* The buffer is free to use by MFC */
#define BUF_FREE 0
/* The buffer is currently queued in MFC */
#define BUF_MFC 1

struct instance {
	/* Input file related parameters */
	struct {
		char *name;
		int fd;
		char *p;
		int size;
		int offs;
	} in;

	/* Frame buffer related parameters */
	struct {
		char *name;
		int fd;
		char *p[FB_MAX_BUFS];
		int cur_buf;
		int buffers;
		int width;
		int height;
		int virt_width;
		int virt_height;
		int bpp;
		int stride;
		int size;
		int full_size;
		int double_buf;
	} fb;

	/* MFC related parameters */
	struct {
		char *name;
		int fd;

		/* Output queue related */
		int out_buf_cnt;
		int out_buf_size;
		int out_buf_off[MFC_MAX_OUT_BUF];
		char *out_buf_addr[MFC_MAX_OUT_BUF];
		int out_buf_flag[MFC_MAX_OUT_BUF];

		/* Capture queue related */
		int cap_w;
		int cap_h;
		int cap_crop_w;
		int cap_crop_h;
		int cap_crop_left;
		int cap_crop_top;
		int cap_buf_cnt;
		int cap_buf_cnt_min;
		int cap_buf_size[MFC_CAP_PLANES];
		int cap_buf_off[MFC_MAX_CAP_BUF][MFC_CAP_PLANES];
		char *cap_buf_addr[MFC_MAX_CAP_BUF][MFC_CAP_PLANES];
		int cap_buf_flag[MFC_MAX_CAP_BUF];
		int cap_buf_queued;
	} mfc;

	/* Parser related parameters */
	struct {
		struct mfc_parser_context ctx;
		unsigned long codec;
		/* Callback function to the real parsing function.
		 * Dependent on the codec used. */
		int (*func)( struct mfc_parser_context *ctx,
		        char* in, int in_size, char* out, int out_size,
		        int *consumed, int *frame_size, char get_head);
		/* Set when the parser has finished and end of file has
		 * been reached */
		int finished;
	} parser;

	/* Control */
	int error; /* The error flag */
	int finish;  /* Flag set when decoding has been completed and all
			threads finish */
};

extern char g_scan_char;
#endif /* INCLUDE_COMMON_H */

