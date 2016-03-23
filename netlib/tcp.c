/*
 * tcp.c
 *
 *  Created on: Jun 12, 2015
 *      Author: ian
 */

#include <stdlib.h>
#include <kernel.h>
#include <net.h>
#include <netlib.h>

extern struct TCB *tcbs;

void dummy()
{
	printf("Debugging\n");
}

unsigned short tcp_checksum(struct tcp_packet *packet)
{
	packet->tcp.checksum = 0;
	int i;
	unsigned int sum = 0;
	unsigned short tcp_segment_length = be(packet->ip.length) - (packet->ip.version_ihl & 0xf) * 4;

	sum += (packet->ip.source_ip.add1 << 8) + packet->ip.source_ip.add2;
	sum += (packet->ip.source_ip.add3 << 8) + packet->ip.source_ip.add4;
	sum += (packet->ip.destination_ip.add1 << 8) + packet->ip.destination_ip.add2;
	sum += (packet->ip.destination_ip.add3 << 8) + packet->ip.destination_ip.add4;
	sum += 0x6;
	sum += tcp_segment_length;

	unsigned short *buffer = (unsigned short *)&packet->tcp;
	for (i = 0; i < tcp_segment_length / 2; i++)
		sum += be(buffer[i]);

	unsigned short checksum = (unsigned short)((sum >> 16) & 0xFFFF) + (unsigned short)(sum & 0xFFFF);
	checksum = ~checksum;
	checksum &= 0xFFFF;
	return checksum;

}

void prepareReturn(struct tcp_packet *packet)
{
	set_mac_address(&packet->eth.destination, packet->eth.source);
	set_mac_address(&packet->eth.source, my_address);
	struct ip_address me;
	set_ip_address(&me, packet->ip.destination_ip);
	set_ip_address(&packet->ip.destination_ip, packet->ip.source_ip);
	set_ip_address(&packet->ip.source_ip, me);
	unsigned short dest_port = packet->tcp.source_port;
	packet->tcp.source_port = packet->tcp.destination_port;
	packet->tcp.destination_port = dest_port;
}

int packetLength(struct tcp_packet *packet)
{
	return sizeof(struct eth_header) + be(packet->ip.length);
}

void HandleTCP(struct tcp_packet *packet, struct TCB *tcbs)
{
	unsigned short port = be(packet->tcp.destination_port);
	struct TCB *temp = tcbs;
	while (temp)
	{
		if (temp->source_port == port)
			break;
		temp = temp->next;
	}
	if (!temp)
	{
		printf("Packet received for unopened tcp port %d\n", port);
		return;
	}
	switch (temp->state)
	{
	case LISTEN:
		if (packet->tcp.control & SYN)
		{
			prepareReturn(packet);
			packet->tcp.control = SYN | ACK;
			packet->tcp.ack_no = be32(be32(packet->tcp.seq_no) + 1);
			packet->tcp.seq_no = be32(1);
			temp->snd_wnd = 1024;
			packet->tcp.windows = be(temp->snd_wnd);
			packet->tcp.checksum = be(tcp_checksum(packet));
			sys_queuepacket(packet, packetLength(packet));
			temp->state = SYN_RECEIVED;
		}
		break;
	case SYN_RECEIVED:
		if (packet->tcp.control & ACK)
		{
			temp->rcv_wnd = be(packet->tcp.windows);
			temp->rcv_nxt = be32(packet->tcp.seq_no);
			temp->snd_una = 1;
			temp->snd_nxt = 1;
			temp->state = ESTABLISHED;
		}
		break;
	case ESTABLISHED:
		temp->rcv_wnd = be(packet->tcp.windows);
		temp->rcv_nxt = be32(packet->tcp.seq_no);
		temp->snd_una = 1;
		temp->snd_nxt = 1;

		int length = be(packet->ip.length);
		unsigned char acknowledge = 0;
		if (temp->buffer_start == temp->buffer_end)
			acknowledge = 1;
		if ((long)(temp->buffer_start - temp->buffer + length) < TCP_BUFFER_SIZE)
		{
			memcpy(temp->buffer_start, &(packet->data), length);
			temp->buffer_end += length;
		}
		else
		{
			memcpy(temp->buffer_start, &(packet->data), TCP_BUFFER_SIZE - (long)(temp->buffer_start));
			memcpy(temp->buffer, (long)&(packet->data) + TCP_BUFFER_SIZE - (long)(temp->buffer_start), length - (TCP_BUFFER_SIZE - (long)(temp->buffer_start)));
			temp->buffer_end += (length - TCP_BUFFER_SIZE);
		}

		prepareReturn(packet);
		packet->tcp.control = ACK;
		packet->ip.length = be(40);
		packet->ip.checksum = 0;
		packet->ip.checksum = checksum(&packet->ip, 20);
		packet->tcp.ack_no = be32(be32(packet->tcp.seq_no) + length - 40);
		packet->tcp.seq_no = be32(2);
		temp->snd_wnd = 1024;
		packet->tcp.windows = be(temp->snd_wnd);
		packet->tcp.checksum = be(tcp_checksum(packet));
		sys_queuepacket(packet, packetLength(packet));
		if (acknowledge)
		{
			struct Message msg;
			sys_sendmessage(temp->msgport, &msg);
		}
		break;
	default:
		break;
	}
}


