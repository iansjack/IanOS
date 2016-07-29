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

#endif /* USB_ECHI_H_ */
