/*
 * arp.h
 *
 *  Created on: May 15, 2015
 *      Author: ian
 */

#ifndef ARP_H_
#define ARP_H_

#include <ip.h>
#include <eth.h>

struct arp_table_entry
{
	struct ip_address ip;
	struct mac_address mac;
	struct arp_table_entry *next;
};

struct __attribute__ ((__packed__)) ARP_data
{
	unsigned short arp_hw_type;
	unsigned short arp_prot_type;
	unsigned char arp_hw_size;
	unsigned char arp_prot_size;
	unsigned short arp_opcode;
	struct mac_address arp_sender_mac;
	struct ip_address arp_sender_ip;
	struct mac_address arp_target_mac;
	struct ip_address arp_target_ip;
	unsigned char padding[18];
};

struct __attribute__ ((__packed__)) ARP_Packet
{
	struct eth_header eth;
	struct ARP_data data;
};

#define ARP_REQUEST	0x0001
#define ARP_REPLY	0x0002

long MacFromIP(struct ip_address ip);

#endif /* ARP_H_ */
