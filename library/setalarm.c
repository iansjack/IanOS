#include <kernel.h>

long sys_alarm(int, struct MessagePort *);

int setalarm(int timeout, struct MessagePort *port)
{
	return sys_alarm(timeout, port);
}
