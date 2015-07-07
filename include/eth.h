/*
 * eth.h
 *
 *  Created on: May 14, 2015
 *      Author: ian
 */

#ifndef ETH_H_
#define ETH_H_

#include <types.h>

struct mac_address
{
	uint_8	add1;
	uint_8	add2;
	uint_8	add3;
	uint_8	add4;
	uint_8	add5;
	uint_8	add6;
};

struct eth_header
{
	struct mac_address	destination;
	struct mac_address	source;
	uint_16		type;
};

#define ETH_TYPE_IP4	0x0008
#define ETH_TYPE_ARP	0x0608

#define HARDWARE_ETHERNET	0x0001

#define PROTOCOL_IP 	0x0800
#define PROTOCOL_UDP 	0x11

#endif /* ETH_H_ */
