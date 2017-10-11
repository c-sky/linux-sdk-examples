/*
 * lcdc test example
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include "csky_fb_test.h"

#define COLOR_RED	0x00ff0000
#define COLOR_GREEN	0x0000ff00
#define COLOR_BLUE	0x000000ff

static struct csky_fb_test_info info;

static int pwm;
static const char *pwm_on_state = "7";
static const char *pwm_off_state = "0";

static int load_file(void *ptr, char *path)
{
	int fd;
	struct stat statbuff;
	unsigned long size;

	fd = open(path, O_RDONLY);
	if (fd < 0) {
		printf("error: open %s failed!\n", path);
		return -1;
	}

	if (stat(path, &statbuff) < 0) {
		printf("error: get stat failed!\n");
		close(fd);
		return -1;
	}

	size = statbuff.st_size;
	if (read(fd, ptr, size) != size) {
		printf("error: read file failed!\n");
		close(fd);
		return -1;
	}

	close(fd);
	return size;
}

int test_display_rectangle_32bpp(void)
{
	int tmp;
	enum csky_fb_pixel_format pixel_fmt_new;
	enum csky_fb_out_mode out_mode = info.out_mode;
	unsigned int width, height, pixel_len;
	unsigned int i, j;

	/* turn off LCD backlight */
	write(pwm, pwm_off_state, 1);

	/* reset lcdc */
	ioctl(info.fd, FBIO_WAITFORVSYNC, &tmp);
	ioctl(info.fd, FBIOBLANK, FB_BLANK_POWERDOWN);

	/* set out mode */
	ioctl(info.fd, CSKY_FBIO_SET_OUT_MODE, &out_mode);
	/* get var again. var is changed */
	if (ioctl(info.fd, FBIOGET_VSCREENINFO, &info.var)) {
		printf("error: FBIOGET_VSCREENINFO Failed\n");
		close(info.fd);
		return -1;
	}
	printf("res: %dx%d\n", info.var.xres, info.var.yres);

	/* set pixel format */
	pixel_fmt_new = CSKY_FB_PIXEL_FMT_RGB;
	ioctl(info.fd, CSKY_FBIO_SET_PIXEL_FMT, &pixel_fmt_new);

	/* draw rectangle */

	width = info.var.xres;
	height = info.var.yres;
	pixel_len = 4; /* 32bpp */

	memset(info.ptr, 0, width * height * pixel_len);

	/* draw red rectangle */
	j = 0;
	for (i = 0; i < width; i++)
		*(unsigned int *)((unsigned int)info.ptr +
				  (j * width + i) * pixel_len) = COLOR_RED;
	j = height - 1;
	for (i = 0; i < width; i++)
		*(unsigned int *)((unsigned int)info.ptr +
				  (j * width + i) * pixel_len) = COLOR_RED;
	i = 0;
	for (j = 0; j < height; j++)
		*(unsigned int *)((unsigned int)info.ptr +
				  (j * width + i) * pixel_len) = COLOR_RED;
	i = width - 1;
	for (j = 0; j < height; j++)
		*(unsigned int *)((unsigned int)info.ptr +
				  (j * width + i) * pixel_len) = COLOR_RED;

	/* init lcdc */
	ioctl(info.fd, FBIOBLANK, FB_BLANK_UNBLANK);

	/* turn on LCD backlight */
	if (out_mode == CSKY_FB_OUT_LCD_MODE)
		write(pwm, pwm_on_state, 1);

	return 0;
}

int test_display_yuv_image(void)
{
	unsigned long base = info.fix.smem_start;
	struct csky_fb_lcd_pbase_yuv base_yuv;
	enum csky_fb_pixel_format pixel_fmt_new;
	int tmp;
	int size;
	enum csky_fb_out_mode out_mode = info.out_mode;

	/* turn off LCD backlight */
	write(pwm, pwm_off_state, 1);

	/* reset lcdc */
	ioctl(info.fd, FBIO_WAITFORVSYNC, &tmp);
	ioctl(info.fd, FBIOBLANK, FB_BLANK_POWERDOWN);

	/* read image data into framebuffer */
	size = load_file(info.ptr, info.file_name);
	if (size > 0) {
		/* set out mode */
		ioctl(info.fd, CSKY_FBIO_SET_OUT_MODE, &out_mode);
		/* get var again. var is changed */
		if (ioctl(info.fd, FBIOGET_VSCREENINFO, &info.var)) {
			printf("error: FBIOGET_VSCREENINFO Failed\n");
			close(info.fd);
			return -1;
		}
		printf("res: %dx%d\n", info.var.xres, info.var.yres);

		/* set pixel format to yuv420 */
		pixel_fmt_new = CSKY_FB_PIXEL_FMT_YUV420;
		ioctl(info.fd, CSKY_FBIO_SET_PIXEL_FMT, &pixel_fmt_new);

		/* set y/u/v base address */
		base_yuv.y = base;
		base_yuv.u = base_yuv.y + info.var.xres * info.var.yres;
		base_yuv.v = base_yuv.u + info.var.xres * info.var.yres / 4;
		ioctl(info.fd, CSKY_FBIO_SET_PBASE_YUV, &base_yuv);
	}

	/* init lcdc */
	ioctl(info.fd, FBIOBLANK, FB_BLANK_UNBLANK);

	/* turn on LCD backlight */
	if (out_mode == CSKY_FB_OUT_LCD_MODE)
		write(pwm, pwm_on_state, 1);

	return 0;
}

static void print_usage(const char *prog)
{
	printf("Usage: %s [OPTION]\n", prog);
	printf(
"  -d                 framebuffer device name (default /dev/fb0)\n"
"  -f                 yuv data from a file (e.g. /media/yuv420_1280x720.yuv)\n"
"  -p --pixel-format  pixel format (rgb or yuv420, default rgb)\n"
"  --hdmi             display image via HDMI\n");
}

static int parse_args(int argc, char **argv, struct csky_fb_test_info *info)
{
	int option_index;
	int c;
	const char short_options[] = "d:f:hp:";
	const struct option long_options[] = {
		{"help", 0, 0, 'h'},
		{"pixel-format", 1, 0, 'p'},
		{"hdmi", 0, 0, OPT_HDMI},
		{0, 0, 0, 0}
	};

	/* set the default args */
	info->device_name = "/dev/fb0";
	info->out_mode = CSKY_FB_OUT_LCD_MODE;
	info->pixel_format = CSKY_FB_PIXEL_FMT_RGB;

	while ((c = getopt_long(argc, argv, short_options, long_options,
				&option_index)) != -1) {
		switch (c) {
		case 'h':
			return -1;
		case 'd':
			info->device_name = optarg;
			break;
		case 'f':
			info->file_name = optarg;
			break;
		case 'p':
			if (strcasecmp(optarg, "rgb") == 0)
				info->pixel_format = CSKY_FB_PIXEL_FMT_RGB;
			else if (strcasecmp(optarg, "yuv420") == 0)
				info->pixel_format = CSKY_FB_PIXEL_FMT_YUV420;
			else {
				printf("error: Invalid format %s\n", optarg);
				return -1;
			}
			break;
		case OPT_HDMI:
			info->out_mode = CSKY_FB_OUT_HDMI_MODE;
			break;
		default:
			return -1;
		}
	}

	return 0;
}

int main(int argc, char **argv)
{
	if (parse_args(argc, argv, &info)) {
		print_usage(argv[0]);
		return -1;
	}

	printf("%s  %s  %s\n", argv[0], __DATE__, __TIME__);

	info.fd = open(info.device_name, O_RDWR);
	if (info.fd < 0) {
		printf("error: Failed to open %s\n", info.device_name);
		return -1;
	}

	if (ioctl(info.fd, FBIOGET_VSCREENINFO, &info.var)) {
		printf("error: FBIOGET_VSCREENINFO Failed\n");
		close(info.fd);
		return -1;
	}

	if (ioctl(info.fd, FBIOGET_FSCREENINFO, &info.fix)) {
		printf("error: FBIOGET_FSCREENINFO Failed\n");
		close(info.fd);
		return -1;
	}

	info.ptr = mmap(NULL,
			info.var.yres_virtual * info.fix.line_length,
			PROT_WRITE | PROT_READ, MAP_SHARED, info.fd, 0);
	if (info.ptr == MAP_FAILED) {
		printf("error: mmap Failed\n");
		close(info.fd);
		return -1;
	}

	pwm = open("/sys/class/backlight/soc:backlight/brightness", O_RDWR);
	if (pwm < 0)
		printf("error: open pwm Failed\n");

	if (info.pixel_format == CSKY_FB_PIXEL_FMT_RGB)
		test_display_rectangle_32bpp();
	else if (info.pixel_format == CSKY_FB_PIXEL_FMT_YUV420)
		test_display_yuv_image();

	close(info.fd);
	close(pwm);
	munmap(info.ptr, info.var.yres_virtual * info.fix.line_length);
	return 0;
}
