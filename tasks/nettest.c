#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <net.h>
#include <syscalls.h>
#include <netlib.h>

#define BUFF_SIZE 1024

int main(int argc, char **argv)
{
	struct TCPSocket *socket = (struct TCPSocket *)malloc(sizeof(struct TCPSocket));
	openListeningSocket(socket, 80);

	unsigned char buffer[1024];

	struct Message msg;

	int read = 0;

	read = readTCPSocket(socket, buffer, 1000);
	buffer[1000] = 0;

	printf("Data read from socket (%d bytes):\n%s\n", read, buffer);

	return (0);
}
