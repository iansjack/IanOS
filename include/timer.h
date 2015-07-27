/*
 * timer.h
 *
 *  Created on: Jul 25, 2015
 *      Author: ian
 */

#ifndef TIMER_H_
#define TIMER_H_

struct timer
{
	int delta;
	int id;
	struct MessagePort *port;
	struct timer *next;
};

#define TIMER_MSG	0xff

#endif /* TIMER_H_ */
