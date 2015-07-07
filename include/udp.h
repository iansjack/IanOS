/*
 * udp.h
 *
 *  Created on: Jun 6, 2015
 *      Author: ian
 */

#ifndef UDP_H_
#define UDP_H_

struct __attribute__ ((__packed__)) udp_header
{
	unsigned short source_port;
	unsigned short destination_port;
	unsigned short length;
	unsigned short checksum;
};

struct __attribute__ ((__packed__)) udp_packet
{
	struct eth_header eth;
	struct ip_header ip;
	struct udp_header udp;
	unsigned char * data;
};

struct udp_handle
{
	struct MessagePort *port;
	char *buffer;
};

struct socket
{
	long int ip;
	long int port;
	unsigned char *buffer;
	struct MessagePort *msgport;
	struct socket *next;
};

#endif /* UDP_H_ */
