#include <sys/socket.h>
#include <sys/types.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <net/if_arp.h>
#include <netinet/ether.h>
#include <sys/sysinfo.h>
#include <netinet/in.h>
#include <arpa/inet.h>


struct in_addr src;
struct in_addr dst;
struct sockaddr_ll me;
struct sockaddr_ll he;
int sock_fd;

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

static int
recv_pack(char *buf, int len, struct sockaddr_ll *FROM)
{
	struct arphdr *ah = (struct arphdr *) buf;
	unsigned char *p = (unsigned char *) (ah + 1);
	struct in_addr src_ip, dst_ip;

	/* Filter out wild packets */
	if (FROM->sll_pkttype != PACKET_HOST
	 && FROM->sll_pkttype != PACKET_BROADCAST
	 && FROM->sll_pkttype != PACKET_MULTICAST)
		return 0;

	/* Only these types are recognized */
	if (ah->ar_op != htons(ARPOP_REPLY))
		return 0;

	/* ARPHRD check and this darned FDDI hack here :-( */
	if (ah->ar_hrd != htons(FROM->sll_hatype)
	 && (FROM->sll_hatype != ARPHRD_FDDI || ah->ar_hrd != htons(ARPHRD_ETHER)))
		return 0;

	/* Protocol must be IP. */
	if (ah->ar_pro != htons(ETH_P_IP)
	 || (ah->ar_pln != 4)
	 || (ah->ar_hln != me.sll_halen)
	 || (len < (int)(sizeof(*ah) + 2 * (4 + ah->ar_hln))))
		return 0;

	(src_ip.s_addr) = *(uint32_t*)(p + ah->ar_hln);
	(dst_ip.s_addr) = *(uint32_t*)(p + ah->ar_hln + 4 + ah->ar_hln);

	if (dst.s_addr != src_ip.s_addr)
		return 0;

	if ((src.s_addr != dst_ip.s_addr) || (memcmp(p + ah->ar_hln + 4, &me.sll_addr, ah->ar_hln)))
		return 0;

	return 1;
}


int
arping(char *targetIP, char *device, long tmo)
{
	struct sockaddr_in saddr;
	struct ifreq ifr;
	int probe_fd;

	sock_fd = socket(AF_PACKET, SOCK_DGRAM, 0);

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, device, IF_NAMESIZE);

	if (ioctl(sock_fd, SIOCGIFINDEX, &ifr, sizeof(ifr)) < 0) {
		close(sock_fd);
		return 0;
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
	int connected = 0;
	int cc = -1;

	fd_set read_fds, write_fds, except_fds;
	FD_ZERO(&read_fds);
	FD_ZERO(&write_fds);
	FD_ZERO(&except_fds);
	FD_SET(sock_fd, &read_fds);

	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = tmo * 1000;

	if (select(sock_fd + 1, &read_fds, &write_fds, &except_fds, &timeout) == 1)
	{
		cc = recvfrom(sock_fd, packet, sizeof(packet), 0, (struct sockaddr *) &from, &alen);
	}

	if (cc >= 0)
		connected = recv_pack(packet, cc, &from);

	close(sock_fd);

	return connected;
}
