/*
 * Network setting test module
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

#include "csky_net_test.h"

static void help_info(void);
static const char *prog_name = "csky_netsetting_example\0";

int main(int argc, char **argv)
{
	if (argc < 2) {
		help_info();
		return -1;
	}

	char *netdev;
	int sock, i, ret;
	char *ip_addr, *netmask_addr, *dst_addr;

	netdev = argv[1];

	/* Initialization */
	ip_addr = (char *)malloc(14);
	netmask_addr = (char *)malloc(14);
	dst_addr = (char *)malloc(14);

	struct ifreq req;

	sock = socket(PF_INET, SOCK_DGRAM, 0);

	memset(&req, 0, sizeof(struct ifreq));
	strncpy(req.ifr_name, netdev, IFNAMSIZ);
	/* Do get information actions */
	if (argc == 2) {

		ret = get_ip_addr(sock, &req, ip_addr);
		if (ret < 0)
			goto EXIT;
		printf("IP Address: %s\n", ip_addr);

		ret = get_netmask_addr(sock, &req, netmask_addr);
		if (ret < 0)
			goto EXIT;
		printf("Netmask: %s\n", netmask_addr);

		ret = get_dst_addr(sock, &req, dst_addr);
		if (ret < 0)
			goto EXIT;
		printf("Gateway IP: %s\n", dst_addr);

	}

	/* Do set ip address action */
	if (argc >= 3) {
		ip_addr = argv[2];
		ret = set_ip_addr(sock, &req, ip_addr);
		if (ret < 0) {
			printf("Fail to set IP Address.\n");
			goto EXIT;
		}
		printf("Set IP Address successfully.\n");
	}

	/* Do set netmask action */
	if (argc >= 4) {
		netmask_addr = argv[3];
		ret = set_netmask_addr(sock, &req, netmask_addr);
		if (ret < 0) {
			printf("Fail to set Netmask.\n");
			goto EXIT;
		}
		printf("Set Netmask Address successfully.\n");
	}

	/* Do set gateway ip address action */
	if (argc >= 5) {
		dst_addr = argv[4];
		ret = set_dst_addr(sock, &req, dst_addr);
		if (ret < 0) {
			printf("Fail to set Gateway IP.\n");
			goto EXIT;
		}
		printf("Set GateWay IP Address successfully.\n");
	}

EXIT:
	free(ip_addr);
	free(netmask_addr);
	free(dst_addr);
	close(sock);
	return ret;
}

/**
  \brief        Check if it is a valid IP.
  \param[in]   ipaddr   ip address string just like 127.0.0.1
  \return INET host value ; -1 invalid IP
  */
static int32_t check_ip(const char *ipaddr)
{
	int32_t host;
	host = inet_addr(ipaddr);
	if (host < 0)
		printf("Invalid IP address given.\n");
	return host;
}

/**
  \brief         set IP address
  \param[in]   fd       socket file descriptor
  \param[in]   req      pointer to request that will be set
  \param[in]   ipaddr   ip address string just like 127.0.0.1
  \return 0: success, others: error
  */

int set_ip_addr(int fd, struct ifreq *req, const char *ipaddr)
{
	struct sockaddr_in *sockaddr;
	int ret;
	sockaddr = (struct sockaddr_in *)&req->ifr_addr;
	sockaddr->sin_port = 0;
	sockaddr->sin_family = AF_INET;
	sockaddr->sin_addr.s_addr = check_ip(ipaddr);
	memset(&sockaddr->sin_zero, 0, 8);

	ret = ioctl(fd, SIOCSIFADDR, req);
	return ret;
}

/**
  \brief         set Netmask address
  \param[in]   fd       socket file descriptor
  \param[in]   req      pointer to request that will be set
  \param[in]   ipaddr   netmask address string just like 255.255.255.0
  \return 0: success, others: error
  */

int set_netmask_addr(int fd, struct ifreq *req, const char *ipaddr)
{
	struct sockaddr_in *sockaddr;
	int ret;
	sockaddr = (struct sockaddr_in *)&req->ifr_netmask;
	sockaddr->sin_port = 0;
	sockaddr->sin_family = AF_INET;
	sockaddr->sin_addr.s_addr = check_ip(ipaddr);
	memset(&sockaddr->sin_zero, 0, 8);

	ret = ioctl(fd, SIOCSIFNETMASK, req);
	return ret;
}

/**
  \brief         set Gateway IP
  \param[in]   fd       socket file descriptor
  \param[in]   req      pointer to request that will be set
  \param[in]   ipaddr   gateway ip address string just like 192.168.0.1
  \return 0: success, others: error
  */

int set_dst_addr(int fd, struct ifreq *req, const char *ipaddr)
{
	struct sockaddr_in *sockaddr;
	int ret;
	sockaddr = (struct sockaddr_in *)&req->ifr_dstaddr;
	sockaddr->sin_port = 0;
	sockaddr->sin_family = AF_INET;
	sockaddr->sin_addr.s_addr = check_ip(ipaddr);
	memset(&sockaddr->sin_zero, 0, 8);

	ret = ioctl(fd, SIOCSIFDSTADDR, req);
	return ret;
}

/**
  \brief         Get IP address
  \param[in]   fd       socket file descriptor
  \param[out]   req      pointer to request that will be got
  \param[out]  addr     It will store the IP address
  \return 0: success, others: error
  */

int get_ip_addr(int fd, struct ifreq *req, char *addr)
{
	struct sockaddr_in *sockaddr;
	int ret;
	sockaddr = (struct sockaddr_in *)&req->ifr_addr;
	ret = ioctl(fd, SIOCGIFADDR, req);
	strncpy(addr, inet_ntoa(sockaddr->sin_addr), IPADDRLEN);
	return ret;
}

/**
  \brief         Get Netmask address
  \param[in]   fd       socket file descriptor
  \param[out]   req      pointer to request that will be got
  \param[out]  addr     It will store the Netmask address
  \return 0: success, others: error
  */

int get_netmask_addr(int fd, struct ifreq *req, char *addr)
{
	struct sockaddr_in *sockaddr;
	int ret;
	sockaddr = (struct sockaddr_in *)&req->ifr_netmask;
	ret = ioctl(fd, SIOCGIFNETMASK, req);
	strncpy(addr, inet_ntoa(sockaddr->sin_addr), IPADDRLEN);
	return ret;
}

/**
  \brief         Get GatewayIP address
  \param[in]   fd       socket file descriptor
  \param[out]   req      pointer to request that will be got
  \param[out]  addr     It will store the Gateway IP address
  \return 0: success, others: error
  */

int get_dst_addr(int fd, struct ifreq *req, char *addr)
{
	struct sockaddr_in *sockaddr;
	int ret;
	sockaddr = (struct sockaddr_in *)&req->ifr_netmask;
	ret = ioctl(fd, SIOCGIFDSTADDR, req);
	strncpy(addr, inet_ntoa(sockaddr->sin_addr), IPADDRLEN);
	return ret;
}

/**
  \brief        Display help information
  */

static void help_info(void)
{
	printf("%s %d.%d\n", prog_name, CSKY_NETWORK_MAJOR_NUM,
	       CSKY_NETWORK_MINOR_NUM);
	printf("Usage: %s device_name [ipaddr] [netmask] [gateway] \n",
	       prog_name);
	printf
	    ("You must check you have the permission to work on the ethernet card\n");
	printf
	    ("When only device_name is given, it will show information of this card\n");
	printf("Otherwise, it will modify ethernet settings.\n");
}
