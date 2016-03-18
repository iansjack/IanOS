/*
 * eth.h
 *
 *  Created on: May 14, 2015
 *      Author: ian
 */

#ifndef ETH_H_
#define ETH_H_

#include <stdint.h>

struct mac_address
{
	uint8_t	add1;
	uint8_t	add2;
	uint8_t	add3;
	uint8_t	add4;
	uint8_t	add5;
	uint8_t	add6;
};

struct eth_header
{
	struct mac_address	destination;
	struct mac_address	source;
	uint16_t		type;
};

#define ETH_TYPE_IP4	0x0008
#define ETH_TYPE_ARP	0x0608

#define HARDWARE_ETHERNET	0x0001

#define PROTOCOL_IP 	0x0800
#define PROTOCOL_UDP 	0x11

#endif /* ETH_H_ */
