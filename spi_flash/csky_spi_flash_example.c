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
#define DEBUG_SCAN_STEP		//dbg("press any key to continue:");scanf("%c", &g_scan_char);

void init_to_defaults(struct instance *inst)
{
	memset(inst, 0, sizeof(*inst));
	inst->dev_fd = -1;
	inst->file_fd = -1;
}

int main(int argc, char **argv)
{
	int ret = -1;
	struct instance inst;

	if (argc == 1 || parse_args(&inst, argc, argv)) {
		print_usage(argv[0]);
		goto EXIT;
	}

	inst.dev_fd = open(inst.dev_name, O_RDWR);
	if (inst.dev_fd >= 0) {
		printf("Device '%s' exists and open OK.\n", inst.dev_name);
	} else {
		err("Can't open device: '%s'", inst.dev_name);
		goto EXIT;
	}

	if (get_mtdinfo(&inst) != 0) {
		goto EXIT;
	}

	if (inst.op_info) {
		print_mtdinfo(&inst);
	}

	if (inst.op_read) {
		if (flash_read(&inst) != 0) {
			goto EXIT;
		}
	}

	if (inst.op_write) {
		if (flash_write(&inst) != 0) {
			goto EXIT;
		}
	}

	if (inst.op_erase) {
		if (flash_erase(&inst) != 0) {
			goto EXIT;
		}
	}

	ret = 0;
EXIT:
	if (inst.dev_fd >= 0) {
		close(inst.dev_fd);
	}

	if (inst.file_fd >= 0) {
		close(inst.file_fd);
	}

	return ret;
}
