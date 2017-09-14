/*
 * Zhang Minfeng <minfeng_zhang@c-sky.com>
 *
 * Main file of the application
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

#include <stdio.h>
#include <termios.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>

int kbhit(void)
{
	struct termios oldt, newt;
	int ch;
	int oldf;

	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
	ch = getchar();
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);

	if (ch != EOF) {
		ungetc(ch, stdin);
		return 1;
	}

	return 0;
}


int get_maxlevel(void)
{
	int fd, level, ret;
	char temp[3];

	fd = open("/sys/class/backlight/soc:backlight/max_brightness", O_RDONLY);
	if (fd < 0) {
		return -1;
	}

	ret = read(fd, temp, 2);
	if (ret < 1) {
		return -1;
	}
	close(fd);

	level = atoi(temp);

	return level;
}

void print_usage(void)
{
	printf("Usage:\n");
	printf("\tlcd_backlight [num]\n");
	printf("\tnum is the circle times. num should be bigger than 0\n");
	printf("\tFor example: ./lcd_backlight 5 \n");
}

int set_backlight(int fd, int lv, int max_lv)
{
	int ret;
	char light[2];

	light[0] = '0';
	light[1] = '\0';

	if (fd < 0) {
		printf("Backlight invalid FD\n");
		return -1;
	}

	if (lv < 0 || lv > max_lv) {
		printf("Invalid backlight level\n");
		return -1;
	}

	light[0] += lv;

	ret = write(fd, light, 1);

	if (ret > 0) {
		return 0;
	} else {
		printf("Write backlight error\n");
		return ret;
	}
}

int main(int argc, char **argv)
{
	int max_level;
	int fd = 0, i, ret = -1;
	int num;

	if (argc != 2) {
		print_usage();
		return 0;
	} else {
		num = atoi(argv[1]);
	}

	max_level = get_maxlevel();

	if (max_level < 0) {
		printf("Fail to read max level\n");
		return -1;
	}

	printf("The backlight max level is %d\n", max_level);

	fd = open("/sys/class/backlight/soc:backlight/brightness", O_RDWR);

	if (fd < 0) {
		printf("Fail to open backlight dev\n");
		return -1;
	}

	printf("\tPress any key to exit test\n");
	while(num && (!kbhit())) {

		for (i = 0; i < (max_level + 1); ++i) {
			ret = set_backlight(fd, i, max_level);
			if (ret < 0) {
				printf("Fail to set backlight\n");
				close(fd);
				return -1;
			}
			usleep(100000);
		}

		if (kbhit())
			break;

		for (i = max_level; i > 0; --i) {
			ret = set_backlight(fd, i, max_level);
			if (ret < 0) {
				printf("Fail to set backlight\n");
				close(fd);
				return -1;
			}
			usleep(100000);
		}
		num--;
	}

	printf("Test success\n");
	close(fd);
	return 0;
}
