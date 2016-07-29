#include "types-old.h"
#include "kernel.h"
#include "usb_echi.h"

void handleEHCI(uint32_t base_addr)
{
//	kprintf(line++, 6, "Base address %x", base_addr);

	// We need to map this base_addr so that we can use the registers
	CreatePTE(base_addr, base_addr, 0, 7);
	uint32_t *base = (uint32_t *)(long)base_addr;

	// Get the command registers base address
	uint32_t *cmd_base = (uint32_t *)((long)base_addr + (uint16_t)base[0]);

	// Reset the controller
	cmd_base[E_USBCMD/4] = 2;

	// Wait for the reset to happen
	GoToSleep(10);
	cmd_base[E_USBINTR/4] = 0xF;

	asm("jmp .");

/*	// print control register
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
	kprintf(line++, 0, "%x", base[6]); */
}
