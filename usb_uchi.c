#include "usb_uchi.h"


uint32_t address;
uint32_t *buf;
struct USBdevice devices[2];
uint16_t USBaddress;
uint32_t frameList;

long keypresses[64];
int currentkeypress;

uint32_t *initializeFrameList(long frameList)
{
	int i;
	long buffer = AllocAndCreatePTE(0x4000, 0, 0x7);

	for (i = 0; i < 1024; i++)
		((int *) frameList)[i] = buffer + 2;;

	return (uint32_t *) buffer;
}

void runQueue()
{
	// Clear status register
	outw(address + USBSTS, 0xFFFF);
	buf[1] = (uint32_t)(uint64_t)&buf[4];
	while (inw(address + USBSTS) == 0);
	buf[1] = 1;
}

uint32_t createLinkAddress(uint32_t address, uint32_t Vf, uint32_t Q, uint32_t T)
{
	return ((address & 0xFFFFFFF0) | Q << 1 | T);
}

uint32_t createControl(uint32_t spd, uint32_t c_err, uint32_t iso, uint32_t ioc, uint32_t status)
{
	uint32_t ls;
	if (devices[1].speed == USB_LOW_SPEED)
		ls = 1;
	else
		ls = 0;
	return (spd << 29 | c_err << 27 | ls << 26 | iso << 25 | ioc << 24 | status << 16);
}

uint32_t createToken(uint32_t maxLen, uint32_t D, uint32_t endPoint, uint32_t deviceAddress, uint32_t pid)
{
	return (maxLen << 21 | D << 19 | endPoint << 15 | deviceAddress << 8 | pid);
}

void createSetQueue(uint32_t addr, uint32_t request, uint32_t value)
{
	struct transferDescriptor *descriptors = (struct transferDescriptor *)(&buf[4]);
	descriptors[0].linkAddress = createLinkAddress((uint32_t)(uint64_t)&descriptors[1], 1, 0, 0);
	descriptors[0].control = createControl(0, 3, 0, 0, 0x80);
	descriptors[0].token = createToken(7, 1, 0, addr, 0x2d);
	descriptors[0].bufferPointer = (uint32_t)(uint64_t)&buf[20];

	descriptors[1].linkAddress = createLinkAddress(0, 0, 0, 1);
	descriptors[1].control = createControl(0, 3, 0, 1, 0x80);
	descriptors[1].token = createToken(0x7FF, 0, 0, addr, 0x69);
	descriptors[1].bufferPointer = 0;

	struct USBpacket *p;
	p = (struct USBpacket *)(&buf[20]);
	p->index = 0;
	p->length = 0;
	p->request = request;
	p->type = 0;
	p->value = value;
}

void createDescriptorQueue(uint32_t addr, uint32_t index, uint32_t size, uint32_t type)
{
	struct transferDescriptor *descriptors = (struct transferDescriptor *)(&buf[4]);
	descriptors[0].linkAddress = createLinkAddress((uint32_t)(uint64_t)&descriptors[1], 1, 0, 0);
	descriptors[0].control = createControl(0, 3, 0, 0, 0x80);
	descriptors[0].token = createToken(7, 1, 0, addr, 0x2d);
	descriptors[0].bufferPointer = (uint32_t)(uint64_t)&buf[28];

	descriptors[1].linkAddress = createLinkAddress((uint32_t)(uint64_t)&descriptors[2], 1, 0, 0);
	descriptors[1].control = createControl(0, 3, 0, 0, 0x80);
	descriptors[1].token = createToken(size - 1, 0, 0, addr, 0x69);
	descriptors[1].bufferPointer = (uint32_t)(uint64_t)&buf[32];

	descriptors[2].linkAddress = createLinkAddress(0, 0, 0, 1);
	descriptors[2].control = createControl(0, 3, 0, 1, 0x80);
	descriptors[2].token = createToken(0x7FF, 1, 0, addr, 0xE1);
	descriptors[2].bufferPointer = 0;

	struct USBpacket *p;
	p = (struct USBpacket *)(&buf[28]);

	p->index = 0x0409;
	p->length = size;
	p->request = 6;
	p->type = 0x80;
	p->value = index | type << 8;
}

void createInterruptQueue()
{
	struct transferDescriptor *descriptors = (struct transferDescriptor *)(&buf[4]);
	descriptors[0].linkAddress = createLinkAddress(0, 0, 0, 1);
	descriptors[0].control = createControl(0, 3, 0, 1, 0x80);
	descriptors[0].token = createToken(7, 1, 1, 1, 0x69);
	descriptors[0].bufferPointer = (uint32_t)(uint64_t)&buf[28];
}

void activateInterruptQueue(/*uint32_t addr, uint32_t index, uint32_t size, uint32_t type*/)
{
	struct transferDescriptor *descriptors = (struct transferDescriptor *)(&buf[4]);
	descriptors[0].control = createControl(0, 3, 0, 1, 0x80);
}

void createDevqueryQueue(uint32_t addr, uint32_t size)
{
	createDescriptorQueue(addr, 0, size, 1);
}

void createConfigurationDescriptorQueue(uint32_t addr, uint32_t index, uint32_t size)
{
	createDescriptorQueue(addr, index, size, 2);
}

void createStringDescriptorQueue(uint32_t addr, uint32_t index, uint32_t size)
{
	createDescriptorQueue(addr, index, size, 3);
}

uint8_t getDevice(uint32_t addr, uint32_t size)
{
	createDevqueryQueue(addr, size);
	runQueue();
	return (uint8_t)buf[32];
}

void setAddress(uint32_t addr)
{
	createSetQueue(0, 5, addr);
	runQueue();
}

void setConfiguration(struct USBdevice *dev, uint32_t conf)
{
	createSetQueue(dev->address, 9, conf);
	runQueue();
}

void setIdle(uint32_t addr)
{
	createSetQueue(addr, 0, 0);
	struct USBpacket *p;
	p = (struct USBpacket *)(&buf[20]);

	p->index = 1;
	p->length = 0;
	p->request = 0xa;
	p->type = 0x21;
	p->value = 0;
	runQueue();
}

void getStringDescriptor(uint32_t addr, uint32_t index)
{
	createStringDescriptorQueue(addr, index, 8);
	runQueue();
	createStringDescriptorQueue(addr, index, buf[32] & 0xFF);
	runQueue();
}

void getConfigurationDescriptor(uint32_t addr, uint32_t index)
{
	createConfigurationDescriptorQueue(addr, index, 9);
	runQueue();
	createConfigurationDescriptorQueue(addr, index, buf[34] & 0xFF);
	runQueue();
}

void setupController(uint32_t base_addr)
{
	// Reset the controller
	int i = 0;
	for (i = 0; i < 5; i++)
	{
		outw(base_addr + USBCMD, 0x4);
		// Wait for the reset to happen
		GoToSleep(10);
		outw(base_addr + USBCMD, 0);
	}
	outw(base_addr + USBSTS, 0xFF);
	outw(base_addr + USBCMD, 0x2);
	GoToSleep(50);

	// Setup controller
	outw(base_addr + USBINTR, 0/*xF*/);
	outw(base_addr + FRNUM, 0);
	frameList = AllocAndCreatePTE(0x3000L, 0, 0x7);
	outw(base_addr + FRBASEADDR, (int) frameList);
	outw(base_addr + USBSTS, 0xFFFF);
}

void detectDevices(uint32_t base_addr)
{
	int i;

	// Reset the ports
	outw(base_addr + PORTSC1, 0x200);
	GoToSleep(50);
	outw(base_addr + PORTSC1, 0);
	GoToSleep(10);

	outw(base_addr + PORTSC1, 0xF);
	outw(base_addr + PORTSC2, 0xF);
	for (i = 0; i < 16; i++)
	GoToSleep(10);

	for (i = 0; i < 2; i++)
	if (inw(base_addr + PORTSC1 + 2*i) & 0x1)
	{
		devices[i].address = ++USBaddress;
		if (inw(base_addr + PORTSC1 + 2*i) & 0x100)
			devices[i].speed = USB_LOW_SPEED;
		else
			devices[i].speed = USB_HIGH_SPEED;
	}
}

void getDeviceInformation(struct USBdevice *dev)
{
	uint8_t size;

	size = getDevice(0, 8);
	setAddress(dev->address);
	getDevice(devices->address, size);
	struct Device *d = (struct Device *)&buf[32];
	dev->class = d->class;
	dev->subclass = d->subclass;
	dev->protocol = d->protocol;
}

struct Configuration *getConfiguration(struct USBdevice *dev)
{
	getConfigurationDescriptor(dev->address, 0);
	struct Configuration *c = (struct Configuration *)&buf[32];
	void *v = (void *)c;
	struct Interface *i = (struct Interface *)(v + c->length);
	dev->class = i->class;
	dev->subclass = i->subclass;
	dev->protocol = i->protocol;
	return c;
}

void processUHCIQueue()
{
	if (!inw(address + USBSTS)) return;
	keypresses[currentkeypress++] = buf[28];
	createInterruptQueue();
	outw(address + USBSTS, 0xFFFF);
	buf[1] = (uint32_t)(uint64_t)&buf[4];
}

void handleUHCI(uint32_t base_addr)
{
	short line = 16;
	base_addr &= 0xFFFFFFFE;
	address = base_addr;
	USBaddress = 0;

	setupController(base_addr);
	detectDevices(base_addr);

	buf = initializeFrameList(frameList);

	buf[0] = 1;
	buf[1] = 1;

	// Clear status register
	outw(address + USBSTS, 0xFFFF);
	// Start the controller
	outw(address + USBCMD, 0x1);

	// get basic device configuration of devices[0]
	getDeviceInformation(&devices[0]);
	struct Configuration *conf = getConfiguration(&devices[0]);
	setConfiguration(&devices[0], conf->configvalue);
	// asm("jmp .");
	outw(address + USBINTR, 0xF);
	createInterruptQueue();
	currentkeypress = 0;
	outw(address + USBSTS, 0xFFFF);
	buf[1] = (uint32_t)(uint64_t)&buf[4];
}
