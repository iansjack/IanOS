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

#define	U_USBCMD		0x00
#define	U_USBSTS		0x02
#define	U_USBINTR		0x04
#define U_FRNUM			0x06
#define	U_FRBASEADDR	0x08
#define	U_SOfMOD		0x0C
#define	U_PORTSC1		0x10
#define	U_PORTSC2		0x12

struct transferDescriptor
{
	uint32_t linkAddress;
	uint32_t control;
	uint32_t token;
	uint32_t bufferPointer;
	uint32_t spare[4];
}__attribute__((packed));

void processUHCIQueue();

#endif /* USB_UHCI_H_ */
