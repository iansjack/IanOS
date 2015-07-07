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

unsigned short tcp_checksum(struct tcp_packet *packet)
{
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

void HandleTCP(struct tcp_packet *packet)
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
			printf("SYN Packet received for port %d\n", port);
			set_mac_address(&packet->eth.destination, packet->eth.source);
			set_mac_address(&packet->eth.source, my_address);
			struct ip_address me;
			set_ip_address(&me, packet->ip.destination_ip);
			set_ip_address(&packet->ip.destination_ip, packet->ip.source_ip);
			set_ip_address(&packet->ip.source_ip, me);
			packet->tcp.control = SYN | ACK;
			unsigned short dest_port = packet->tcp.source_port;
			packet->tcp.source_port = packet->tcp.destination_port;
			packet->tcp.destination_port = dest_port;
			packet->tcp.ack_no = be32(be32(packet->tcp.seq_no) + 1);
			packet->tcp.seq_no = be32(1);
			temp->snd_wnd = 1024;
			packet->tcp.windows = be(temp->snd_wnd);
			packet->tcp.checksum = 0;
			packet->tcp.checksum = be(tcp_checksum(packet));
			sys_queuepacket(packet, 0x3a /*sizeof(*packet)*/);
			temp->state = SYN_RECEIVED;
		}
		break;
	case SYN_RECEIVED:
		if (packet->tcp.control & ACK)
		{
			printf("ACK received for port %d\n", port);
			temp->rcv_wnd = be(packet->tcp.windows);
			temp->rcv_nxt = be32(packet->tcp.seq_no);
			temp->snd_una = 1;
			temp->snd_nxt = 1;
			temp->state = ESTABLISHED;
			printf("snd.una = %x, snd.nxt = %x, snd_wnd = %x, rcv_nxt = %x, rcv_wnd = %x\n",
					temp->snd_una, temp->snd_nxt, temp->snd_wnd, temp->rcv_nxt, temp->rcv_wnd);
		}
		break;
	case ESTABLISHED:
		printf("Data packet received for port %d\n", port);
		temp->rcv_wnd = be(packet->tcp.windows);
		temp->rcv_nxt = be32(packet->tcp.seq_no);
		temp->snd_una = 1;
		temp->snd_nxt = 1;
		printf("snd.una = %x, snd.nxt = %x, snd_wnd = %x, rcv_nxt = %x, rcv_wnd = %x\n",
				temp->snd_una, temp->snd_nxt, temp->snd_wnd, temp->rcv_nxt, temp->rcv_wnd);

		set_mac_address(&packet->eth.destination, packet->eth.source);
		set_mac_address(&packet->eth.source, my_address);
		struct ip_address me;
		set_ip_address(&me, packet->ip.destination_ip);
		set_ip_address(&packet->ip.destination_ip, packet->ip.source_ip);
		set_ip_address(&packet->ip.source_ip, me);
		packet->tcp.control = ACK;
		unsigned short dest_port = packet->tcp.source_port;
		int length = be(packet->ip.length);
		packet->ip.length = be(40);
		packet->ip.checksum = 0;
		packet->ip.checksum = checksum(&packet->ip, 20);
		packet->tcp.source_port = packet->tcp.destination_port;
		packet->tcp.destination_port = dest_port;
		packet->tcp.ack_no = be32(be32(packet->tcp.seq_no) + length - 40);
		packet->tcp.seq_no = be32(2);
		temp->snd_wnd = 1024;
		packet->tcp.windows = be(temp->snd_wnd);
		packet->tcp.checksum = 0;
		packet->tcp.checksum = be(tcp_checksum(packet));
		sys_queuepacket(packet, 0x36 /*sizeof(*packet)*/);

		break;
	default:
		break;
	}
}


