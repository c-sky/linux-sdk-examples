/*
 * RTC test include file
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

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <linux/rtc.h>
#include <sys/select.h>

/**************************************************************************
 * Macro Defination
 * **********************************************************************/
#define CSKY_RTC_MAJOR_NUM 1
#define CSKY_RTC_MINOR_NUM 0

/****************************************************************************
 * Public Functions
 ****************************************************************************/
int read_rtc(int rtcfd, struct rtc_time *time);
int write_rtc(int rtcfd, struct rtc_time time);
int alarm_rtc(int rtcfd, struct rtc_time time);
int check_datetime(const char *datetime, struct rtc_time *time);
int compare_datetime(struct rtc_time time0, struct rtc_time time1);
