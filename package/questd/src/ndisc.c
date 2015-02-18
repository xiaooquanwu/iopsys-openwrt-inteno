/*
 *  ndisc.c - ICMPv6 neighbour discovery command line tool
 *
 *  Author: Rémi Denis-Courmont
 *  questd port: Sukru Senli sukru.senli@inteno.se
 *
 *  Copyright © 2004-2007 Rémi Denis-Courmont.
 *  This program is free software: you can redistribute and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, versions 2 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h> /* div() */
#include <inttypes.h> /* uint8_t */
#include <limits.h> /* UINT_MAX */
#include <locale.h>
#include <stdbool.h>

#include <errno.h> /* EMFILE */
#include <sys/types.h>
#include <unistd.h> /* close() */
#include <time.h> /* clock_gettime() */
#include <poll.h> /* poll() */
#include <sys/socket.h>
#include <sys/uio.h>
#include <fcntl.h>

#include <sys/times.h> /* times() fallback */

#include <netdb.h> /* getaddrinfo() */
#include <arpa/inet.h> /* inet_ntop() */
#include <net/if.h> /* if_nametoindex() */

#include <netinet/in.h>
#include <netinet/icmp6.h>

#ifndef IPV6_RECVHOPLIMIT
/* Using obsolete RFC 2292 instead of RFC 3542 */ 
# define IPV6_RECVHOPLIMIT IPV6_HOPLIMIT
#endif

#ifndef AI_IDN
# define AI_IDN 0
#endif

enum ndisc_flags
{
	NDISC_VERBOSE1=0x1,
	NDISC_VERBOSE2=0x2,
	NDISC_VERBOSE3=0x3,
	NDISC_VERBOSE =0x3,
	NDISC_NUMERIC =0x4,
	NDISC_SINGLE  =0x8,
};

#if defined (CLOCK_HIGHRES) && !defined (CLOCK_MONOTONIC)
# define CLOCK_MONOTONIC CLOCK_HIGHRES
#endif

static inline void
mono_gettime (struct timespec *ts)
{
#ifdef CLOCK_MONOTONIC
	if (clock_gettime (CLOCK_MONOTONIC, ts))
#endif
	{
		static long freq = 0;
		if (freq == 0)
			freq = sysconf (_SC_CLK_TCK);

		struct tms dummy;
		clock_t t = times (&dummy);
		ts->tv_sec = t / freq;
		ts->tv_nsec = (t % freq) * (1000000000 / freq);
	}
}


static int
getipv6byname (const char *name, const char *ifname, int numeric, struct sockaddr_in6 *addr)
{
	struct addrinfo hints, *res;
	memset (&hints, 0, sizeof (hints));
	hints.ai_family = PF_INET6;
	hints.ai_socktype = SOCK_DGRAM; /* dummy */
	hints.ai_flags = numeric ? AI_NUMERICHOST : 0;

	int val = getaddrinfo (name, NULL, &hints, &res);
	if (val)
	{
		fprintf (stderr, "%s: %s\n", name, gai_strerror (val));
		return -1;
	}

	memcpy (addr, res->ai_addr, sizeof (struct sockaddr_in6));
	freeaddrinfo (res);

	val = if_nametoindex (ifname);
	if (val == 0)
	{
		perror (ifname);
		return -1;
	}
	addr->sin6_scope_id = val;

	return 0;
}


static inline int
sethoplimit (int fd, int value)
{
	return (setsockopt (fd, IPPROTO_IPV6, IPV6_MULTICAST_HOPS,
	                    &value, sizeof (value))
	     || setsockopt (fd, IPPROTO_IPV6, IPV6_UNICAST_HOPS,
	                    &value, sizeof (value))) ? -1 : 0;
}


static char MACADDR[24];

void
clear_macaddr() {
	memset(MACADDR, '\0', sizeof(MACADDR));
}

char *
get_macaddr()
{
	return MACADDR;
}


static void
printmacaddress (const uint8_t *ptr, size_t len)
{
	char mac[4];
	while (len > 1)
	{
		sprintf(mac, "%02X:", *ptr);
		strcat(MACADDR, mac);
		ptr++;
		len--;
	}

	if (len == 1) {
		sprintf(mac, "%02X", *ptr);
		strcat(MACADDR, mac);
	}
}


# ifdef __linux__
#  include <sys/ioctl.h>
# endif

static int
getmacaddress (const char *ifname, uint8_t *addr)
{
# ifdef SIOCGIFHWADDR
	struct ifreq req;
	memset (&req, 0, sizeof (req));

	if (((unsigned)strlen (ifname)) >= (unsigned)IFNAMSIZ)
		return -1; /* buffer overflow = local root */
	strcpy (req.ifr_name, ifname);

	int fd = socket (AF_INET6, SOCK_DGRAM, 0);
	if (fd == -1)
		return -1;

	if (ioctl (fd, SIOCGIFHWADDR, &req))
	{
		perror (ifname);
		close (fd);
		return -1;
	}
	close (fd);

	memcpy (addr, req.ifr_hwaddr.sa_data, 6);
	return 0;
# else
	(void)ifname;
	(void)addr;
	return -1;
# endif
}


static const uint8_t nd_type_advert = ND_NEIGHBOR_ADVERT;
static const unsigned nd_delay_ms = 1000;
static const unsigned ndisc_default = NDISC_VERBOSE1 | NDISC_SINGLE;


typedef struct
{
	struct nd_neighbor_solicit hdr;
	struct nd_opt_hdr opt;
	uint8_t hw_addr[6];
} solicit_packet;


static ssize_t
buildsol (solicit_packet *ns, struct sockaddr_in6 *tgt, const char *ifname)
{
	/* builds ICMPv6 Neighbor Solicitation packet */
	ns->hdr.nd_ns_type = ND_NEIGHBOR_SOLICIT;
	ns->hdr.nd_ns_code = 0;
	ns->hdr.nd_ns_cksum = 0; /* computed by the kernel */
	ns->hdr.nd_ns_reserved = 0;
	memcpy (&ns->hdr.nd_ns_target, &tgt->sin6_addr, 16);

	/* determines actual multicast destination address */
	memcpy (tgt->sin6_addr.s6_addr, "\xff\x02\x00\x00\x00\x00\x00\x00"
	                                "\x00\x00\x00\x01\xff", 13);

	/* gets our own interface's link-layer address (MAC) */
	if (getmacaddress (ifname, ns->hw_addr))
		return sizeof (ns->hdr);
	
	ns->opt.nd_opt_type = ND_OPT_SOURCE_LINKADDR;
	ns->opt.nd_opt_len = 1; /* 8 bytes */
	return sizeof (*ns);
}


static int
parseadv (const uint8_t *buf, size_t len, const struct sockaddr_in6 *tgt, bool verbose)
{
	const struct nd_neighbor_advert *na =
		(const struct nd_neighbor_advert *)buf;
	const uint8_t *ptr;
	
	/* checks if the packet is a Neighbor Advertisement, and
	 * if the target IPv6 address is the right one */
	if ((len < sizeof (struct nd_neighbor_advert))
	 || (na->nd_na_type != ND_NEIGHBOR_ADVERT)
	 || (na->nd_na_code != 0)
	 || memcmp (&na->nd_na_target, &tgt->sin6_addr, 16))
		return -1;

	len -= sizeof (struct nd_neighbor_advert);

	/* looks for Target Link-layer address option */
	ptr = buf + sizeof (struct nd_neighbor_advert);

	while (len >= 8)
	{
		uint16_t optlen;

		optlen = ((uint16_t)(ptr[1])) << 3;
		if (optlen == 0)
			break; /* invalid length */

		if (len < optlen) /* length > remaining bytes */
			break;
		len -= optlen;


		/* skips unrecognized option */
		if (ptr[0] != ND_OPT_TARGET_LINKADDR)
		{
			ptr += optlen;
			continue;
		}

		/* Found! displays link-layer address */
		ptr += 2;
		optlen -= 2;
		if (verbose)
			fputs ("Target link-layer address: ", stdout);

		printmacaddress (ptr, optlen);
		return 0;
	}

	return -1;
}
static ssize_t
recvfromLL (int fd, void *buf, size_t len, int flags, struct sockaddr_in6 *addr)
{
	char cbuf[CMSG_SPACE (sizeof (int))];
	struct iovec iov =
	{
		.iov_base = buf,
		.iov_len = len
	};
	struct msghdr hdr =
	{
		.msg_name = addr,
		.msg_namelen = sizeof (*addr),
		.msg_iov = &iov,
		.msg_iovlen = 1,
		.msg_control = cbuf,
		.msg_controllen = sizeof (cbuf)
	};

	ssize_t val = recvmsg (fd, &hdr, flags);
	if (val == -1)
		return val;

	/* ensures the hop limit is 255 */
	struct cmsghdr *cmsg;
	for (cmsg = CMSG_FIRSTHDR (&hdr);
	     cmsg != NULL;
	     cmsg = CMSG_NXTHDR (&hdr, cmsg))
	{
		if ((cmsg->cmsg_level == IPPROTO_IPV6)
		 && (cmsg->cmsg_type == IPV6_HOPLIMIT))
		{
			if (255 != *(int *)CMSG_DATA (cmsg))
			{
				// pretend to be a spurious wake-up
				errno = EAGAIN;
				return -1;
			}
		}
	}

	return val;
}


static ssize_t
recvadv (int fd, const struct sockaddr_in6 *tgt, unsigned wait_ms, unsigned flags)
{
	struct timespec now, end;
	unsigned responses = 0;

	/* computes deadline time */
	mono_gettime (&now);
	{
		div_t d;
		
		d = div (wait_ms, 1000);
		end.tv_sec = now.tv_sec + d.quot;
		end.tv_nsec = now.tv_nsec + (d.rem * 1000000);
	}

	/* receive loop */
	for (;;)
	{
		/* waits for reply until deadline */
		ssize_t val = 0;
		if (end.tv_sec >= now.tv_sec)
		{
			val = (end.tv_sec - now.tv_sec) * 1000
				+ (int)((end.tv_nsec - now.tv_nsec) / 1000000);
			if (val < 0)
				val = 0;
		}

		val = poll (&(struct pollfd){ .fd = fd, .events = POLLIN }, 1, val);
		if (val < 0)
			break;

		if (val == 0)
			return responses;

		/* receives an ICMPv6 packet */
		// TODO: use interface MTU as buffer size
		union
		{
			uint8_t  b[1460];
			uint64_t align;
		} buf;
		struct sockaddr_in6 addr;

		val = recvfromLL (fd, &buf, sizeof (buf), MSG_DONTWAIT, &addr);
		if (val == -1)
		{
			if (errno != EAGAIN)
				perror ("Receiving ICMPv6 packet");
			continue;
		}

		/* ensures the response came through the right interface */
		if (addr.sin6_scope_id
		 && (addr.sin6_scope_id != tgt->sin6_scope_id))
			continue;

		if (parseadv (buf.b, val, tgt, (flags & NDISC_VERBOSE) != 0) == 0)
		{
			if (flags & NDISC_VERBOSE)
			{
				char str[INET6_ADDRSTRLEN];

				if (inet_ntop (AF_INET6, &addr.sin6_addr, str,
						sizeof (str)) != NULL)
					printf (" from %s\n", str);
			}

			if (responses < INT_MAX)
				responses++;

			if (flags & NDISC_SINGLE)
				return 1 /* = responses */;
		}
		mono_gettime (&now);
	}

	return -1; /* error */
}

bool
ndisc (const char *name, const char *ifname, unsigned flags, unsigned retry, unsigned wait_ms)
{
	struct sockaddr_in6 tgt;

	int fd = socket (PF_INET6, SOCK_RAW, IPPROTO_ICMPV6);

	if (fd == -1)
	{
		perror ("Raw IPv6 socket");
		return false;
	}

	fcntl (fd, F_SETFD, FD_CLOEXEC);

	/* set ICMPv6 filter */
	{
		struct icmp6_filter f;

		ICMP6_FILTER_SETBLOCKALL (&f);
		ICMP6_FILTER_SETPASS (nd_type_advert, &f);
		setsockopt (fd, IPPROTO_ICMPV6, ICMP6_FILTER, &f, sizeof (f));
	}

	setsockopt (fd, SOL_SOCKET, SO_DONTROUTE, &(int){ 1 }, sizeof (int));

	/* sets Hop-by-hop limit to 255 */
	sethoplimit (fd, 255);
	setsockopt (fd, IPPROTO_IPV6, IPV6_RECVHOPLIMIT,
	            &(int){ 1 }, sizeof (int));

	/* resolves target's IPv6 address */
	if (getipv6byname (name, ifname, (flags & NDISC_NUMERIC) ? 1 : 0, &tgt))
		goto error;
	else
	{
		char s[INET6_ADDRSTRLEN];

		inet_ntop (AF_INET6, &tgt.sin6_addr, s, sizeof (s));
		if (flags & NDISC_VERBOSE)
			printf ("Soliciting %s (%s) on %s...\n", name, s, ifname);
	}

	{
		solicit_packet packet;
		struct sockaddr_in6 dst;
		ssize_t plen;

		memcpy (&dst, &tgt, sizeof (dst));
		plen = buildsol (&packet, &dst, ifname);
		if (plen == -1)
			goto error;

		while (retry > 0)
		{
			/* sends a Solitication */
			if (sendto (fd, &packet, plen, 0,
			            (const struct sockaddr *)&dst,
			            sizeof (dst)) != plen)
			{
				//perror ("Sending ICMPv6 packet");
				goto error;
			}
			retry--;
	
			/* receives an Advertisement */
			ssize_t val = recvadv (fd, &tgt, wait_ms, flags);
			if (val > 0)
			{
				close (fd);
				return true;
			}
			else
			if (val == 0)
			{
				if (flags & NDISC_VERBOSE)
					puts ("Timed out.");
			}
			else
				goto error;
		}
	}

	close (fd);
	if (flags & NDISC_VERBOSE)
		puts ("No response.");
	return false;

error:
	close (fd);
	return false;
}

