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

static const char *str_flash_type[] = {
	"absent",		//MTD_ABSENT            0
	"ram",			//MTD_RAM               1
	"rom",			//MTD_ROM               2
	"nor-flash",		//MTD_NORFLASH          3
	"nand-flash(slc)",	//MTD_NANDFLASH         4       /* SLC NAND */
	"unknown",		//                      5
	"data-flash",		//MTD_DATAFLASH         6
	"ubi-volume",		//MTD_UBIVOLUME         7
	"nand-flash(mlc)"	//MTD_MLCNANDFLASH      8       /* MLC NAND (including TLC) */
};

int get_mtdinfo(struct instance *inst)
{
	int num;

	common_time_start();
	num = ioctl(inst->dev_fd, MEMGETINFO, &(inst->mtd_info));
	common_time_elapse("Get MTD device info", NULL);
	if (num < 0) {
		err("Could not get mtd info from device(%s), [errno=%d]\n",
		    inst->dev_name, errno);
		return -1;
	}

	return 0;
}

int print_mtdinfo(struct instance *inst)
{
	printf("mtd info of '%s': \n\t"
	       "type=%u('%s'), flags=%u, size=%u, erasesize=%u, writesize=%u, oobsize=%u, padding=%llu\n",
	       inst->dev_name,
	       inst->mtd_info.type,
	       (inst->mtd_info.type <=
		MTD_MLCNANDFLASH) ? str_flash_type[inst->mtd_info.
						   type] : "unknown",
	       inst->mtd_info.flags, inst->mtd_info.size,
	       inst->mtd_info.erasesize, inst->mtd_info.writesize,
	       inst->mtd_info.oobsize, inst->mtd_info.padding);
	return 0;
}

#define READ_STEP (16*1024)
#define PRINT_BUF_SIZE 128
int flash_read(struct instance *inst)
{
	int ret = -1;
	char *read_buf = NULL;
	unsigned int bytes_read;
	unsigned int bytes_read_total;
	unsigned int elapsed_usec_total;

	char print_buf[PRINT_BUF_SIZE];
	unsigned int print_buf_len = 0;

	if (inst->operate_length == 0) {
		printf("Read length should not be 0\n");
		goto EXIT;
	}

	if (inst->operate_offset + inst->operate_length > inst->mtd_info.size) {
		printf("offset(%d)+length(%d) is beyond the chip size(%d).\n",
		       inst->operate_offset, inst->operate_length,
		       inst->mtd_info.size);
		goto EXIT;
	}

	if (inst->file_name != NULL) {
		if (inst->file_fd >= 0) {
			close(inst->file_fd);
		}
		// If exist open and truncate, otherwise create it
		inst->file_fd =
		    open(inst->file_name, O_RDWR | O_TRUNC | O_CREAT);
		if (inst->file_fd >= 0) {
			printf("Save file '%s' is opened.\n", inst->file_name);
		}
	}

	if (lseek(inst->dev_fd, inst->operate_offset, SEEK_SET) < 0) {
		err("flash lseek to %u failed", inst->operate_offset);
		goto EXIT;
	}

	read_buf = malloc(READ_STEP);
	if (read_buf == NULL) {
		err("malloc %u bytes failed", inst->operate_length);
		goto EXIT;
	}

	bytes_read_total = 0;
	elapsed_usec_total = 0;

	printf("Reading process: 00%%");
	fflush(stdout);
	while (bytes_read_total < inst->operate_length) {
		unsigned int bytes_need_read =
		    ((inst->operate_length - bytes_read_total) >= READ_STEP) ?
		    bytes_need_read = READ_STEP :
		    inst->operate_length - bytes_read_total;

		unsigned int elapsed_usec;

		common_time_start();
		bytes_read = read(inst->dev_fd, read_buf, bytes_need_read);
		common_time_elapse(NULL, &elapsed_usec);

		if (bytes_read_total == 0) {
			//first read, copy to print_buf
			print_buf_len = (bytes_need_read >= PRINT_BUF_SIZE) ?
			    PRINT_BUF_SIZE : bytes_need_read;
			memcpy(print_buf, read_buf, print_buf_len);
		}

		if (bytes_read != bytes_need_read) {
			err("Read %d bytes from flash but got %u bytes",
			    inst->operate_length, bytes_read);
			goto EXIT;
		}

		if (inst->file_fd >= 0) {
			int bytes_write =
			    write(inst->file_fd, read_buf, bytes_read);
			if (bytes_write != bytes_read) {
				err("Write %u bytes into file but got %d bytes",
				    bytes_read, bytes_write);
				goto EXIT;
			}
		}

		if (bytes_read_total != inst->operate_length) {
			printf("\b\b\b%02d%%",
			       100 * bytes_read_total / inst->operate_length);
			fflush(stdout);
		}

		elapsed_usec_total += elapsed_usec;
		bytes_read_total += bytes_need_read;
	}
	printf("\b\b\b100%%.\n");
	printf("  <<%8u usec costed for 'Read %u bytes %s'>>\n",
	       elapsed_usec_total, inst->operate_length,
	       (inst->file_fd >= 0) ? "and save to file" : "test");

	printf("\nFirst %u bytes are:\n", print_buf_len);
	common_dump_hex(print_buf, print_buf_len);
	ret = 0;
EXIT:
	if (read_buf != NULL) {
		free(read_buf);
	}
	return ret;
}

#define WRITE_STEP (16*1024)
int flash_write(struct instance *inst)
{
	int ret = -1;
	if (inst->file_name == NULL) {
		printf("Write operate must input a filename\n");
		return -1;
	}

	unsigned int file_size;
	struct stat statbuff;
	if (stat(inst->file_name, &statbuff) < 0) {
		printf("File %s does not exist\n", inst->file_name);
		return -1;
	}
	file_size = statbuff.st_size;

	if (inst->operate_length == 0) {
		inst->operate_length = file_size;
	} else {
		if (inst->operate_length > file_size) {
			printf
			    ("operate length(%d) is beyond than the file length(%u)\n",
			     inst->operate_length, file_size);
			return -1;
		}
	}

	if ((inst->operate_offset + inst->operate_length) > inst->mtd_info.size) {
		printf
		    ("offset(%d) + file_size(%d) is beyond the device size(%d)\n",
		     inst->operate_offset, inst->operate_length,
		     inst->mtd_info.size);
		return -1;
	}
	// Open file and seek to 0
	if (inst->file_fd >= 0) {
		if (lseek(inst->file_fd, 0, SEEK_SET) < 0) {
			err("file lseek to %u failed", 0);
			return -1;
		}
	} else {
		inst->file_fd = open(inst->file_name, O_RDONLY);
		if (inst->file_fd < 0) {
			printf("File '%s' can't open.\n", inst->file_name);
			return -1;
		}
	}

	if (lseek(inst->dev_fd, inst->operate_offset, SEEK_SET) < 0) {
		err("flash lseek to %u failed", inst->operate_offset);
		return -1;
	}

	char *pfile =
	    mmap(0, file_size, PROT_READ, MAP_SHARED, inst->file_fd, 0);
	//common_dump_hex(pfile, inst->operate_length);

	unsigned int bytes_write_total = 0;
	unsigned int elapsed_usec_total = 0;
	printf("Write process: 00%%");
	fflush(stdout);
	while (bytes_write_total < inst->operate_length) {
		unsigned int bytes_need_write =
		    (inst->operate_length - bytes_write_total) >= WRITE_STEP ?
		    WRITE_STEP : (inst->operate_length - bytes_write_total);

		unsigned int elapsed_usec;

		common_time_start();
		ssize_t bytes_write = write(inst->dev_fd,
					    pfile + bytes_write_total,
					    bytes_need_write);
		common_time_elapse(NULL, &elapsed_usec);

		if (bytes_write != bytes_need_write) {
			err("Write %u bytes return %d",
			    bytes_need_write, bytes_write);
			goto EXIT;
		}

		if (bytes_write_total != inst->operate_length) {
			printf("\b\b\b%02d%%",
			       100 * bytes_write_total / inst->operate_length);
			fflush(stdout);
		}

		elapsed_usec_total += elapsed_usec;
		bytes_write_total += bytes_need_write;
	}
	printf("\b\b\b100%%.\n");
	printf("  <<%8u usec costed for 'Write %u bytes'>>\n",
	       elapsed_usec_total, inst->operate_length);

	ret = 0;
EXIT:
	if (pfile != NULL) {
		munmap(pfile, file_size);
	}
	return ret;
}

#define ERASE_STEP (4*4*1024)
int flash_erase(struct instance *inst)
{
	if ((inst->operate_offset + inst->operate_length) > inst->mtd_info.size) {
		printf("offset(%d)+length(%d) is beyond the chip size(%d).\n",
		       inst->operate_offset, inst->operate_length,
		       inst->mtd_info.size);
		return -1;
	}

	if ((inst->operate_offset % inst->mtd_info.erasesize) != 0 ||
	    (inst->operate_length % inst->mtd_info.erasesize) != 0) {
		printf
		    ("offset(%d) and length(%d) should aligned with erase_size(%d)",
		     inst->operate_offset, inst->operate_length,
		     inst->mtd_info.erasesize);
		return -1;
	}

	char scan = 'N';
	printf("\nErase offset=%d, len=%d? y/N: ",
	       inst->operate_offset, inst->operate_length);
	scanf("%c", &scan);
	if (scan != 'y' && scan != 'Y') {
		return -1;
	}

	unsigned int bytes_erase_total = 0;
	unsigned int elapsed_usec_total = 0;
	erase_info_t erase;

	printf("Erase process: 00%%");
	fflush(stdout);
	while (bytes_erase_total < inst->operate_length) {
		unsigned int bytes_erase =
		    min((inst->operate_length - bytes_erase_total), ERASE_STEP);

		erase.start = inst->operate_offset + bytes_erase_total;
		erase.length = bytes_erase;

		unsigned int elapsed_usec;
		common_time_start();
		if (ioctl(inst->dev_fd, MEMERASE, &erase) != 0) {
			err("Erase offset=%d, length=%d failed, errno = %d",
			    erase.start, erase.length, errno);
			return -1;
		}
		common_time_elapse(NULL, &elapsed_usec);

		if (bytes_erase_total != inst->operate_length) {
			printf("\b\b\b%02d%%",
			       100 * bytes_erase_total / inst->operate_length);
			fflush(stdout);
		}

		elapsed_usec_total += elapsed_usec;
		bytes_erase_total += bytes_erase;
	}
	printf("\b\b\b100%%.\n");
	printf("  <<%8u usec costed for 'Erase %u bytes'>>\n",
	       elapsed_usec_total, inst->operate_length);

	return 0;
}
