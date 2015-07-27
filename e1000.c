#include <e1000.h>
#include <memory.h>

typedef unsigned long physaddr_t;

// LAB 6: Your driver code here
struct tx_desc *transmit_descriptor;
struct rx_desc *receive_descriptor;
physaddr_t buffers[PageSize/sizeof(struct tx_desc)];
int nexttxpacket = 0;
int nextrxpacket = 1;
int *registers;

int E1000_enable(void *base)
{
	CreatePTE(base, base, 0, 7);
	CreatePTE(base + 0x1000, base + 0x1000, 0, 7);
	CreatePTE(base + 0x2000, base + 0x2000, 0, 7);
	CreatePTE(base + 0x3000, base + 0x3000, 0, 7);
	CreatePTE(base + 0x4000, base + 0x4000, 0, 7);
	CreatePTE(base + 0x5000, base + 0x5000, 0, 7);
	e1000 = base;
	registers = (int *)e1000;
	//registers[0] = 0x48000249; //1 << 3;
	registers[0] = 0x00000040;
	//kprintf(23, 0, "%x %x\n", registers, registers[2]);

	// Allocate one page for the transmit descriptor queue
	physaddr_t transmit_descriptors = AllocPage(0);
	transmit_descriptor = (struct tx_desc*)transmit_descriptors;

	registers[TDBAL / sizeof(int)] = transmit_descriptors;
	registers[TDBAH / sizeof(int)] = 0;
	registers[TDLEN / sizeof(int)] = PageSize;
	registers[TDH / sizeof(int)] = 0;
	registers[TDT / sizeof(int)] = 0;
	registers[TCTL / sizeof(int)] = TCTL_EN + TCTL_PSP + (0x10 << 4) + (0x40 << 12);
	registers[TIPG / sizeof(int)] = 10;

	int i;
	for (i = 0; i < PageSize / sizeof(struct tx_desc); i++)
	{
		buffers[i] = AllocPage(0);
	}

	// Allocate one page for the receiver descriptor queue
	physaddr_t receive_descriptors = AllocPage(0);
	receive_descriptor = (struct rx_desc*)receive_descriptors;

	registers[RDBAL / sizeof(int)] = receive_descriptors;
	registers[RDBAH / sizeof(int)] = 0;
	registers[RDLEN / sizeof(int)] = PageSize;
	registers[RDH / sizeof(int)] = 1;
	registers[RDT / sizeof(int)] = 0;
	uint8_t *array = (uint8_t *)(&registers[RAL / sizeof(int)]);
	registers[RAL / sizeof(int)] = 0x12005452;
	registers[RAH / sizeof(int)] = 0x80005634;
	registers[IMC / sizeof(int)] = 0xFF;
	registers[IMS / sizeof(int)] = 0x80;

	#define KADDR(name) ((long) name + VAddr)

	for (i = 0; i < PageSize / sizeof(struct rx_desc); i++)
	{
		((struct rx_desc *)(KADDR((physaddr_t)receive_descriptor)))[i].buffer_addr = AllocPage(0);
	}

	registers[RCTL / sizeof(int)] = RCTL_EN | RCTL_BAM | RCTL_SECRC;
//	registers[0x38 / sizeof(int)] = 0x8100;
//	registers[0xc4 / sizeof(int)] = 0xc3;

	//kprintf(24, 0, "%x %x\n", registers, registers[2]);

	return 0;
}

void queue_packet(void *packet, int length)
{
	int checkDD = 0;
	struct tx_desc *descriptor = (struct tx_desc *)(KADDR((physaddr_t)transmit_descriptor));
	if (!(descriptor[nexttxpacket].status & 0x00100000) && checkDD) return;
	int i = nexttxpacket++;
	if (nexttxpacket >= PageSize / sizeof(struct tx_desc))
	{
		nexttxpacket = 0;
		checkDD = 1;
	}
	memcpy((void *)((physaddr_t *)KADDR(buffers[i])), packet, length);
	descriptor[i].addr = buffers[i];
	descriptor[i].length = length;
	descriptor[i].cso = 0;
	descriptor[i].cmd = 9;
	descriptor[i].status = 0;
	descriptor[i].special = 0;
	descriptor->css = 0;
	registers[TDT / sizeof(int)] = nexttxpacket;
}

int receive_packet(void *buffer, int *length)
{
	struct rx_desc *descriptor = (struct rx_desc *)(KADDR((physaddr_t)receive_descriptor));

	int i = nextrxpacket;
	// if (i == PageSize / sizeof(struct rx_desc)) i = 0;
	if (!(descriptor[i].status & 0x1)) return -1;
	nextrxpacket++;
	memcpy(buffer, (void *)((physaddr_t *)KADDR(descriptor[i].buffer_addr)), descriptor[i].length);
	*length = descriptor[i].length;
	registers[RDT / sizeof(int)] = i;
	descriptor[i].status = 0;
	return 0;
}

