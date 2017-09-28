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

#ifndef __CSKY_FB_TEST_H__
#define __CSKY_FB_TEST_H__

#include <stdio.h>
#include <linux/fb.h>

#define CSKY_FBIO_BASE	0x30
#define CSKY_FBIO_SET_PIXEL_FMT	_IOW('F', CSKY_FBIO_BASE+0, \
					enum csky_fb_pixel_format)
#define CSKY_FBIO_GET_PIXEL_FMT	_IOW('F', CSKY_FBIO_BASE+1, \
					enum csky_fb_pixel_format)
#define CSKY_FBIO_SET_PBASE_YUV	_IOW('F', CSKY_FBIO_BASE+2, \
					struct csky_fb_lcd_pbase_yuv)
#define CSKY_FBIO_SET_OUT_MODE	_IOW('F', CSKY_FBIO_BASE+3, \
					enum csky_fb_out_mode)


/*
 * DFS, Data format select(Storage format of YUV is planar mode)
 */
#define CSKY_LCDCON_DFS_RGB		(0 << 13)
#define CSKY_LCDCON_DFS_YUV444		(1 << 13)
#define CSKY_LCDCON_DFS_YUV422		(2 << 13)
#define CSKY_LCDCON_DFS_YUV420		(3 << 13)

enum csky_fb_pixel_format {
	CSKY_FB_PIXEL_FMT_RGB = CSKY_LCDCON_DFS_RGB,
	CSKY_FB_PIXEL_FMT_YUV444 = CSKY_LCDCON_DFS_YUV444,
	CSKY_FB_PIXEL_FMT_YUV422 = CSKY_LCDCON_DFS_YUV422,
	CSKY_FB_PIXEL_FMT_YUV420 = CSKY_LCDCON_DFS_YUV420,
};

enum csky_fb_out_mode {
	CSKY_FB_OUT_LCD_MODE = 0,
	CSKY_FB_OUT_HDMI_MODE,
};

struct csky_fb_lcd_pbase_yuv {
	unsigned int y;		/* LCD_PBASE_Y */
	unsigned int u;		/* LCD_PBASE_U */
	unsigned int v;		/* LCD_PBASE_V */
};

struct csky_fb_test_info {
	int fd;
	struct fb_var_screeninfo var;
	struct fb_fix_screeninfo fix;
	void *ptr;
};

#define MENU_EXIT		"Q"
#define MENU_LCDC_ENABLE	"L"
#define MENU_SET_PIXEL_FORMAT	"F"
#define MENU_GET_PIXEL_FORMAT	"G"
#define MENU_WAIT_FOR_VSYNC	"V"
#define MENU_PAN_DISPLAY	"P"
#define MENU_DISPLAY_YUV_IMG	"Y"
#define MENU_DISPLAY_HDMI_YUV_IMG "H"
#define MENU_DISPLAY_YUV_IMG2	"Y2"
#define MENU_DISPLAY_RECT_IMG	"R"
#define MENU_STRESS_TEST	"ST"

#endif /* __CSKY_FB_TEST_H__ */
