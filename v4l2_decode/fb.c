/*
 * V4L2 Codec decoding example application
 * Lu Chongzhi <chongzhi_lu@c-sky.com>
 *
 * Framebuffer operations
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

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <linux/fb.h>

#include "common.h"
#include "fb.h"

int fb_open(struct instance *i, char *name)
{
	struct fb_var_screeninfo fbinfo;
	int ret;

	i->fb.fd = open(name, O_RDWR);
	if (i->fb.fd < 0) {
		err("Failed to open frame buffer: %s", name);
		return -1;
	}

	ret = ioctl(i->fb.fd, FBIOGET_VSCREENINFO, &fbinfo);
	if (ret != 0) {
		err("Failed to get frame buffer properties");
		return -1;
	}
	dbg("Framebuffer properties: xres=%d, yres=%d, bpp=%d",
		fbinfo.xres, fbinfo.yres, fbinfo.bits_per_pixel);
	dbg("Virtual resolution: vxres=%d vyres=%d",
		fbinfo.xres_virtual, fbinfo.yres_virtual);

	i->fb.width		= fbinfo.xres;
	i->fb.height		= fbinfo.yres;
	i->fb.virt_width	= fbinfo.xres_virtual;
	i->fb.virt_height	= fbinfo.yres_virtual;
	i->fb.bpp		= fbinfo.bits_per_pixel;
	i->fb.stride		= i->fb.virt_width * i->fb.bpp / 8;
	i->fb.full_size		= i->fb.stride * i->fb.virt_height;
	i->fb.size		= i->fb.stride * fbinfo.yres;

	if (ioctl(i->fb.fd, FBIOBLANK, FB_BLANK_POWERDOWN) < 0) {
		dbg("set fb power down failed");
		return -1;
	}
	dbg("set fb power down OK");

	enum csky_fb_pixel_format pixel_fmt_new;
	pixel_fmt_new = CSKY_LCDCON_DFS_YUV420;
	if (ioctl(i->fb.fd, CSKY_FBIO_SET_PIXEL_FMT, &pixel_fmt_new) < 0) {
		dbg("set fb fmt failed");
		return -1;
	}
	dbg("set fb fmt to be CSKY_LCDCON_DFS_YUV420 OK");
	fb_power_on(i);

	return 0;
}

static int s_fb_is_on = 0;
int fb_power_on(struct instance *i)
{
	if (s_fb_is_on) {
		return 0;
	}

	if (ioctl(i->fb.fd, FBIOBLANK, FB_BLANK_UNBLANK) < 0) {
		dbg("set fb power on failed");
		return -1;
	}

	dbg("set fb power on OK");
	s_fb_is_on = 1;
	return 0;
}

int fb_wait_for_vsync(struct instance *i)
{
	unsigned long temp;

	if (ioctl(i->fb.fd, FBIO_WAITFORVSYNC, &temp) < 0) {
		err("Wait for vsync failed");
		return -1;
	}
	return 0;
}

void fb_close(struct instance *i)
{
	close(i->fb.fd);
}

