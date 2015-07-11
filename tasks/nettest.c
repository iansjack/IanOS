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

	// Wait until there is data to read
	sys_receivemessage(socket->messagePort, &msg);
	readTCPSocket(socket, buffer, 10);

	printf("Data read from socket:\n%s\n", buffer);

	return (0);
}
