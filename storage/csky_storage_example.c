/*
 * Storage test module
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

#include "csky_storage_test.h"

static void help_info(void);
static const char *prog_name = "csky_storage_example\0";

int main(int argc, char **argv)
{
	/*  If action filename and size are not all provided */
	if (argc < 4) {
		help_info();
		return -1;
	}

	/* Check size and iterations */
	unsigned int size;
	if ((argv[3][1] == 'x' || argv[3][1] == 'X') && argv[3][0] == '0')
		size = strtoul(argv[3], NULL, 16);
	else
		size = strtoul(argv[3], NULL, 10);
	if (size == 0) {
		printf("Please provide a valid size value\n");
		help_info();
		return -2;
	}
	int iterations = 1;

	if (argc >= 5) {
		iterations = atoi(argv[4]);
		if (iterations < 0) {
			printf("Please provide a valid iterations value\n");
			help_info();
			return -3;
		}
	}

	/* Prepare data */
	char buffer[BUFFER_SIZE];

	data_prep(buffer, BUFFER_SIZE);

	/* Do write action */
	if (strcmp(argv[1], "write") == 0 && argc >= 4)
		write_file(buffer, argv[2], size, iterations);

	/* Do read action */
	else if (strcmp(argv[1], "read") == 0 && argc >= 4)
		read_file(buffer, argv[2], size, iterations);

	/* Do write performance test  */
	else if (strcmp(argv[1], "writeperf") == 0)
		write_perf(buffer, argv[2], size);

	/* Do read performance test */
	else if (strcmp(argv[1], "readperf") == 0)
		read_perf(argv[2], size);

	/* Wrong parameters given */
	else {
		help_info();
		return -4;
	}

	return 0;
}

/**
  \brief        Prepare buffer data
  \param[out]  buffer   Buffer data to be written
  \param[in]   size     Buffer size
  */
void data_prep(char *buffer, unsigned int size)
{
	int i;
	char ch = 0x20;
	for (i = 0; i < size; i++) {
		buffer[i] = ch;
		if (++ch > 0x7e)
			ch = 0x20;
	}

}

/**
  \brief       Write buffer data into file
  \param[in]   buffer     Data to be written in files
  \param[in]   filename   File name (iteration number will be added)
  \param[in]   size       File size
  \param[in]   iterations How many file should be written
  return 0:success ; others: error
  */
int write_file(char *buffer, char *filename, unsigned int size, int iterations)
{
	int namelen = strlen(filename) + 3;
	char file[iterations][namelen];
	int fd[iterations], i, writecnt, count;
	for (i = 0; i < iterations; i++) {
		/* Set File Name */
		sprintf(file[i], "%s-%d", filename, i);
		fd[i] = open(file[i], O_CREAT | O_WRONLY, 0664);
		if (fd[i] < 0) {
			printf("%s cannot be open or created!\n", file[i]);
			return -1;
		}

		file[i][namelen - 1] = '\0';

		/* Write into file */
		for (count = 0; count < size;) {
			if (size - count > BUFFER_SIZE)
				writecnt = write(fd[i], buffer, BUFFER_SIZE);
			/* Last data */
			else
				writecnt = write(fd[i], buffer, size - count);
			/* if write failed */
			if (writecnt < 0) {
				printf("File %s write failed\n", file[i]);
				return -2;
			} else
				count += writecnt;
		}

		close(fd[i]);
	}

	return 0;
}

/**
  \brief       Read file data
  \param[in]   buffer     Data to be compared with
  \param[in]   filename   File name (iteration number will be added)
  \param[in]   size       File size
  \param[in]   iterations How many file should be read
  return  0:success ; others: error
  */
int read_file(char *buffer, char *filename, unsigned int size, int iterations)
{
	char file[iterations][strlen(filename) + 3];
	int fd[iterations], i, readcnt, count;
	char readbuf[BUFFER_SIZE];
	for (i = 0; i < iterations; i++) {
		/* Set File Name */
		sprintf(file[i], "%s-%d", filename, i);
		fd[i] = open(file[i], O_RDONLY, 0664);
		if (fd[i] < 0) {
			printf("%s cannot be openned!\n", file[i]);
			return -1;
		}

		/* Read data from file  */
		for (count = 0; count < size;) {
			if (size - count > BUFFER_SIZE)
				readcnt = read(fd[i], readbuf, BUFFER_SIZE);
			/* Last Data */
			else
				readcnt = read(fd[i], readbuf, size - count);
			/* if read failed */
			if (readcnt < 0) {
				printf("File %s read failed\n", file[i]);
				return -2;
				/* If provided size is more than file size */
			} else if (readcnt < BUFFER_SIZE
				   && readcnt < size - count) {
				printf("File size less than %u Bytes\n", size);
				printf("Please provide a smaller size\n");
				return -3;
			} else {
				/* Check File data */
				count += readcnt;
				if (strncmp(readbuf, buffer, readcnt) != 0) {
					printf
					    ("File %s data not as expected!\n",
					     file[i]);
					return -4;
				}
			}
		}
		close(fd[i]);
	}

	return 0;

}

/**
  \brief       Storage device write performance test
  \param[in]   buffer     Data to be written in files
  \param[in]   filename   File name
  \param[in]   size       File size
  return  0: success ; others: error
  */
int write_perf(char *buffer, char *filename, unsigned int size)
{
	/* File size should be more than 30MB */
	if (size <= 30000000) {
		printf
		    ("For performance test, size should be more than 30000000\n");
		return -1;
	}

	int fd, ret = 0, count, writecnt;
	struct timeval start, end;
	unsigned int speed, timespent;

	fd = open(filename, O_CREAT | O_WRONLY, 0644);
	if (fd < 0) {
		printf("File %s cannot be created or opened\n", filename);
		return -1;
	}
	/* Get start time */
	gettimeofday(&start, NULL);

	/* Write performance test */
	for (count = 0; count < size;) {
		if (size - count > BUFFER_SIZE)
			writecnt = write(fd, buffer, BUFFER_SIZE);
		else
			writecnt = write(fd, buffer, size - count);
		if (writecnt < 0) {
			printf("File %s write failed\n", filename);
			ret = -2;
			goto WR_PERF_EXIT;
		} else
			count += writecnt;
	}

	/* Get end time */
	gettimeofday(&end, NULL);

	timespent = (end.tv_sec - start.tv_sec) * 1000000
	    + end.tv_usec - start.tv_usec;
	speed = (unsigned int)(size / 1024 / ((double)timespent / 1000000));
	printf("Write Speed %u KB/s\n", speed);

WR_PERF_EXIT:
	close(fd);
	return ret;

}

/**
  \brief       Storage device write performance test
  \param[in]   filename   File name
  \param[in]   size       File size
  return 0:success ; others: error
  */
int read_perf(char *filename, unsigned int size)
{
	/* Size should be more than 30MB */
	if (size <= 30000000) {
		printf
		    ("For performance test, size should be more than 30000000\n");
		return -1;
	}

	char buffer[BUFFER_SIZE];
	int fd, ret = 0, readcnt, count;
	struct timeval start, end;
	unsigned int speed, timespent;

	fd = open(filename, O_RDONLY, 0644);
	if (fd < 0) {
		printf("File %s cannot be opened\n", filename);
		return -1;
	}
	/* Get start time */
	gettimeofday(&start, NULL);

	/* Read performance test */
	for (count = 0; count < size;) {
		if (size - count > BUFFER_SIZE)
			readcnt = read(fd, buffer, BUFFER_SIZE);
		else
			readcnt = read(fd, buffer, size - count);
		if (readcnt < 0) {
			printf("File %s read failed\n", filename);
			ret = -2;
			goto RD_PERF_EXIT;
		} else if (readcnt < BUFFER_SIZE && readcnt < size - count) {
			printf("File size less than %u Bytes\n", size);
			printf("Please provide a smaller size\n");
			ret = -3;
			goto RD_PERF_EXIT;
		} else
			count += readcnt;
	}

	/* Get end time */
	gettimeofday(&end, NULL);

	timespent = (end.tv_sec - start.tv_sec) * 1000000
	    + end.tv_usec - start.tv_usec;
	speed = (unsigned int)(size / 1024 / ((double)timespent / 1000000));
	printf("Read Speed %u KB/s\n", speed);

RD_PERF_EXIT:
	close(fd);
	return ret;

}

/**
  \brief       Show help information
  */
static void help_info(void)
{
	printf("%s %d.%d\n", prog_name, CSKY_STORAGE_MAJOR_NUM,
	       CSKY_STORAGE_MINOR_NUM);
	printf("usage: %s action filename size [iterations]\n", prog_name);
	printf("action: read, write, readperf, writeperf\n");
	printf
	    ("filename: name of the file. iteration number will also be added\n");
	printf("size: read or write size\n");
	printf("iterations: how many times you want to write or read.");
	printf("Default is value is 1, unused for performance test.\n");
	printf
	    ("When using read action, you should provide a file created by %s\n",
	     prog_name);
}
