/*
 * ip.h
 *
 *  Created on: May 15, 2015
 *      Author: ian
 */

#ifndef IP_H_
#define IP_H_

struct ip_address
{
	uint_8	add1;
	uint_8	add2;
	uint_8	add3;
	uint_8	add4;
};

struct __attribute__ ((__packed__)) ip_header
{
	uint_8	version_ihl;
	uint_8	dsp_ecn;
	uint_16 length;
	uint_16 identification;
	uint_16 flags_fragmentoffset;
	uint_8	ttl;
	uint_8	protocol;
	uint_16	checksum;
	struct	ip_address source_ip;
	struct	ip_address destination_ip;
};

struct __attribute__ ((__packed__)) ip4_packet
{
	struct eth_header eth;
	struct ip_header ip;
	uint_8 *ip_data;
};

#define	PROTOCOL_ICMP	0x01
#define	PROTOCOL_TCP	0x06
#define PROTOCOL_UPD	0x11

#endif /* IP_H_ */
