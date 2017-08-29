/*
 * Network transferring test module
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
static const char *prog_name = "csky_nettrans_example\0";

/**
  \brief       Send tcp packet to the TCP Receiving server
  \param[in]   host   TCP Receiving server host
  */
void sendtcp_client(unsigned long host)
{
	struct sockaddr_in myaddr;
	char *outbuf;
	int sockfd;
	socklen_t addrlen;
	int nbytessent, ch, i;

	/* Allocate buffers */

	outbuf = (char *)malloc(SENDSIZE);
	if (!outbuf) {
		printf("client: failed to allocate buffers\n");
		exit(1);
	}

	/* Create a new TCP socket */

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		printf("client socket failure %d\n", errno);
		free(outbuf);
		exit(-1);
	}

	/* Connect the socket to the server */

	myaddr.sin_family = AF_INET;
	myaddr.sin_port = htons(PORTNO);

	myaddr.sin_addr.s_addr = host;

	addrlen = sizeof(struct sockaddr_in);

	printf("client: Connecting...\n");
	if (connect(sockfd, (struct sockaddr *)&myaddr, addrlen) < 0) {
		printf("client: connect failure: %d\n", errno);
		goto EXIT;
	}

	printf("client: Connected\n");

	/* Initialize the buffer */

	ch = 0x20;
	for (i = 0; i < SENDSIZE; i++) {
		outbuf[i] = ch;
		if (++ch > 0x7e) {
			ch = 0x20;
		}
	}
	/* Then send messages forever */

	while (1) {
		nbytessent = send(sockfd, outbuf, SENDSIZE, 0);
		if (nbytessent < 0) {
			printf("client: send failed: %d\n", errno);
			goto EXIT;
		} else if (nbytessent != SENDSIZE) {
			printf("client: Bad send length=%d: %d of \n",
			       nbytessent, SENDSIZE);
			goto EXIT;
		}

		printf("Sent %d bytes\n", nbytessent);
	}

EXIT:
	close(sockfd);
	free(outbuf);
}

/**
  \brief     To build a TCP Receiving server
  */

void recvtcp_server(void)
{
	struct sockaddr_in myaddr;
	int listensd, acceptsd;
	socklen_t addrlen;
	int nbytesread, optval;

	char buffer[SENDSIZE];

	/* Create a new TCP socket */

	listensd = socket(AF_INET, SOCK_STREAM, 0);
	if (listensd < 0) {
		printf("server: socket failure: %d\n", errno);
		exit(1);
	}

	/* Set socket to reuse address */

	optval = 1;
	if (setsockopt
	    (listensd, SOL_SOCKET, SO_REUSEADDR, (void *)&optval,
	     sizeof(int)) < 0) {
		printf("server: setsockopt SO_REUSEADDR failure: %d\n", errno);
		goto EXIT;
	}

	/* Bind the socket to a local address */

	myaddr.sin_family = AF_INET;
	myaddr.sin_port = htons(PORTNO);
	myaddr.sin_addr.s_addr = INADDR_ANY;
	addrlen = sizeof(struct sockaddr_in);
	if (bind(listensd, (struct sockaddr *)&myaddr, addrlen) < 0) {
		printf("server: bind failure: %d\n", errno);
		goto EXIT;
	}

	/* Listen for connections on the bound TCP socket */

	if (listen(listensd, 5) < 0) {
		printf("server: listen failure %d\n", errno);
		goto EXIT;
	}

	/* Accept only one connection */

	printf("server: Accepting connections on port %d\n", PORTNO);
	acceptsd = accept(listensd, (struct sockaddr *)&myaddr, &addrlen);
	if (acceptsd < 0) {
		printf("server: accept failure: %d\n", errno);
		goto EXIT;
	}
	printf("server: Connection accepted -- receiving\n");

	printf("server: Reading...\n");

	/* Receive data forever */
	while (1) {
		nbytesread = recv(acceptsd, &buffer, SENDSIZE, 0);
		if (nbytesread < 0) {
			printf("server: recv failed: %d\n", errno);
			break;
		} else if (nbytesread == 0) {
			printf("server: The client broke the connection\n");
			break;
		}
		printf("server: Received data \n");

	}
	close(acceptsd);
EXIT:
	close(listensd);
}

/**
  \brief        Do a ping test to the  given host 
  \param[in]   host   Destination Host 
  */

void ping_dst(unsigned long host)
{
	struct icmphdr icmp_hdr;
	struct sockaddr_in addr;
	int sequence = 0;
	unsigned char data[SENDSIZE];
	struct timeval timeout = { 3, 0 };
	fd_set read_set;
	socklen_t slen;
	struct icmphdr rcv_hdr;

	/* Create a icmp protocol socket */
	int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);
	if (sock < 0) {
		perror("socket");
		return;
	}

	/* Initialization */
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = host;
	memset(&icmp_hdr, 0, sizeof(struct icmphdr));
	icmp_hdr.type = ICMP_ECHO;
	icmp_hdr.un.echo.id = 1234;

	/* Ping destination forever */
	while (1) {

		icmp_hdr.un.echo.sequence = sequence++;
		memcpy(data, &icmp_hdr, sizeof(struct icmphdr));
		memcpy(data + sizeof icmp_hdr, "hello", 5);
		if (sendto(sock, data, sizeof(struct icmphdr) + 5,
			   0, (struct sockaddr *)&addr,
			   sizeof(struct sockaddr_in)) <= 0) {
			perror("Send ping");
			break;
		}

		/* Try to check if there is a ping response */
		memset(&read_set, 0, sizeof(fd_set));
		FD_SET(sock, &read_set);

		if (select(sock + 1, &read_set, NULL, NULL, &timeout) <= 0) {
			printf("Cannot receivce ping response.\n");
			break;
		}

		/* Receive ping response */
		slen = 0;
		if (recvfrom(sock, data, SENDSIZE, 0, NULL, &slen) <= 0) {
			perror("ping reply");
			break;
		}
		memcpy(&rcv_hdr, data, sizeof(struct icmphdr));
		if (rcv_hdr.type == ICMP_ECHOREPLY)
			printf("ICMP Reply Got from %s\n",
			       inet_ntoa(addr.sin_addr));

	}
}

int main(int argc, char *argv[])
{
	int32_t host;
	if (argc < 2) {
		help_info();
		return -1;
	}

	if (argc >= 3) {
		host = inet_addr(argv[2]);
		if (host < 0) {
			printf("Invalid IP address given!\n");
			return -1;
		}
	}

	/* Send TCP Packet */
	if (strcmp(argv[1], "sendtcp") == 0 && argc >= 3)
		sendtcp_client(host);

	/* Receive TCP Packet */
	else if (strcmp(argv[1], "tcpserver") == 0)
		recvtcp_server();

	/* Ping Test */
	else if (strcmp(argv[1], "ping") == 0 && argc >= 3)
		ping_dst(host);
	else {
		help_info();
		return -1;
	}

	return 0;
}

static void help_info(void)
{
	printf("%s %d.%d\n", prog_name, CSKY_NETWORK_MAJOR_NUM,
	       CSKY_NETWORK_MINOR_NUM);
	printf("Usage: %s sendtcp/tcpserver/ping [ipaddr] \n", prog_name);
	printf
	    ("If you want to send a ping test , you can type \" %s ping someip \"\n",
	     prog_name);
	printf("If you want test tcp packet tranferring, you can \
type \" %s tcpserver \" to build a tcp receiving server\nor you can\
type \" %s sendtcp someip\" to send tcp packet\n", prog_name, prog_name);
}