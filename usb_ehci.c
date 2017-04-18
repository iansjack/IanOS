#include "usb.h"
#include "usb_ehci.h"
#include "memory.h"
#include "scsi.h"

uint32_t *cmd_base;
uint32_t dtIN;
uint32_t dtOUT;
struct USBpacket *p;
struct MSD device;
struct CBW *cbw;
uint8_t *csw;
struct QueueHead *queueHead;
struct QueueHeadTransferDescriptor *descriptor;

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

struct EHCITransferControl transferControl(uint8_t dt, uint16_t length, uint8_t pid, uint8_t ioc)
{
	struct EHCITransferControl c;
	c.dt = dt;
	c.length = length;
	c.ioc = ioc;
	c.cpage = 0;
	c.cerr = 3;
	c.pid = pid;
	c.status = 0x80;
	return c;
}

setupQueryPacket(uint8_t *buffer)
{
	descriptor[0].nextqTDPointer = (uint32_t) (uint64_t)&descriptor[1];
	descriptor[0].alternatePointer = 1;
	descriptor[0].control = transferControl(0, 8, SETUP, 0);
	descriptor[0].buffers[0] = (uint32_t) (uint64_t)p;
	descriptor[0].buffers[1] = 0x8000;
	descriptor[0].buffers[2] = 0x9000;
	descriptor[0].buffers[3] = 0xA000;
	descriptor[0].buffers[4] = 0xB000;

	descriptor[1].nextqTDPointer = (uint32_t) (uint64_t)&descriptor[2];
	descriptor[1].alternatePointer = (uint32_t) (uint64_t)&descriptor[3];
	descriptor[1].control = transferControl(1, 0x40, IN, 0);
	descriptor[1].buffers[0] = (uint32_t) (uint64_t)buffer;
	descriptor[1].buffers[1] = 0x8000;
	descriptor[1].buffers[2] = 0x9000;
	descriptor[1].buffers[3] = 0xA000;
	descriptor[1].buffers[4] = 0xB000;

	descriptor[2].nextqTDPointer = (uint32_t) (uint64_t)&descriptor[3];
	descriptor[2].alternatePointer = (uint32_t) (uint64_t)&descriptor[3];
	descriptor[2].control = transferControl(0, 0x40, IN, 0);
	descriptor[2].buffers[0] = (uint32_t) (uint64_t)buffer + 0x40;
	descriptor[2].buffers[1] = 0x8000;
	descriptor[2].buffers[2] = 0x9000;
	descriptor[2].buffers[3] = 0xA000;
	descriptor[2].buffers[4] = 0xB000;

	descriptor[3].nextqTDPointer = 1;
	descriptor[3].alternatePointer = 4;
	descriptor[3].control = transferControl(1, 0, IN, 1);
}

setupCmdPacket()
{
	descriptor[0].nextqTDPointer = (uint32_t) (uint64_t)&descriptor[1];
	descriptor[0].alternatePointer = 1;
	descriptor[0].control = transferControl(0, 8, SETUP, 0);
	descriptor[0].buffers[0] = (uint32_t) (uint64_t)p;
	descriptor[0].buffers[1] = 0x8000;
	descriptor[0].buffers[2] = 0x9000;
	descriptor[0].buffers[3] = 0xA000;
	descriptor[0].buffers[4] = 0xB000;

	descriptor[1].nextqTDPointer = 1;
	descriptor[1].alternatePointer = 1;
	descriptor[1].control = transferControl(1, 0, IN, 1);
}

void linkAndRun(struct MSD *device, uint8_t ep)
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

	setCmdReg(E_USBSTS, 0x3F);

	queueHead[1].horizontalLinkPointer = (uint32_t)(uint64_t)queueHead + 2;
	//struct EndpointCharacteristics c;
	//c.address = device->address;
	//c.inactive = 0;
	//c.endpoint = endpoint;
	//c.eps = 2;
	//c.dtc = 1;
	//c.head = 0;
	//c.max_packet_length = max_size;
	//c.c = 0;
	//c.rl = 8;
	//asm("jmp .");
	queueHead[1].control1 = 0x80006000 + device->address + (endpoint << 8) + (max_size << 16);
	queueHead[1].control2 = 0x40000000;
	queueHead[1].qTDPointer = 0;
	queueHead[1].overlay[0] = (uint32_t) (uint64_t) &descriptor[0];

	// link in the new queue
	queueHead[0].horizontalLinkPointer = (uint32_t) (uint64_t) &queueHead[1] + 2;
	while ((getCmdReg(E_USBSTS) & 1) == 0)
		;
	// unlink the queue
	queueHead[0].horizontalLinkPointer = (uint32_t) (uint64_t) &queueHead[1] + 2;
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

void transferRequest(uint32_t size, uint8_t direction, uint32_t buffer, uint8_t des)
{
	descriptor[des].nextqTDPointer = 1;
	descriptor[des].alternatePointer = 1;

	if (direction)
	{
		descriptor[des].control = transferControl(dtOUT, size, OUT, 1);
		dtOUT ^= 1;
	}
	else
	{
		descriptor[des].control = transferControl(dtIN, size, IN, 1);
		dtIN ^= 1;
	}
	descriptor[des].buffers[0] = buffer;
	descriptor[des].buffers[1] = 0x8000;
	descriptor[des].buffers[2] = 0x9000;
	descriptor[des].buffers[3] = 0xa000;
	descriptor[des].buffers[4] = 0xb000;
}

uint32_t CBWtag;

void createCBW(uint32_t size, uint8_t direction, uint8_t commandLen, uint8_t command, uint8_t bytes[15])
{
	transferRequest(0x1F, 1, (uint32_t) (uint64_t) cbw, 0);

	cbw->signature = 0x43425355;
	cbw->tag = CBWtag++;
	cbw->transferLength = size;
	cbw->flags = direction;
	cbw->lun = 0;
	cbw->commandLength = commandLen;
	cbw->command[0] = command;
	int i;
	for (i = 1; i < 16; i++)
		cbw->command[i] = bytes[i-1];
}

createInquiry()
{
	uint8_t bytes[] = {0, 0, 0, 0x24, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	createCBW(0x24, 0x80, 0x06, INQUIRY, bytes);
	linkAndRun(&device, BO);
}

createReadFormat()
{
	uint8_t bytes[] = {0, 0, 0, 0, 0, 0, 0, 0xFC, 0, 0, 0, 0, 0, 0, 0};
	createCBW(0xFC, 0x80, 0x0a, READ_FORMAT_CAPACITY, bytes);
	linkAndRun(&device, BO);
}

createRead(uint32_t lba, uint32_t blocks)
{
	uint8_t bytes[] = {0,
			(lba >> 24) & 0xFF, (lba >> 16) & 0xFF, (lba >> 8) & 0xFF, lba & 0xFF,
			0,
			(blocks >> 8) & 0xFF, blocks & 0xFF,
			0, 0, 0, 0, 0, 0};
	createCBW(blocks * 512, 0x80, 0x0a, READ_10, bytes);
	linkAndRun(&device, BO);
}

void readData(uint32_t size, void *buffer, char *message)
{
	// Read Data
	transferRequest(size, 0, (uint32_t) (uint64_t) buffer, 0);
	descriptor[0].nextqTDPointer = (uint32_t) (uint64_t)&descriptor[1];

	// Read CSW
	transferRequest(0xD, 0, (uint32_t) (uint64_t) csw, 1);

	linkAndRun(&device, BI);
	ifError(message);
}

void resetMSD()
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

void requestSense(uint8_t *buffer)
{
	uint8_t bytes[] = {0, 0, 0, 0x12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	createCBW(0x12, 0x80, 0x06, REQUEST_SENSE, bytes);
	linkAndRun(&device, BO);
	readData(0x12, buffer, "Error with Request Sense");
}

void handleEHCI(uint32_t base_addr)
{
	p = (struct USBpacket *)AllocKMem(sizeof(struct USBpacket));

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

	uint32_t frameList = AllocAndCreatePTE(0x5000L, 0, 0x7);
	setCmdReg(E_PERIODICLISTBASE, (uint32_t) frameList);

	initializeECHIFrameList(frameList);

	uint8_t *buf = (uint8_t *)(uint64_t)AllocAndCreatePTE(0x6000L, 0, 0x7);

	int i;
	for (i = 0; i < 1024; i++)
		buf[i] = 0;

	// Initialize the first queue head
	queueHead = (struct QueueHead *)(uint64_t)buf;
	queueHead[0].horizontalLinkPointer = (uint32_t)(uint64_t)queueHead + 2;
	queueHead[0].control1 = 0x8040C000;
	queueHead[0].control2 = 0x40000000;
	queueHead[0].qTDPointer = 0;
	queueHead[0].overlay[0] = 1;
	setCmdReg(E_ASYNCLISTADDR, (uint32_t)(uint64_t)queueHead);

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

	AllocAndCreatePTE(0x8000, 0, 0x7);
	AllocAndCreatePTE(0x9000, 0, 0x7);
	AllocAndCreatePTE(0xA000, 0, 0x7);
	AllocAndCreatePTE(0xB000, 0, 0x7);

	descriptor = (struct QueueHeadTransferDescriptor *)&buf[0x800];

	device.address = 0;
	device.ctl = 0;
	device.ctlMaxPacket = 0x40;

	// Device query setup
	struct Device *dev = (struct Device *)AllocKMem(sizeof(struct Device));
	setupQueryPacket((uint8_t *)dev);
	createPacket(0x0409, 0x12, GET_DESCRIPTOR, 0x80, DEVICE << 8);
	linkAndRun(&device, CTL);
	ifError("Error with Device Query");

	// Set address
	setupCmdPacket();
	createPacket(0, 0, SET_ADDRESS, 0, nextaddress);
	linkAndRun(&device, CTL);
	ifError("Error with Set Address");
	device.address = nextaddress++;

	// Get configuration
	struct Configuration *configuration = (struct Configuration *)AllocKMem(64);
	setupQueryPacket((uint8_t *)configuration);
	createPacket(0, 0x60, GET_DESCRIPTOR, 0x80, CONFIGURATION << 8);
	linkAndRun(&device, CTL);
	ifError("Error with Get Configuration");

	// Get the bulk in and out Endpoints
	uint8_t *b = (uint8_t *) configuration;
	uint8_t *end = (uint8_t *)((uint64_t)b + configuration->totallength);

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
//	setupQueryPacket();
//	createPacket(0, 1, GET_CONFIGURATION, 0x80, 0);
//	linkAndRun(&device, CTL);
//	ifError("Error with GetConfiguration Number");

// Initialize the data toggle bits;
	dtIN = 0;
	dtOUT = 0;
	CBWtag = 0x12345678;

// Now for some SCSI commands
	cbw = (struct CBW *)AllocKMem(sizeof(struct CBW));
	csw = (uint8_t *)AllocKMem(9);

// Send an INQUIRY command
	void *inquiry = (uint8_t *)AllocKMem(32);
	createInquiry();
	readData(0x24, inquiry, "Error with Inquiry");

// Read Format
	for (i = 0; i < 3; i++)
	{
		void *format = (uint8_t *)AllocKMem(0xFC);
		createReadFormat();
		readData(0xFC, format, "");

		if ((getCmdReg(E_USBSTS) & 0x02) == 0)
			break;
		resetMSD();
		if (i == 2)
			kprintf(24, 0, "Error with Read Format Capacities");
	}

// Read some sectors
	void *readBuf = (void *)AllocKMem(2048);
	createRead(0, 4);
	readData(0x500, readBuf, "Error with Read Sector");

	asm("jmp .");
}
