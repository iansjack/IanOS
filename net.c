#include <kernel.h>
#include <net.h>

/*
 *
 */

/*extern struct MessagePort *NetPort;
typedef unsigned long physaddr_t;
extern struct rx_desc *receive_descriptor;
extern int nextrxpacket;

//==========================================================
// Send a PACKETRECEIVED message to netTaskCode
//==========================================================
void packetReceived()
{
	// Send a message to the network task saying that a packet has been received.
	struct rx_desc *descriptor = (struct rx_desc *) (KADDR(
			(physaddr_t) receive_descriptor));

	int i = nextrxpacket;
	// if (i == PageSize / sizeof(struct rx_desc)) i = 0;
	if (!(descriptor[i].status & 0x1))
		return;
	struct Message *NetMsg = (struct Message *) ALLOCMSG;

	NetMsg->nextMessage = 0;
	NetMsg->byte = PACKETRECEIVED;
	SendMessage(NetPort, NetMsg);
	DeallocMem(NetMsg);

}*/
