#include <kernel.h>
#include <net.h>

extern struct MessagePort *NetPort;

//==========================================================
// Send a PACKETRECEIVED message to netTaskCode
//==========================================================
void packetReceived()
{
	// Send a message to the network task saying that a packet has been received.
	struct Message *NetMsg = (struct Message *) ALLOCMSG;

	NetMsg->nextMessage = 0;
	NetMsg->byte = PACKETRECEIVED;
	SendMessage(NetPort, NetMsg);
	DeallocMem(NetMsg);
}
