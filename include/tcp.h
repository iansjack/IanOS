/*
 * tcp.h
 *
 *  Created on: Jun 6, 2015
 *      Author: ian
 */

#ifndef TCP_H_
#define TCP_H_

#define CLOSED			1
#define	LISTEN			2
#define	SYN_SENT		3
#define SYN_RECEIVED	4
#define	ESTABLISHED		5
#define	CLOSE_WAIT		6
#define	LAST_ACK		7
#define	FIN_WAIT_1		8
#define	FIN_WAIT_2		9
#define	CLOSING			10
#define	TIME_WAIT		11

#define SYN	0x2
#define ACK	0x10

struct __attribute__ ((__packed__)) tcp_header
{
	unsigned short source_port;
	unsigned short destination_port;
	unsigned int seq_no;
	unsigned int ack_no;
	unsigned char data_offset;
	unsigned char control;
	unsigned short windows;
	unsigned short checksum;
	unsigned short urgent_ptr;
};

struct __attribute__ ((__packed__)) tcp_packet
{
	struct eth_header eth;
	struct ip_header ip;
	struct tcp_header tcp;
	unsigned char * data;
};

#define TCP_BUFFER_SIZE	1024

struct TCB
{
	long int source_ip;
	long int source_port;
	long int dest_ip;
	long int dest_port;
	unsigned char state;
	unsigned char *transfer_buffer;
	unsigned char *buffer;
	unsigned char *buffer_start;
	unsigned char *buffer_end;
	struct MessagePort *msgport;
	unsigned int snd_una;
	unsigned int snd_nxt;
	unsigned int snd_wnd;
	unsigned int rcv_nxt;
	unsigned int rcv_wnd;
	struct TCB *next;
};

struct TCPSocket
{
	struct MessagePort *messagePort;
	unsigned char *transfer_buffer;
	struct TCB *tcb;
};

#endif /* TCP_H_ */
