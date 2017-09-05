/*
 * Storage test include file
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>

/**************************************************************************
 * Macro Defination
 * **********************************************************************/
#define CSKY_STORAGE_MAJOR_NUM 1
#define CSKY_STORAGE_MINOR_NUM 0
#define BUFFER_SIZE            1024 * 32

/**************************************************************************
 * Public Functions
 * **********************************************************************/
void data_prep(char *buffer, unsigned int size);
int write_file(char *buffer, char *filename, unsigned int size, int iterations);
int read_file(char *buffer, char *filename, unsigned int size, int iteration);
int write_perf(char *buffer, char *filename, unsigned int size);
int read_perf(char *filename, unsigned int size);
