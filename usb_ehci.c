#include "usb.h"
#include "usb_ehci.h"

uint32_t frameList;
uint32_t asyncQueue;
uint32_t *buf;
uint32_t *cmd_base;
uint32_t dtIN;
uint32_t dtOUT;
struct USBpacket *p;
struct MSD device;

uint32_t *initializeECHIFrameList(long frameList)
{
	int i;
	long buffer = AllocAndCreatePTE(0x7000, 0, 0x7);

	for (i = 0; i < 1024; i++)
		((int *) frameList)[i] = buffer + 2;

	return (uint32_t *) buffer;
}

uint32_t getCmdReg(uint32_t reg)
{
	return cmd_base[reg / 4];
}

void setCmdReg(uint32_t reg, uint32_t val)
{
	cmd_base[reg / 4] = val;
}

void ifError(char *message)
{
	if (getCmdReg(E_USBSTS) & 0x02)
		kprintf(24, 0, message);
}

setupQueryPacket()
{
	buf[0x18] = (uint32_t) (uint64_t) &buf[0x28];
	buf[0x19] = 1;
	buf[0x1a] = 0x00080e80;
	buf[0x1b] = (uint32_t) (uint64_t) &buf[0x58];
	buf[0x1c] = 0x8000;
	buf[0x1d] = 0x9000;
	buf[0x1e] = 0xa000;
	buf[0x1f] = 0xb000;
	buf[0x28] = (uint32_t) (uint64_t) &buf[0x38];
	buf[0x29] = (uint32_t) (uint64_t) &buf[0x48];
	buf[0x2a] = 0x80400d80;
	buf[0x2b] = (uint32_t) (uint64_t) &buf[0x5a];
	buf[0x2c] = 0x8000;
	buf[0x2d] = 0x9000;
	buf[0x2e] = 0xa000;
	buf[0x2f] = 0xb000;
	buf[0x38] = (uint32_t) (uint64_t) &buf[0x48];
	buf[0x39] = (uint32_t) (uint64_t) &buf[0x48];
	buf[0x3a] = 0x00400d80;
	buf[0x3b] = (uint32_t) (uint64_t) &buf[0x6a];
	buf[0x3c] = 0x8000;
	buf[0x3d] = 0x9000;
	buf[0x3e] = 0xa000;
	buf[0x3f] = 0xb000;
	buf[0x48] = 1;
	buf[0x49] = 1;
	buf[0x4a] = 0x80008d80;
}

setupCmdPacket()
{
	buf[0x18] = (uint32_t) (uint64_t) &buf[0x28];
	buf[0x19] = 1;
	buf[0x1a] = 0x00080e80;
	buf[0x1b] = (uint32_t) (uint64_t) &buf[0x58];
	buf[0x1c] = 0x8000;
	buf[0x1d] = 0x9000;
	buf[0x1e] = 0xa000;
	buf[0x1f] = 0xb000;
	buf[0x28] = 1;
	buf[0x29] = 1;
	buf[0x2a] = 0x80008d80;
	buf[0x2b] = 0;
	buf[0x2c] = 0;
	buf[0x2d] = 0;
	buf[0x2e] = 0;
	buf[0x2f] = 0;
}

void linkAndRun(/*uint32_t addr, uint32_t endpoint, uint32_t max_size*/struct MSD *device, uint8_t ep)
{
	uint32_t endpoint;
	uint32_t max_size;

	switch (ep)
	{
	case CTL:
		endpoint = device->ctl;
		max_size = device->ctlMaxPacket;
		break;
	case BI:
		endpoint = device->bulkIn;
		max_size = device->bIMaxPacket;
		break;
	case BO:
		endpoint = device->bulkOut;
		max_size = device->bOMaxPacket;
	}

	buf = (int *) (long) asyncQueue;

	setCmdReg(E_USBSTS, 0x3F);
	buf[0x100] = (uint32_t) (uint64_t) &buf[0x0] + 2;
	buf[0x101] = 0x80006000 + device->address + (endpoint << 8) + (max_size << 16);
	buf[0x102] = 0x40000000;
	buf[0x103] = 0;
	buf[0x104] = (uint32_t) (uint64_t) &buf[0x18];
	buf[0x105] = 0;
	buf[0x106] = 0;
	buf[0x107] = 0;
	buf[0x108] = 0;
	buf[0x109] = 0;
	buf[0x10a] = 0;
	buf[0x10b] = 0;
	buf[0x10c] = 0;
	buf[0x10d] = 0;
	buf[0x10e] = 0;
	buf[0x10f] = 0;

	// link in the new queue
	buf[0x0] = (uint32_t) (uint64_t) &buf[0x100] + 2;
	while ((getCmdReg(E_USBSTS) & 1) == 0)
		;
	// unlink the queue
	buf[0x0] = (uint32_t) (uint64_t) &buf[0x0] + 2;
}

createPacket(uint32_t index, uint32_t length, uint32_t request, uint32_t type,
		uint32_t value)
{
	p->index = index;
	p->length = length;
	p->request = request;
	p->type = type;
	p->value = value;
}

void transferRequest(uint32_t size, uint8_t direction, uint32_t buffer)
{
	buf[0x18] = 1;
	buf[0x19] = 1;
	if (direction)
	{
		buf[0x1a] = 0x00008C80 + (dtOUT << 31) + (size << 16);
		dtOUT ^= 1;
	}
	else
	{
		buf[0x1a] = 0x00008D80 + (dtIN << 31) + (size << 16);
		dtIN ^= 1;
	}
	buf[0x1b] = buffer;
	buf[0x1c] = 0x8000;
	buf[0x1d] = 0x9000;
	buf[0x1e] = 0xa000;
	buf[0x1f] = 0xb000;
}

uint32_t CBWtag;

void createCBW(uint32_t size, uint32_t FLCC, uint32_t byte5, uint32_t byte6,
		uint32_t byte7, uint32_t byte8)
{
	buf[0x58] = 0x43425355;
	buf[0x59] = CBWtag++;
	buf[0x5a] = size;
	buf[0x5b] = FLCC;
	buf[0x5c] = byte5;
	buf[0x5d] = byte6;
	buf[0x5e] = byte7;
	buf[0x5f] = byte8;
}

void resetMSD(/*struct USBpacket *p*/)
{
	// Reset
	setupCmdPacket();
	createPacket(0, 0, 0xFF, 0x21, 0);
	linkAndRun(&device, CTL);
	ifError("Error with Reset");

	setupCmdPacket();
	createPacket(device.bulkIn, 0, CLEAR_FEATURE, 2, 0);
	linkAndRun(&device, CTL);
	ifError("Error with clear bulkIn Endpoint");

	setupCmdPacket();
	createPacket(device.bulkOut, 0, CLEAR_FEATURE, 2, 0);
	linkAndRun(&device, CTL);
	ifError("Error with clear bulkOut Endpoint");

	dtIN = 0;
	dtOUT = 0;
}

void requestSense()
{
	// Request Sense
	transferRequest(0x1F, 1, (uint32_t) (uint64_t) &buf[0x58]);
	createCBW(0x12, 0x03060080, 0x12000000, 0, 0, 0);
	linkAndRun(&device, BO);

	// Read Data
	transferRequest(0x12, 0, (uint32_t) (uint64_t) &buf[0x58]);
	linkAndRun(&device, BI);

	// Read CSW
	transferRequest(0xD, 0, (uint32_t) (uint64_t) &buf[0x70]);
	linkAndRun(&device, BI);
}

void handleEHCI(uint32_t base_addr)
{
	uint32_t nextaddress = 1;

	// We need to map this base_addr so that we can use the registers
	CreatePTE(base_addr, base_addr, 0, 7);
	uint32_t *base = (uint32_t *) (long) base_addr;

	// Get the command registers base address
	cmd_base = (uint32_t *) ((long) base_addr + (uint16_t) base[0]);

	// Reset the controller
	// Make sure the controller is halted
	setCmdReg(E_USBCMD, 0);
	while (!getCmdReg(E_USBSTS) & 0x100)
		;
	setCmdReg(E_USBCMD, 2);
	while (getCmdReg(E_USBCMD) & 2)
		;
	setCmdReg(E_CONFIGFLAG, 1);
	// Wait for the reset to happen
	GoToSleep(10);

	frameList = AllocAndCreatePTE(0x5000L, 0, 0x7);
	setCmdReg(E_PERIODICLISTBASE, (uint32_t) frameList);

	initializeECHIFrameList(frameList);

	asyncQueue = AllocAndCreatePTE(0x6000L, 0, 0x7);

	buf = (int *) (long) asyncQueue;

	int i;
	for (i = 0; i < 1024; i++)
		buf[i] = 0;

	buf[0] = (uint32_t) (uint64_t) &buf[0] + 2;
	buf[1] = 0x8040C000;
	buf[2] = 0x40000000;
	buf[3] = 0;
	buf[4] = 1;

	setCmdReg(E_ASYNCLISTADDR, asyncQueue);

	setCmdReg(E_USBCMD, 0x80001);
	while (getCmdReg(E_USBSTS) & 0x100)
		;

	// Reset the first Port
	setCmdReg(E_PORTSC1, 0x10a);
	GoToSleep(50);
	setCmdReg(E_PORTSC1, getCmdReg(E_PORTSC1) & 0xFEFF);
	GoToSleep(2);

	setCmdReg(E_USBINTR, 0x7);
	setCmdReg(E_USBCMD, 0x80021);

	device.address = 0;
	device.ctl = 0;
	device.ctlMaxPacket = 0x40;

	// Device query setup
	p = (struct USBpacket *) (&buf[0x58]);

	setupQueryPacket();
	createPacket(0x0409, 0x12, GET_DESCRIPTOR, 0x80, DEVICE << 8);
	linkAndRun(&device, CTL);
	ifError("Error with Device Query");

	// Set address
	setupCmdPacket();
	createPacket(0, 0, SET_ADDRESS, 0, ++nextaddress);
	linkAndRun(&device, CTL);
	ifError("Error with Set Address");
	device.address = nextaddress;

	// Get configuration
	setupQueryPacket();
	createPacket(0, 0x60, GET_DESCRIPTOR, 0x80, CONFIGURATION << 8);
	linkAndRun(&device, CTL);
	ifError("Error with Get Configuration");

	// Get the bulk in and out Endpoints
	uint8_t *b = (uint8_t *) &buf[0x5a];
	uint8_t *end = (uint8_t *)((uint64_t)b + ((struct Configuration *)&buf[0x5a])->totallength);

	while (b < end)
	{
		while ((b[1] != ENDPOINT) & (b < end))
			b += b[0];

		uint8_t endpoint = ((struct Endpoint *)b)->address;
		if (endpoint & 0x80)
		{
			device.bulkIn = endpoint & 0x7F;
			device.bIMaxPacket = ((struct Endpoint *)b)->packetsize;
		}
		else
		{
			device.bulkOut = endpoint;
			device.bOMaxPacket = ((struct Endpoint *)b)->packetsize;
		}
		b += b[0];
	}

// Set configuration 1
	setupCmdPacket();
	createPacket(0, 0, SET_CONFIGURATION, 0, 1);
	linkAndRun(&device, CTL);
	ifError("Error with SetConfiguration");

// Get configuration number
	setupQueryPacket();
	createPacket(0, 1, GET_CONFIGURATION, 0x80, 0);
	linkAndRun(&device, CTL);
	ifError("Error with GetConfiguration Number");

// Initialize the data toggle bits;
	dtIN = 0;
	dtOUT = 0;
	CBWtag = 0x12345678;

// Now for some SCSI commands
// Send an INQUIRY command
	transferRequest(0x1F, 1, (uint32_t) (uint64_t) &buf[0x58]);
	createCBW(0x24, 0x12060080, 0x24000000, 0, 0, 0);
	linkAndRun(&device, BO);

// Read Data
	transferRequest(0x24, 0, (uint32_t) (uint64_t) &buf[0x58]);
	linkAndRun(&device, BI);

// Read CSW
	transferRequest(0xD, 0, (uint32_t) (uint64_t) &buf[0x70]);
	linkAndRun(&device, BI);
	ifError("Error with Inquiry");

// Read Format
	for (i = 0; i < 3; i++)
	{
		transferRequest(0x1F, 1, (uint32_t) (uint64_t) &buf[0x58]);
		createCBW(0xFC, 0x230a0080, 0, 0xFC000000, 0, 0);
		linkAndRun(&device, BO);

		// Read Data
		transferRequest(0xFC, 0, (uint32_t) (uint64_t) &buf[0x58]);
		linkAndRun(&device, BI);

		// Read CSW
		transferRequest(0xD, 0, (uint32_t) (uint64_t) &buf[0x70]);
		linkAndRun(&device, BI);
		if ((getCmdReg(E_USBSTS) & 0x02) == 0)
			break;
		resetMSD();
		if (i == 2)
			kprintf(24, 0, "Error with Read Format Capacities");
	}

// Read some sectors
	transferRequest(0x1f, 1, (uint32_t) (uint64_t) &buf[0x58]);
	createCBW(0x1000, 0x280A0080, 0/*x9e000000*/, 0x08000000, 0, 0);
	linkAndRun(&device, BO);

// Read Data
	transferRequest(0x1000, 0, (uint32_t) (uint64_t) &buf[0x200]);
	linkAndRun(&device, BI);

// Read CSW
	transferRequest(0xD, 0, (uint32_t) (uint64_t) &buf[0x70]);
	linkAndRun(&device, BI);
	ifError("Error with Read Sector");

asm("jmp .");
}
