/*
 * RTC test module
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

#include "csky_rtc_test.h"

static void help_info(void);
static const char *prog_name = "csky_rtc_example\0";

int main(int argc, char *argv[])
{
	int rtcfd;
	if (argc < 3) {
		help_info();
		return -1;
	}

	rtcfd = open(argv[2], O_RDWR);

	if (rtcfd < 0) {
		printf("RTC open failed\n");
		return -1;
	}

	struct rtc_time time;

	if (strcmp(argv[1], "read") == 0 && argc >= 3)
		read_rtc(rtcfd, &time);

	else if (strcmp(argv[1], "write") == 0 && argc >= 4) {
		if (check_datetime(argv[3], &time) == 0)
			write_rtc(rtcfd, time);
		else {
			help_info();
			goto EXIT;
		}
	}

	else if (strcmp(argv[1], "alarm") == 0 && argc >= 4) {
		if (check_datetime(argv[3], &time) == 0)
			alarm_rtc(rtcfd, time);
		else {
			help_info();
			goto EXIT;
		}
	}

	else {
		help_info();
		goto EXIT;
	}

EXIT:
	close(rtcfd);
	return 0;
}

/**
  \brief       Read Realtime Clock data from RTC device
  \param[in]   rtcfd   RTC file description
  \param[out]  time    RTC time to be read
  \return  0:success ; others: error
  */
int read_rtc(int rtcfd, struct rtc_time *time)
{
	int ret = ioctl(rtcfd, RTC_RD_TIME, time);
	if (ret < 0) {
		printf("RTC read failed\n");
		return ret;
	}

	printf("Now it is Year %d ", time->tm_year + 1900);
	printf("Month %d ", time->tm_mon + 1);
	printf("Day %d ", time->tm_mday);
	printf("Hour %d ", time->tm_hour);
	printf("Minute %d ", time->tm_min);
	printf("Second %d ", time->tm_sec);
	printf("\n");

	return ret;
}

/**
  \brief       Write Realtime Clock data to RTC device
  \param[in]   rtcfd   RTC file description
  \param[in]   time    Realtime clock value to be set
  \return  0:success ; others: error
  */
int write_rtc(int rtcfd, struct rtc_time time)
{
	int ret;
	ret = ioctl(rtcfd, RTC_SET_TIME, &time);
	if (ret == -1)
		printf("RTC set failed\n");
	return ret;

}

/**
  \brief       RTC Alarm function
  \param[in]   rtcfd   RTC file description
  \param[in]   time    Alarm time value to be set
  \return  0:success ; others: error
  */
int alarm_rtc(int rtcfd, struct rtc_time time)
{
	struct rtc_time now;
	int intervals, data;
	ioctl(rtcfd, RTC_RD_TIME, &now);

	int ret = ioctl(rtcfd, RTC_ALM_SET, &time);
	if (ret < 0) {
		printf("Alarm setting failed\n");
		return ret;
	}

	ret = ioctl(rtcfd, RTC_AIE_ON, 0);
	if (ret < 0) {
		printf("Alarm interrupt was failed to turn on\n");
		return ret;
	}

	intervals = compare_datetime(time, now);
	if (intervals < 0) {
		printf
		    ("Please don't provide a datetime over a half day or a passed datetime\n");
		return -1;
	}
	sleep(intervals + 1);

	ret = read(rtcfd, &data, sizeof(unsigned long));
	if (ret < 0) {
		printf("Data read failed\n");
		return -1;
	}

	printf("Alarm rang!\n");
	ret = ioctl(rtcfd, RTC_AIE_OFF, 0);
	if (ret < 0) {
		printf("Alarm interrupt was failed to turn off\n");
		return ret;
	}

	return 0;
}

/**
  \brief       Check if the third parameter given is a valid datetime
  \param[in]   datetime   datetime string
  \param[out]  time       rtc_time needs to be transferred
  \return  0:success ; others: error
  */
int check_datetime(const char *datetime, struct rtc_time *time)
{
	int i, tmpint;

	if (strlen(datetime) < 19) {
		printf("datetime format：2017-01-01_15:23:45");
	}

	char tmpyear[5];
	char tmpdata[3];
	tmpyear[4] = '\0';
	tmpdata[2] = '\0';

	/* set year */
	for (i = 0; i < 4; i++)
		tmpyear[i] = datetime[i];

	tmpint = atoi(tmpyear);
	if (tmpint < 1970 || tmpint > 2038) {
		printf("Invalid year given, year range [1970,2038]");
		printf("datetime format：2017-01-01_15:23:45");
		return -1;
	}

	time->tm_year = tmpint - 1900;
	/* set month */

	tmpdata[0] = datetime[5];
	tmpdata[1] = datetime[6];

	tmpint = atoi(tmpdata);
	if (tmpint < 1 || tmpint > 12) {
		printf("Invalid month given, month range [1,12]");
		printf("datetime format：2017-01-01_15:23:45");
		return -2;
	}
	time->tm_mon = tmpint - 1;

	/* set day */
	tmpdata[0] = datetime[8];
	tmpdata[1] = datetime[9];

	tmpint = atoi(tmpdata);
	if (tmpint < 1 || tmpint > 31) {
		printf("Invalid day given, day range [1,31]");
		printf("datetime format：2017-01-01_15:23:45");
		return -3;
	}
	time->tm_mday = tmpint;

	/* set hour */
	tmpdata[0] = datetime[11];
	tmpdata[1] = datetime[12];

	tmpint = atoi(tmpdata);
	if (tmpint < 0 || tmpint > 23) {
		printf("Invalid hour given, hour range [0,23]");
		printf("datetime format：2017-01-01_15:23:45");
		return -4;
	}
	time->tm_hour = tmpint;

	/* set minute */
	tmpdata[0] = datetime[14];
	tmpdata[1] = datetime[15];

	tmpint = atoi(tmpdata);
	if (tmpint < 0 || tmpint > 59) {
		printf("Invalid minute given, minute range [0,59]");
		printf("datetime format：2017-01-01_15:23:45");
		return -5;
	}
	time->tm_min = tmpint;

	/* set second */
	tmpdata[0] = datetime[17];
	tmpdata[1] = datetime[18];

	tmpint = atoi(tmpdata);
	if (tmpint < 0 || tmpint > 59) {
		printf("Invalid hour second, second range [0,59]");
		printf("datetime format：2017-01-01_15:23:45");
		return -6;
	}
	time->tm_sec = tmpint;

	return 0;
}

/**
  \brief       time intervals between two time value
  \param[in]   time0    bigger time;
  \param[in]   time1    smaller time;
  \return  time interval( in seconds) 
  */
int compare_datetime(struct rtc_time time0, struct rtc_time time1)
{
	int total_seconds = (time0.tm_hour - time1.tm_hour) * 3600;
	total_seconds += (time0.tm_min - time1.tm_min) * 60;
	total_seconds += time0.tm_sec - time1.tm_sec;
	return total_seconds;
}

/**
  \brief        Display help information
  */
static void help_info(void)
{
	printf("%s %d.%d\n", prog_name, CSKY_RTC_MAJOR_NUM, CSKY_RTC_MINOR_NUM);
	printf("Usage: %s /dev/rtcN read/write/alarm [datetime] \n", prog_name);
	printf("read: it will give the Realtime clock value\n");
	printf("write: it will write datetime into RTC device\n");
	printf
	    ("alarm: it will make an alarm when time is datetime you given\n");
	printf("datetime format：2017-01-01_15:23:45\n");
}
