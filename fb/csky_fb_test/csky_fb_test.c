/*
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

#ifndef MAX_PATH
# define MAX_PATH 256
#endif
static char action[MAX_PATH];

/*
 * reads a line from stdin and stores the first word into the buffer @str
 */
char *get_first_str(char *str)
{
	char buf[MAX_PATH] = {0};

	if ((fgets(buf, sizeof(buf), stdin) == NULL) && ferror(stdin)) {
		printf("fgets error\n");
		return NULL;
	}

	sscanf(buf, "%s", str);
	return str;
}

void show_help(void)
{
	printf("Usage: csky_fb_test  path\n");
	printf("Where path = framebuffer device name, e.g. /dev/fb0\n");
}

void show_menu(void)
{
	printf("\n---------- main menu ----------\n");
	printf("%s - exit\n", MENU_EXIT);
	printf("%s - enable/disable LCDC\n", MENU_LCDC_ENABLE);
	printf("%s - set pixel format\n", MENU_SET_PIXEL_FORMAT);
	printf("%s - wait for VSYNC\n", MENU_WAIT_FOR_VSYNC);
	printf("%s - pan display(RGB only)\n", MENU_PAN_DISPLAY);
	printf("%s - display YUV image(YUV420 only)\n", MENU_DISPLAY_YUV_IMG);
	printf("%s - display rectangle(RGB only)\n", MENU_DISPLAY_RECT_IMG);
	printf("%s - Stress Test\n", MENU_STRESS_TEST);
	printf("-------------------------------\n");
	printf("Input Your Choice: ");
	return;
}

int test_lcdc_enable(void)
{
	char choice[MAX_PATH] = {0};
	int tmp;

	while (1) {
		printf("\n--- enable/disable LCDC ---\n");
		printf("e - enable lcdc\n");
		printf("d - disable lcdc\n");
		printf("q - quit\n");
		printf("Input Your Choice: ");

		get_first_str(choice);

		if (strcasecmp(choice, "q") == 0)
			break;
		else if (strcasecmp(choice, "e") == 0) {
			printf("enable lcdc\n");
			ioctl(info.fd, FBIOBLANK, FB_BLANK_UNBLANK);
		} else if (strcasecmp(choice, "d") == 0) {
			printf("disable lcdc\n");
			ioctl(info.fd, FBIO_WAITFORVSYNC, &tmp);
			ioctl(info.fd, FBIOBLANK, FB_BLANK_POWERDOWN);
		}
		else
			printf("invalid input\n");
	}

	return 0;
}

int test_set_pixel_format(void)
{
	enum csky_fb_pixel_format pixel_fmt;
	char choice[MAX_PATH] = {0};
	int tmp;

	while (1) {
		printf("\n--- set pixel format ---\n");
		printf("0 - RGB\n");
		printf("1 - YUV444\n");
		printf("2 - YUV422\n");
		printf("3 - YUV420\n");
		printf("q - quit\n");
		printf("Input Your Choice: ");

		get_first_str(choice);

		if (strcasecmp(choice, "q") == 0)
			break;
		else if (strcasecmp(choice, "0") == 0)
			pixel_fmt = CSKY_LCDCON_DFS_RGB;
		else if (strcasecmp(choice, "1") == 0)
			pixel_fmt = CSKY_LCDCON_DFS_YUV444;
		else if (strcasecmp(choice, "2") == 0)
			pixel_fmt = CSKY_LCDCON_DFS_YUV422;
		else if (strcasecmp(choice, "3") == 0)
			pixel_fmt = CSKY_LCDCON_DFS_YUV420;
		else
			printf("invalid input\n");

		/* reset lcdc */
		ioctl(info.fd, FBIO_WAITFORVSYNC, &tmp);
		ioctl(info.fd, FBIOBLANK, FB_BLANK_POWERDOWN);
		/* set pixel format */
		ioctl(info.fd, CSKY_FBIO_SET_PIXEL_FMT, &pixel_fmt);
		/* init lcdc */
		ioctl(info.fd, FBIOBLANK, FB_BLANK_UNBLANK);
	}

	return 0;
}

int test_wait_for_vsync(void)
{
	int tmp;
	char buf[MAX_PATH] = {0};
	int count;
	int i;

	printf("\n--- wait for VSYNC (at most COUNT times) ---\n");
	printf("Input the value of COUNT: ");

	if (get_first_str(buf) == NULL)
		return -1;

	count = atoi(buf);
	printf("COUNT = %d\n", count);

	for (i = 0; i < count; i++) {
		if (ioctl(info.fd, FBIO_WAITFORVSYNC, &tmp)) {
			printf("ERROR: FBIO_WAITFORVSYNC Failed\n");
			return -1;
		}
		printf("V\n");
	}
	return 0;
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
	pixel_len = 4; //32bpp

	for (y = 0; y < height; y++)
		for (x = 0; x < width; x++)
			*(unsigned int *)(addr + (y*width+x)*pixel_len) = color;

	return;
}

int test_pan_display(void)
{
	int tmp;
	char choice[MAX_PATH] = {0};
	int size = info.fix.line_length * info.var.yres;
	enum csky_fb_pixel_format pixel_fmt = CSKY_LCDCON_DFS_RGB;

	/* reset lcdc */
	ioctl(info.fd, FBIO_WAITFORVSYNC, &tmp);
	ioctl(info.fd, FBIOBLANK, FB_BLANK_POWERDOWN);
	/* set pixel format */
	ioctl(info.fd, CSKY_FBIO_SET_PIXEL_FMT, &pixel_fmt);

	set_screen_color_32bpp(info.ptr, COLOR_RED);
	set_screen_color_32bpp(info.ptr+size, COLOR_BLUE);

	/* init lcdc */
	ioctl(info.fd, FBIOBLANK, FB_BLANK_UNBLANK);

	while (1) {
		printf("\n--- pan display ---\n");
		printf("1 - show frame #1\n");
		printf("2 - show frame #2\n");
		printf("q - quit\n");
		printf("Input Your Choice: ");

		get_first_str(choice);

		if (strcasecmp(choice, "q") == 0)
			break;
		else if (strcasecmp(choice, "1") == 0) {
			printf("frame1\n");
			info.var.yoffset = 0;
			ioctl(info.fd, FBIO_WAITFORVSYNC, &tmp);
			ioctl(info.fd, FBIOPAN_DISPLAY, &info.var);
		}
		else if (strcasecmp(choice, "2") == 0) {
			printf("frame2\n");
			info.var.yoffset = info.var.yres;
			ioctl(info.fd, FBIO_WAITFORVSYNC, &tmp);
			ioctl(info.fd, FBIOPAN_DISPLAY, &info.var);
		}
		else
			printf("invalid input\n");
	}

	return 0;
}

int test_display_yuv_image(void)
{
	enum csky_fb_pixel_format pixel_fmt;
	unsigned long base = info.fix.smem_start;
	struct csky_fb_lcd_pbase_yuv base_yuv;
	int tmp;
	int size;

	/* reset lcdc */
	ioctl(info.fd, FBIO_WAITFORVSYNC, &tmp);
	ioctl(info.fd, FBIOBLANK, FB_BLANK_POWERDOWN);

	/* read image data into framebuffer */
	size = load_file(info.ptr, "/yuvtest.bin");
	if (size > 0)
	{
		/* set pixel format to yuv420 */
		pixel_fmt = CSKY_LCDCON_DFS_YUV420;
		ioctl(info.fd, CSKY_FBIO_SET_PIXEL_FMT, &pixel_fmt);

		/* set y/u/v base address */
		base_yuv.y = base;
		base_yuv.u = base_yuv.y +
			     info.var.xres * info.var.yres;
		base_yuv.v = base_yuv.u +
			     info.var.xres * info.var.yres / 4;
		ioctl(info.fd, CSKY_FBIO_SET_PBASE_YUV, &base_yuv);
	}

	/* init lcdc */
	ioctl(info.fd, FBIOBLANK, FB_BLANK_UNBLANK);
	return 0;
}

int test_display_rectangle_32bpp(void)
{
	int tmp;
	enum csky_fb_pixel_format pixel_fmt = CSKY_LCDCON_DFS_RGB;
	unsigned int width, height, pixel_len;
	unsigned int i, j;

	/* reset lcdc */
	ioctl(info.fd, FBIO_WAITFORVSYNC, &tmp);
	ioctl(info.fd, FBIOBLANK, FB_BLANK_POWERDOWN);
	/* set pixel format */
	ioctl(info.fd, CSKY_FBIO_SET_PIXEL_FMT, &pixel_fmt);

	/* draw rectangle */

	width = info.var.xres;
	height = info.var.yres;
	pixel_len = 4; //32bpp

	memset(info.ptr, 0, width*height*pixel_len);
	j = 0;
	for (i=0; i<width; i++)
		*(unsigned int *)(info.ptr + (j*width+i)*pixel_len) = COLOR_RED;
	j = height - 1;
	for (i=0; i<width; i++)
		*(unsigned int *)(info.ptr + (j*width+i)*pixel_len) = COLOR_RED;
	i = 0;
	for (j=0; j<height; j++)
		*(unsigned int *)(info.ptr + (j*width+i)*pixel_len) = COLOR_RED;
	i = width - 1;
	for (j=0; j<height; j++)
		*(unsigned int *)(info.ptr + (j*width+i)*pixel_len) = COLOR_RED;

	/* pan display to frame #1 */
	info.var.yoffset = 0;
	ioctl(info.fd, FBIO_WAITFORVSYNC, &tmp);
	ioctl(info.fd, FBIOPAN_DISPLAY, &info.var);

	/* init lcdc */
	ioctl(info.fd, FBIOBLANK, FB_BLANK_UNBLANK);
	return 0;
}

int test_stress_test(void)
{
	char choice[MAX_PATH] = {0};
	int tmp;

	while (1) {
		printf("\n--- Stress Test ---\n");
		printf("1 - RGB <-> YUV420\n");
		printf("q - quit\n");
		printf("Input Your Choice: ");

		get_first_str(choice);

		if (strcasecmp(choice, "q") == 0)
			break;
		else if (strcasecmp(choice, "1") == 0) {
			printf("\ntesting RGB <-> YUV420...\n");
			while (1) {
				test_display_yuv_image();
				sleep(1);
				test_display_rectangle_32bpp();
				sleep(1);
			}
		}
		else
			printf("invalid input\n");
	}

	return 0;
}

int do_fb_test(char *choice)
{
	int ret;

	if (strcasecmp(choice, MENU_EXIT) == 0)
		ret = EXIT_TEST;
	else if (strcasecmp(choice, MENU_LCDC_ENABLE) == 0)
		ret = test_lcdc_enable();
	else if (strcasecmp(choice, MENU_SET_PIXEL_FORMAT) == 0)
		ret = test_set_pixel_format();
	else if (strcasecmp(choice, MENU_WAIT_FOR_VSYNC) == 0)
		ret = test_wait_for_vsync();
	else if (strcasecmp(choice, MENU_PAN_DISPLAY) == 0)
		ret = test_pan_display();
	else if (strcasecmp(choice, MENU_DISPLAY_YUV_IMG) == 0)
		ret = test_display_yuv_image();
	else if (strcasecmp(choice, MENU_DISPLAY_RECT_IMG) == 0)
		ret = test_display_rectangle_32bpp();
	else if (strcasecmp(choice, MENU_STRESS_TEST) == 0)
		ret = test_stress_test();
	else {
		printf("invalid input\n");
		ret = 0;
	}

	return ret;
}

int load_file(void *ptr, char *path)
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
			PROT_WRITE | PROT_READ,
			MAP_SHARED, info.fd, 0);
	if (info.ptr == MAP_FAILED) {
		printf("ERROR: mmap Failed\n");
		close(info.fd);
		return -1;
	}

	while (1) {
		show_menu();
		get_first_str(action);
		if (do_fb_test(action) == EXIT_TEST)
			break;
	}

	close(info.fd);
	munmap(info.ptr, info.var.yres_virtual * info.fix.line_length);
	return 0;
}
