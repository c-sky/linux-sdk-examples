/*
 * Lu Chongzhi <chongzhi_lu@c-sky.com>
 *
 * Common stuff header file
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

#ifndef INCLUDE_COMMON_H
#define INCLUDE_COMMON_H

#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>

#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <mtd/mtd-user.h>

typedef int bool;
#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

extern char g_scan_char;
#define DEBUG_SCAN_STEP		//dbg("press any key to continue:");scanf("%c", &g_scan_char);

/* When ADD_DETAILS is defined every debug and error message contains
 * information about the file, function and line of code where it has
 * been called */
#define ADD_DETAILS
/* When DEBUG is defined debug messages are printed on the screen.
 * Otherwise only error messages are displayed. */
//#define DEBUG

#ifdef ADD_DETAILS
#define err(msg, ...) \
	fprintf(stderr, "Error (%s:%s:%d): " msg "\n", __FILE__, \
		__func__, __LINE__, ##__VA_ARGS__)
#else
#define err(msg, ...) \
	fprintf(stderr, "Error: " msg "\n", __FILE__, ##__VA_ARGS__)
#endif /* ADD_DETAILS */

#ifdef DEBUG
#ifdef ADD_DETAILS
#define dbg(msg, ...) \
	fprintf(stdout, "(%s:%s:%d): " msg "\n", __FILE__, \
		__func__, __LINE__, ##__VA_ARGS__)
#else
#define dbg(msg, ...) \
	fprintf(stdout, msg "\n", ##__VA_ARGS__)
#endif /* ADD_DETAILS */
#else /* DEBUG */
#define dbg(...) {}
#endif /* DEBUG */

#define memzero(x)\
        memset(&(x), 0, sizeof (x));

#define min(x,y) (x < y) ? x : y

void common_time_start(void);
void common_time_elapse(char *str, unsigned int *usec);
void common_dump_hex(char *buf, unsigned int len);

struct instance {
	/* Input args */
	char *dev_name;		// e.g. "/dev/mtd0"
	bool op_info;
	bool op_read;
	bool op_write;
	bool op_erase;
	unsigned int operate_offset;
	unsigned int operate_length;
	char *file_name;	// e.g. "/tmp/spiflash.hex"

	/* Internal args */
	int dev_fd;
	int file_fd;
	struct mtd_info_user mtd_info;
};

void init_to_defaults(struct instance *inst);

/* Print usage information of the application */
void print_usage(char *name);

/* revert string to number */
bool string_to_num(char *str, unsigned int *value);

/* Parse the arguments that have been given to the application */
int parse_args(struct instance *i, int argc, char **argv);

#endif /* INCLUDE_COMMON_H */
