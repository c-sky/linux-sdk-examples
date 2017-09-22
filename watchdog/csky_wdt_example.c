/*
 * Watchdog test module
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
#include <fcntl.h>
#include <linux/watchdog.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "csky_common.h"

#define CSKY_WDT_MAJOR_NUM (1)
#define CSKY_WDT_MINOR_NUM (0)

static const char *const prog_name = "csky_wdt_example";

static void help_info(void);
static int feed_watchdog(int fd, int mode);

int main(int argc, char **argv)
{
	int timeout, i, fd, sleep_sec, mode, feed_times;
	int ret = 0;

	if (argc < 5) {
		help_info();
		return -1;
	}

	timeout = atoi(argv[1]);
	sleep_sec = atoi(argv[2]);
	mode = (atoi(argv[3]) == 0) ? 0 : 1;
	feed_times = atoi(argv[4]);

	printf("start wdt (timeout: %d, sleep: %d, mode: %s, times: %d)\n",
	       timeout, sleep_sec, (mode == 0) ? "ioctl" : "write", feed_times);

	fd = open("/dev/watchdog", O_WRONLY);
	if (fd == -1) {
		perror("open wdt error");
		return -1;
	}

	ret = ioctl(fd, WDIOC_SETTIMEOUT, &timeout);
	if (ret < 0) {
		perror("wdt set time error");
		goto out;
	} else {
		printf("set wdt time out %d seconds\n", timeout);
	}
	ret = ioctl(fd, WDIOC_GETTIMEOUT, &timeout);
	if (ret < 0) {
		perror("wdt get time error");
		goto out;
	} else {
		printf("read back wdt timeout is %d seconds\n", timeout);
	}

	for (i = 0; i < feed_times; i++) {
		sleep(sleep_sec);
		ret = feed_watchdog(fd, mode);
		if (ret < 0) {
			printf("feed wdt %d error\n", i + 1);
			goto out;
		} else {
			printf("feed wdt %d ok\n", i + 1);
		}
	}

out:
	if (close(fd) == -1) {
		perror("close wdt error");
		return -1;
	}

	return ret;
}

void help_info(void)
{
	printf("%s %d.%d\n", prog_name, CSKY_WDT_MAJOR_NUM, CSKY_WDT_MINOR_NUM);
	printf("Usage: %s <timeout> <sleep> <mode> <feed times>\n", prog_name);
	printf("    timeout: value in seconds to cause wdt timeout/reset\n");
	printf("    sleep: value in seconds to service the wdt\n");
	printf("    mode: 0 - Service wdt with ioctl(), 1 - with write()\n");
	printf("    feed times: the times of feeding dog\n");
}

/**
  \brief         feed wdt
  \param[in]     fd  wdt device file descriptor
  \param[in]     mode 0 is ioctl, else write
  \return err code or output parameter
  \retval -1: error
  \retval others: success
  */
int feed_watchdog(int fd, int mode)
{
	int ret;

	if (mode == 0)
		ret = ioctl(fd, WDIOC_KEEPALIVE, 0);
	else
		ret = write(fd, '\0', 1);
	return ret;
}
