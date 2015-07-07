/*
 * pci.c
 *
 *  Created on: Feb 1, 2014
 *      Author: ian
 */
#include <memory.h>
extern NicInt;

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

static inline
void outl(unsigned short port, unsigned int val)
{
	asm volatile( "outl %0, %1"
			: : "a"(val), "Nd"(port) );
}

static inline
unsigned int inl(unsigned short port)
{
	unsigned int ret;
	asm volatile( "inl %1, %0"
			: "=a"(ret) : "Nd"(port) );
	return ret;
}

uint32_t pciConfigReadWord(uint8_t bus, uint8_t slot, uint8_t func,
		uint8_t offset)
{
	uint32_t address;
	uint32_t lbus = (uint32_t) bus;
	uint32_t lslot = (uint32_t) slot;
	uint32_t lfunc = (uint32_t) func;
	uint32_t tmp = 0;

	/* create configuration address as per Figure 1 */
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

	/* create configuration address as per Figure 1 */
	address = (uint32_t) (0x80000000 | (lbus << 16) | (lslot << 11)
			| (lfunc << 8) | offset);

	/* write out the address */
	outl(0xCF8, address);
	outl(0xCFC, value);
}

void handleOHCI(uint32_t base_addr)
{
	int line = 10;
	kprintf(line++, 6, "Base address %x", base_addr);

	// We need to map this base_addr so that we can use the registers
	CreatePTE(base_addr, base_addr, 0, 7);
	// Reset the controller
	uint32_t *base = (uint32_t *)((long)base_addr);
	base[2] = 1;
	// Wait for the reset to happen
	GoToSleep(10);
	// print control register
	if ((base[1] & 0xC0) == 0xC0)
		kprintf(line++, 0, "Controller reset");
	if ((base[13] & 0x00003FFF) == 0x2edf)
		kprintf(line++, 0,
				"Interval OK - USB controller is now present and reset.");
	line++;
	base[6] = 0xFFFFFFFF;
	kprintf(line++, 0, "%x", base[6]);
	uint32_t HCCA = AllocPage(0);
	// Reset all ports
	base[1]=0;
	GoToSleep(50);
	base[0xd] = 0xa7782edf;	// HcFmInterval
	base[0x10] = 0x2a2f;	// HcPeriodicStart
	int numberOfPorts = base[0x12];
	kprintf(line++, 0, "There are %x ports", numberOfPorts & 0xff);
	base[0x12] = (numberOfPorts & 0xfffffc00) | 0x100;
	int i = 0;
	for (i = 0; i < numberOfPorts & 0xff; i++)
		base[0x13] =   base[0x13] | 1 << i + 17;
	// Set HCCA
	base[6] = HCCA;
	kprintf(line++, 0, "%x", base[6]);
}
void enumeratePCIBus()
{
	uint16_t bus;
	uint8_t device;
	uint8_t function;
	uint16_t vendor_id;
	uint16_t device_id;
	short line = 6;

	for (bus = 0; bus < 256; bus++)
	{
		for (device = 0; device < 32; device++)
		{
			for (function = 0; function < 8; function++)
			{
				device_id = (pciConfigReadWord(bus, device, function, 0) >> 16) & 0xFFFF;
				vendor_id = pciConfigReadWord(bus, device, function, 0) & 0xFFFF;
				if (vendor_id != 0xFFFF)
				{
					uint32_t class = pciConfigReadWord(bus, device, function,
							8);
					kprintf(line++, 0, "Found PCI device, vendor = %x; device =  %x; class = %x", vendor_id, device_id, (class >> 16) & 0xFFFF);
					if (vendor_id == 0x8086 && device_id == 0x100E)
					{
						// Enable the PCI device
						pciConfigWriteWord(6, bus, device, function, 0x04);

						kprintf(line++, 0, "Found Intel E1000 NIC!");
						unsigned char interrupt_line = (pciConfigReadWord(bus, device, function, 0x3C)) & 0xff;
						// kprintf(line++, 0, "%x", interrupt_line /*pciConfigReadWord(bus, device, function, 0x3C)*/);
						CreateIntGate(code64, (long)&NicInt, 32 + interrupt_line, 0);
						uint32_t base = pciConfigReadWord(bus, device, function, 0x10) & 0xFFFFFFF0;
						pciConfigWriteWord(0xFFFFFFFF, bus, device, function, 0x10);
						uint32_t size = ~(pciConfigReadWord(bus, device, function, 0x10) & 0xFFFFFFF0) + 1;
						pciConfigWriteWord(base, bus, device, function, 0x10);

						// Set up the NIC
						E1000_enable(base);
					}
				}
			}
		}
	}
}

