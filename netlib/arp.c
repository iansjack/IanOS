#include <kernel.h>
#include <stdlib.h>
#include <net.h>
#include <netlib.h>

//struct ip_address my_ip = {0, 0, 0, 0};

void construct_arp_request(struct ARP_data *data, struct ip_address ip)
{
	data->arp_hw_type = be(HARDWARE_ETHERNET);
	data->arp_prot_type = be(PROTOCOL_IP);
	data->arp_hw_size = 6;
	data->arp_prot_size = 4;
	data->arp_opcode = be(ARP_REQUEST);
	set_mac_address(&data->arp_target_mac, null_address);
	set_mac_address(&data->arp_sender_mac, my_address);
	set_ip_address(&data->arp_sender_ip, long_to_ip(GetMyIP()));
	set_ip_address(&data->arp_target_ip, ip);
}

struct ARP_Packet *arp_request(struct ip_address ip)
{
	struct ARP_Packet *packet = (struct ARP_Packet *)malloc(sizeof (struct ARP_Packet));
	construct_eth_header(&packet->eth, broadcast_address, TYPE_ARP);
	construct_arp_request(&packet->data, ip);
	return packet;
}

void construct_arp_reply(struct ARP_data *data, struct ip_address ip, struct mac_address mac)
{
	data->arp_hw_type = be(HARDWARE_ETHERNET);
	data->arp_prot_type = be(PROTOCOL_IP);
	data->arp_hw_size = 6;
	data->arp_prot_size = 4;
	data->arp_opcode = be(ARP_REPLY);
	set_mac_address(&data->arp_target_mac, mac);
	set_mac_address(&data->arp_sender_mac, my_address);
	set_ip_address(&data->arp_sender_ip, long_to_ip(GetMyIP()));
	set_ip_address(&data->arp_target_ip, ip);
}

struct ARP_Packet *arp_reply(struct ip_address ip, struct mac_address mac)
{
	struct ARP_Packet *packet = (struct ARP_Packet *)malloc(sizeof (struct ARP_Packet));
	construct_eth_header(&packet->eth, mac, TYPE_ARP);
	construct_arp_reply(&packet->data, ip, mac);
	return packet;
}

long MacFromIP(struct ip_address ip)
{
	struct Message *message = (struct Message *)malloc(sizeof(struct Message));

	struct MessagePort *NetPort = sys_getnetport();
	message->byte = GETMACOFIP;
	message->quad1 = ip_to_long(ip);
	message->quad2 = 0;
	message->quad3 = 0;
	sys_sendreceive(NetPort, message);
	if (message->quad3 == -1)
	{
		sys_nanosleep(10);
		message->quad3 = 0;
		sys_sendreceive(NetPort, message);
	}
	long retval = message->quad2;
	free(message);
	return retval;
}
