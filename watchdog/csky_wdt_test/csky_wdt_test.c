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

void help_info(void);
int feed_watchdog(int fd, int mode);

int main(int argc, char **argv)
{
	int timeout, timeleft, i, fd,
	    sleep_sec, mode, feed_times, ret;

	if (argc < 3) {
		help_info();
		return 1;
	}

	timeout = atoi(argv[1]);
	sleep_sec = atoi(argv[2]);
	mode = (atoi(argv[3]) == 0) ? 0 : 1;
	feed_times = atoi(argv[4]);

	printf("start wdt (timeout: %d, sleep: %d, mode: %s, times: %d)\n",
		timeout, sleep_sec,
		(mode == 0) ? "ioctl" : "write",
		feed_times);

	fd = open("/dev/watchdog", O_WRONLY);
	if (fd == -1) {
		perror("open wdt error");
		return 1;
	}

	ret = ioctl(fd, WDIOC_SETTIMEOUT, &timeout);
	if (ret < 0)
		perror("wdt set time error");
	else
		printf("set wdt time out % seconds\n", timeout);
	ret = ioctl(fd, WDIOC_GETTIMEOUT, &timeout);
	if (ret < 0)
		perror("wdt set time error");
	else
		printf("read back wdt timeout is %d seconds\n", timeout);

	for (i = 0; i < feed_times; i ++) {
		sleep(sleep_sec);
		ret = feed_watchdog(fd, mode);
		if (ret < 0) {
			printf("feed wdt %d error", i + 1);
			break;
		}
		else
			printf("feed wdt %d over\n", i + 1);
	}

	return 0;
}

void help_info(void)
{
	printf("Usage: wdt_driver_test <timeout> <sleep> <mode>\n");
	printf("    timeout: value in seconds to cause wdt timeout/reset\n");
	printf("    sleep: value in seconds to service the wdt\n");
	printf("    mode: 0 - Service wdt with ioctl(), 1 - with write()\n");
	printf("    feed times: the times of feeding dog\n");
}

int feed_watchdog(int fd, int mode)
{
	int ret;

	if (mode == 0)
		ret = ioctl(fd, WDIOC_KEEPALIVE, 0);
	else
		ret = write(fd, "\0", 1);
	return ret;
}



