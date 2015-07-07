#include <kernel.h>
#include <stdlib.h>
#include <net.h>
#include <netlib.h>

unsigned short option_pointer = 0;

void construct_dhcp_data(struct dhcp_data *data, unsigned char type)
{
	data->op = type;
	data->htype = 1;
	data->hlen = 6;
	data->CIAddr = long_to_ip(0);
	set_mac_address(&data->CHAddr, my_address);
	data->cookie = 0x63538263;
}

void add_option(struct dhcp *packet, unsigned char option, unsigned char length, /*unsigned*/ char *value)
{
	packet->data.options[option_pointer++] = option;
	packet->data.options[option_pointer++] = length;
	int i;
	for (i = 0; i < length; i++)
		packet->data.options[option_pointer++] = value[i];
}

void add_end_option(struct dhcp *packet)
{
	packet->data.options[option_pointer] = 0xff;
	option_pointer = 0;
}

struct dhcp *dhcp()
{
	struct dhcp *packet = (struct dhcp *)(malloc(sizeof(struct dhcp)));

    construct_eth_header(&(packet->eth), broadcast_address, TYPE_IP);

    construct_ip_header(&packet->ip, PROTOCOL_UDP, null_ip, broadcast_ip);
    construct_udp_header(&packet->udp, 0x44, 0x43);
    construct_dhcp_data(&packet->data, BOOT_REQUEST);

    packet->ip.identification = be(1);
    packet->ip.length = be(sizeof(struct ip_header) + sizeof(struct udp_header) + sizeof(struct dhcp_data));
    packet->ip.checksum = checksum((unsigned char *)(&packet->ip), 20);

    packet->udp.length = be(sizeof(struct udp_header) + sizeof(struct dhcp_data));

    packet->data.xid = 0x01000000;

    return packet;
}

void add_option_message_type(struct dhcp *packet, char type)
{
	add_option(packet, 53, 1, &type);
}

void add_option_server_ip(struct dhcp *packet, struct ip_address ip)
{
	add_option(packet, 54, 4, (char *)(&ip));
}
void add_option_requested_ip(struct dhcp *packet, struct ip_address ip)
{
	add_option(packet, 50, 4, (char *)(&ip));
}

long GetMyIP()
{
	struct Message *message = (struct Message *)malloc(sizeof(struct Message));
	message->byte = GETMYIP;
	sys_sendreceive(sys_getnetport(), message);
	long retval = message->quad1;
	free(message);
	return retval;
}
