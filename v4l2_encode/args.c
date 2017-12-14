 /*
 * V4L2 Codec encoding example application
 * Cai Huoqing <huoqing_cai@c-sky.com>
 *
 * Argument parser
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
#include <unistd.h>
#include <linux/videodev2.h>

#include "common.h"

#define CSKY_V4L2_ENCODE_MAJOR_NUM (1)
#define CSKY_V4L2_ENCODE_MINOR_NUM (0)

void print_usage(char *name)
{
	/* for example: ./csky_v4l2_encode_example -c h264 -o
	 * ./media/csky_encode_h264.es -i ./media/video_1280x720.yuv
	 * -m /dev/video2 -b 6700000 -g 12 -f 25 -q 30 -p 30
	 */
	printf("version: %d.%d\n", CSKY_V4L2_ENCODE_MAJOR_NUM,
	       CSKY_V4L2_ENCODE_MINOR_NUM);
	printf("Usage:\n");
	printf("\t./%s\n", name);
	printf("\t-c <codec> - The codec of the encoded stream\n");
	printf("\t\t     Available codecs: h264\n");
	printf("\t-i <file> - Input YUV420 file path name\n");
	printf("\t-m <device> - video encode device (e.g. /dev/video2)\n");
	printf("\t-V - synchronise to vsync\n");
	printf("\t-t <num> - timeout(s)\n");
	printf("\t-o <file> - Output file name (e.g. ./video.es)\n");
	printf("\t-g <num> - GOP size (e.g. 12 default:16)\n");
	printf("\t-b <num> - bitrate(bps) (e.g. 6700000)\n");
	printf("\t-f <num> - frame rate(/s) (e.g. 30 default:25)\n");
	printf("\t-q <num> - I frame picture quantity (e.g. 12~51 default:30)\n");
	printf("\t-p <num> - P frame picture quantity (e.g. 12~51 default:30)\n");
	printf("\n");
}

void init_to_defaults(struct instance *i)
{
	memset(i, 0, sizeof(*i));
}

int get_codec(char *str)
{
	if (strncasecmp("mpeg4", str, 5) == 0) {
		return V4L2_PIX_FMT_MPEG4;
	} else if (strncasecmp("h264", str, 5) == 0) {
		return V4L2_PIX_FMT_H264;
	} else if (strncasecmp("h263", str, 5) == 0) {
		return V4L2_PIX_FMT_H263;
	} else if (strncasecmp("xvid", str, 5) == 0) {
		return V4L2_PIX_FMT_XVID;
	} else if (strncasecmp("mpeg2", str, 5) == 0) {
		return V4L2_PIX_FMT_MPEG2;
	} else if (strncasecmp("mpeg1", str, 5) == 0) {
		return V4L2_PIX_FMT_MPEG1;
	}
	return 0;
}

int parse_args(struct instance *i, int argc, char **argv)
{
	int c;

	init_to_defaults(i);

	while ((c = getopt(argc, argv, "c:i:m:t:o:g:b:f:q:p:")) != -1) {
		switch (c) {
		case 'c':
			i->parser.codec = get_codec(optarg);
			break;
		case 'i':
			i->in.name = optarg;
			break;
		case 'm':
			i->mfc.name = optarg;
			break;
		case 't':
			i->misc.timeout = atoi(optarg);
			break;
		case 'o':
			i->misc.output_filename = optarg;
			break;
		case 'g':
			i->mfc.gop_size = atoi(optarg);
			break;
		case 'b':
			i->mfc.bitrate = atoi(optarg);
			break;
		case 'f':
			i->mfc.frame_rate = atoi(optarg);
			break;
		case 'q':
			i->mfc.i_pq = atoi(optarg);
			break;
		case 'p':
			i->mfc.p_pq = atoi(optarg);
			break;
		default:
			err("Bad argument");
			return -1;
		}
	}

	if (!i->in.name || !i->mfc.name) {
		err("The following arguments are required: -i -m -c -o -b");
		return -1;
	}

	if (!i->parser.codec) {
		err("Unknown or not set codec (-c)");
		return -1;
	}

	if (!i->misc.output_filename) {
		err("Unknown or not set output_filename (-o");
		return -1;
	}

	if (!i->mfc.bitrate) {
		err("Unknown or not set bitrate (-b");
		return -1;
	}

	return 0;
}
