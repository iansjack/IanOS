#include <syscalls.h>
#include <stdlib.h>
#include <kernel.h>
#include <net.h>
#include <timer.h>
#include <netlib.h>
#include <stdint.h>

struct arp_table_entry *arp_table = 0;
struct ip_address my_ip = {0, 0, 0, 0};
struct ip_address netmask = {0, 0, 0, 0};
struct ip_address dns_server = {0, 0, 0, 0};
struct socket *udp_sockets = 0;
struct TCB *tcbs = 0;
void *server_shared_memory;

void HandleTCP(struct tcp_packet *packet);

void HandleUDP(struct udp_packet *packet)
{
	int port = be(packet->udp.destination_port);
	struct socket *temp = udp_sockets;
	while (temp)
	{
		if (temp->port == port)
			break;
		temp = temp->next;
	}
	if (temp)
	{
		int length = be(packet->udp.length);
		struct Message *msg = (struct Message *)malloc(sizeof(struct Message));
		msg->byte = PACKET_READY;
		msg->quad1 = length;
		memcpy(temp->buffer, packet, length);
		sys_sendmessage(temp->msgport, msg);
	}
}

void HandleICMP(struct icmp_packet *packet)
{
	switch (packet->icmp.type)
	{
	case 8 /* ECHO */:
		set_mac_address(&packet->eth.destination, packet->eth.source);
		set_mac_address(&packet->eth.source, my_address);
		set_ip_address(&packet->ip.destination_ip, packet->ip.source_ip);
		set_ip_address(&packet->ip.source_ip, my_ip);
		packet->icmp.type = 0;
		packet->icmp.checksum = 0;
		unsigned short length = be(packet->ip.length);
		packet->icmp.checksum = checksum(&packet->icmp, length - sizeof(struct ip_header));
		sys_queuepacket(packet, length + sizeof(struct eth_header));
		break;
	default:
		printf("Unknown ICMP packet\n");
	}
}

void HandleIP4(struct ip4_packet *packet)
{
	switch (packet->ip.protocol)
	{
	case PROTOCOL_UDP:
		HandleUDP((struct udp_packet *)packet);
		break;
	case PROTOCOL_TCP:
		HandleTCP((struct tcp_packet *)packet);
		break;
	case PROTOCOL_ICMP:
		HandleICMP((struct icmp_packet *)packet);
		break;
	default:
		printf("Unknown IP4 type\n");
	}
}

void HandleArp(struct ARP_Packet *packet)
{
	uint16_t op = be(packet->data.arp_opcode);
	switch (op)
	{
	case ARP_REQUEST:
		if (ip_to_long(packet->data.arp_target_ip) == ip_to_long(my_ip))
		{
			struct ARP_Packet *reply = arp_reply(packet->data.arp_sender_ip, packet->data.arp_sender_mac);
			sys_queuepacket(reply, sizeof(struct ARP_Packet));
			free(reply);
		}
		break;
	case ARP_REPLY:
		;
		struct arp_table_entry *temp = (struct arp_table_entry *)malloc(sizeof(struct arp_table_entry));
		set_ip_address(&temp->ip, packet->data.arp_sender_ip);
		set_mac_address(&temp->mac, packet->data.arp_sender_mac);
		temp->next = arp_table;
		arp_table = temp;
		break;
	default:
		printf("Unknown ARP opcode\n");
	}
}

int main(int argc, char **argv)
{
	server_shared_memory = (void *)SharedMemory;
	struct MessagePort *NetPort = sys_getnetport();
	struct Message *NetMsg = (struct Message *)malloc(sizeof(struct Message));

	struct packet *packet = (struct packet *)malloc(MAX_PACKET_SIZE);
	struct socket *newsocket = 0;
	struct TCB *newtcb;

	int length;
	while (1)
	{
		sys_receivemessage(NetPort, NetMsg);
		switch (NetMsg->byte)
		{
		case PACKETRECEIVED:
			while (sys_receivepacket(packet, &length) != -1)
			{
				switch (packet->eth_header.type)
				{
				case ETH_TYPE_IP4:
					HandleIP4((struct ip4_packet *)packet);
					break;
				case ETH_TYPE_ARP:
					HandleArp((struct ARP_Packet *)packet);
					break;
				default:
					printf("*** Unrecognized ethernet packet ***\n");
				}
			}
			break;

		case OPEN_UDP_SOCKET:
			newsocket = (struct socket *)malloc(sizeof(struct socket));
			newsocket->ip = NetMsg->quad1;
			newsocket->port = NetMsg->quad2;
			newsocket->msgport = (struct MessagePort *)(NetMsg->tempPort);
			newsocket->next = udp_sockets;
			udp_sockets = newsocket;
			Alloc_Shared_Page(NetMsg->pid, server_shared_memory, (void *)NetMsg->quad3);
			newsocket->buffer = server_shared_memory;
			server_shared_memory += 0x1000;
			break;

		case CLOSE_UDP_SOCKET:
			;
			struct socket *temp2 = udp_sockets;
			struct socket *last = 0;
			long port = NetMsg->quad2;
			while (temp2->next)
			{
				if (temp2->port == port)
					break;
				last = temp2;
				temp2 = temp2->next;
			}
			if (temp2)
			{
				if (last)
					last->next = temp2->next;
				else
					udp_sockets = temp2->next;
				free(temp2);
			}
			break;

		case OPEN_TCP_SOCKET_PASSIVE:
			newtcb = (struct TCB *)malloc(sizeof(struct TCB));
			newtcb->source_ip = ip_to_long(my_ip);
			newtcb->source_port = NetMsg->quad2;
			newtcb->dest_ip = 0;
			newtcb->dest_port = 0;
			newtcb->msgport = (struct MessagePort *)(NetMsg->tempPort);
			newtcb->next = tcbs;
			newtcb->state = LISTEN;
			tcbs = newtcb;
			Alloc_Shared_Page(NetMsg->pid, server_shared_memory, (void *)NetMsg->quad3);
			newtcb->transfer_buffer = server_shared_memory;
			newtcb->buffer = (unsigned char *)malloc(TCP_BUFFER_SIZE);
			newtcb->buffer_start = newtcb->buffer;
			newtcb->buffer_end = newtcb->buffer;
			server_shared_memory += 0x1000;
			NetMsg->quad1 = (long)newtcb;
			sys_sendmessage(NetMsg->tempPort, NetMsg);
			break;

		case GETMACOFIP:
			;
			int firsttime = 1;
			struct arp_table_entry *temp = arp_table;
			while (temp)
			{
				int ip = ip_to_long(temp->ip);
				if (NetMsg->quad1 == ip)
					break;
				temp = temp->next;
			}
			if (temp)
				NetMsg->quad2 = mac_to_long(temp->mac);
			else
			{
				struct ARP_Packet *packet = arp_request(long_to_ip(NetMsg->quad1));
				sys_queuepacket(packet, sizeof(struct ARP_Packet));
				NetMsg->quad3 = -1;
			}
			sys_sendmessage(NetMsg->tempPort, NetMsg);
			break;

		case GETMYIP:
			NetMsg->quad1 = ip_to_long(my_ip);
			sys_sendmessage(NetMsg->tempPort, NetMsg);
			break;

		case SETMYIP:
			set_ip_address(&my_ip, NetMsg->quad1);
			break;

		case READ_SOCKET:
			;
			struct TCB *tcb = (struct TCB *)(NetMsg->quad2);
			int available = tcb->buffer_end - tcb->buffer_start;
			int requested = NetMsg->quad1;
			if (requested > available)
				requested = available;
			if (available)
			{
				if ((long)(tcb->buffer_start - tcb->buffer + requested) < TCP_BUFFER_SIZE)
				{
					memcpy(tcb->transfer_buffer, tcb->buffer_start, requested);
					tcb->buffer_start += requested;
				}
				else
				{
					memcpy(tcb->transfer_buffer, tcb->buffer_start, TCP_BUFFER_SIZE - (long)(tcb->buffer_start));
					memcpy((long)(tcb->transfer_buffer) + TCP_BUFFER_SIZE - (long)tcb->buffer_start, tcb->buffer, length - (TCP_BUFFER_SIZE - (long)(tcb->buffer_start)));
					tcb->buffer_start += requested - TCP_BUFFER_SIZE;
				}

				NetMsg->quad1 = requested;
			}
			else
				NetMsg->quad1 = 0;
			sys_sendmessage(NetMsg->tempPort, NetMsg);
			break;

		case TIMER_MSG:
			printf("Timer message received by netserver task from timer %d\n", NetMsg->quad1);
			break;

		default:
			printf("*** Unrecognized message received by netserver ***\n");
		}
	}
	return (0);
}
