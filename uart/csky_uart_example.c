/*
 * Uart test module
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

#include "csky_uart_test.h"

static const char *prog_name = "csky_uart_example\0";
static void help_info(void);

int main(int argc, char *argv[])
{
	int fd;
	char buf[200] = { 0 };
	int ret, i;
	char wait;
	int Baud_list[] = { 9600, 19200, 38400, 57600, 115200 };
	int baudrate[] = { B9600, B19200, B38400, B57600, B115200 };
	struct termios p_termio, def_termio;
	memset(buf, 0, 100);
	if (argc < 2) {
		help_info();
		return -1;
	}
	/*Open Serial Device */
	fd = open(argv[1], O_RDWR);
	if (fd < 0) {
		perror("open");
		help_info();
		return -1;
	}

	printf("Start the serial test\n");

	/*First get default serial configuration, then save it */
	ret = tcgetattr(fd, &p_termio);
	if (ret == -1) {
		printf("Get Serial configuration failed.\n");
		goto EXIT;
	}
	def_termio = p_termio;
	printf("c_iflag:%u, c_oflag:%u, c_cflag:%u, c_lflag %u.\n",
	       p_termio.c_iflag, p_termio.c_oflag, p_termio.c_cflag,
	       p_termio.c_lflag);
	p_termio.c_iflag = 5376;
	p_termio.c_oflag = 5;
	p_termio.c_cflag = 7346;
	p_termio.c_lflag = 2619;

	ret = tcsetattr(fd, TCSADRAIN, &p_termio);
	if (ret < 0) {
		printf("Uart attribute set failed\n");
		goto EXIT;
	}

	/*Test Default configuration */
	sprintf(buf,
		"When you see this message, It is the default configuration.\n");
	ret = write(fd, buf, strlen(buf));

	memset(buf, 0, 100);

	if (ret < 0) {
		printf("fail to send message\n");
		goto EXIT;
	}

	/*Baudrate configuration Test */
	/*From B9600 to B115200 */
	for (i = 0; i < 5; i++) {
		sprintf(buf,
			"Plese change the baudrate to %d,when you ok ,please enter Y\n",
			Baud_list[i]);
		ret = write(fd, buf, strlen(buf));

		memset(buf, 0, 100);

		if (ret < 0) {
			printf("fail to send message\n");
			goto EXIT;
		}
		/*Change Baudrate */
		cfsetspeed(&p_termio, baudrate[i]);
		ret = tcsetattr(fd, TCSADRAIN, &p_termio);
		if (ret < 0) {
			printf("Set baudrate failed");
			goto EXIT;
		}

		while (1) {
			int num;
			num = read(fd, &wait, 1);
			if (num > 0 && wait == 'Y') {
				memset(buf, 0, 100);
				wait = 'N';
				break;
			}
		}

		sprintf(buf, "When you see this message, the baudrate is %d.\n",
			Baud_list[i]);
		ret = write(fd, buf, strlen(buf));

		memset(buf, 0, 100);

		if (ret < 0) {
			printf("fail to send message\n");
			goto EXIT;
		}
	}
	/* Change Character Size */
	/*First ,change to CS7 */
	sprintf(buf, "Now let's change Character Size to 7\n");
	write(fd, buf, strlen(buf));
	memset(buf, 0, 100);
	p_termio.c_cflag &= ~CSIZE;
	p_termio.c_cflag |= CS7;
	ret = tcsetattr(fd, TCSADRAIN, &p_termio);
	if (ret < 0) {
		printf("Set Csize failed");
		goto EXIT;
	}
	/*Ascii Data output */
	for (i = 0; i < 128; i++) {
		sprintf(buf, "%d:%c\n", i, i);
		write(fd, buf, strlen(buf));
		memset(buf, 0, 100);
	}

	/*Change to CS8 */
	sprintf(buf, "Now let's change Character Size to 8\n");
	write(fd, buf, strlen(buf));
	p_termio.c_cflag |= CS8;
	ret = tcsetattr(fd, TCSADRAIN, &p_termio);
	if (ret < 0) {
		printf("Set CSize failed");
		goto EXIT;
	}
	/*Ascii Data output */
	memset(buf, 0, 100);
	for (i = 0; i < 128; i++) {
		sprintf(buf, "%d:%c\n", i, i);
		write(fd, buf, strlen(buf));
		memset(buf, 0, 100);
	}

	/* Final Test: 10000 line output test. */
	sprintf(buf,
		"Final Test:Let's have a outputing party, when you are ready,please enter Y\n");
	ret = write(fd, buf, strlen(buf));

	memset(buf, 0, 100);

	if (ret < 0) {
		printf("fail to send message\n");
		goto EXIT;
	}
	while (1) {
		int num;
		num = read(fd, &wait, 1);
		if (num > 0 && wait == 'Y') {
			memset(buf, 0, 100);
			wait = 'N';
			break;
		}
	}

	/* Uart Stress Test */
	for (i = 1; i <= 10000; i++) {
		sprintf(buf,
			"It is just a uart stress test, it will test up to 10000 and now it is line%d!\n",
			i);
		write(fd, buf, strlen(buf));
		memset(buf, 0, 100);
	}

	tcsetattr(fd, TCSADRAIN, &def_termio);
EXIT:
	close(fd);
	return ret;
}

/**
  \brief        Display help information
  */
static void help_info(void)
{
	printf("%s %d.%d\n", prog_name, CSKY_UART_MAJOR_NUM,
	       CSKY_UART_MINOR_NUM);
	printf("Usage: %s serial_device \n", prog_name);
	printf("serial_device: Serial Device File. for example /dev/ttyS2\n");
}
