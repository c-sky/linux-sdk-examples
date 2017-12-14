/*
 * V4L2 Codec encoding example application
 * Cai Huoqing <huoqing_cai@c-sky.com>
 *
 * V4L2 print common func
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 *
 * Authors:	Alan Cox, <alan@lxorguk.ukuu.org.uk> (version 1)
 *              Mauro Carvalho Chehab <mchehab@infradead.org> (version 2)
 */

#include "v4l2_common.h"

typedef int bool;
#define pr_cont printf
#define printk printf
#define KERN_DEBUG

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))
#endif

#define prt_names(a, arr) (((unsigned)(a)) < ARRAY_SIZE(arr) ? arr[a] : "unknown")

const char *v4l2_type_names[] = {
	[0] = "0",
	[V4L2_BUF_TYPE_VIDEO_CAPTURE] = "vid-cap",
	[V4L2_BUF_TYPE_VIDEO_OVERLAY] = "vid-overlay",
	[V4L2_BUF_TYPE_VIDEO_OUTPUT] = "vid-out",
	[V4L2_BUF_TYPE_VBI_CAPTURE] = "vbi-cap",
	[V4L2_BUF_TYPE_VBI_OUTPUT] = "vbi-out",
	[V4L2_BUF_TYPE_SLICED_VBI_CAPTURE] = "sliced-vbi-cap",
	[V4L2_BUF_TYPE_SLICED_VBI_OUTPUT] = "sliced-vbi-out",
	[V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY] = "vid-out-overlay",
	[V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE] = "vid-cap-mplane",
	[V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE] = "vid-out-mplane",
	[V4L2_BUF_TYPE_SDR_CAPTURE] = "sdr-cap",
	[V4L2_BUF_TYPE_SDR_OUTPUT] = "sdr-out",
};

const char *v4l2_field_names[] = {
	[V4L2_FIELD_ANY] = "any",
	[V4L2_FIELD_NONE] = "none",
	[V4L2_FIELD_TOP] = "top",
	[V4L2_FIELD_BOTTOM] = "bottom",
	[V4L2_FIELD_INTERLACED] = "interlaced",
	[V4L2_FIELD_SEQ_TB] = "seq-tb",
	[V4L2_FIELD_SEQ_BT] = "seq-bt",
	[V4L2_FIELD_ALTERNATE] = "alternate",
	[V4L2_FIELD_INTERLACED_TB] = "interlaced-tb",
	[V4L2_FIELD_INTERLACED_BT] = "interlaced-bt",
};

void v4l_print_format(const void *arg)
{
	const struct v4l2_format *p = arg;
	const struct v4l2_pix_format *pix;
	const struct v4l2_pix_format_mplane *mp;
	const struct v4l2_vbi_format *vbi;
	const struct v4l2_sliced_vbi_format *sliced;
	const struct v4l2_window *win;
	const struct v4l2_sdr_format *sdr;
	unsigned i;

	pr_cont("type=%s", prt_names(p->type, v4l2_type_names));
	switch (p->type) {
	case V4L2_BUF_TYPE_VIDEO_CAPTURE:
	case V4L2_BUF_TYPE_VIDEO_OUTPUT:
		pix = &p->fmt.pix;
		pr_cont(", width=%u, height=%u, "
			"pixelformat=%c%c%c%c, field=%s, "
			"bytesperline=%u, \n\tsizeimage=%u, colorspace=%d, "
			"flags=0x%x, ycbcr_enc=%u, quantization=%u, "
			"xfer_func=%u\n",
			pix->width, pix->height,
			(pix->pixelformat & 0xff),
			(pix->pixelformat >> 8) & 0xff,
			(pix->pixelformat >> 16) & 0xff,
			(pix->pixelformat >> 24) & 0xff,
			prt_names(pix->field, v4l2_field_names),
			pix->bytesperline, pix->sizeimage,
			pix->colorspace, pix->flags, pix->ycbcr_enc,
			pix->quantization, pix->xfer_func);
		break;
	case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:
	case V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE:
		mp = &p->fmt.pix_mp;
		pr_cont(", width=%u, height=%u, "
			"format=%c%c%c%c, field=%s, "
			"colorspace=%d, num_planes=%u, flags=0x%x, "
			"ycbcr_enc=%u, quantization=%u, xfer_func=%u\n",
			mp->width, mp->height,
			(mp->pixelformat & 0xff),
			(mp->pixelformat >> 8) & 0xff,
			(mp->pixelformat >> 16) & 0xff,
			(mp->pixelformat >> 24) & 0xff,
			prt_names(mp->field, v4l2_field_names),
			mp->colorspace, mp->num_planes, mp->flags,
			mp->ycbcr_enc, mp->quantization, mp->xfer_func);
		for (i = 0; i < mp->num_planes; i++)
			printk(KERN_DEBUG
			       "plane %u: bytesperline=%u sizeimage=%u\n", i,
			       mp->plane_fmt[i].bytesperline,
			       mp->plane_fmt[i].sizeimage);
		break;
	case V4L2_BUF_TYPE_VIDEO_OVERLAY:
	case V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY:
		win = &p->fmt.win;
		/* Note: we can't print the clip list here since the clips
		 * pointer is a userspace pointer, not a kernelspace
		 * pointer. */
		pr_cont(", wxh=%dx%d, x,y=%d,%d, field=%s, chromakey=0x%08x, "
			"clipcount=%u, clips=%p, bitmap=%p, global_alpha=0x%02x\n",
		     	win->w.width, win->w.height, win->w.left, win->w.top,
		     	prt_names(win->field, v4l2_field_names), win->chromakey,
		     	win->clipcount, win->clips, win->bitmap,
		    	 win->global_alpha);
		break;
	case V4L2_BUF_TYPE_VBI_CAPTURE:
	case V4L2_BUF_TYPE_VBI_OUTPUT:
		vbi = &p->fmt.vbi;
		pr_cont(", sampling_rate=%u, offset=%u, samples_per_line=%u, "
			"sample_format=%c%c%c%c, start=%u,%u, count=%u,%u\n",
			vbi->sampling_rate, vbi->offset,
			vbi->samples_per_line,
			(vbi->sample_format & 0xff),
			(vbi->sample_format >> 8) & 0xff,
			(vbi->sample_format >> 16) & 0xff,
			(vbi->sample_format >> 24) & 0xff,
			vbi->start[0], vbi->start[1],
			vbi->count[0], vbi->count[1]);
		break;
	case V4L2_BUF_TYPE_SLICED_VBI_CAPTURE:
	case V4L2_BUF_TYPE_SLICED_VBI_OUTPUT:
		sliced = &p->fmt.sliced;
		pr_cont(", service_set=0x%08x, io_size=%d\n",
			sliced->service_set, sliced->io_size);
		for (i = 0; i < 24; i++)
			printk(KERN_DEBUG "line[%02u]=0x%04x, 0x%04x\n", i,
			       sliced->service_lines[0][i],
			       sliced->service_lines[1][i]);
		break;
	case V4L2_BUF_TYPE_SDR_CAPTURE:
	case V4L2_BUF_TYPE_SDR_OUTPUT:
		sdr = &p->fmt.sdr;
		pr_cont(", pixelformat=%c%c%c%c\n",
			(sdr->pixelformat >> 0) & 0xff,
			(sdr->pixelformat >> 8) & 0xff,
			(sdr->pixelformat >> 16) & 0xff,
			(sdr->pixelformat >> 24) & 0xff);
		break;
	}
}
