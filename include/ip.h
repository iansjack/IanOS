/*
 * ip.h
 *
 *  Created on: May 15, 2015
 *      Author: ian
 */

#ifndef IP_H_
#define IP_H_

#include <stdint.h>

struct ip_address
{
	uint8_t	add1;
	uint8_t	add2;
	uint8_t	add3;
	uint8_t	add4;
};

struct __attribute__ ((__packed__)) ip_header
{
	uint8_t	version_ihl;
	uint8_t	dsp_ecn;
	uint16_t length;
	uint16_t identification;
	uint16_t flags_fragmentoffset;
	uint8_t	ttl;
	uint8_t	protocol;
	uint16_t	checksum;
	struct	ip_address source_ip;
	struct	ip_address destination_ip;
};

struct __attribute__ ((__packed__)) ip4_packet
{
	struct eth_header eth;
	struct ip_header ip;
	uint8_t *ip_data;
};

#define	PROTOCOL_ICMP	0x01
#define	PROTOCOL_TCP	0x06
#define PROTOCOL_UPD	0x11

#endif /* IP_H_ */
