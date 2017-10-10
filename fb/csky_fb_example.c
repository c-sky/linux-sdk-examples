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

#define EXIT_TEST 0xEEEE

static struct csky_fb_test_info info;
static int pwm;
static const char *pwm_on_state = "7";
static const char *pwm_off_state = "0";


#ifndef MAX_PATH
#define MAX_PATH 256
#endif
static char action[MAX_PATH];

static int load_file(void *ptr, char *path);

#define CSKY_FB_MAJOR_NUM (1)
#define CSKY_FB_MINOR_NUM (0)

/*
 * reads a line from stdin and stores the first word into the buffer @str
 */
char *get_first_str(char *str)
{
	char buf[MAX_PATH] = { 0 };

	if ((fgets(buf, sizeof(buf), stdin) == NULL) && ferror(stdin)) {
		printf("fgets error\n");
		return NULL;
	}

	sscanf(buf, "%s", str);
	return str;
}

void show_help(void)
{
	printf("version: %d.%d\n", CSKY_FB_MAJOR_NUM, CSKY_FB_MINOR_NUM);
	printf("Usage: csky_fb_example path\n");
	printf("Where path = framebuffer device name, e.g. /dev/fb0\n");
}

void show_menu(void)
{
	printf("\n---------- main menu ----------\n");
	printf("%s - exit\n", MENU_EXIT);
	printf("%s - display red rectangle(RGB only)\n", MENU_DISPLAY_RECT_IMG);
	printf("%s - display YUV image(YUV420 only)\n", MENU_DISPLAY_YUV_IMG);
	printf("%s - display image via HDMI(YUV420 only)\n",
	       MENU_DISPLAY_HDMI_YUV_IMG);
	printf("-------------------------------\n");
	printf("Input Your Choice: ");
	return;
}

#define COLOR_RED	0x00ff0000
#define COLOR_GREEN	0x0000ff00
#define COLOR_BLUE	0x000000ff

void set_screen_color_32bpp(void *ptr, unsigned int color)
{
	unsigned int addr = (unsigned int)ptr;
	unsigned int x, y;
	unsigned int width, height, pixel_len;

	width = info.var.xres;
	height = info.var.yres;
	pixel_len = 4;		//32bpp

	for (y = 0; y < height; y++)
		for (x = 0; x < width; x++)
			*(unsigned int *)(addr + (y * width + x) * pixel_len) =
			    color;

	return;
}

int test_display_yuv_image(enum csky_fb_out_mode out_mode)
{
	enum csky_fb_pixel_format pixel_fmt_new;
	unsigned long base = info.fix.smem_start;
	struct csky_fb_lcd_pbase_yuv base_yuv;
	int tmp;
	int size;

	/* reset lcdc */
	ioctl(info.fd, FBIO_WAITFORVSYNC, &tmp);
	ioctl(info.fd, FBIOBLANK, FB_BLANK_POWERDOWN);

	/* read image data into framebuffer */
	size = load_file(info.ptr, (out_mode == CSKY_FB_OUT_LCD_MODE) ?
			 "../../media/yuv420_800x480.yuv" :
			 "../../media/yuv420_1280x720.yuv");
	if (size > 0) {
		/* control LCD backlight */
		write(pwm, (out_mode == CSKY_FB_OUT_HDMI_MODE) ?
		      pwm_off_state : pwm_on_state, 1);

		/* set out mode */
		ioctl(info.fd, CSKY_FBIO_SET_OUT_MODE, &out_mode);
		/* get var again. var is changed */
		if (ioctl(info.fd, FBIOGET_VSCREENINFO, &info.var)) {
			printf("ERROR: FBIOGET_VSCREENINFO Failed\n");
			close(info.fd);
			return -1;
		}

		/* set pixel format to yuv420 */
		pixel_fmt_new = CSKY_LCDCON_DFS_YUV420;
		ioctl(info.fd, CSKY_FBIO_SET_PIXEL_FMT, &pixel_fmt_new);

		/* set y/u/v base address */
		base_yuv.y = base;
		base_yuv.u = base_yuv.y + info.var.xres * info.var.yres;
		base_yuv.v = base_yuv.u + info.var.xres * info.var.yres / 4;
		ioctl(info.fd, CSKY_FBIO_SET_PBASE_YUV, &base_yuv);
	}

	/* init lcdc */
	ioctl(info.fd, FBIOBLANK, FB_BLANK_UNBLANK);

	return 0;
}

int test_display_rectangle_32bpp(void)
{
	int tmp;
	enum csky_fb_pixel_format pixel_fmt_new = CSKY_LCDCON_DFS_RGB;
	enum csky_fb_out_mode out_mode;
	unsigned int width, height, pixel_len;
	unsigned int i, j;

	/* reset lcdc */
	ioctl(info.fd, FBIO_WAITFORVSYNC, &tmp);
	ioctl(info.fd, FBIOBLANK, FB_BLANK_POWERDOWN);

	/* open LCD backlight */
	write(pwm, pwm_on_state, 1);

	/* set out mode to LCD */
	out_mode = CSKY_FB_OUT_LCD_MODE;
	ioctl(info.fd, CSKY_FBIO_SET_OUT_MODE, &out_mode);
	/* get var again. var is changed */
	if (ioctl(info.fd, FBIOGET_VSCREENINFO, &info.var)) {
		printf("ERROR: FBIOGET_VSCREENINFO Failed\n");
		close(info.fd);
		return -1;
	}

	/* set pixel format */
	ioctl(info.fd, CSKY_FBIO_SET_PIXEL_FMT, &pixel_fmt_new);

	/* draw rectangle */

	width = info.var.xres;
	height = info.var.yres;
	pixel_len = 4; //32bpp

	memset(info.ptr, 0, width * height * pixel_len);
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
	return 0;
}

int do_fb_test(char *choice)
{
	int ret;

	if (strcasecmp(choice, MENU_EXIT) == 0)
		ret = EXIT_TEST;
	else if (strcasecmp(choice, MENU_DISPLAY_YUV_IMG) == 0)
		ret = test_display_yuv_image(CSKY_FB_OUT_LCD_MODE);
	else if (strcasecmp(choice, MENU_DISPLAY_HDMI_YUV_IMG) == 0)
		ret = test_display_yuv_image(CSKY_FB_OUT_HDMI_MODE);
	else if (strcasecmp(choice, MENU_DISPLAY_RECT_IMG) == 0)
		ret = test_display_rectangle_32bpp();
	else {
		printf("invalid input\n");
		ret = 0;
	}

	return ret;
}

static int load_file(void *ptr, char *path)
{
	int fd;
	struct stat statbuff;
	unsigned long size;

	fd = open(path, O_RDONLY);
	if (fd < 0) {
		printf("error! open %s failed!\n", path);
		return -1;
	}

	if (stat(path, &statbuff) < 0) {
		printf("error! get stat failed!\n");
		close(fd);
		return -1;
	}

	size = statbuff.st_size;
	/* printf("file size: %d bytes\n", size); */

	if (read(fd, ptr, size) != size) {
		printf("error! read file failed!\n");
		close(fd);
		return -1;
	}
	close(fd);
	return size;
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		show_help();
		return 0;
	}

	printf("csky_fb_test  %s  %s\n", __DATE__, __TIME__);

	info.fd = open(argv[1], O_RDWR);
	if (info.fd < 0) {
		printf("ERROR: Failed to open %s\n", argv[1]);
		return -1;
	}

	if (ioctl(info.fd, FBIOGET_VSCREENINFO, &info.var)) {
		printf("ERROR: FBIOGET_VSCREENINFO Failed\n");
		close(info.fd);
		return -1;
	}
	if (ioctl(info.fd, FBIOGET_FSCREENINFO, &info.fix)) {
		printf("ERROR: FBIOGET_FSCREENINFO Failed\n");
		close(info.fd);
		return -1;
	}

	printf("fb res %dx%d virtual %dx%d, line_len %d, bpp %d\n",
	       info.var.xres, info.var.yres,
	       info.var.xres_virtual, info.var.yres_virtual,
	       info.fix.line_length, info.var.bits_per_pixel);

	info.ptr = mmap(NULL,
			info.var.yres_virtual * info.fix.line_length,
			PROT_WRITE | PROT_READ, MAP_SHARED, info.fd, 0);
	if (info.ptr == MAP_FAILED) {
		printf("ERROR: mmap Failed\n");
		close(info.fd);
		return -1;
	}
	pwm = open("/sys/class/backlight/soc:backlight/brightness", O_RDWR);
	if (pwm < 0)
		printf("ERROR: open pwm Failed\n");

	while (1) {
		show_menu();
		get_first_str(action);
		if (do_fb_test(action) == EXIT_TEST)
			break;
	}

	close(info.fd);
	close(pwm);
	munmap(info.ptr, info.var.yres_virtual * info.fix.line_length);
	return 0;
}
