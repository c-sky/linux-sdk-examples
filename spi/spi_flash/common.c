/*
 * Lu Chongzhi <chongzhi_lu@c-sky.com>
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

#include "common.h"
#include "spi_flash.h"

char g_scan_char;

static struct timeval s_tv;

void common_time_start(void)
{
	gettimeofday(&s_tv,NULL);
}

void common_time_elapse(char *str, unsigned int *usec)
{
	struct timeval tv;
	gettimeofday(&tv,NULL);
	unsigned int elapsed_usec = tv.tv_sec*1000*1000 + tv.tv_usec -
				    s_tv.tv_sec*1000*1000 - s_tv.tv_usec;
	if (str != NULL) {
		printf("  <<%8u usec costed for '%s' >>\n",
			elapsed_usec, (str != NULL) ? str : "something");
	}

	if (usec != NULL) {
		*usec = elapsed_usec;
	}
	s_tv = tv;
}

void common_dump_hex(char *buf, unsigned int len)
{
	int i;
	for (i = 0; i < len; ++i) {
		if (i % 16 == 0) {
			printf("\t");
		}
		printf("%02x ", buf[i]);
		if (i % 16 == 15) {
			printf("\n");
		}
	}
	printf("\n");
}

