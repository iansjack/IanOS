/*
 * net.h
 *
 *  Created on: May 15, 2015
 *      Author: ian
 */

#ifndef NET_H_
#define NET_H_

#include <eth.h>
#include <ip.h>
#include <udp.h>
#include <tcp.h>
#include <arp.h>
#include <icmp.h>

#define	PACKETRECEIVED			1
#define OPEN_UDP_SOCKET			2
#define GETMACOFIP				3
#define GETMYIP					4
#define SETMYIP					5
#define OPEN_TCP_SOCKET_PASSIVE	6
#define	GET_BUFFER				7
#define CLOSE_UDP_SOCKET		8
#define PACKET_READY			9

#define MAX_PACKET_SIZE 4096

struct __attribute__ ((__packed__)) packet
{
	struct eth_header eth_header;
	unsigned char *data;
};

struct __attribute__ ((__packed__)) dhcp_data
{
	unsigned char op;
	unsigned char htype;
	unsigned char hlen;
	unsigned char hops;
	unsigned int xid;
	unsigned short secs;
	unsigned short flags;
	struct ip_address CIAddr;
	struct ip_address YIAddr;
	struct ip_address SIAddr;
	struct ip_address GIAddr;
	unsigned char CHAddr[16];
	unsigned char SName[64];
	unsigned char Name[128];
	unsigned int cookie;
	unsigned char options[60];
};

struct __attribute__ ((__packed__)) dhcp
{
	struct eth_header eth;
	struct ip_header ip;
	struct udp_header udp;
	struct dhcp_data data;
};

struct __attribute__ ((__packed__)) ping_data
{
	unsigned char type;
	unsigned char code;
	unsigned short checksum;
	unsigned short identifier;
	unsigned short sequence;
	unsigned char timestamp[8];
	unsigned char data[40];
};

struct __attribute__ ((__packed__)) ping
{
	struct eth_header eth;
	struct ip_header ip;
	struct ping_data data;
};

#define DHCPDISCOVER	1
#define DHCPOFFER		2
#define DHCPREQUEST		3
#define DHCPACK			5

#define TYPE_ARP 	0x0806
#define TYPE_IP 	0x0800

#define BOOT_REQUEST	1

extern struct mac_address null_address;
extern struct mac_address broadcast_address;
extern struct mac_address my_address;
extern struct ip_address my_ip;
extern struct ip_address null_ip;
extern struct ip_address broadcast_ip;

#endif /* NET_H_ */
