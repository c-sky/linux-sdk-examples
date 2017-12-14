/*
 * V4L2 Codec encoding example application
 * Cai Huoqing <huoqing_cai@c-sky.com>
 *
 * MFC operations
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

#include <linux/videodev2.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "common.h"
#include "mfc.h"
#include "v4l2_common.h"

#define DEBUG_SCAN_STEP		//dbg("press any key to continue:");scanf("%c", &g_scan_char);

static char *dbg_type[2] = { "OUTPUT", "CAPTURE" };
static char *dbg_status[2] = { "ON", "OFF" };

int mfc_open(struct instance *i, char *name)
{
	struct v4l2_capability cap;
	int ret;

	i->mfc.fd = open(name, O_RDWR, 0);
	if (i->mfc.fd < 0) {
		err("Failed to open MFC: %s", name);
		return -1;
	}

	memzero(cap);
	ret = ioctl(i->mfc.fd, VIDIOC_QUERYCAP, &cap);
	if (ret != 0) {
		err("Failed to verify capabilities");
		return -1;
	}

	dbg("MFC Info (%s): driver=\"%s\" bus_info=\"%s\" card=\"%s\" fd=0x%x",
	    name, cap.driver, cap.bus_info, cap.card, i->mfc.fd);

	if (!(cap.capabilities & V4L2_CAP_VIDEO_M2M) ||
	    !(cap.capabilities & V4L2_CAP_STREAMING)) {
		err("Insufficient capabilities of MFC device (is %s correct?)",
		    name);
		return -1;
	}

	return 0;
}

void mfc_close(struct instance *i)
{
	close(i->mfc.fd);
}

int mfc_enc_setup_output(struct instance *i, unsigned long codec,
			 unsigned int count, int width, int height)
{
	struct v4l2_format fmt;
	struct v4l2_control ctrl;
	struct v4l2_streamparm stream_parm;
	struct v4l2_requestbuffers reqbuf;
	struct v4l2_buffer buf;
	int ret;
	int n;

	memzero(fmt);

	dbg("Get MFC encoding");
	fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	fmt.fmt.pix.pixelformat = codec;
	ret = ioctl(i->mfc.fd, VIDIOC_G_FMT, &fmt);
	if (ret != 0) {
		err
		    ("Failed to Get OUTPUT(mfc.fd=%d, pixformat=0x%08x = '%c%c%c%c') "
		     "for MFC decoding, ret=%d, errno=%d", i->mfc.fd,
		     (unsigned int)codec, (char)(codec & 0xff),
		     (char)((codec & 0x0000ff00) >> 8),
		     (char)((codec & 0x00ff0000) >> 16), (char)(codec >> 24),
		     ret, errno);
		return -1;
	}
	fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	fmt.fmt.pix.pixelformat = codec;
	fmt.fmt.pix.sizeimage = width * height * 3 / 2;
	fmt.fmt.pix.width = width;
	fmt.fmt.pix.height = height;

	ret = ioctl(i->mfc.fd, VIDIOC_S_FMT, &fmt);
	if (ret != 0) {
		err("Failed to setup OUTPUT for MFC encoding, errno=%d", errno);
		return -1;
	}

	/* set bitrate kbps */
	if (i->mfc.bitrate > 0) {
		ctrl.id = V4L2_CID_MPEG_VIDEO_BITRATE;
		ctrl.value = i->mfc.bitrate;
		ret = ioctl(i->mfc.fd, VIDIOC_S_CTRL, &ctrl);
		if (ret != 0) {
			err("Failed to set bitrate OUTPUT "
			    "for MFC encoding, errno=%d", errno);
			return -1;
		}
	}

	/* set gop size */
	if (i->mfc.gop_size > 0) {
		ctrl.id = V4L2_CID_MPEG_VIDEO_GOP_SIZE;
		ctrl.value = i->mfc.gop_size;
		ret = ioctl(i->mfc.fd, VIDIOC_S_CTRL, &ctrl);
		if (ret != 0) {
			err("Failed to set GOP OUTPUT "
			    "for MFC encoding, errno=%d", errno);
			return -1;
		}
	}

	/* set I frame picture quantity */
	if (i->mfc.i_pq > 0) {
		ctrl.id = V4L2_CID_MPEG_VIDEO_H264_I_FRAME_QP;
		ctrl.value = i->mfc.i_pq;
		ret = ioctl(i->mfc.fd, VIDIOC_S_CTRL, &ctrl);
		if (ret != 0) {
			err("Failed to set I-frame QP OUTPUT "
			    "for MFC encoding, errno=%d", errno);
			return -1;
		}
	}

	/* set P frame picture quantity */
	if (i->mfc.p_pq > 0) {
		ctrl.id = V4L2_CID_MPEG_VIDEO_H264_P_FRAME_QP;
		ctrl.value = i->mfc.p_pq;
		ret = ioctl(i->mfc.fd, VIDIOC_S_CTRL, &ctrl);
		if (ret != 0) {
			err("Failed to set P-frame QP OUTPUT "
			    "for MFC encoding, errno=%d", errno);
			return -1;
		}
	}

	/* set frame rate */
	if (i->mfc.frame_rate > 0) {
		memset(&stream_parm, 0, sizeof(struct v4l2_streamparm));
		stream_parm.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
		stream_parm.parm.output.timeperframe.denominator = i->mfc.frame_rate;
		stream_parm.parm.output.timeperframe.numerator = 1;
		ret = ioctl(i->mfc.fd, VIDIOC_S_PARM, &stream_parm);
		if (ret != 0) {
			err("Failed to set timeperframe OUTPUT "
			    "for MFC encoding, errno=%d", errno);
			return -1;
		}
	}

	dbg("Setup MFC encoding OUTPUT buffer size=%u (requested=%u)",
	    fmt.fmt.pix_mp.plane_fmt[0].sizeimage, fmt.fmt.pix.sizeimage);

	i->mfc.out_buf_size = fmt.fmt.pix.sizeimage;

	memzero(reqbuf);
	reqbuf.count = count;
	reqbuf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	reqbuf.memory = V4L2_MEMORY_MMAP;

	ret = ioctl(i->mfc.fd, VIDIOC_REQBUFS, &reqbuf);
	if (ret != 0) {
		err("REQBUFS failed on OUTPUT queue of MFC");
		return -1;
	}
	i->mfc.out_buf_cnt = reqbuf.count;
	v4l_print_format(&fmt);
	dbg("Number of MFC OUTPUT buffers is %d (requested %d)",
	    i->mfc.out_buf_cnt, count);

	for (n = 0; n < i->mfc.out_buf_cnt; n++) {
		memzero(buf);
		buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = n;

		ret = ioctl(i->mfc.fd, VIDIOC_QUERYBUF, &buf);
		if (ret != 0) {
			err("QUERYBUF failed on OUTPUT buffer of MFC");
			return -1;
		}

		i->mfc.out_buf_off[n] = buf.m.offset;
		dbg("buf.length=%d frame =%d, i->mfc.fd=%d, buf.m.offset=%d",
		    buf.length, n, i->mfc.fd, buf.m.offset);

		i->mfc.out_buf_addr[n] = mmap(NULL, buf.length,
					      PROT_READ | PROT_WRITE,
					      MAP_SHARED, i->mfc.fd,
					      buf.m.offset);

		if (i->mfc.out_buf_addr[n] == MAP_FAILED) {
			err("Failed to MMAP MFC OUTPUT buffer");
			return -1;
		}

		i->mfc.out_buf_flag[n] = 0;
	}

	dbg("Succesfully mmapped %d MFC OUTPUT buffers", n);

	return 0;
}

int mfc_enc_queue_buf(struct instance *i, int n, int l1, int l2, int type,
		      int nplanes)
{
	struct v4l2_buffer qbuf;
	int ret;

	memzero(qbuf);
	qbuf.type = type;
	qbuf.memory = V4L2_MEMORY_MMAP;
	qbuf.index = n;
	qbuf.bytesused = l1;

	ret = ioctl(i->mfc.fd, VIDIOC_QBUF, &qbuf);

	if (ret) {
		err("Failed to queue buffer (index=%d) on %s", n,
		    dbg_type[type == V4L2_BUF_TYPE_VIDEO_CAPTURE]);
		return -1;
	}

	return 0;
}

int mfc_enc_queue_buf_out(struct instance *i, int n, int length)
{
	if (n >= i->mfc.out_buf_cnt) {
		err("Tried to queue a non exisiting buffer");
		return -1;
	}
	return mfc_enc_queue_buf(i, n, length, 0,
				 V4L2_BUF_TYPE_VIDEO_OUTPUT, MFC_OUT_PLANES);
}

int mfc_enc_queue_buf_cap(struct instance *i, int n)
{
	if (n >= i->mfc.cap_buf_cnt) {
		err("Tried to queue a non exisiting buffer");
		return -1;
	}
	dbg("mfc_enc_queue_buf_cap i->mfc.cap_buf_size[0]is %d",
	    i->mfc.cap_buf_size[0]);
	return mfc_enc_queue_buf(i, n, i->mfc.cap_buf_size[0], 0,
				 V4L2_BUF_TYPE_VIDEO_CAPTURE, MFC_CAP_PLANES);
}

int mfc_enc_dequeue_buf(struct instance *i, struct v4l2_buffer *qbuf)
{
	int ret;
	ret = ioctl(i->mfc.fd, VIDIOC_DQBUF, qbuf);
	if (ret) {
		err("Failed to dequeue '%s' buffer",
		    dbg_type[qbuf->type == V4L2_BUF_TYPE_VIDEO_CAPTURE]);
		return -1;
	}

	return 0;
}

int mfc_stream(struct instance *i, enum v4l2_buf_type type, int status)
{
	int ret;

	ret = ioctl(i->mfc.fd, status, &type);
	if (ret) {
		err("Failed to change streaming on MFC (type=%s, status=%s)"
		    "ret=%d, errno=%d",
		    dbg_type[type == V4L2_BUF_TYPE_VIDEO_CAPTURE],
		    dbg_status[status == VIDIOC_STREAMOFF], ret, errno);
		return -1;
	}

	dbg("Stream %s on %s queue", dbg_status[status == VIDIOC_STREAMOFF],
	    dbg_type[type == V4L2_BUF_TYPE_VIDEO_CAPTURE]);

	return 0;
}

int mfc_enc_setup_capture(struct instance *i, int count)
{
	struct v4l2_format fmt;
	struct v4l2_requestbuffers reqbuf;
	struct v4l2_buffer buf;
	int ret;
	int n;

	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.pixelformat = i->parser.codec;
	ret = ioctl(i->mfc.fd, VIDIOC_G_FMT, &fmt);
	if (ret) {
		err("Failed to read format (after parsing header)");
		return -1;
	}
	fmt.fmt.pix.pixelformat = i->parser.codec;
	fmt.fmt.pix.sizeimage = STREAM_BUFFER_WIDTH *
				STREAM_BUFFER_HEIGHT * 3 / 2;
	fmt.fmt.pix.width = STREAM_BUFFER_WIDTH;
	fmt.fmt.pix.height = STREAM_BUFFER_HEIGHT;
	ret = ioctl(i->mfc.fd, VIDIOC_S_FMT, &fmt);
	if (ret) {
		err("Failed to set format (after parsing header)");
		return -1;
	}
	DEBUG_SCAN_STEP;

	i->mfc.cap_w = fmt.fmt.pix.width;
	i->mfc.cap_h = fmt.fmt.pix.height;
	i->mfc.cap_buf_size[0] = fmt.fmt.pix.sizeimage;
	i->mfc.cap_buf_cnt = count;
	i->mfc.cap_buf_cnt_min = count;
	i->mfc.cap_buf_queued = 0;
	dbg("MFC buffer parameters: %dx%d plane[0]=%d",
	    fmt.fmt.pix.width, fmt.fmt.pix.height, i->mfc.cap_buf_size[0]);

	DEBUG_SCAN_STEP;

	memzero(reqbuf);
	reqbuf.count = i->mfc.cap_buf_cnt;
	reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	reqbuf.memory = V4L2_MEMORY_MMAP;

	ret = ioctl(i->mfc.fd, VIDIOC_REQBUFS, &reqbuf);
	if (ret != 0) {
		err("REQBUFS failed on CAPTURE queue of MFC");
		return -1;
	}

	dbg("Number of MFC CAPTURE buffers is %d", reqbuf.count);

	DEBUG_SCAN_STEP;

	i->mfc.cap_buf_cnt = reqbuf.count;

	for (n = 0; n < i->mfc.cap_buf_cnt; n++) {
		memzero(buf);
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = n;

		ret = ioctl(i->mfc.fd, VIDIOC_QUERYBUF, &buf);
		if (ret != 0) {
			err("QUERYBUF failed on CAPTURE buffer of MFC");
			return -1;
		}

		i->mfc.cap_buf_off[n][0] = buf.m.offset;
		i->mfc.cap_buf_addr[n][0] = mmap(NULL, buf.length,
						 PROT_READ | PROT_WRITE,
						 MAP_SHARED, i->mfc.fd,
						 buf.m.offset);

		if (i->mfc.cap_buf_addr[n][0] == MAP_FAILED) {
			err("Failed to MMAP MFC CAPTURE buffer");
			return -1;
		}
	}
	DEBUG_SCAN_STEP;

	dbg("Succesfully mmapped %d MFC CAPTURE buffers", n);

	return 0;
}
