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
#include <mtd/mtd-user.h>
#include "common.h"

#ifndef SPI_FLASH_H
#define SPI_FLASH_H

int get_mtdinfo(struct instance *inst);
int print_mtdinfo(struct instance *inst);

int flash_read(struct instance *inst);
int flash_write(struct instance *inst);
int flash_erase(struct instance *inst);

#endif /* SPI_FLASH_H */
