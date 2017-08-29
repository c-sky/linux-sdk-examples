/*
 * Network test include file
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

#include <sys/socket.h>
#include <netinet/in.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <netinet/ip_icmp.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include <arpa/inet.h>
#include <sys/select.h>

/**************************************************************************
 * Macro Defination 
 * **********************************************************************/
#define CSKY_NETWORK_MAJOR_NUM 1
#define CSKY_NETWORK_MINOR_NUM 0
#define IPADDRLEN   14
#define SENDSIZE    2048
#define PORTNO      2345

/****************************************************************************
 * Public Functions
 ****************************************************************************/
int set_ip_addr(int fd, struct ifreq *req, const char *ipaddr);
int set_netmask_addr(int fd, struct ifreq *req, const char *ipaddr);
int set_dst_addr(int fd, struct ifreq *req, const char *ipaddr);
int get_ip_addr(int fd, struct ifreq *req, char *addr);
int get_netmask_addr(int fd, struct ifreq *req, char *addr);
int get_dst_addr(int fd, struct ifreq *req, char *addr);

void sendtcp_client(unsigned long host);
void recvtcp_server(void);
void ping_dst(unsigned long host);
