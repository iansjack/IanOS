/*
 * usb.h
 *
 *  Created on: Jul 30, 2016
 *      Author: ian
 */

#ifndef USB_H_
#define USB_H_
#include "types-old.h"

struct USBpacket
{
	uint8_t  type;
	uint8_t  request;
	uint16_t value;
	uint16_t index;
	uint16_t length;
}__attribute__((packed));

#define USB_LOW_SPEED  	1
#define USB_HIGH_SPEED	2

#define	GET_STATUS			0
#define	CLEAR_FEATURE		1
#define	SET_FEATURE			3
#define	SET_ADDRESS			5
#define	GET_DESCRIPTOR		6
#define	SET_DESCRIPTOR		7
#define	GET_CONFIGURATION	8
#define	SET_CONFIGURATION	9
#define	GET_INTERFACE		10
#define	SET_INTERFACE		11
#define	SYNCH_FRAME			12

#define	DEVICE						1
#define	CONFIGURATION				2
#define	STRING						3
#define	INTERFACE					4
#define	ENDPOINT					5
#define	DEVICE_QUALIFIER			6
#define	OTHER_SPEED_CONFGURATION	7
#define	INTERFACE_POWER				8

struct USBdevice
{
	uint16_t 	address;
	uint8_t		speed;
	uint8_t		class;
	uint8_t		subclass;
	uint8_t		protocol;
};

struct Device
{
	uint8_t		length;
	uint8_t		type;
	uint16_t	release;
	uint8_t		class;
	uint8_t		subclass;
	uint8_t		protocol;
	uint8_t		packetsize;
	uint16_t	vendorid;
	uint16_t	productid;
	uint16_t	device;
	uint8_t		manufacturer;
	uint8_t		product;
	uint8_t		serialno;
	uint8_t		configurations;
}__attribute__((packed));

struct Configuration
{
	uint8_t		length;
	uint8_t		type;
	uint16_t	totallength;
	uint8_t		nointerfaces;
	uint8_t		configvalue;
	uint8_t		configuration;
	uint8_t		attributes;
	uint8_t		power;
}__attribute__((packed));

struct Interface
{
	uint8_t		length;
	uint8_t		type;
	uint8_t		number;
	uint8_t		alternate;
	uint8_t		noendpoints;
	uint8_t		class;
	uint8_t		subclass;
	uint8_t		protocol;
	uint8_t		index;
}__attribute__((packed));

struct Endpoint
{
	uint8_t		length;
	uint8_t		type;
	uint8_t		address;
	uint8_t		attributes;
	uint16_t	packetsize;
	uint8_t		interval;
}__attribute__((packed));


#endif /* USB_H_ */
