/*
 * Lu Chongzhi <chongzhi_lu@c-sky.com>
 *
 * Argument parser
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

void print_usage(char *name)
{
	printf("Usage:\n");
	printf("\t%s\n", name);
	printf("\t-d <device>  - SPI flash device (e.g. /dev/mtd0)\n");
	printf("\t-i           - Show flash informations\n");
	printf("\t-r           - Read flash\n");
	printf("\t-w           - Write flash\n");
	printf("\t-e           - Erase flash\n");
	printf("\t-o           - Offset of flash(decimal)\n");
	printf("\t-l           - Length of operate(decimal)\n");
	printf("\t-f <file>    - File read into or write from\n");
	printf("\nExamples:\n");
	printf("\t%s -d /dev/mtd0 -r -o 1024 -l 64\n", name);
	printf("\t%s -d /dev/mtd0 -r -o 1024 -l 64 -f ./flash.bin\n", name);
	printf("\t%s -d /dev/mtd0 -w -f ./flash.bin\n", name);
	printf("\t%s -d /dev/mtd0 -w -f ./flash.bin -o 512\n", name);
	printf("\t%s -d /dev/mtd0 -w -f ./flash.bin -o 512 -l 1024\n", name);
	printf("\t%s -d /dev/mtd0 -e\n", name);
	printf("\t%s -d /dev/mtd0 -e -o 4096\n", name);
	printf("\t%s -d /dev/mtd0 -e -o 4096 -l 8192\n", name);

	printf("\n");
}

int parse_args(struct instance *inst, int argc, char **argv)
{
	int c;
	init_to_defaults(inst);

	while ((c = getopt(argc, argv, "d:irweo:l:f:")) != -1) {
		switch (c) {
		case 'd':
			inst->dev_name = optarg;
			break;
		case 'i':
			inst->op_info = true;
			break;
		case 'r':
			inst->op_read = true;
			break;
		case 'w':
			inst->op_write = true;
			break;
		case 'e':
			inst->op_erase = true;
			break;
		case 'o':
			inst->operate_offset = atoi(optarg);
			break;
		case 'l':
			inst->operate_length = atoi(optarg);
			break;
		case 'f':
			inst->file_name = optarg;
			break;
		default:
			err("Bad argument: %c", c);
			return -1;
		}
	}

	if (inst->dev_name == NULL) {
		printf("MTD device name is required, e.g. '-d /dev/mtd0'\n");
		return -1;
	}

	if (inst->op_read && inst->op_write) {
		err("Can't to read & write in same time");
		return -1;
	}

	return 0;
}

