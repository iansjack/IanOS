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
	openListeningSocket(80);

	printf("After DHCP request - %x\n", GetMyIP());

	return (0);
}
