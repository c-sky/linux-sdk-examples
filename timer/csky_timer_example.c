/*
 * Timer test module
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
#include "csky_timer_test.h"

static char *prog_name = "csky_timer_example\0";
static void help_info(void);
/**
  \brief        Callback function
  */
static void timer_expiration(union sigval v)
{
	printf("Timer has expired.\n");
}

int main(int argc, char *argv[])
{
	struct sigevent notify;
	int ret, i;
	int timercount, timeout;
	struct itimerspec timespec, getspec;

	if (argc < 3) {
		help_info();
		return -1;
	}

	/* Make sure timer count value is an integer */
	timercount = atoi(argv[1]);
	if (timercount <= 0) {
		help_info();
		return -1;
	}

	/* Make sure timeout value is an integer */
	if ((timeout = atoi(argv[2])) <= 0) {
		help_info();
		return -1;
	}

	timer_t timer[timercount];

	memset(&notify, 0, sizeof(notify));
	notify.sigev_value.sival_int = 3;
	notify.sigev_notify = SIGEV_THREAD;
	notify.sigev_notify_function = timer_expiration;

	for (i = 0; i < timercount; i++) {
		/* Create timer */
		ret = timer_create(CLOCK_REALTIME, &notify, &timer[i]);
		if (ret != 0) {
			printf("Timer creating failed\n");
			return -1;
		}

		/* Set timeout value */

		timespec.it_value.tv_sec = timeout;
		timespec.it_value.tv_nsec = 0;
		timespec.it_interval.tv_sec = timeout;
		timespec.it_interval.tv_nsec = 0;

		/* Set timeout to start timer */
		ret = timer_settime(timer[i], CLOCK_REALTIME, &timespec, NULL);
		if (ret != 0) {
			printf("Timeout setting failed\n");
			return -2;
		}

	}

	sleep(timeout / 2);

	/* Get Left time value */
	ret = timer_gettime(timer[0], &getspec);
	printf("There are still %ld seconds to wait.\n",
	       getspec.it_value.tv_sec);
	if (timeout % 2 == 0)
		sleep(timeout / 2);
	else
		sleep(timeout / 2 + 1);

	printf("All the timers should timeout\n");

	sleep(2);
	/* Delete timer */
	for (i = 0; i < timercount; i++) {
		ret = timer_delete(timer[i]);
		if (ret != 0) {
			printf("Timer delete failed\n");
			return -1;
		}
	}

	return 0;
}

/**
  \brief        Display help information
  */
static void help_info(void)
{
	printf("%s %d.%d\n", prog_name, CSKY_TIMER_MAJOR_NUM,
	       CSKY_TIMER_MINOR_NUM);
	printf("Usage: %s timercount timeout \n", prog_name);
	printf("timercount: how many timer you need to create\n");
	printf("timeout: timeout value.\n");
}
