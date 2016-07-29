/*
 * usb_uhci.h
 *
 *  Created on: Jul 8, 2016
 *      Author: ian
 */

#ifndef USB_UHCI_H_
#define USB_UHCI_H_

#include "types-old.h"
#include "kernel.h"

#define	USBCMD		0x00
#define	USBSTS		0x02
#define	USBINTR		0x04
#define FRNUM		0x06
#define	FRBASEADDR	0x08
#define	SOfMOD		0x0C
#define	PORTSC1		0x10
#define	PORTSC2		0x12
struct transferDescriptor
{
	uint32_t linkAddress;
	uint32_t control;
	uint32_t token;
	uint32_t bufferPointer;
	uint32_t spare[4];
}__attribute__((packed));

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

struct USBdevice
{
	uint16_t 	address;
	uint8_t		speed;
	uint8_t		class;
	uint8_t		subclass;
	uint8_t		protocol;
};

#define	DEVICE		0x01	
#define CONFIGURATION	0x02
#define INTERFACE	0x04
#define	ENDPOINT	0x05

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

void processUHCIQueue();

#endif /* USB_UHCI_H_ */
