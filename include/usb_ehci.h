/*
 * usb_echi.h
 *
 *  Created on: Jul 28, 2016
 *      Author: ian
 */

#ifndef USB_ECHI_H_
#define USB_ECHI_H_

#define	CAPLENGTH			0x00
#define	HCIVERSION			0x02
#define	HCSPARAMS			0x04
#define	HCCPARAMS			0x08
#define	HCPS_PORTROUTE		0x0C

#define	E_USBCMD			0x00
#define	E_USBSTS			0x04
#define	E_USBINTR			0x08
#define	E_FRINDEX			0x0C
#define	E_CTRLDSSEGMENT		0x10
#define	E_PERIODICLISTBASE	0x14
#define	E_ASYNCLISTADDR		0x18
#define	E_CONFIGFLAG		0x40
#define	E_PORTSC1			0x44

void handleEHCI(uint32_t base_addr);

#define	CTL 0
#define BO	1
#define BI	2

struct MSD
{
	uint32_t address;
	uint32_t ctl;
	uint32_t ctlMaxPacket;
	uint32_t bulkIn;
	uint32_t bIMaxPacket;
	uint32_t bulkOut;
	uint32_t bOMaxPacket;
};

struct CBW
{
	uint32_t	signature;
	uint32_t 	tag;
	uint32_t	transferLength;
	uint8_t		flags;
	uint8_t		lun;
	uint8_t		commandLength;
	uint8_t		command[16];
}__attribute__((packed));

struct EndpointCharacteristics
{
	uint8_t address				:7;
	uint8_t	inactive			:1;
	uint8_t	endpoint			:4;
	uint8_t	eps					:2;
	uint8_t	dtc					:1;
	uint8_t	head				:1;
	uint16_t max_packet_length	:11;
	uint16_t c					:1;
	uint16_t rl					:4;
}__attribute__((packed));

struct EndpointCapabilities
{
	uint8_t	s_mask;
	uint8_t	c_mask;
	uint16_t hub_address	:7;
	uint16_t port_number	:7;
	uint16_t mult			:2;
}__attribute__((packed));

struct QueueHead
{
	uint32_t 	horizontalLinkPointer;
	uint32_t	control1;
	uint32_t	control2;
	uint32_t	qTDPointer;
	uint32_t	overlay[13];
	uint32_t	padding[7];
};

#define	OUT		0
#define	IN		1
#define SETUP	2

struct EHCITransferControl
{
	uint8_t		status;
	uint8_t 	pid		:2;
	uint8_t		cerr	:2;
	uint8_t		cpage	:3;
	uint8_t		ioc		:1;
	uint16_t	length	:15;
	uint16_t	dt	:1;
}__attribute__((packed));

struct QueueHeadTransferDescriptor
{
	uint32_t 	nextqTDPointer;
	uint32_t 	alternatePointer;
	struct EHCITransferControl	control;
	uint32_t	buffers[5];
	uint32_t	hiBuffers[5];
	uint32_t	padding[3];
};

#endif /* USB_ECHI_H_ */
