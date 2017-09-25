/*
 * V4L2 Codec decoding example application
 * Lu Chongzhi <chongzhi_lu@c-sky.com>
 *
 * File operations
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

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "common.h"
#include "fileops.h"

int input_open(struct instance *i, char *name)
{
	struct stat in_stat;

	i->in.fd = open(name, O_RDONLY);
	if (i->in.fd <= 0) {
		err("Failed to open file: %s", i->in.name);
		return -1;
	}
	fstat(i->in.fd, &in_stat);
	i->in.size = in_stat.st_size;
	i->in.offs = 0;
	return 0;
}

void input_close(struct instance *i)
{
	close(i->in.fd);
}