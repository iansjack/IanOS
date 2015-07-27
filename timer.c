/*
 * timer.c
 *
 *  Created on: Jul 25, 2015
 *      Author: ian
 */

#include <timer.h>
#include <kernel.h>

struct timer *timers = 0;
int next_timer_id = 0;

int newtimer(int timeout, struct MessagePort *port)
{
	next_timer_id++;
	struct timer *temp = (struct timer *)AllocKMem(sizeof(struct timer));
	temp->id = next_timer_id;
	temp->port = port;
	temp->delta = timeout;

	if (!timers || timers->delta > timeout)
	{
		if (timers)
			timers->delta -= timeout;
		temp->next = timers;
		timers = temp;
		return temp->id;
	}

	struct timer *current = timers;

	while(current)
	{
		temp->delta -= current->delta;

		if (!current->next)
		{
			current->next = temp;
			return temp->id;
		}

		if (current->next->delta > temp->delta)
		{
			current->next->delta -= temp->delta;
			temp->next = current->next;
			current->next = temp;
			return temp->id;
		}

		current = current->next;
	}
	return -1;
}

void removetimer(int id)
{
	struct timer *last = 0;
	struct timer *current = timers;
	if (current->id == id)
	{
		timers = current->next;
		if (timers)
			timers->delta += current->delta;
		DeallocMem(current);
		return;
	}
	while (current->next)
	{
		last = current;
		current = current->next;
		if (current->id == id)
		{
			last->next = current->next;
			if (last->next)
				last->next->delta += current->delta;
			DeallocMem(current);
			return;
		}
	}
	return;
}

void checktimers()
{
	if (timers)
	{
		timers->delta--;
		while (timers && timers->delta <= 0)
		{
			struct Message msg;
			msg.byte = TIMER_MSG;
			msg.quad1 = timers->id;
			SendMessage(timers->port, &msg);
			void *temp = timers;
			timers = timers->next;
			DeallocMem(temp);
		}
	}
}
