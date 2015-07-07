/*
 * dhcp.c
 *
 *  Created on: Jun 17, 2015
 *      Author: ian
 */

#include <net.h>
#include <syscalls.h>
#include <stdlib.h>
#include <netlib.h>

int main(int argc, char **argv)
{
	// Open a socket to DHCP server
	struct udp_handle *handle = (struct udp_handle *)malloc(sizeof(struct udp_handle));
	openUDPSocket(handle, 0, 68);

	// Send a DISCOVER packet
	struct dhcp *dhcp_packet = dhcp();
	add_option_message_type(dhcp_packet, DHCPDISCOVER);
	add_end_option(dhcp_packet);
	sys_queuepacket(dhcp_packet, sizeof(struct dhcp));

	struct Message *msg = (struct Message *)malloc(sizeof(struct Message));
	int done = 0;
	while (!done)
	{
		sys_receivemessage(handle->port, msg);
		struct udp_packet *reply = (struct udp_packet *)handle->buffer;
		struct dhcp_data *data = (struct dhcp_data *)(&reply->data);
		unsigned char *options = data->options;
		unsigned char messageType = 0;

		if (options[0] == 53)
			messageType = options[2];
		else
			printf("Error - first DHCP option not DHCP Message Type\n");

		switch (messageType)
		{
		case DHCPOFFER:
			;
			struct dhcp *dhcp_packet = dhcp();
			add_option_message_type(dhcp_packet, DHCPREQUEST);
			add_option_requested_ip(dhcp_packet, data->YIAddr);
			add_option_server_ip(dhcp_packet, data->SIAddr);
			add_end_option(dhcp_packet);
			sys_queuepacket(dhcp_packet, sizeof(struct dhcp));
			free(dhcp_packet);
			break;

		case DHCPACK:
			msg->byte = SETMYIP;
			msg->quad1 = ip_to_long(data->YIAddr);
			sys_sendmessage(sys_getnetport(), msg);
			done = 1;
			break;

		default:
			printf("Unexpected DHCP message type\n");
		}
	}
	free(dhcp_packet);
	free(handle);
	free(msg);
	closeUDPSocket(0, 68);
	return (0);
}
