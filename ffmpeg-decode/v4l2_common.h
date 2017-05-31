/*
 * Copyright (C) 2017 C-SKY Microsystems
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */
#ifndef _V4L2_COMMON_H
#define _V4L2_COMMON_H

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>

#include <linux/videodev2.h>

enum vb2_memory {
        VB2_MEMORY_UNKNOWN      = 0,
        VB2_MEMORY_MMAP         = 1,
        VB2_MEMORY_USERPTR      = 2,
        VB2_MEMORY_DMABUF       = 4,
};

/*
 *
 * Selection interface definitions
 *
 */

/* Current cropping area */
#define V4L2_SEL_TGT_CROP		0x0000
/* Default cropping area */
#define V4L2_SEL_TGT_CROP_DEFAULT	0x0001
/* Cropping bounds */
#define V4L2_SEL_TGT_CROP_BOUNDS	0x0002
/* Native frame size */
#define V4L2_SEL_TGT_NATIVE_SIZE	0x0003
/* Current composing area */
#define V4L2_SEL_TGT_COMPOSE		0x0100
/* Default composing area */
#define V4L2_SEL_TGT_COMPOSE_DEFAULT	0x0101
/* Composing bounds */
#define V4L2_SEL_TGT_COMPOSE_BOUNDS	0x0102
/* Current composing area plus all padding pixels */
#define V4L2_SEL_TGT_COMPOSE_PADDED	0x0103


#define V4L2_DEV_COUNT 4

#define V4L2_DEV_NAME_0 "/dev/video0"
#define V4L2_DEV_NAME_1 "/dev/video1"
#define V4L2_DEV_NAME_2 "/dev/video2"
#define V4L2_DEV_NAME_3 "/dev/video3"

void v4l_print_querycap(struct v4l2_capability *arg);
void v4l_print_fmtdesc(const void *arg);
void v4l_print_format(const void *arg);

#endif
