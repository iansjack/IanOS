#include <net.h>

struct ping *ping_packet(struct mac mac, struct ip_address ip)
{
	struct ping *packet = (struct ping *)malloc(sizeof(struct ping));
	construct_eth_header(&packet->eth, mac, TYPE_IP);
	construct_ip_header(&packet->ip, 0x01, my_ip, ip);
	packet->data.type = 8;
	packet->data.code = 0;
	packet->data.checksum = 0;
	packet->data.identifier = 0x0883;
	packet->data.sequence = 0;
    packet->ip.length = be(sizeof(struct ip_header) + sizeof(struct ping_data));
    packet->ip.dsp_ecn = 0;
    packet->ip.identification = 0x0800;
    packet->ip.checksum = 0;
    packet->ip.checksum = checksum((unsigned char *)(&packet->ip), 20);
    unsigned char data[40] = {
    		0x2a, 0x7f, 0x0a, 0x00, 0x00, 0x00,
    		0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
    		0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d,
    		0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
    		0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d,
    		0x1e, 0x1f};
    int i;
    for (i = 0; i < 40; i++)
    	packet->data.data[i] = data[i];
    packet->data.checksum = checksum((unsigned char *)(&packet->data), 56);
	return packet;
}

