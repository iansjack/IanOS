#include <stdlib.h>
#include <kernel.h>
#include <net.h>
#include <syscalls.h>

struct mac_address null_address = {0,0,0,0,0,0};
struct mac_address broadcast_address = {255,255,255,255,255,255};
struct mac_address my_address = {0x52,0x54,0x00,0x12,0x34,0x56};
//struct ip_address my_ip;
struct ip_address null_ip = {0, 0, 0, 0};
struct ip_address broadcast_ip = {255, 255, 255, 255};
void *shared_memory = (void *)SharedMemory;

unsigned short be(unsigned short number)
{
	return ((number & 0xff) << 8) + ((number >> 8) & 0xff);
}

unsigned int be32(unsigned int number)
{
	return ((number & 0xff) << 24) + ((number & 0xff00) << 8) + ((number & 0xff0000) >> 8) + ((number & 0xff000000) >> 24);
}

void set_ip_address(struct ip_address *to, struct ip_address from)
{
	to->add1 = from.add1;
	to->add2 = from.add2;
	to->add3 = from.add3;
	to->add4 = from.add4;
}

void set_mac_address(struct mac_address *to, struct mac_address from)
{
	to->add1 = from.add1;
	to->add2 = from.add2;
	to->add3 = from.add3;
	to->add4 = from.add4;
	to->add5 = from.add5;
	to->add6 = from.add6;
}

unsigned short checksum(unsigned char *buffer, int length)
{
	int i;
	unsigned int sum = 0;

	for (i = 0; i < length; i += 2)
	{
		unsigned short temp = (buffer[i + 1] << 8) + buffer[i];
		sum += temp;
	}
	unsigned short checksum = (unsigned short)((sum >> 16) & 0xFFFF) + (unsigned short)(sum & 0xFFFF);
	checksum = ~checksum;
	checksum &= 0xFFFF;
	return checksum;
}

void construct_eth_header(struct eth_header *header, struct mac_address destination, unsigned short type)
{
	int i = 0;
	for (i = 0; i < 6; i++)
	{
		header->destination.add1 = destination.add1;
		header->source.add1 = my_address.add1;
		header->destination.add2 = destination.add2;
		header->source.add2 = my_address.add2;
		header->destination.add3 = destination.add3;
		header->source.add3 = my_address.add3;
		header->destination.add4 = destination.add4;
		header->source.add4 = my_address.add4;
		header->destination.add5 = destination.add5;
		header->source.add5 = my_address.add5;
		header->destination.add6 = destination.add6;
		header->source.add6 = my_address.add6;
	}
	header->type = be(type);
}

void construct_ip_header(struct ip_header *header, unsigned char protocol, struct ip_address source, struct ip_address dest)
{
	header->version_ihl = 0x45;
	header->dsp_ecn = 0x0;
	header->length = 0;
	header->identification = 0;
	header->flags_fragmentoffset = 0;
	header->ttl = 0x40;
	header->protocol = 0;
	header->checksum = 0;
	header->protocol = protocol;
	set_ip_address(&header->source_ip, source);
	set_ip_address(&header->destination_ip, dest);
}

void construct_udp_header(struct udp_header *header, unsigned char source, unsigned char dest)
{
	header->source_port = be(source);
	header->destination_port = be(dest);
	header->length = 0;
	header->checksum = 0;
}

long ip_to_long(struct ip_address ip)
{
	long ret = ip.add1;
	ret <<= 8;
	ret += ip.add2;
	ret <<= 8;
	ret += ip.add3;
	ret <<= 8;
	ret += ip.add4;
	return ret;
}

struct ip_address long_to_ip(long ip)
{
	struct ip_address ret;
	ret.add4 = ip & 0xff;
	ip >>= 8;
	ret.add3 = ip & 0xff;
	ip >>= 8;
	ret.add2 = ip & 0xff;
	ip >>= 8;
	ret.add1 = ip &0xff;
	return ret;
}

long mac_to_long(struct mac_address mac)
{
	long ret = mac.add1;
	ret <<= 8;
	ret += mac.add2;
	ret <<= 8;
	ret += mac.add3;
	ret <<= 8;
	ret += mac.add4;
	ret <<= 8;
	ret += mac.add5;
	ret <<= 8;
	ret += mac.add6;
	return ret;
}

void openUDPSocket(struct udp_handle *handle, long ip, long port)
{
	struct MessagePort *NetPort = sys_getnetport();
	struct MessagePort *msgport = sys_allocmessageport();
	handle->port = msgport;
	struct Message *message = (struct Message *)malloc(sizeof(struct Message));
	message->byte = OPEN_UDP_SOCKET;
	message->tempPort = msgport;
	message->quad1 = ip;
	message->quad2 = port;
	message->quad3 = (long)shared_memory;
	message->pid = getpid();
	sys_sendmessage(NetPort, message);
	handle->buffer = shared_memory;
	shared_memory += 0x1000;
	free(message);
}

void closeUDPSocket(long ip, long port)
{
	struct MessagePort *NetPort = sys_getnetport();
	struct Message *message = (struct Message *)malloc(sizeof(struct Message));
	message->byte = CLOSE_UDP_SOCKET;
	message->quad1 = ip;
	message->quad2 = port;
	sys_sendmessage(NetPort, message);
	free(message);
}

void openListeningSocket(struct TCPSocket *socket, int port)
{
	struct MessagePort *NetPort = sys_getnetport();
	socket->messagePort = sys_allocmessageport();
	struct Message *message = (struct Message *)malloc(sizeof(struct Message));
	message->byte = OPEN_TCP_SOCKET_PASSIVE;
	message->tempPort = socket->messagePort;
	message->quad1 = 0;
	message->quad2 = port;
	socket->transfer_buffer = shared_memory;
	message->quad3 = (long)shared_memory;
	sys_sendmessage(NetPort, message);
	sys_receivemessage(socket->messagePort, message);
	socket->tcb = (struct TCB *)(message->quad1);
	shared_memory += 0x1000;
	free(message);
}

int readTCPSocket(struct TCPSocket *socket, unsigned char *buffer, unsigned int size)
{
	struct MessagePort *NetPort = sys_getnetport();
	struct Message *message = (struct Message *)malloc(sizeof(struct Message));
	message->byte = READ_SOCKET;
	message->quad1 = size;
	message->quad2 = (long)(socket->tcb);
	sys_sendreceive(NetPort, message);
	// If no data is returned wait for a message saying data is available and try again
	if (!message->quad1)
	{
		sys_receivemessage(socket->messagePort, message);
		message->byte = READ_SOCKET;
		message->quad1 = size;
		message->quad2 = (long)(socket->tcb);
		sys_sendreceive(NetPort, message);
	}

	memcpy(buffer, socket->transfer_buffer, message->quad1);
	return (message->quad1);
}
