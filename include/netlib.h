/*
 * netlib.h
 *
 *  Created on: May 24, 2015
 *      Author: ian
 */

#ifndef NETLIB_H_
#define NETLIB_H_

unsigned short be(unsigned short number);
struct dhcp *dhcp();
struct ARP_Packet *arp_request(struct ip_address ip);
struct ARP_Packet *arp_reply(struct ip_address ip, struct mac_address mac);
long ip_to_long(struct ip_address ip);
long mac_to_long(struct mac_address mac);
struct ip_address long_to_ip(long ip);
long GetMyIP();
void openUDPSocket(struct udp_handle *handle, long ip, long port);
void openListeningSocket(struct TCPSocket *socket, int port);

#endif /* NETLIB_H_ */
