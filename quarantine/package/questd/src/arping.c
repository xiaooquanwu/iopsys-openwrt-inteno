/*
 * arping -- arping tool for questd
 *
 * Author: Alexey Kuznetsov <kuznet@ms2.inr.ac.ru>
 * Author: Sukru Senli sukru.senli@inteno.se
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; If not, see <http://www.gnu.org/licenses/>.
 */

#include <sys/socket.h>
#include <sys/types.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <net/if_arp.h>
#include <netinet/ether.h>
#include <string.h>

#include "questd.h"


struct in_addr src;
struct in_addr dst;
struct sockaddr_ll me;
struct sockaddr_ll he;
int sock_fd;

//void *mempcpy(void *dst, const void *src, size_t n); 

static int
send_pack(struct in_addr *src_addr, struct in_addr *dst_addr, struct sockaddr_ll *ME, struct sockaddr_ll *HE)
{
	int err;
	unsigned char buf[256];
	struct arphdr *ah = (struct arphdr *) buf;
	unsigned char *p = (unsigned char *) (ah + 1);

	ah->ar_hrd = htons(ARPHRD_ETHER);
	ah->ar_pro = htons(ETH_P_IP);
	ah->ar_hln = ME->sll_halen;
	ah->ar_pln = 4;
	ah->ar_op = htons(ARPOP_REQUEST);

	p = mempcpy(p, &ME->sll_addr, ah->ar_hln);
	p = mempcpy(p, src_addr, 4);

	p = mempcpy(p, &HE->sll_addr, ah->ar_hln);
	p = mempcpy(p, dst_addr, 4);

	err = sendto(sock_fd, buf, p - buf, 0, (struct sockaddr *) HE, sizeof(*HE));
	return err;
}

static bool
recv_pack(char *buf, int len, struct sockaddr_ll *FROM)
{
	struct arphdr *ah = (struct arphdr *) buf;
	unsigned char *p = (unsigned char *) (ah + 1);
	struct in_addr src_ip, dst_ip;

	/* Filter out wild packets */
	if (FROM->sll_pkttype != PACKET_HOST
	 && FROM->sll_pkttype != PACKET_BROADCAST
	 && FROM->sll_pkttype != PACKET_MULTICAST)
		return false;

	/* Only these types are recognized */
	if (ah->ar_op != htons(ARPOP_REPLY))
		return false;

	/* ARPHRD check and this darned FDDI hack here :-( */
	if (ah->ar_hrd != htons(FROM->sll_hatype)
	 && (FROM->sll_hatype != ARPHRD_FDDI || ah->ar_hrd != htons(ARPHRD_ETHER)))
		return false;

	/* Protocol must be IP. */
	if (ah->ar_pro != htons(ETH_P_IP)
	 || (ah->ar_pln != 4)
	 || (ah->ar_hln != me.sll_halen)
	 || (len < (int)(sizeof(*ah) + 2 * (4 + ah->ar_hln))))
		return false;

	(src_ip.s_addr) = *(uint32_t*)(p + ah->ar_hln);
	(dst_ip.s_addr) = *(uint32_t*)(p + ah->ar_hln + 4 + ah->ar_hln);

	if (dst.s_addr != src_ip.s_addr)
		return false;

	if ((src.s_addr != dst_ip.s_addr) || (memcmp(p + ah->ar_hln + 4, &me.sll_addr, ah->ar_hln)))
		return false;

	return true;
}


bool
arping(char *targetIP, char *device, int toms)
{
	struct sockaddr_in saddr;
	struct ifreq ifr;
	int probe_fd;

	sock_fd = socket(AF_PACKET, SOCK_DGRAM, 0);

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, device, IF_NAMESIZE);

	if (ioctl(sock_fd, SIOCGIFINDEX, &ifr, sizeof(ifr)) < 0) {
		close(sock_fd);
		return false;
	}

	me.sll_family = AF_PACKET;
	me.sll_ifindex = ifr.ifr_ifindex;
	me.sll_protocol = htons(ETH_P_ARP);
	bind(sock_fd, (struct sockaddr *) &me, sizeof(me));

	socklen_t mlen = sizeof(me);
	getsockname(sock_fd, (struct sockaddr *) &me, &mlen);

	he = me;
	memset(he.sll_addr, -1, he.sll_halen);

	inet_pton(AF_INET, targetIP, &(dst.s_addr));

	/* Get the sender IP address */
	probe_fd = socket(AF_INET, SOCK_DGRAM, 0);
	setsockopt(probe_fd, SOL_SOCKET, SO_BINDTODEVICE, &ifr, sizeof(ifr));
	memset(&saddr, 0, sizeof(saddr));
	saddr.sin_family = AF_INET;
	socklen_t slen = sizeof(saddr);
	saddr.sin_port = htons(1025);
	saddr.sin_addr = dst;
	connect(probe_fd, (struct sockaddr *) &saddr, sizeof(saddr));
	getsockname(probe_fd, (struct sockaddr *) &saddr, &slen);
	src = saddr.sin_addr;
	close(probe_fd);

	send_pack(&src, &dst, &me, &he);

	char packet[64];
	struct sockaddr_ll from;
	socklen_t alen = sizeof(from);
	bool connected = false;
	int cc = -1;

	fd_set read_fds, write_fds, except_fds;
	FD_ZERO(&read_fds);
	FD_ZERO(&write_fds);
	FD_ZERO(&except_fds);
	FD_SET(sock_fd, &read_fds);

	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = toms * 1000;

	if (select(sock_fd + 1, &read_fds, &write_fds, &except_fds, &timeout) == 1)
	{
		cc = recvfrom(sock_fd, packet, sizeof(packet), 0, (struct sockaddr *) &from, &alen);
	}

	if (cc >= 0)
		connected = recv_pack(packet, cc, &from);

	close(sock_fd);

	return connected;
}
