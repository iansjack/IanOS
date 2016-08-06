/*
 * pci.c
 *
 *  Created on: Feb 1, 2014
 *      Author: ian
 */
#include <memory.h>
#include "types-old.h"
#include "kernel.h"
#include "pci.h"
#include "usb_uhci.h"
#include "usb_ehci.h"

extern SharedInt;

struct interrupt_queue *interrupts;

void packetReceived();

void processEHCIQueue()
{
	// asm("jmp .");
}

void link_interrupt(void (*fn)())
{
	struct interrupt_queue *new_entry = AllocKMem(
			sizeof(struct interrupt_queue));
	new_entry->handler = fn;
	new_entry->next = interrupts;
	interrupts = new_entry;
}

uint32_t pciConfigReadWord(uint8_t bus, uint8_t slot, uint8_t func,
		uint8_t offset)
{
	uint32_t address;
	uint32_t lbus = (uint32_t) bus;
	uint32_t lslot = (uint32_t) slot;
	uint32_t lfunc = (uint32_t) func;
	uint32_t tmp = 0;

	/* create configuration address */
	address = (uint32_t) (0x80000000 | (lbus << 16) | (lslot << 11)
			| (lfunc << 8) | offset);

	/* write out the address */
	outl(0xCF8, address);
	return inl(0xCFC);
}

void pciConfigWriteWord(uint32_t value, uint8_t bus, uint8_t slot, uint8_t func,
		uint8_t offset)
{
	uint32_t address;
	uint32_t lbus = (uint32_t) bus;
	uint32_t lslot = (uint32_t) slot;
	uint32_t lfunc = (uint32_t) func;
	uint32_t tmp = 0;

	/* create configuration address */
	address = (uint32_t) (0x80000000 | (lbus << 16) | (lslot << 11)
			| (lfunc << 8) | offset);

	/* write out the address */
	outl(0xCF8, address);
	outl(0xCFC, value);
}

void processSharedInterrupts()
{
	struct interrupt_queue *temp;
	temp = interrupts;
	while (temp)
	{
		temp->handler();
		temp = temp->next;
	}
}

void enumeratePCIBus()
{
	uint16_t bus;
	uint8_t device;
	uint8_t function;
	uint16_t vendor_id;
	uint16_t device_id;
	short line = 6;
	interrupts = (struct interrupt_queue *) 0;

	for (bus = 0; bus < 256; bus++)
	{
		for (device = 0; device < 32; device++)
		{
			for (function = 0; function < 8; function++)
			{
				device_id = (pciConfigReadWord(bus, device, function, 0) >> 16)
						& 0xFFFF;
				vendor_id = pciConfigReadWord(bus, device, function, 0)
						& 0xFFFF;
				uint8_t interrupt_line;
				if (vendor_id != 0xFFFF)
				{
					uint32_t class = pciConfigReadWord(bus, device, function,
							8);
					kprintf(line++, 0,
							"Found PCI device, vendor = %x; device =  %x; class = %x",
							vendor_id, device_id, class); //(class >> 16) & 0xFFFF);
					if (vendor_id == 0x8086 && device_id == 0x100E)
					{
						// Enable the PCI device
						pciConfigWriteWord(6, bus, device, function, 0x04);

						unsigned char interrupt_line = (pciConfigReadWord(bus,
								device, function, 0x3C)) & 0xff;
						kprintf(line++, 0, "Found Intel E1000 NIC!");
						CreateIntGate(code64, (long) &SharedInt,
								32 + interrupt_line, 0);
						link_interrupt(packetReceived);
						uint32_t base = pciConfigReadWord(bus, device, function,
								0x10) & 0xFFFFFFF0;
						pciConfigWriteWord(0xFFFFFFFF, bus, device, function,
								0x10);
						uint32_t size = ~(pciConfigReadWord(bus, device,
								function, 0x10) & 0xFFFFFFF0) + 1;
						pciConfigWriteWord(base, bus, device, function, 0x10);

						// Set up the NIC
						E1000_enable(base);
					}
					if (((class >> 16) & 0xFFFF) == 0xC03)
					// It's a USB controller
					{
						uint8_t p_interface = (class >> 8) & 0xFF;
						uint32_t base;
						switch (p_interface)
						{
						case 0:
							base = pciConfigReadWord(bus, device, function,
									0x20);
							interrupt_line = (pciConfigReadWord(bus, device,
									function, 0x3C));
							kprintf(line++, 0,
									"Found UHCI controller! - Interrupt line: %x; base: %x",
									interrupt_line, base);
							CreateIntGate(code64, (long) &SharedInt,
									32 + interrupt_line, 0);
							pciConfigWriteWord(0, bus, device, function, 0x34);
							pciConfigWriteWord(0, bus, device, function, 0x38);
							pciConfigWriteWord(0x8f00, bus, device, function,
									0xC0);

							link_interrupt(processUHCIQueue);

							// Enable the PCI device
							pciConfigWriteWord(5, bus, device, function, 0x04);

							handleUHCI(base);
							break;

						case 0x10:
							kprintf(line++, 0, "OHCI controller");
							break;

						case 0x20:
							interrupt_line = (pciConfigReadWord(bus, device,
									function, 0x3C));
							base = pciConfigReadWord(bus, device, function,
									0x10);
							kprintf(line++, 0,
									"Found EHCI controller! - Interrupt line: %x; base: %x",
									interrupt_line, base);
							CreateIntGate(code64, (long) &SharedInt,
									32 + interrupt_line, 0);
							link_interrupt(processEHCIQueue);

							// Enable the PCI device
							pciConfigWriteWord(6, bus, device, function, 0x04);

							handleEHCI(base);
							break;

						case 0x30:
							kprintf(line++, 0, "XHCI controller");
							break;

						default:
							kprintf(line, 0, "Error - unrecognized device.");
						}
					}
				}
			}
		}
	}
}

