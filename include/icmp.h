/*
 * icmp.h
 *
 *  Created on: Jun 16, 2015
 *      Author: ian
 */

#ifndef ICMP_H_
#define ICMP_H_


struct __attribute__ ((__packed__)) icmp_header
{
	unsigned char type;
	unsigned char code;
	unsigned short checksum;
};

struct __attribute__ ((__packed__)) icmp_packet
{
	struct eth_header eth;
	struct ip_header ip;
	struct icmp_header icmp;
	unsigned char * data;
};

#endif /* ICMP_H_ */
