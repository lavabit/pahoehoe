/*
* libslack - http://libslack.org/
*
* Copyright (C) 1999-2002, 2004, 2010, 2020-2021 raf <raf@raf.org>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <https://www.gnu.org/licenses/>.
*
* 20210220 raf <raf@raf.org>
*/

/*

=head1 NAME

I<libslack(net)> - network module

=head1 SYNOPSIS

    #define _GNU_SOURCE

    #include <slack/std.h>
    #include <slack/net.h>

    typedef struct sockaddr sockaddr_t;
    typedef unsigned short sockport_t;
    typedef struct sockopt_t sockopt_t;

    typedef union sockaddr_any_t sockaddr_any_t;
    typedef struct sockaddr_un sockaddr_un_t;
    typedef struct sockaddr_in sockaddr_in_t;
    typedef struct sockaddr_in6 sockaddr_in6_t;

    typedef struct net_interface_t net_interface_t;
    typedef struct rudp_t rudp_t;

    struct sockopt_t
    {
        int level;
        int optname;
        const void *optval;
        int optlen;
    };

    union sockaddr_any_t
    {
        sockaddr_t any;
        sockaddr_un_t un;
        sockaddr_in_t in;
        sockaddr_in6_t in6;
    };

    struct net_interface_t
    {
        char name[IFNAMSIZ];
        unsigned int index;
        short flags;
        int mtu;
        sockaddr_any_t *addr;
        sockaddr_any_t *brdaddr;
        sockaddr_any_t *dstaddr;
        sockaddr_any_t *hwaddr;
    };

    int net_server(const char *interface, const char *service, sockport_t port, int rcvbufsz, int sndbufsz, sockaddr_t *addr, size_t *addrsize);
    int net_client(const char *host, const char *service, sockport_t port, long timeout, int rcvbufsz, int sndbufsz, sockaddr_t *addr, size_t *addrsize);
    int net_udp_server(const char *interface, const char *service, sockport_t port, int rcvbufsz, int sndbufsz, sockaddr_t *addr, size_t *addrsize);
    int net_udp_client(const char *host, const char *service, sockport_t port, int rcvbufsz, int sndbufsz, sockaddr_t *addr, size_t *addrsize);
    int net_create_server(const char *interface, const char *service, sockport_t port, int type, int protocol, sockopt_t *sockopts, sockaddr_t *addr, size_t *addrsize);
    int net_create_client(const char *host, const char *service, sockport_t port, sockport_t localport, int type, int protocol, long timeout, sockopt_t *sockopts, sockaddr_t *addr, size_t *addrsize);
    int net_multicast_sender(const char *group, const char *service, sockport_t port, sockopt_t *sockopts, sockaddr_t *addr, size_t *addrsize, const char *ifname, unsigned int ifindex, int ttl, unsigned int noloopback);
    int net_multicast_receiver(const char *group, const char *service, sockport_t port, sockopt_t *sockopts, sockaddr_t *addr, size_t *addrsize, const char *ifname, unsigned int ifindex);
    int net_multicast_join(int sockfd, const sockaddr_t *addr, size_t addrsize, const char *ifname, unsigned int ifindex);
    int net_multicast_leave(int sockfd, const sockaddr_t *addr, size_t addrsize, const char *ifname, unsigned int ifindex);
    int net_multicast_set_interface(int sockfd, const char *ifname, unsigned int ifindex);
    int net_multicast_get_interface(int sockfd);
    int net_multicast_set_loopback(int sockfd, unsigned int loopback);
    int net_multicast_get_loopback(int sockfd);
    int net_multicast_set_ttl(int sockfd, int ttl);
    int net_multicast_get_ttl(int sockfd);
    int net_tos_lowdelay(int sockfd);
    int net_tos_throughput(int sockfd);
    int net_tos_reliability(int sockfd);
    int net_tos_lowcost(int sockfd);
    int net_tos_normal(int sockfd);
    struct hostent *net_gethostbyname(const char *name, struct hostent *hostbuf, void **buf, size_t *size, int *herrno);
    struct servent *net_getservbyname(const char *name, const char *proto, struct servent *servbuf, void **buf, size_t *size);
    int net_options(int sockfd, sockopt_t *sockopts);
    List *net_interfaces(void);
    List *net_interfaces_with_locker(Locker *locker);
    List *net_interfaces_by_family(int family);
    List *net_interfaces_by_family_with_locker(int family, Locker *locker);
    rudp_t *rudp_create(void);
    void rudp_release(rudp_t *rudp);
    void *rudp_destroy(rudp_t **rudp);
    ssize_t net_rudp_transact(int sockfd, rudp_t *rudp, const void *obuf, size_t osize, void *ibuf, size_t isize);
    ssize_t net_rudp_transactwith(int sockfd, rudp_t *rudp, const void *obuf, size_t osize, int oflags, void *ibuf, size_t isize, int iflags, sockaddr_any_t *addr, size_t addrsize);
    ssize_t net_pack(int sockfd, long timeout, int flags, const char *format, ...);
    ssize_t net_vpack(int sockfd, long timeout, int flags, const char *format, va_list args);
    ssize_t net_packto(int sockfd, long timeout, int flags, const sockaddr_t *to, size_t tosize, const char *format, ...);
    ssize_t net_vpackto(int sockfd, long timeout, int flags, const sockaddr_t *to, size_t tosize, const char *format, va_list args);
    ssize_t net_unpack(int sockfd, long timeout, int flags, const char *format, ...);
    ssize_t net_vunpack(int sockfd, long timeout, int flags, const char *format, va_list args);
    ssize_t net_unpackfrom(int sockfd, long timeout, int flags, sockaddr_t *from, size_t *fromsize, const char *format, ...);
    ssize_t net_vunpackfrom(int sockfd, long timeout, int flags, sockaddr_t *from, size_t *fromsize, const char *format, va_list args);
    ssize_t pack(void *buf, size_t size, const char *format, ...);
    ssize_t vpack(void *buf, size_t size, const char *format, va_list args);
    ssize_t unpack(void *buf, size_t size, const char *format, ...);
    ssize_t vunpack(void *buf, size_t size, const char *format, va_list args);
    ssize_t net_read(int sockfd, long timeout, char *buf, size_t count);
    ssize_t net_write(int sockfd, long timeout, const char *buf, size_t count);
    ssize_t net_expect(int sockfd, long timeout, const char *format, ...);
    ssize_t net_vexpect(int sockfd, long timeout, const char *format, va_list args);
    ssize_t net_send(int sockfd, long timeout, const char *format, ...);
    ssize_t net_vsend(int sockfd, long timeout, const char *format, va_list args);
    ssize_t sendfd(int sockfd, const void *buf, size_t nbytes, int flags, int fd);
    ssize_t recvfd(int sockfd, void *buf, size_t nbytes, int flags, int *fd);
    #ifdef SO_PASSCRED
    #ifdef SCM_CREDENTIALS
    ssize_t recvcred(int sockfd, void *buf, size_t nbytes, int flags, struct ucred *cred);
    ssize_t recvfromcred(int sockfd, void *buf, size_t nbytes, int flags, struct sockaddr *src_addr, socklen_t *src_addrlen, struct ucred *cred);
    #endif
    #endif
    int mail(const char *server, const char *sender, const char *recipients, const char *subject, const char *message);

=head1 DESCRIPTION

This module provides functions that create client and server sockets (IPv4,
IPv6, and UNIX domain sockets, stream or datagram), that expect and send
text dialogues/protocols, and that pack and unpack packets according to
templates. IPv4 and IPv6 multicasting is supported. Reliability over UDP is
provided. There are also a function to send mail, and functions to send and
receive open file descriptors via UNIX domain sockets from one process to
another, and functions to send and receive user credentials via UNIX domain
sockets (if supported by the operating system).

=over 4

=cut

*/

#include "config.h"

#ifndef NO_POSIX_SOURCE
#define NO_POSIX_SOURCE /* For EINPROGRESS, EPROTONOSUPPORT, ETIMEDOUT on FreeBSD-8.0 */
#endif

#ifndef _BSD_SOURCE
#define _BSD_SOURCE /* For gethostbyname_r() on Linux */
#endif

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE /* New name for _BSD_SOURCE */
#endif

#ifndef __BSD_VISIBLE
#define __BSD_VISIBLE 1 /* For htons(), htonl(), ntohl() on FreeBSD-8.0 */
#endif

#ifndef _NETBSD_SOURCE
#define _NETBSD_SOURCE  /* So <netinet/ip.h> won't be broken on NetBSD-5.0.2 */
#endif

#ifndef _XOPEN_SOURCE_EXTENDED
#define _XOPEN_SOURCE_EXTENDED 1  /* For msghdr.msg_control[len], CMSG_FIRSTHDR, CMSG_DATA on Solaris-10 10/09 and OpenSolaris 200906 */
#endif

#ifndef _GNU_SOURCE
#define _GNU_SOURCE /* For receiving user credentials over UNIX domain sockets */
#endif

#include "std.h"

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <netdb.h>
#include <net/if.h>
#define BSD_COMP /* for SIOCGIF... on Solaris */
#include <sys/ioctl.h>
#include <netinet/in_systm.h>
#include <netinet/in.h> /* needed by <netinet/ip.h> under OpenBSD */
#include <netinet/ip.h>

#include "net.h"
#include "err.h"
#include "str.h"
#include "fio.h"
#include "mem.h"

#ifndef HAVE_SNPRINTF
#include "snprintf.h"
#endif

#ifndef HAVE_VSSCANF
#include "vsscanf.h"
#endif

#ifdef SOCKS
#include "socks.h"
#endif

#ifndef MSG_SIZE
#define MSG_SIZE 8192
#endif

#ifndef EPROTO /* Mac OS X doesn't have EPROTO */
#define EPROTO EPROTOTYPE
#endif

#ifndef AF_LOCAL /* Solaris 2.6 doesn't have AF_LOCAL */
#define AF_LOCAL AF_UNIX
#endif

#ifndef TEST

#ifndef HAVE_IFREQ_IFR_IFINDEX
#define ifr_ifindex ifr_index
#endif
#ifndef HAVE_IFREQ_IFR_MTU
#define ifr_mtu ifr_ifindex /* ? */
#endif

struct rudp_t
{
	double rtt;        /* most recent round trip time in seconds */
	double srtt;       /* smoothed round trip time estimator in seconds */
	double rttvar;     /* smoothed mean deviation in seconds */
	double rto;        /* current retransmission timeout in seconds */
	int nrexmt;        /* number of times retransmitted */
	uint32_t base;     /* number of seconds since epoch at start */
	uint32_t sequence; /* sequence number */
};

#ifndef RUDP_RXTMIN
#define RUDP_RXTMIN 2 /* minimum retransmission timeout in seconds */
#endif

#ifndef RUDP_RXTMAX
#define RUDP_RXTMAX 60 /* maximum retransmission timeout in seconds */
#endif

#ifndef RUDP_MAXNREXMT
#define RUDP_MAXNREXMT 3 /* maximum number of times to retransmit */
#endif

/*

=item C<int net_server(const char *interface, const char *service, sockport_t port, int rcvbufsz, int sndbufsz, sockaddr_t *addr, size_t *addrsize)>

Creates a TCP server socket ready to I<accept(2)> connections on
C<interface> (as determined by I<gethostbyname(3)>).

If C<interface> is C<null>, connections will be accepted on all local
network interfaces. Otherwise, connections will only be accepted on the
specified interface (as determined by I<gethostbyname(3)>).

If C<service> is non-C<null>, and is either numeric, or is a service name
(as determined by I<getservbyname(3)>), the specified port is used.
Otherwise, C<port> (which must be in host byte order) is used.

If C<interface> is equal to C<"/unix"> and C<service> is an absolute file
system path, the server socket created will be a I<UNIX domain stream
socket>. Otherwise, a TCP server socket is created. If the C<RES_OPTIONS>
environment variable exists and contains the string C<"inet6">, or the
C</etc/resolv.conf> file contains the C<inet6> option, the TCP socket will
be an IPv6 socket. Otherwise, it will be an IPv4 socket.

If C<rcvbufsz> is non-zero, the socket's receive buffer size is set to this
size. Note that you may not get the size you request. If this is important,
use I<getsockopt(2)> to obtain the actual receive buffer size.

If C<sndbufsz> is non-zero, the socket's send buffer size is set to this
size. Note that you may not get the size you ask for. If this is important,
use I<getsockopt(2)> to obtain the actual send buffer size.

If C<addr> and C<addrsize> are not C<null>, the address bound to is stored
in the buffer pointed to by C<addr>. C<*addrsize> specifies the size of the
buffer pointed to by C<addr>. If there is insufficient space, the bound
address is not stored in C<addr>. If C<addrsize> is not C<null>, the length
of the address is stored there.

On success, returns the new socket descriptor. On error, returns C<-1> with
C<errno> set appropriately.

=cut

*/

static sockopt_t *build_sockopts(sockopt_t *sockopts, int *rcvbufsz, int *sndbufsz)
{
	size_t so = 0;

	if (*rcvbufsz)
	{
		sockopts[so].level = SOL_SOCKET;
		sockopts[so].optname = SO_RCVBUF;
		sockopts[so].optval = rcvbufsz;
		sockopts[so].optlen = sizeof(int);
		so++;
	}

	if (*sndbufsz)
	{
		sockopts[so].level = SOL_SOCKET;
		sockopts[so].optname = SO_SNDBUF;
		sockopts[so].optval = sndbufsz;
		sockopts[so].optlen = sizeof(int);
		so++;
	}

	sockopts[so].optval = NULL;

	return sockopts;
}

int net_server(const char *interface, const char *service, sockport_t port, int rcvbufsz, int sndbufsz, sockaddr_t *addr, size_t *addrsize)
{
	sockopt_t sockopts[3];

	build_sockopts(sockopts, &rcvbufsz, &sndbufsz);

	return net_create_server(interface, service, port, SOCK_STREAM, 0, sockopts, addr, addrsize);
}

/*

=item C<int net_client(const char *host, const char *service, sockport_t port, long timeout, int rcvbufsz, int sndbufsz, sockaddr_t *addr, size_t *addrsize)>

Creates a TCP client socket and connects to the server listening at C<host>
(as determined by I<gethostbyname(3)>) on the port number specified by
C<service>. C<service> must either be numeric, or a service name as
determined by I<getservbyname(3)>. Otherwise, the port number to connect to
is given by C<port> (which must be in host byte order). If C<host> is
C<null>, the client socket connects to the loopback address.

If C<host> is equal to C<"/unix"> and C<service> is an absolute file system
path, the client socket created will be a I<UNIX domain stream socket>.
Otherwise, a TCP client socket is created. If the C<RES_OPTIONS> environment
variable exists and contains the string C<"inet6">, or the
C</etc/resolv.conf> file contains the C<inet6> option, the TCP socket will
be an IPv6 socket. Otherwise, it will be an IPv4 socket.

If C<timeout> is non-zero, it specifies the number of seconds after which to
timeout the attempt to connect to the specified server. This can be useful
if the client may attempt to connect to a service that is blocked by a
firewall that drops its packets or if the host that you are connecting to
does not protect itself from SYN floods. The native TCP timeouts are very
long (usually minutes) when faced with an unresponsive network and you may
not want your programs or their users to wait that long.

If C<rcvbufsz> is non-zero, the socket's receive buffer size is set to this
size. Note that you may not get the size you request. If this is important,
use I<getsockopt(2)> to obtain the actual receive buffer size.

If C<sndbufsz> is non-zero, the socket's send buffer size is set to this
size. Note that you may not get the size you ask for. If this is important,
use I<getsockopt(2)> to obtain the actual send buffer size.

If C<addr> and C<addrsize> are not C<null>, the address of the peer is
stored in the buffer pointed to by C<addr>. C<*addrsize> specifies the size
of the buffer pointed to by C<addr>. If there is insufficient space, the
peer's address is not stored in C<addr>. If C<addrsize> is not C<null>, the
size of the peer's address is stored there.

On success, returns the new socket descriptor. On error, returns C<-1> with
C<errno> set appropriately.

=cut

*/

int net_client(const char *host, const char *service, sockport_t port, long timeout, int rcvbufsz, int sndbufsz, sockaddr_t *addr, size_t *addrsize)
{
	sockopt_t sockopts[3];

	build_sockopts(sockopts, &rcvbufsz, &sndbufsz);

	return net_create_client(host, service, port, 0, SOCK_STREAM, 0, timeout, sockopts, addr, addrsize);
}

/*

=item C<int net_udp_server(const char *interface, const char *service, sockport_t port, int rcvbufsz, int sndbufsz, sockaddr_t *addr, size_t *addrsize)>

Equivalent to I<net_server(3)> except that a UDP server is socket is
created. If C<interface> is equal to C<"/unix"> and C<service> is an
absolute file system path, the server socket created will be a I<UNIX domain
datagram socket>. On success, returns the new socket's file descriptor. On
error, returns C<-1> with C<errno> set appropriately.

=cut

*/

int net_udp_server(const char *interface, const char *service, sockport_t port, int rcvbufsz, int sndbufsz, sockaddr_t *addr, size_t *addrsize)
{
	sockopt_t sockopts[3];

	build_sockopts(sockopts, &rcvbufsz, &sndbufsz);

	return net_create_server(interface, service, port, SOCK_DGRAM, 0, sockopts, addr, addrsize);
}

/*

=item C<int net_udp_client(const char *host, const char *service, sockport_t port, int rcvbufsz, int sndbufsz, sockaddr_t *addr, size_t *addrsize)>

Equivalent to I<net_client(3)> except that a UDP client socket is created.
If C<interface> is equal to C<"/unix"> and C<service> is an absolute file
system path, the server socket created will be a I<UNIX domain datagram
socket>. On success, returns the new socket's file descriptor. On error,
returns C<-1> with C<errno> set appropriately.

=cut

*/

int net_udp_client(const char *host, const char *service, sockport_t port, int rcvbufsz, int sndbufsz, sockaddr_t *addr, size_t *addrsize)
{
	sockopt_t sockopts[3];

	build_sockopts(sockopts, &rcvbufsz, &sndbufsz);

	return net_create_client(host, service, port, 0, SOCK_DGRAM, 0, 0, sockopts, addr, addrsize);
}

/*

=item C<int net_create_server(const char *interface, const char *service, sockport_t port, int type, int protocol, sockopt_t *sockopts, sockaddr_t *addr, size_t *addrsize)>

Equivalent to I<net_server(3)> and I<net_udp_server(3)> only more general.
The type of socket is specified by C<type> (e.g. C<SOCK_STREAM> or
C<SOCK_DGRAM>) and C<protocol> (usually zero). If C<sockopts> is not
C<null>, the socket options specified are set before calling I<bind(2)>. On
success, returns the new socket's file descriptor. On error, returns C<-1>
with C<errno> set appropriately.

=cut

*/

static sockaddr_t *net_unaddr(sockaddr_un_t *un, size_t family, const char *path)
{
	memset(un, 0, sizeof(sockaddr_un_t));
	un->sun_family = family;
	strlcpy(un->sun_path, path, sizeof(un->sun_path));
	return (sockaddr_t *)un;
}

static sockaddr_t *net_inaddr(sockaddr_in_t *in, size_t family, const void *addr, size_t addrsize, sockport_t port)
{
	memset(in, 0, sizeof(sockaddr_in_t));
	in->sin_family = family;
	memcpy(&in->sin_addr, addr, addrsize);
	in->sin_port = port;
	return (sockaddr_t *)in;
}

#ifdef AF_INET6
static sockaddr_t *net_in6addr(sockaddr_in6_t *in6, size_t family, const void *addr, size_t addrsize, sockport_t port)
{
	memset(in6, 0, sizeof(sockaddr_in6_t));
	in6->sin6_family = family;
	memcpy(&in6->sin6_addr, addr, addrsize);
	in6->sin6_port = port;
	return (sockaddr_t *)in6;
}
#endif

#ifdef AF_INET6
static int inet6_required(void)
{
	char *res_options;
	FILE *resolv_conf;

	if ((res_options = getenv("RES_OPTIONS")) && strstr(res_options, "inet6"))
		return 1;

	if ((resolv_conf = fopen("/etc/resolv.conf", "r")))
	{
		char line[BUFSIZ];

		while (fgets(line, BUFSIZ, resolv_conf))
		{
			if (!strncmp(line, "options", 7) && strstr(line + 8, "inet6"))
			{
				fclose(resolv_conf);
				return 1;
			}
		}

		fclose(resolv_conf);
	}

	return 0;
}
#endif

static const char *getprotonamebysocktype(int socktype)
{
	switch (socktype)
	{
		case SOCK_STREAM: return "tcp";
		case SOCK_DGRAM: return "udp";
		default: return NULL;
	}
}

static sockport_t getservportbynameandtype(const char *name, int type)
{
	struct servent servbuf[1];
	struct servent *serv;
	void *buf = NULL;
	size_t size = 0;
	sockport_t port = 0;
	const char *proto;

	proto = getprotonamebysocktype(type);

	if ((serv = net_getservbyname(name, proto, servbuf, &buf, &size)))
		port = serv->s_port;

	free(buf);

	return port;
}

static int service_number(const char *service)
{
	char *endptr = NULL;
	unsigned long val = strtoul(service, &endptr, 10);

	if ((val = strtoul(service, &endptr, 10)) > USHRT_MAX)
		return set_errno(ERANGE);

	if (endptr == service || *endptr != '\0')
		return set_errno(EDOM);

	return (int)val;
}

static sockport_t service_port(const char *service, int type, int port)
{
	if (service)
	{
		int ret;

		if ((ret = service_number(service)) != -1)
			return htons((sockport_t)ret);

		if ((ret = getservportbynameandtype(service, type)) != 0)
			return ret;
	}

	return htons(port);
}

static int is_multicast(sockaddr_t *address)
{
	sockaddr_any_t *addr = (sockaddr_any_t *)address;
	long *longptr;

	switch (addr->any.sa_family)
	{
		case AF_INET:
			/* Avoid dereferencing type-punned pointer to avoid gcc warning */
			/*return IN_MULTICAST(ntohl(*(long *)&addr->in.sin_addr));*/
			longptr = (long *)&addr->in.sin_addr;
			return IN_MULTICAST(ntohl(*longptr));

#ifdef AF_INET6
		case AF_INET6:
			return IN6_IS_ADDR_MULTICAST(&addr->in6.sin6_addr);
#endif
	}

	return 0;
}

int net_create_server(const char *interface, const char *service, sockport_t port, int type, int protocol, sockopt_t *sockopts, sockaddr_t *addr, size_t *addrsize)
{
	int sockfd;
	sockaddr_any_t localany;
	sockaddr_t *localaddr;
	size_t localsize;
	struct hostent *hostent;
	int reuse_addr = 1;

	/* Check for UNIX domain socket specification */

	if (interface && !strcmp(interface, "/unix"))
	{
		if (!service || *service != '/' || !service[1] || strlen(service) >= sizeof localany.un.sun_path)
			return set_errno(EINVAL);

		localaddr = net_unaddr(&localany.un, AF_LOCAL, service);
		localsize = sizeof localany.un;
		unlink(localany.un.sun_path);
	}
	else /* IPv4 or IPv6 */
	{
		/* Set port to service's port number if possible */

		port = service_port(service, type, port);

		/* Set localaddr and localsize to the specified interface, or any */

		if (interface)
		{
			struct hostent hostbuf[1];
			void *buf = NULL;
			size_t size = 0;
			int herrno;

			if (!(hostent = net_gethostbyname(interface, hostbuf, &buf, &size, &herrno)))
			{
				free(buf);
				return set_errno(ENOENT);
			}

			if (hostent->h_addrtype == AF_INET)
			{
				localaddr = net_inaddr(&localany.in, hostent->h_addrtype, hostent->h_addr_list[0], hostent->h_length, port);
				localsize = sizeof localany.in;
			}
#ifdef AF_INET6
			else if (hostent->h_addrtype == AF_INET6)
			{
				localaddr = net_in6addr(&localany.in6, hostent->h_addrtype, hostent->h_addr_list[0], hostent->h_length, port);
				localsize = sizeof localany.in6;
			}
#endif
			else
			{
				free(buf);
				return set_errno(ENOSYS);
			}

			free(buf);
		}
		else /* wildcard */
		{
#ifdef AF_INET6
			if (inet6_required())
			{
				localaddr = net_in6addr(&localany.in6, AF_INET6, &in6addr_any, sizeof in6addr_any, port);
				localsize = sizeof localany.in6;
			}
			else
#endif
			{
				unsigned long inaddr_any = htonl(INADDR_ANY);
				localaddr = net_inaddr(&localany.in, AF_INET, &inaddr_any, sizeof inaddr_any, port);
				localsize = sizeof localany.in;
			}
		}
	}

	/* Create the socket */

	if ((sockfd = socket(localaddr->sa_family, type, protocol)) == -1)
		return -1;

	/* Set reuseaddr for tcp servers and udp multicast receivers */

	if ((type == SOCK_STREAM && localaddr->sa_family != AF_LOCAL) || (type == SOCK_DGRAM && is_multicast(localaddr)))
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&reuse_addr, sizeof reuse_addr) == -1)
			return close(sockfd), -1;

	/* Set any user supplied socket options */

	if (sockopts && net_options(sockfd, sockopts) == -1)
		return close(sockfd), -1;

	/* bind to localaddr */

	if (bind(sockfd, localaddr, localsize) == -1)
		return close(sockfd), -1;

	/* If connection-oriented, listen */

	if (type == SOCK_STREAM && listen(sockfd, 1024) == -1)
		return close(sockfd), -1;

	/* Return sockfd, localaddr and localsize */

	if (addr && addrsize && *addrsize >= localsize)
		memcpy(addr, localaddr, localsize);

	if (addrsize)
		*addrsize = localsize;

	return sockfd;
}

/*

=item C<int net_create_client(const char *host, const char *service, sockport_t port, sockport_t localport, int type, int protocol, long timeout, sockopt_t *sockopts, sockaddr_t *addr, size_t *addrsize)>

Equivalent to I<net_client(3)> and I<net_udp_client(3)> only more general.
The type of socket is specified by C<type> (e.g. C<SOCK_STREAM> or
C<SOCK_DGRAM>) and C<protocol> (usually zero). If C<localport> is not zero,
it is the port (in host byte order) that the local endpoint binds to. If
C<sockopts> is not C<null>, the socket options specified are set before
calling I<bind(2)>. On success, returns the new socket's file descriptor. On
error, returns C<-1> with C<errno> set appropriately.

=cut

*/

static int net_client_connect(sockaddr_t *remoteaddr, size_t remotesize, sockport_t localport, int type, int protocol, int timeout, sockopt_t *sockopts)
{
	int sockfd;
	int rc;

	/* Create the socket */

	if ((sockfd = socket(remoteaddr->sa_family, type, protocol)) == -1)
		return -1;

	/* Set any user specified socket options */

	if (sockopts && net_options(sockfd, sockopts) == -1)
		return close(sockfd), -1;

	/* If connectionless (or requested), bind (not always needed) */

	if (type == SOCK_DGRAM && (localport || remoteaddr->sa_family == AF_LOCAL))
	{
		sockaddr_any_t localany;
		sockaddr_t *localaddr;
		size_t localsize;

		if (remoteaddr->sa_family == AF_LOCAL)
		{
#if HAVE_UNIX_DOMAIN_WILDCARD
			localaddr = net_unaddr(&localany.un, AF_LOCAL, "");
#else

			/*
			** There is a race condition here. Between the time the path is
			** constructed and bind() creates the inode, another process
			** might create a file with the same path. However, since bind()
			** fails if the path already exists, there's no security risk.
			** Please correct me if I'm wrong. There are bugs, though.
			** bind() will fail when another process creates a file with the
			** same path and the number of possible pathnames is limited by
			** tmpnam(). Fortunately, it's a very large limit.
			** Unfortunately, there's no way around this on some systems
			** (e.g. Solaris). Another annoyance is that the path to which
			** we bind the socket must be unlinked by the application. To
			** get the name, the application must use getsockname() and then
			** unlink() the path when finished with the socket.
			**
			** Linux doesn't have this problem since it lets us bind to ""
			** (the AF_LOCAL equivalent of INADDR_ANY).
			**
			** The easy, elegant, portable solution is to never use UNIX
			** domain datagram sockets. Always use stream sockets instead.
			*/

			char path[L_tmpnam];
			if (!tmpnam(path))
				return close(sockfd), -1;

			localaddr = net_unaddr(&localany.un, AF_LOCAL, path);
#endif
			localsize = sizeof localany.un;
		}
		else
		{
#ifdef AF_INET6
			if (inet6_required())
			{
				localaddr = net_in6addr(&localany.in6, AF_INET6, &in6addr_any, sizeof in6addr_any, htons(localport));
				localsize = sizeof localany.in6;
			}
			else
#endif
			{
				unsigned long inaddr_any = htonl(INADDR_ANY);
				localaddr = net_inaddr(&localany.in, AF_INET, &inaddr_any, sizeof inaddr_any, htons(localport));
				localsize = sizeof localany.in;
			}
		}

		if (bind(sockfd, localaddr, localsize) == -1)
			return close(sockfd), -1;
	}

	/* Connect to remoteaddr (possibly with a timeout) */

	if (timeout && nonblock_on(sockfd) == -1)
		return close(sockfd), -1;

	if ((rc = connect(sockfd, remoteaddr, remotesize)) == -1 && errno != EINPROGRESS)
	{
		int saved_errno = errno;
		close(sockfd);
		return set_errno(saved_errno);
	}

	if (rc == -1)
	{
		int access, err = 0;
		size_t size = sizeof err;

		if ((access = rw_timeout(sockfd, timeout, 0)) == -1)
			return close(sockfd), -1;

		if (!(access & R_OK) && !(access & W_OK))
			return close(sockfd), -1;

		if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, (void *)&err, (void *)&size) == -1)
			return close(sockfd), -1;

		if (err)
			return close(sockfd), set_errno(err);
	}

	if (timeout && nonblock_off(sockfd) == -1)
		return close(sockfd), -1;

	return sockfd;
}

int net_create_client(const char *host, const char *service, sockport_t port, sockport_t localport, int type, int protocol, long timeout, sockopt_t *sockopts, sockaddr_t *addr, size_t *addrsize)
{
	int sockfd;
	sockaddr_any_t remoteany;
	sockaddr_t *remoteaddr;
	size_t remotesize;
	struct hostent *hostent = NULL;
	struct hostent hostbuf[1];
	void *buf = NULL;
	size_t size = 0;
	int herrno;
	size_t h = 0;

	/* Check for UNIX domain socket specification */

	if (host && !strcmp(host, "/unix"))
	{
		if (!service || *service != '/' || !service[1] || strlen(service) >= sizeof remoteany.un.sun_path)
			return set_errno(EINVAL);

		remoteaddr = net_unaddr(&remoteany.un, AF_LOCAL, service);
		remotesize = sizeof remoteany.un;
	}
	else /* IPv4 or IPv6 */
	{
		/* Set port to service's port number if possible */

		port = service_port(service, type, port);

		/* Set remoteaddr and remotesize to the specified host address, or loopback */

		if (host)
		{
			if (!(hostent = net_gethostbyname(host, hostbuf, &buf, &size, &herrno)))
			{
				free(buf);
				return set_errno(ENOENT);
			}

			if (hostent->h_addrtype == AF_INET)
			{
				remoteaddr = net_inaddr(&remoteany.in, hostent->h_addrtype, hostent->h_addr_list[0], hostent->h_length, port);
				remotesize = sizeof remoteany.in;
			}
#ifdef AF_INET6
			else if (hostent->h_addrtype == AF_INET6)
			{
				remoteaddr = net_in6addr(&remoteany.in6, hostent->h_addrtype, hostent->h_addr_list[0], hostent->h_length, port);
				remotesize = sizeof remoteany.in6;
			}
#endif
			else
			{
				free(buf);
				return set_errno(ENOSYS);
			}
		}
		else /* loopback */
		{
#ifdef AF_INET6
			if (inet6_required())
			{
				remoteaddr = net_in6addr(&remoteany.in6, AF_INET6, &in6addr_loopback, sizeof in6addr_loopback, port);
				remotesize = sizeof remoteany.in6;
			}
			else
#endif
			{
				unsigned long inaddr_loopback = htonl(INADDR_LOOPBACK);
				remoteaddr = net_inaddr(&remoteany.in, AF_INET, &inaddr_loopback, sizeof inaddr_loopback, port);
				remotesize = sizeof remoteany.in;
			}
		}
	}

	/* Try to connect to all available addresses */

	for (;;)
	{
		if ((sockfd = net_client_connect(remoteaddr, remotesize, localport, type, protocol, timeout, sockopts)) != -1)
			break;

		/* Try the next address in h_addr_list, if any */

		if (!hostent || !hostent->h_addr_list[++h])
			break;

		if (hostent->h_addrtype == AF_INET)
		{
			remoteaddr = net_inaddr(&remoteany.in, hostent->h_addrtype, hostent->h_addr_list[h], hostent->h_length, port);
			remotesize = sizeof remoteany.in;
		}
#ifdef AF_INET6
		else if (hostent->h_addrtype == AF_INET6)
		{
			remoteaddr = net_in6addr(&remoteany.in6, hostent->h_addrtype, hostent->h_addr_list[h], hostent->h_length, port);
			remotesize = sizeof remoteany.in6;
		}
#endif
	}

	free(buf);

	/* None succeeded */

	if (sockfd == -1)
		return -1;

	/* Return sockfd, remoteaddr and remotesize */

	if (addr && addrsize && *addrsize >= remotesize)
		memcpy(addr, remoteaddr, remotesize);

	if (addrsize)
		*addrsize = remotesize;

	return sockfd;
}

/*

=item C<int net_multicast_sender(const char *group, const char *service, sockport_t port, sockopt_t *sockopts, sockaddr_t *addr, size_t *addrsize, const char *ifname, unsigned int ifindex, int ttl, unsigned int noloopback)>

Creates a UDP multicast sender socket. C<group> specifies the multicast
group that packets will be sent to.

If the C<RES_OPTIONS> environment variable exists and contains the string
C<"inet6"> or the C</etc/resolv.conf> file contains the C<inet6> option, the
multicast sender will be an IPv6 socket. Otherwise, it will be an IPv4
socket.

C<service> must specify a service name or a numeric port number to use.
Otherwise, C<port> (which must be in host byte order) specifies the port
number to use.

C<sockopts> may contain extra socket options to set.

If C<addr> and C<addrsize> are not C<null>, the multicast group's address is
stored in the buffer pointed to by C<addr>. C<*addrsize> specifies the size
of the buffer pointed to by C<addr>. If there is insufficient space, the
address is not stored in C<addr>. If C<addrsize> is not C<null>, the size of
the address is stored there.

If I<ifname> is not C<null>, it specifies the name of the interface on which
to send the multicast packets. Otherwise, if C<ifindex> is not zero, it
specifies the index of the interface on which to send multicast packets.
Otherwise, the kernel will choose the interface on which to send multicast
packets based on the routing table (which is the default behaviour).

If C<ttl> is greater than C<1>, it specifies the multicast packets' TTL. By
default the TTL is C<1>. See the Multicast-HOWTO for details on the scoping
semantics of the TTL field in multicast packets.

If C<noloopback> is not zero, multicast loopback is disabled. This would
prevent any process on the sending host from receiving the multicast packets
sent via this socket. Multicast loopback is enabled by default.

The socket is connected to the specified multicast group address so that
I<send(2)> must be used to send packets, rather than I<sendto(2)>. This
reduces the time spent sending packets by one third because an unconnected
UDP socket is temporarily connected to the destination address by the kernel
every time I<sendto(2)> is called.

On success, returns the new socket descriptor. On error, returns C<-1> with
C<errno> set appropriately.

=cut

*/

int net_multicast_sender(const char *group, const char *service, sockport_t port, sockopt_t *sockopts, sockaddr_t *addr, size_t *addrsize, const char *ifname, unsigned int ifindex, int ttl, unsigned int noloopback)
{
	int sockfd;
	unsigned int loopback = 0;

	if ((sockfd = net_create_client(group, service, port, 0, SOCK_DGRAM, 0, 0, sockopts, addr, addrsize)) == -1)
		return -1;

	if ((ifname || ifindex) && net_multicast_set_interface(sockfd, ifname, ifindex) == -1)
		return close(sockfd), -1;

	if (ttl > 1 && net_multicast_set_ttl(sockfd, ttl) == -1)
		return close(sockfd), -1;

	if (noloopback && net_multicast_set_loopback(sockfd, loopback) == -1)
		return close(sockfd), -1;

	return sockfd;
}

/*

=item C<int net_multicast_receiver(const char *group, const char *service, sockport_t port, sockopt_t *sockopts, sockaddr_t *addr, size_t *addrsize, const char *ifname, unsigned int ifindex)>

Creates a UDP multicast receiver socket. C<group> specifies the multicast
group that the socket will join.

If the C<RES_OPTIONS> environment variable exists and contains the string
C<"inet6"> or the C</etc/resolv.conf> file contains the C<inet6> option, the
multicast receiver socket will be an IPv6 socket. Otherwise, it will be an
IPv4 socket.

C<service> must specify a service name or a numeric port number to use.
Otherwise, C<port> (which must be in host byte order) specifies the port
number to use.

C<sockopts> may contain extra socket options to set.

If C<addr> and C<addrsize> are not C<null>, the multicast group's address is
stored in the buffer pointed to by C<addr>. C<*addrsize> specifies the size
of the buffer pointed to by C<addr>. If there is insufficient space, the
address is not stored in C<addr>. If C<addrsize> is not C<null>, the size of
the address is stored there.

If I<ifname> is not C<null>, it specifies the name of the interface on which
to receive multicast packets. Otherwise, if C<ifindex> is not zero, it
specifies the index of the interface on which to receive multicast packets.
Otherwise, the kernel will choose the interface on which to receive
multicast packets based on the routing table (which is the default
behaviour). The new socket may join the same group on more interfaces by
subsequent calls to I<net_multicast_join(3)>.

On success, returns the new socket descriptor. On error, returns C<-1> with
C<errno> set appropriately.

=cut

*/

int net_multicast_receiver(const char *group, const char *service, sockport_t port, sockopt_t *sockopts, sockaddr_t *addr, size_t *addrsize, const char *ifname, unsigned int ifindex)
{
	sockaddr_any_t any[1];
	size_t anysize = sizeof(any);
	int sockfd;

	if (!addr)
		addr = (sockaddr_t *)any;

	if (!addrsize)
		addrsize = &anysize;

	if ((sockfd = net_create_server(group, service, port, SOCK_DGRAM, 0, sockopts, addr, addrsize)) == -1)
		return -1;

	if (net_multicast_join(sockfd, addr, *addrsize, ifname, ifindex) == -1)
		return close(sockfd), -1;

	return sockfd;
}

/*

=item C<int net_multicast_join(int sockfd, const sockaddr_t *addr, size_t addrsize, const char *ifname, unsigned int ifindex)>

Adds C<sockfd>'s membership to the multicast group specified by C<addr>
whose size is C<addrsize>. If I<ifname> is not C<null>, it specifies the
name of the interface on which to receive multicast packets. Otherwise, if
C<ifindex> is not zero, it specifies the index of the interface on which to
receive multicast packets. Otherwise, the kernel will choose the interface
on which to receive multicast packets based on the routing table (which is
the default behaviour). A multicast socket may join the same group on
multiple interfaces by subsequent calls to I<net_multicast_join(3)>. Note
that there is a system-imposed limit on the number of times a socket may
join a multicast group (this limit can be about 20). On success, returns
C<0>. On error, returns C<-1> with C<errno> set appropriately.

=cut

*/

#ifndef HAVE_IF_INDEXTONAME
static char *if_indextoname(unsigned int ifindex, char *ifname)
{
	List *ifaces;

	if (!(ifaces = net_interfaces()))
		return NULL;

	while (list_has_next(ifaces))
	{
		net_interface_t *iface = list_next(ifaces);

		if (iface->index == ifindex)
		{
			strlcpy(ifname, iface->name, IFNAMSIZ);
			list_release(ifaces);

			return ifname;
		}
	}

	list_release(ifaces);

	return NULL;
}
#endif

#ifndef HAVE_IF_NAMETOINDEX
static unsigned int if_nametoindex(const char *ifname)
{
	List *ifaces;

	if (!(ifaces = net_interfaces()))
		return 0;

	while (list_has_next(ifaces))
	{
		net_interface_t *iface = list_next(ifaces);

		if (!strcmp(ifname, iface->name))
		{
			unsigned int ifindex = iface->index;

			list_release(ifaces);

			return ifindex;
		}
	}

	list_release(ifaces);

	return 0;
}
#endif

#ifndef IPV6_JOIN_GROUP
#define IPV6_JOIN_GROUP IPV6_ADD_MEMBERSHIP
#endif

#ifndef IPV6_LEAVE_GROUP
#define IPV6_LEAVE_GROUP IPV6_DROP_MEMBERSHIP
#endif

int net_multicast_join(int sockfd, const sockaddr_t *addr, size_t addrsize, const char *ifname, unsigned int ifindex)
{
	sockaddr_any_t *any = (sockaddr_any_t *)addr;

	switch (any->any.sa_family)
	{
		case AF_INET:
		{
			struct ip_mreq mreq[1];
			struct ifreq ifreq[1];

			memcpy(&mreq->imr_multiaddr, &any->in.sin_addr, sizeof mreq->imr_multiaddr);

			if (!ifindex && !ifname)
			{
				mreq->imr_interface.s_addr = htonl(INADDR_ANY);
			}
			else
			{
				if (ifname)
				{
					strlcpy(ifreq->ifr_name, ifname, IFNAMSIZ);
				}
				else if (ifindex)
				{
					if (!if_indextoname(ifindex, ifreq->ifr_name))
						return set_errno(ENXIO);
				}

				if (ioctl(sockfd, SIOCGIFADDR, ifreq) == -1)
					return -1;

				memcpy(&mreq->imr_interface, &((sockaddr_in_t *)&ifreq->ifr_addr)->sin_addr, sizeof mreq->imr_multiaddr);
			}

			return setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, mreq, sizeof mreq);
		}

#ifdef AF_INET6
		case AF_INET6:
		{
			struct ipv6_mreq mreq[1];

			memcpy(&mreq->ipv6mr_multiaddr, &any->in6.sin6_addr, sizeof mreq->ipv6mr_multiaddr);

			if (ifname)
			{
				if ((mreq->ipv6mr_interface = if_nametoindex(ifname)) == 0)
					return set_errno(ENXIO);
			}
			else
			{
				mreq->ipv6mr_interface = ifindex;
			}

			return setsockopt(sockfd, IPPROTO_IPV6, IPV6_JOIN_GROUP, mreq, sizeof mreq);
		}
#endif

		default:
			return set_errno(EPROTONOSUPPORT);
	}
}

/*

=item C<int net_multicast_leave(int sockfd, const sockaddr_t *addr, size_t addrsize, const char *ifname, unsigned int ifindex)>

Drops C<sockfd>'s membership from the multicast group specified by C<addr>
whose size is C<addrsize>. If I<ifname> is not C<null>, it specifies the
name of the interface on which to drop group membership. Otherwise, if
C<ifindex> is not zero, it specifies the index of the interface on which to
drop group membership. Otherwise, the interface that joined most recently
will be dropped from the multicast group. On success, returns C<0>. On
error, returns C<-1> with C<errno> set appropriately.

=cut

*/

int net_multicast_leave(int sockfd, const sockaddr_t *addr, size_t addrsize, const char *ifname, unsigned int ifindex)
{
	sockaddr_any_t *any = (sockaddr_any_t *)addr;

	switch (any->any.sa_family)
	{
		case AF_INET:
		{
			struct ip_mreq mreq[1];
			struct ifreq ifreq[1];

			memcpy(&mreq->imr_multiaddr, &any->in.sin_addr, sizeof mreq->imr_multiaddr);

			if (!ifindex && !ifname)
			{
				mreq->imr_interface.s_addr = htonl(INADDR_ANY);
			}
			else
			{
				if (ifname)
				{
					strlcpy(ifreq->ifr_name, ifname, IFNAMSIZ);
				}
				else if (ifindex)
				{
					if (!if_indextoname(ifindex, ifreq->ifr_name))
						return set_errno(ENXIO);
				}

				if (ioctl(sockfd, SIOCGIFADDR, ifreq) == -1)
					return -1;

				memcpy(&mreq->imr_interface, &((sockaddr_in_t *)&ifreq->ifr_addr)->sin_addr, sizeof mreq->imr_interface);
			}

			return setsockopt(sockfd, IPPROTO_IP, IP_DROP_MEMBERSHIP, mreq, sizeof mreq);
		}

#ifdef AF_INET6
		case AF_INET6:
		{
			struct ipv6_mreq mreq[1];

			memcpy(&mreq->ipv6mr_multiaddr, &any->in6.sin6_addr, sizeof mreq->ipv6mr_multiaddr);

			if (ifname)
			{
				if ((mreq->ipv6mr_interface = if_nametoindex(ifname)) == 0)
					return set_errno(ENXIO);
			}
			else
			{
				mreq->ipv6mr_interface = ifindex;
			}

			return setsockopt(sockfd, IPPROTO_IPV6, IPV6_LEAVE_GROUP, mreq, sizeof mreq);
		}
#endif

		default:
			return set_errno(EPROTONOSUPPORT);
	}
}

/*

=item C<int net_multicast_set_interface(int sockfd, const char *ifname, unsigned int ifindex)>

Specifies the interface on which C<sockfd> will send multicast packets. If
I<ifname> is not C<null>, it specifies the name of the interface on which to
send the multicast packets. Otherwise, if C<ifindex> is not zero, it
specifies the index of the interface on which to send multicast packets.
Otherwise, the kernel will choose the interface on which to send multicast
packets based on the routing table (which is the default behaviour). On
success, returns C<0>. On error, returns C<-1> with C<errno> set
appropriately.

=cut

*/

int net_multicast_set_interface(int sockfd, const char *ifname, unsigned int ifindex)
{
	sockaddr_any_t any;
	size_t size = sizeof any;

	if (getsockname(sockfd, (void *)&any, (void *)&size) == -1)
		return -1;

	switch (any.any.sa_family)
	{
		case AF_INET:
		{
			struct ifreq ifreq[1];
			struct in_addr inaddr;

			if (!ifindex && !ifname)
			{
				inaddr.s_addr = htonl(INADDR_ANY);
			}
			else
			{
				if (ifname)
				{
					strlcpy(ifreq->ifr_name, ifname, IFNAMSIZ);
				}
				else if (ifindex)
				{
					if (!if_indextoname(ifindex, ifreq->ifr_name))
						return set_errno(ENXIO);
				}

				if (ioctl(sockfd, SIOCGIFADDR, ifreq) == -1)
					return -1;

				memcpy(&inaddr, &((sockaddr_in_t *)&ifreq->ifr_addr)->sin_addr, sizeof inaddr);
			}

			return setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_IF, &inaddr, sizeof inaddr);
		}

#ifdef AF_INET6
		case AF_INET6:
		{
			unsigned int index;

			if (ifname && (index = if_nametoindex(ifname)) == 0)
				return set_errno(ENXIO);

			index = ifindex;

			return setsockopt(sockfd, IPPROTO_IPV6, IPV6_MULTICAST_IF, &index, sizeof index);
		}
#endif

		default:
			return set_errno(EPROTONOSUPPORT);
	}
}

/*

=item C<int net_multicast_get_interface(int sockfd)>

Returns the index of the interface that C<sockfd> sends multicast packets
on. On error, returns C<-1> with C<errno> set appropriately.

=cut

*/

int net_multicast_get_interface(int sockfd)
{
	sockaddr_any_t any;
	size_t size = sizeof any;

	if (getsockname(sockfd, (void *)&any, (void *)&size) == -1)
		return -1;

	switch (any.any.sa_family)
	{
		case AF_INET:
		{
			struct in_addr inaddr;
			size_t size = sizeof inaddr;
			unsigned int index = 0;
			List *ifaces;

			if (getsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_IF, &inaddr, (void *)&size) == -1)
				return -1;

			if (!(ifaces = net_interfaces_by_family(AF_INET)))
				return -1;

			while (list_has_next(ifaces))
			{
				net_interface_t *iface = list_next(ifaces);

				if (iface->flags & IFF_UP)
				{
					if (iface->addr->any.sa_family == AF_INET && !memcmp(&inaddr, &iface->addr->in.sin_addr, sizeof inaddr))
					{
						index = iface->index;
						list_break(ifaces);
						break;
					}
				}
			}

			list_release(ifaces);

			return (int)index;
		}

#ifdef AF_INET6
		case AF_INET6:
		{
			unsigned int index;
			size_t size = sizeof index;

			if (getsockopt(sockfd, IPPROTO_IPV6, IPV6_MULTICAST_IF, &index, (void *)&size) == -1)
				return -1;

			return index;
		}
#endif

		default:
			return set_errno(EPROTONOSUPPORT);
	}
}

/*

=item C<int net_multicast_set_loopback(int sockfd, unsigned int loopback)>

If C<loopback> is zero, multicast loopback is disabled for packets sent on
C<sockfd>. This prevents any process on the sending host from receiving the
multicast packets sent via this socket. If C<loopback> is zero, multicast
loopback is enabled for packets sent on C<sockfd> (this is the default
behaviour). On success, returns C<0>. On error, returns C<-1> with C<errno>
set appropriately.

=cut

*/

int net_multicast_set_loopback(int sockfd, unsigned int loopback)
{
	sockaddr_any_t any;
	size_t size = sizeof any;

	if (getsockname(sockfd, (void *)&any, (void *)&size) == -1)
		return -1;

	switch (any.any.sa_family)
	{
		case AF_INET:
		{
			unsigned char flag = (unsigned char)loopback;

			return setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_LOOP, &flag, sizeof flag);
		}

#ifdef AF_INET6
		case AF_INET6:
		{
			unsigned int flag = loopback;

			return setsockopt(sockfd, IPPROTO_IPV6, IPV6_MULTICAST_LOOP, &flag, sizeof flag);
		}
#endif

		default:
			return set_errno(EPROTONOSUPPORT);
	}
}

/*

=item C<int net_multicast_get_loopback(int sockfd)>

Returns whether or not multicast packets sent on C<sockfd> can be received
by any process on the sending host. A non-zero return value means yes. A
zero return value means no. On error, returns C<-1> with C<errno> set
appropriately.

=cut

*/

int net_multicast_get_loopback(int sockfd)
{
	sockaddr_any_t any;
	size_t size = sizeof any;

	if (getsockname(sockfd, (void *)&any, (void *)&size) == -1)
		return -1;

	switch (any.any.sa_family)
	{
		case AF_INET:
		{
			unsigned char flag;
			size_t size = sizeof flag;

			if (getsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_LOOP, &flag, (void *)&size) == -1)
				return -1;

			return (int)flag;
		}

#ifdef AF_INET6
		case AF_INET6:
		{
			unsigned int flag;
			size_t size = sizeof flag;

			if (getsockopt(sockfd, IPPROTO_IPV6, IPV6_MULTICAST_LOOP, &flag, (void *)&size) == -1)
				return -1;

			return (int)flag;
		}
#endif

		default:
			return set_errno(EPROTONOSUPPORT);
	}
}

/*

=item C<int net_multicast_set_ttl(int sockfd, int ttl)>

Sets the TTL for multicast packets sent on C<sockfd> to C<ttl>. The default
TTL for multicast packets is C<1>. See the Multicast-HOWTO for details on
the scoping semantics of the TTL field in multicast packets. On success,
returns C<0>. On error, returns C<-1> with C<errno> set appropriately.

=cut

*/

int net_multicast_set_ttl(int sockfd, int ttl)
{
	sockaddr_any_t any;
	size_t size = sizeof any;

	if (getsockname(sockfd, (void *)&any, (void *)&size) == -1)
		return -1;

	switch (any.any.sa_family)
	{
		case AF_INET:
		{
			unsigned char hops = (unsigned char)ttl;

			return setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_TTL, &hops, sizeof hops);
		}

#ifdef AF_INET6
		case AF_INET6:
		{
			int hops = ttl;

			return setsockopt(sockfd, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &hops, sizeof hops);
		}
#endif

		default:
			return set_errno(EPROTONOSUPPORT);
	}
}

/*

=item C<int net_multicast_get_ttl(int sockfd)>

Returns the TTL of multicast packets sent on C<sockfd>. On error, returns
C<-1> with C<errno> set appropriately.

=cut

*/

int net_multicast_get_ttl(int sockfd)
{
	sockaddr_any_t any;
	size_t size = sizeof any;

	if (getsockname(sockfd, (void *)&any, (void *)&size) == -1)
		return -1;

	switch (any.any.sa_family)
	{
		case AF_INET:
		{
			unsigned char hops;
			size_t size = sizeof hops;

			if (getsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_TTL, &hops, (void *)&size) == -1)
				return -1;

			return (int)hops;
		}

#ifdef AF_INET6
		case AF_INET6:
		{
			int hops;
			size_t size = sizeof hops;

			if (getsockopt(sockfd, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &hops, (void *)&size) == -1)
				return -1;

			return hops;
		}
#endif

		default:
			return set_errno(EPROTONOSUPPORT);
	}
}

/*

=item C<int net_tos_lowdelay(int sockfd)>

Sets the TOS bits of packets sent on C<sockfd> to request minimum delay.
This is for interactive applications. This results in many small packets.
Use this sparingly. On success, returns C<0>. On error, returns C<-1> with
C<errno> set appropriately.

=cut

*/

int net_tos_lowdelay(int sockfd)
{
	int tos = IPTOS_LOWDELAY;

	return setsockopt(sockfd, IPPROTO_IP, IP_TOS, &tos, sizeof tos);
}

/*

=item C<int net_tos_throughput(int sockfd)>

Sets the TOS bits of packets sent on C<sockfd> to request maximum
throughput. This is for bulk data transfers. Don't forget to also specify
buffer sizes that are large enough to maximise throughput. However, be
warned that this might not be wise on asymmetric links, because large
buffers can lead to bufferbloat. On success, returns C<0>. On error, returns
C<-1> with C<errno> set appropriately.

=cut

*/

int net_tos_throughput(int sockfd)
{
	int tos = IPTOS_THROUGHPUT;

	return setsockopt(sockfd, IPPROTO_IP, IP_TOS, &tos, sizeof tos);
}

/*

=item C<int net_tos_reliability(int sockfd)>

Sets the TOS bits of packets sent on C<sockfd> to request maximum
reliability. This should only be used for datagram-based internet
management. On success, returns C<0>. On error, returns C<-1> with C<errno>
set appropriately.

=cut

*/

int net_tos_reliability(int sockfd)
{
	int tos = IPTOS_RELIABILITY;

	return setsockopt(sockfd, IPPROTO_IP, IP_TOS, &tos, sizeof tos);
}

/*

=item C<int net_tos_lowcost(int sockfd)>

Sets the TOS bits of packets sent on C<sockfd> to request minimum monetary
cost. Probably a good default. On success, returns C<0>. On error, returns
C<-1> with C<errno> set appropriately.

=cut

*/

#ifndef IPTOS_LOWCOST
#define IPTOS_LOWCOST 0x02
#endif

int net_tos_lowcost(int sockfd)
{
	int tos = IPTOS_LOWCOST;

	return setsockopt(sockfd, IPPROTO_IP, IP_TOS, &tos, sizeof tos);
}

/*

=item C<int net_tos_normal(int sockfd)>

Clears the TOS bits of packets sent on C<sockfd> (the default). On success,
returns C<0>. On error, returns C<-1> with C<errno> set appropriately.

=cut

*/

int net_tos_normal(int sockfd)
{
	int tos = 0x00;

	return setsockopt(sockfd, IPPROTO_IP, IP_TOS, &tos, sizeof tos);
}

/*

=item C<struct hostent *net_gethostbyname(const char *name, struct hostent *hostbuf, void **buf, size_t *size, int *herrno)>

A portable, reentrant I<gethostbyname(3)> that handles its own memory
allocation requirements. Looks up I<name>. On success, returns C<hostbuf>
with any extra data in C<*buf>. C<*size> is the length of C<*buf> on entry
and is updated to reflect the length on exit if a larger buffer was required
to perform the lookup. On error, returns C<null> with C<*herrno> set
appropriately if there was a lookup failure or with C<errno> set
appropriately if there was a memory allocation failure. It is the caller's
responsibility to deallocate C<*buf> using I<free(3)> when the lookup
failed, or when the results of the name lookup are no longer required.

Note: If your system has any version of I<gethostbyname_r(3)>, it will be
used. Otherwise, I<gethostbyname(3)> will be used. Even this might be
threadsafe if your system uses thread specific data to make it so.

    struct hostent hostbuf[1], *hostent;
    void *buf = NULL;
    size_t size = 0;
    int herrno;

    if ((hostent = net_gethostbyname("hostname", hostbuf, &buf, &size, &herrno)))
    {
        // use hostent ...
    }

    free(buf);

=cut

*/

#define xor(a, b) (!(a) ^ !(b))

struct hostent *net_gethostbyname(const char *name, struct hostent *hostbuf, void **buf, size_t *size, int *herrno)
{
	if (!name || !hostbuf || !buf || !size || xor(*buf, *size) || !herrno)
		return set_errnull(EINVAL);

#if HAVE_FUNC_GETHOSTBYNAME_R_6

	{
		struct hostent *ret;
		int err;

		if (*size == 0 && !(*buf = malloc(*size = 1024)))
			return NULL;

		while ((err = gethostbyname_r(name, hostbuf, *buf, *size, &ret, herrno)) && errno == ERANGE)
			if (!mem_resize((char **)buf, *size <<= 1))
				return NULL;

		return (err) ? NULL : ret;
	}

#elif HAVE_FUNC_GETHOSTBYNAME_R_5

	{
		struct hostent *ret;

		if (*size == 0 && !(*buf = malloc(*size = 1024)))
			return NULL;

		while (!(ret = gethostbyname_r(name, hostbuf, *buf, *size, herrno)) && errno == ERANGE)
			if (!mem_resize((char **)buf, *size <<= 1))
				return NULL;

		return ret;
	}

#elif HAVE_FUNC_GETHOSTBYNAME_R_3

	{
		if (*size == 0)
		{
			if (!(*buf = calloc(1, *size = sizeof(struct hostent_data))))
				return NULL;
		}
		else if (*size < sizeof(struct hostent_data))
		{
			size_t oldsize = *size;

			if (!mem_resize((char **)buf, *size = sizeof(struct hostent_data)))
				return NULL;

			memset((char *)*buf + oldsize, 0, *size - oldsize);
		}

		if (gethostbyname_r(name, hostbuf, (struct hostent_data *)*buf) == -1)
		{
			*herrno = h_errno;
			return NULL;
		}

		return hostbuf;
	}

#else

	/*
	** Some systems use thread specific data. Even if this isn't one of
	** them, we have to return something, even if it isn't threadsafe.
	** If we're here and it's not threadsafe, this system probably doesn't
	** support threads anyway.
	*/

	{
		struct hostent *ret;

		if (!(ret = gethostbyname(name)))
			*herrno = h_errno;

		return ret;
	}

#endif
}

/*

=item C<struct servent *net_getservbyname(const char *name, const char *proto, struct servent *servbuf, void **buf, size_t *size)>

A portable, reentrant I<getservbyname(3)> that handles its own memory
allocation requirements. Looks up the service C<name> and C<proto>. On
success, returns C<servbuf> with any extra data in C<*buf>. C<*size> is the
length of C<*buf> on entry and is updated to reflect the length on exit if a
larger buffer was required to perform the lookup. On error, returns C<null>
with C<errno> set appropriately. It is the caller's responsibility to
deallocate C<*buf> using I<free(3)> when the lookup failed, or when the
results of the name lookup are no longer required.

Note: If your system has any version of I<getservbyname_r(3)>, it will be
used. Otherwise, I<getservbyname(3)> will be used. Even this might be
threadsafe if your system uses thread specific data to make it so.

    struct servent servbuf[1], *servent;
    void *buf = NULL;
    size_t size = 0;

    if ((servent = net_getservbyname("service", "proto", servbuf, &buf, &size)))
    {
        // use servent ...
    }

    free(buf);

=cut

*/

struct servent *net_getservbyname(const char *name, const char *proto, struct servent *servbuf, void **buf, size_t *size)
{
	if (!name || !servbuf || !buf || !size || xor(*buf, *size))
		return set_errnull(EINVAL);

#if HAVE_FUNC_GETSERVBYNAME_R_6

	{
		struct servent *ret;
		int err;

		if (*size == 0 && !(*buf = malloc(*size = 128)))
			return NULL;

		while ((err = getservbyname_r(name, proto, servbuf, *buf, *size, &ret)) && errno == ERANGE)
			if (!mem_resize((char **)buf, *size <<= 1))
				return NULL;

		return (err) ? NULL : ret;
	}

#elif HAVE_FUNC_GETSERVBYNAME_R_5

	{
		struct servent *ret;

		if (*size == 0 && !(*buf = malloc(*size = 128)))
			return NULL;

		while (!(ret = getservbyname_r(name, proto, servbuf, *buf, *size)) && errno == ERANGE)
			if (!mem_resize((char **)buf, *size <<= 1))
				return NULL;

		return ret;
	}

#elif HAVE_FUNC_GETSERVBYNAME_R_4

	{
		if (*size == 0)
		{
			if (!(*buf = calloc(1, *size = sizeof(struct servent_data))))
				return NULL;
		}
		else if (*size < sizeof(struct servent_data))
		{
			size_t oldsize = *size;

			if (!mem_resize((char **)buf, *size = sizeof(struct servent_data)))
				return NULL;

			memset((char *)*buf + oldsize, 0, *size - oldsize);
		}

		if (getservbyname_r(name, proto, servbuf, (struct servent_data *)*buf) == -1)
			return NULL;

		return servbuf;
	}

#else

	/*
	** Some systems use thread specific data. Even if this isn't one of
	** them, we have to return something, even if it isn't threadsafe.
	** If we're here and it's not threadsafe, this system probably doesn't
	** support threads anyway. Of course, that's no consolation if some
	** function further up the stack is in the middle of a getservent()
	** loop.
	*/

	return getservbyname(name, proto);

#endif
}

/*

=item C<int net_options(int sockfd, sockopt_t *sockopts)>

Sets an arbitrary number of socket options for the socket C<sockfd>. The
options to set are specified by C<sockopts> which is an array of
C<sockopt_t> structures. Each I<sockopt_t> structure contains the C<level>,
C<optname>, C<optval> and C<optlen> parameters to be passed to
I<setsockopt(2)>. The array must end with a structure whose C<optval>
element is C<null>. On success, returns C<0>. On error, returns C<-1> with
C<errno> set appropriately. If I<setsockopt(2)> returns an error,
I<net_options(3)> will continue to set any further options but will
ultimately return an error itself.

=cut

*/

int net_options(int sockfd, sockopt_t *sockopts)
{
	sockopt_t *so;
	int err = 0;

	if (sockfd == -1)
		return set_errno(EBADF);

	if (!sockopts)
		return set_errno(EINVAL);

	for (so = sockopts; so->optval; ++so)
		if (setsockopt(sockfd, so->level, so->optname, so->optval, so->optlen) == -1)
			err = -1;

	return err;
}

static void iface_release(net_interface_t *iface)
{
	if (iface)
	{
		free(iface->addr);
		free(iface->brdaddr);
		free(iface->dstaddr);
		free(iface->hwaddr);
		free(iface);
	}
}

/*

=item C<List *net_interfaces(void)>

Returns the list of network interfaces. For each interface, calls
I<ioctl(2)> to obtain the interface's flags, hardware address, network
address, broadcast address if applicable, destination address if applicable,
MTU and index. On success, returns a list of I<net_interface_t> objects. It
is the caller's responsibility to deallocate the list with
I<list_release(3)>. On error, returns C<null> with C<errno> set
appropriately. Note that on I<Solaris>, neither the hardware address nor the
index can be returned. This function guesses the index in this case which
seems to work. If the C<RES_OPTIONS> environment variable contains the
string C<"inet6">, then only IPv6 interfaces are returned. Otherwise, only
IPv4 interfaces are returned.

=cut

*/

List *net_interfaces(void)
{
	return net_interfaces_with_locker(NULL);
}

/*

=item C<List *net_interfaces_with_locker(Locker *locker)>

Equivalent to I<net_interfaces(3)> except that multiple threads accessing
the returned list will be synchronised by C<locker>.

=cut

*/

List *net_interfaces_with_locker(Locker *locker)
{
	int family;

#ifdef AF_INET6
	if (inet6_required())
		family = AF_INET6;
	else
#endif
		family = AF_INET;

	return net_interfaces_by_family_with_locker(family, locker);
}

/*

=item C<List *net_interfaces_by_family(int family)>

Equivalent to I<net_interfaces(3)> except that C<family> specifies the
required address family.

=cut

*/

List *net_interfaces_by_family(int family)
{
	return net_interfaces_by_family_with_locker(family, NULL);
}

/*

=item C<List *net_interfaces_by_family_with_locker(int family, Locker *locker)>

Equivalent to I<net_interfaces_with_locker(3)> except that C<family>
specifies the required address family.

=cut

*/

List *net_interfaces_by_family_with_locker(int family, Locker *locker)
{
	List *ret;
	int sockfd;
	size_t size, lastsize = 0;
	char *buf = NULL, *ptr;
	struct ifconf ifc[1];
	int index = 0;

	if (!(ret = list_create_with_locker(locker, (list_release_t *)iface_release)))
		return NULL;

	if ((sockfd = socket(family, SOCK_DGRAM, 0)) == -1)
	{
		list_release(ret);
		return NULL;
	}

	/* Obtain the list of network interfaces */

	for (size = 100 * sizeof(struct ifreq); ; size += 10 * sizeof(struct ifreq))
	{
		if (!mem_resize(&buf, size))
		{
			list_release(ret);
			mem_release(buf);
			close(sockfd);
			return NULL;
		}

		ifc->ifc_len = size;
		ifc->ifc_buf = buf;

		if (ioctl(sockfd, SIOCGIFCONF, ifc) == -1)
		{
			if (errno != EINVAL || lastsize != 0)
			{
				list_release(ret);
				mem_release(buf);
				close(sockfd);
				return NULL;
			}
		}
		else
		{
			if (ifc->ifc_len == lastsize)
				break;

			lastsize = ifc->ifc_len;
		}
	}

	/* Obtain details of each network interface */

	for (ptr = buf; ptr < buf + lastsize; )
	{
		struct ifreq *ifr = (struct ifreq *)ptr;
		struct ifreq ifrcopy[1];
		net_interface_t *iface;

		if (!(iface = calloc(1, sizeof(net_interface_t))))
		{
			list_release(ret);
			mem_release(buf);
			close(sockfd);
			return NULL;
		}

		if (!list_append(ret, iface))
		{
			list_release(ret);
			mem_release(buf);
			mem_release(iface);
			close(sockfd);
			return NULL;
		}

#ifdef HAVE_SOCKADDR_SA_LEN
		size = ifr->ifr_addr.sa_len;
#else
		switch (ifr->ifr_addr.sa_family)
		{
#ifdef AF_INET6
			case AF_INET6:
				size = sizeof(sockaddr_in6_t);
				break;
#endif
			case AF_INET:
			default:
				size = sizeof(sockaddr_t);
				break;
		}
#endif

		ptr += sizeof ifr->ifr_name + size;
		*ifrcopy = *ifr;

		/* Get the interface's flags */

		if (ioctl(sockfd, SIOCGIFFLAGS, ifrcopy) == -1)
		{
			list_release(ret);
			mem_release(buf);
			close(sockfd);
			return NULL;
		}

		iface->flags = ifrcopy->ifr_flags;
		strlcpy(iface->name, ifr->ifr_name, IFNAMSIZ);

		/* Get the interface's address */

		if (iface->flags & IFF_UP)
		{
			if (!(iface->addr = calloc(1, size)))
			{
				list_release(ret);
				mem_release(buf);
				close(sockfd);
				return NULL;
			}

			memcpy(iface->addr, &ifr->ifr_addr, size);

			/* Get the interface's hardware address */

#ifdef SIOCGIFHWADDR
			if (ioctl(sockfd, SIOCGIFHWADDR, ifrcopy) == 0)
			{
				if (!(iface->hwaddr = mem_new(sockaddr_t)))
				{
					list_release(ret);
					mem_release(buf);
					close(sockfd);
					return NULL;
				}

				memcpy(iface->hwaddr, &ifrcopy->ifr_hwaddr, sizeof(sockaddr_t));
			}
#endif

			/* Get the interface's broadcast address */

#ifdef SIOCGIFBRDADDR
			if (iface->flags & IFF_BROADCAST)
			{
				if (ioctl(sockfd, SIOCGIFBRDADDR, ifrcopy) == -1)
				{
					list_release(ret);
					mem_release(buf);
					close(sockfd);
					return NULL;
				}

				if (!(iface->brdaddr = calloc(1, size)))
				{
					list_release(ret);
					mem_release(buf);
					close(sockfd);
					return NULL;
				}

				memcpy(iface->brdaddr, &ifrcopy->ifr_broadaddr, size);
			}
#endif

			/* Get the interface's destination address (for Point-To-Point) */

#ifdef SIOCGIFDSTADDR
			if (iface->flags & IFF_POINTOPOINT)
			{
				if (ioctl(sockfd, SIOCGIFDSTADDR, ifrcopy) == -1)
				{
					list_release(ret);
					mem_release(buf);
					close(sockfd);
					return NULL;
				}

				if (!(iface->dstaddr = calloc(1, size)))
				{
					list_release(ret);
					mem_release(buf);
					close(sockfd);
					return NULL;
				}

				memcpy(iface->dstaddr, &ifrcopy->ifr_dstaddr, size);
			}
#endif
		}

		/* Get the interface's Maximum Transmission Unit */

#ifdef SIOCGIFMTU
		if (ioctl(sockfd, SIOCGIFMTU, ifrcopy) != -1)
			iface->mtu = ifrcopy->ifr_mtu;
		else
#endif
			iface->mtu = -1;

		/* Get the interface's index */

#ifdef SIOCGIFINDEX
		if (ioctl(sockfd, SIOCGIFINDEX, ifrcopy) != -1)
			iface->index = ifrcopy->ifr_ifindex;
		else
#endif
			iface->index = ++index; /* Must fake it under Solaris */
	}

	mem_release(buf);
	close(sockfd);

	return ret;
}

/*

=item C<rudp_t *rudp_create(void)>

Allocates and initialises a retransmission timeout (RTO) estimator for
providing reliability over UDP. It is the caller's responsibility to
deallocate the estimator using I<rudp_release(3)> or I<rudp_destroy(3)>. It
is strongly recommended to use I<rudp_destroy(3)>, because it also sets the
pointer variable to C<null>. Note that each retransmission timer may only be
used for a single destination address. If a UDP socket communicates with
multiple peers, a separate estimator must be used for each peer. On success,
returns the RTO estimator. On error, returns C<null> with C<errno> set
appropriately. See the I<EXAMPLES> section.

=cut

*/

#define	RUDP_RTO_CALC(rudp) ((rudp)->srtt + (4.0 * (rudp)->rttvar))

static double rudp_minmax(double rto)
{
	if (rto < RUDP_RXTMIN)
		return RUDP_RXTMIN;

	if (rto > RUDP_RXTMAX)
		return RUDP_RXTMAX;

	return rto;
}

static int rudp_init(rudp_t *rudp)
{
	/*
	** Initialises the RTO estimator, C<rudp>. Must be called when reliable
	** UDP transactions time out. On success, returns C<0>. On error,
	** returns C<-1> with C<errno> set appropriately. See the EXAMPLES
	** section.
	*/

	if (!rudp)
		return set_errno(EINVAL);

	rudp->rtt = 0.0;
	rudp->srtt = 0.0;
	rudp->rttvar = 0.75;
	rudp->rto = rudp_minmax(RUDP_RTO_CALC(rudp));
	rudp->sequence = 0;

	return 0;
}

rudp_t *rudp_create(void)
{
	rudp_t *rudp;
	struct timeval now[1];

	if (gettimeofday(now, NULL) == -1)
		return NULL;

	if (!(rudp = mem_new(rudp_t)))
		return NULL;

	rudp->base = now->tv_sec;
	rudp_init(rudp);

	return rudp;
}

/*

=item C<void rudp_release(rudp_t *rudp)>

Releases (deallocates) the RTO estimator, C<rudp>. See the I<EXAMPLES>
section.

=cut

*/

void rudp_release(rudp_t *rudp)
{
	free(rudp);
}

/*

=item C<void *rudp_destroy(rudp_t **rudp)>

Destroys (deallocates and sets to C<null>) the RTO estimator, C<*rudp>.
Returns C<null>.

=cut

*/

void *rudp_destroy(rudp_t **rudp)
{
	if (rudp && *rudp)
	{
		rudp_release(*rudp);
		*rudp = NULL;
	}

	return NULL;
}

/*

C<uint32_t rudp_timestamp(rudp_t *rudp)>

Returns the number of milliseconds since C<rudp> was created in a 32-bit
integer. This number needs to be stored in reliable UDP packet headers so
that the round trip time can be calculated. On error, returns
C<(uint32_t)-1> with C<errno> set appropriately. See the I<EXAMPLES>
section.

*/

static uint32_t rudp_timestamp(rudp_t *rudp)
{
	struct timeval now[1];

	if (!rudp)
		return (uint32_t)set_errno(EINVAL);

	if (gettimeofday(now, NULL) == -1)
		return (uint32_t)-1;

	return (uint32_t)((now->tv_sec - rudp->base) * 1000) + (now->tv_usec / 1000);
}

/*

C<uint32_t rudp_newpack(rudp_t *rudp)>

Prepares the RTO estimator, C<rudp>, for a new packet that is about to be
sent, and returns a 32-bit sequence number for this new packet. This number
needs to be stored in reliable UDP packet headers so that the round trip
time can be calculated. On error, returns C<(uint32_t)-1> with C<errno> set
appropriately. See the I<EXAMPLES> section.

*/

static uint32_t rudp_newpack(rudp_t *rudp)
{
	if (!rudp)
		return (uint32_t)set_errno(EINVAL);

	rudp->nrexmt = 0;
	++rudp->sequence;

	return rudp->sequence;
}

/*

C<double rudp_start(rudp_t *rudp)>

Returns C<rudp>'s current retransmission timeout in seconds. On error,
returns C<-1.0> with C<errno> set appropriately. See the I<EXAMPLES>
section.

*/

static double rudp_start(rudp_t *rudp)
{
	if (!rudp)
		return (double)set_errno(EINVAL);

	return rudp->rto;
}

/*

C<int rudp_stop(rudp_t *rudp, uint32_t rtt)>

Updates the RTO estimator C<rudp>. C<rtt> is the round trip time in
milliseconds. Call this after successfully receiving a response to a
reliable UDP packet. On success, returns C<0>. On error, returns C<-1> with
C<errno> set appropriately. See the I<EXAMPLES> section.

*/

static int rudp_stop(rudp_t *rudp, uint32_t rtt)
{
	double delta;

	if (!rudp)
		return set_errno(EINVAL);

	rudp->rtt = rtt / 1000.0;
	delta = rudp->rtt - rudp->srtt;
	rudp->srtt += delta / 8;
	rudp->rttvar += (fabs(delta) - rudp->rttvar) / 4;
	rudp->rto = rudp_minmax(RUDP_RTO_CALC(rudp));

	return 0;
}

/*

C<int rudp_timeout(rudp_t *rudp)>

Informs C<rudp> that its retransmission timer has expired. This causes
C<rudp>'s RTO to double until the retransmission limit (C<3>) is reached, at
which point it returns C<-1> with C<errno> set to C<ETIMEDOUT>. On success,
returns C<0>. On error, returns C<-1> with C<errno> set appropriately. See
the I<EXAMPLES> section.

*/

static int rudp_timeout(rudp_t *rudp)
{
	if (!rudp)
		return set_errno(EINVAL);

	rudp->rto *= 2;

	if (++rudp->nrexmt > RUDP_MAXNREXMT)
		return set_errno(ETIMEDOUT);

	return 0;
}

/*

=item C<ssize_t net_rudp_transact(int sockfd, rudp_t *rudp, const void *obuf, size_t osize, void *ibuf, size_t isize)>

Provides reliable (not infallible) UDP transactions over C<sockfd>, a socket
created with I<net_udp_client(3)> or I<net_create_client>. Sends C<osize>
bytes, starting at C<obuf>, to the address to which C<sockfd> is connected.
C<rudp> is the retransmission timeout estimator as created by
I<rudp_create(3)>. The message is prepended by an C<8> byte header that
contains a timestamp and a sequence number. This is required to enable
calculation of the RTT. The peer must expect this header and include it
verbatim in its response. Note that the same retransmission timeout
estimator (C<rudp>) should be used for all transactions. Waits for a
response. If the retransmission timer expires before a response is received,
the retransmission timer is updated, and the packet is retransmitted. This
continues until either a response is received, or the packet has been
retransmitted three times with no response. If there is a response, at most
I<isize> bytes are received in C<ibuf>. On success, returns the number of
bytes received. On error, returns C<-1> with C<errno> set appropriately.

=cut

*/

ssize_t net_rudp_transact(int sockfd, rudp_t *rudp, const void *obuf, size_t osize, void *ibuf, size_t isize)
{
	return net_rudp_transactwith(sockfd, rudp, obuf, osize, 0, ibuf, isize, 0, NULL, 0);
}

/*

=item C<ssize_t net_rudp_transactwith(int sockfd, rudp_t *rudp, const void *obuf, size_t osize, int oflags, void *ibuf, size_t isize, int iflags, sockaddr_any_t *addr, size_t addrsize)>

Equivalent to I<net_rudp_transact(3)> except that C<sockfd> is a socket
created with I<net_udp_server(3)> or I<net_create_server(3)>. C<addr> is the
address of the peer. C<addrsize> is the size of C<addr>. I<sendmsg(2)> and
I<recvmsg(2)> are used instead of using I<writev(2)> and I<readv(2)>.
C<oflags> is passed to I<sendmsg(2)> as the C<flags> argument. C<iflags> is
passed to I<recvmsg(2)> as the C<flags> argument. Note that each
retransmission timer may only be used for a single destination address. If a
UDP socket communicates with multiple peers, a separate estimator must be
used for each peer. On success, returns the number of bytes received. On
error, returns C<-1> with C<errno> set appropriately. The I<EXAMPLES> section
below contains the code for this function.

=cut

*/

ssize_t net_rudp_transactwith(int sockfd, rudp_t *rudp, const void *obuf, size_t osize, int oflags, void *ibuf, size_t isize, int iflags, sockaddr_any_t *addr, size_t addrsize)
{
	struct { uint32_t sequence, timestamp; } ohdr[1], ihdr[1];
	struct msghdr omsg[1], imsg[1];
	struct iovec ovec[2], ivec[2];
	int sequence;
	uint32_t timestamp;
	double timeout;
	long timeout_sec;
	long timeout_usec;
	ssize_t bytes;

	if (sockfd < 0 || !rudp || !obuf || !osize || !ibuf || !isize)
		return set_errno(EINVAL);

	if (addr)
	{
		memset(omsg, 0, sizeof omsg);
		omsg->msg_name = (void *)addr;
		omsg->msg_namelen = addrsize;
		omsg->msg_iov = ovec;
		omsg->msg_iovlen = 2;

		memset(imsg, 0, sizeof imsg);
		imsg->msg_iov = ivec;
		imsg->msg_iovlen = 2;
	}

	ovec[0].iov_base = (void *)ohdr;
	ovec[0].iov_len = sizeof ohdr;
	ovec[1].iov_base = (void *)obuf;
	ovec[1].iov_len = osize;

	ivec[0].iov_base = (void *)ihdr;
	ivec[0].iov_len = sizeof ihdr;
	ivec[1].iov_base = ibuf;
	ivec[1].iov_len = isize;

	if ((sequence = rudp_newpack(rudp)) == -1)
		return -1;

	ohdr->sequence = sequence;

sendagain:

	if ((timestamp = rudp_timestamp(rudp)) == -1)
		return -1;

	ohdr->timestamp = timestamp;

	if (addr)
	{
		if (sendmsg(sockfd, omsg, oflags) == -1)
			return -1;
	}
	else
	{
		if (writev(sockfd, ovec, 2) == -1)
			return -1;
	}

	if ((timeout = rudp_start(rudp)) == -1)
		return -1;

	timeout_sec = (long)timeout;
	timeout_usec = (long)((timeout - timeout_sec) * 1000000);

recvagain:

	if (read_timeout(sockfd, timeout_sec, timeout_usec) == -1)
	{
		if (errno == ETIMEDOUT && rudp_timeout(rudp) != -1)
			goto sendagain;

		rudp_init(rudp);

		return -1;
	}

	if (addr)
	{
		if ((bytes = recvmsg(sockfd, imsg, iflags)) == -1)
			return -1;
	}
	else
	{
		if ((bytes = readv(sockfd, ivec, 2)) == -1)
			return -1;
	}

	if (bytes < sizeof ihdr || ihdr->sequence != ohdr->sequence)
		goto recvagain;

	if (rudp_stop(rudp, rudp_timestamp(rudp) - ihdr->timestamp) == -1)
		return -1;

	return bytes - sizeof ihdr;
}

/*

=item C<ssize_t net_pack(int sockfd, long timeout, int flags, const char *format, ...)>

Creates a packet containing data packed by I<pack(3)> as specified by
C<format>, and sends it on the connected socket, C<sockfd>, with I<send(2)>.
If C<timeout> is non-zero, it is the number of seconds to wait for the send
buffer to have enough space for the new data before timing out (This only
applies to TCP sockets since UDP has no send buffer). C<flags> is passed to
I<send(2)>. This is intended for use with UDP. It can work reliably with
TCP, but only when the application protocol involves each peer packing and
unpacking alternately, each waiting for the other's response before making
their next response. On success, returns the number of bytes packed and
sent. On error, returns C<-1> with C<errno> set appropriately.

Note, the I<net_pack(3)> functions can sometimes be inappropriate as they
inherently involve copying existing data into a new buffer before writing
it. It is much faster to not copy the data at all. When possible (i.e. when
the data is already in network byte order), use I<writev(2)> instead to
write multiple non-contiguous buffers in a single system call.

=cut

*/

ssize_t net_pack(int sockfd, long timeout, int flags, const char *format, ...)
{
	va_list args;
	int rc;

	va_start(args, format);
	rc = net_vpack(sockfd, timeout, flags, format, args);
	va_end(args);

	return rc;
}

/*

=item C<ssize_t net_vpack(int sockfd, long timeout, int flags, const char *format, va_list args)>

Equivalent to I<net_pack(3)> with the variable argument list specified
directly as for I<vprintf(3)>.

=cut

*/

ssize_t net_vpack(int sockfd, long timeout, int flags, const char *format, va_list args)
{
	char buf[MSG_SIZE];
	int rc;

	if ((rc = vpack(buf, MSG_SIZE, format, args)) == -1)
		return -1;

	if (timeout && write_timeout(sockfd, timeout, 0) == -1)
		return -1;

	return send(sockfd, buf, rc, flags);
}

/*

=item C<ssize_t net_packto(int sockfd, long timeout, int flags, const sockaddr_t *to, size_t tosize, const char *format, ...)>

Creates a packet containing data packed by I<pack(3)> as specified by
C<format>, and sends it on the unconnected socket, C<sockfd>, to the address
specified by C<to> with length C<tosize> with I<sendto(2)>. C<flags> is
passed to I<sendto(2)>. If C<timeout> is non-zero, it is the number of
seconds to wait for the send buffer to have enough space for the new data
before timing out. This only applies to TCP sockets since UDP has no send
buffer. On success, returns the number of bytes packed and sent. On error,
returns C<-1> with C<errno> set appropriately.

=cut

*/

ssize_t net_packto(int sockfd, long timeout, int flags, const sockaddr_t *to, size_t tosize, const char *format, ...)
{
	va_list args;
	int rc;

	va_start(args, format);
	rc = net_vpackto(sockfd, timeout, flags, to, tosize, format, args);
	va_end(args);

	return rc;
}

/*

=item C<ssize_t net_vpackto(int sockfd, long timeout, int flags, const sockaddr_t *to, size_t tosize, const char *format, va_list args)>

Equivalent to I<net_packto(3)> with the variable argument list specified
directly as for I<vprintf(3)>.

=cut

*/

ssize_t net_vpackto(int sockfd, long timeout, int flags, const sockaddr_t *to, size_t tosize, const char *format, va_list args)
{
	char buf[MSG_SIZE];
	int rc;

	if ((rc = vpack(buf, MSG_SIZE, format, args)) == -1)
		return -1;

	if (timeout && write_timeout(sockfd, timeout, 0) == -1)
		return -1;

	return sendto(sockfd, buf, rc, flags, to, tosize);
}

/*

=item C<ssize_t net_unpack(int sockfd, long timeout, int flags, const char *format, ...)>

Receives a packet of data on the connected socket, C<sockfd>, with
I<recv(2)>, and unpacks it with I<unpack(3)> as specified by C<format>.
C<flags> is passed to I<recv(2)>. C<timeout> is the number of seconds to
wait before timing out. On success, returns the number of bytes received and
unpacked. On error, returns C<-1> with C<errno> set appropriately.

Note, the I<net_unpack(3)> functions can sometimes be inappropriate as they
inherently involve reading data into a single buffer and then copying it
into multiple target buffers. It is much faster to not copy the data at all.
When possible (i.e. when the data is already in network byte order and host
byte order are the same), use I<readv(2)> instead to read into multiple
non-contiguous buffers in a single system call.

=cut

*/

ssize_t net_unpack(int sockfd, long timeout, int flags, const char *format, ...)
{
	va_list args;
	int rc;

	va_start(args, format);
	rc = net_vunpack(sockfd, timeout, flags, format, args);
	va_end(args);

	return rc;
}

/*

=item C<ssize_t net_vunpack(int sockfd, long timeout, int flags, const char *format, va_list args)>

Equivalent to I<net_unpack(3)> with the variable argument list specified
directly as for I<vprintf(3)>.

=cut

*/

ssize_t net_vunpack(int sockfd, long timeout, int flags, const char *format, va_list args)
{
	char buf[MSG_SIZE];
	int rc;

	if (read_timeout(sockfd, timeout, 0) == -1)
		return -1;

	if ((rc = recv(sockfd, buf, MSG_SIZE, flags)) == -1)
		return -1;

	return vunpack(buf, rc, format, args);
}

/*

=item C<ssize_t net_unpackfrom(int sockfd, long timeout, int flags, sockaddr_t *from, size_t *fromsize, const char *format, ...)>

Receives a packet of data on the unconnected socket, C<sockfd>, with
I<recvfrom(2)>, and unpacks it with I<unpack(3)> as specified by C<format>.
If C<from> is non-C<null>, the source address of the message is stored
there. C<fromsize> is a value-result parameter, initialized to the size of
the C<from> buffer, and modified on return to indicate the actual size of
the address stored there. C<flags> is passed to I<recvfrom(2)>. C<timeout>
is the number of seconds to wait before timing out. On success, returns the
number of bytes received and unpacked. On error, returns C<-1> with C<errno>
set appropriately.

=cut

*/

ssize_t net_unpackfrom(int sockfd, long timeout, int flags, sockaddr_t *from, size_t *fromsize, const char *format, ...)
{
	va_list args;
	int rc;

	va_start(args, format);
	rc = net_vunpackfrom(sockfd, timeout, flags, from, fromsize, format, args);
	va_end(args);

	return rc;
}

/*

=item C<ssize_t net_vunpackfrom(int sockfd, long timeout, int flags, sockaddr_t *from, size_t *fromsize, const char *format, va_list args)>

Equivalent to I<net_unpackfrom(3)> with the variable argument list specified
directly as for I<vprintf(3)>.

=cut

*/

ssize_t net_vunpackfrom(int sockfd, long timeout, int flags, sockaddr_t *from, size_t *fromsize, const char *format, va_list args)
{
	char buf[MSG_SIZE];
	int rc;

	if (read_timeout(sockfd, timeout, 0) == -1)
		return -1;

	if ((rc = recvfrom(sockfd, buf, MSG_SIZE, flags, from, (void *)fromsize)) == -1)
		return -1;

	return vunpack(buf, rc, format, args);
}

/*

=item C<ssize_t pack(void *buf, size_t size, const char *format, ...)>

Packs data into C<buf> as described by C<format>. The arguments after
C<format> contain the data to be packed. C<size> is the size of C<buf>.
Returns the number of bytes packed on success, or -1 on error with C<errno>
set appropriately.

Note, this is based on the I<pack(3)> function in I<perl(1)> (in fact, the
following documentation is from I<perlfunc(1)>) except that the C<*> count
specifier has different semantics, the C<?> count specifier is new, there's
no non C<nul>-terminated strings or machine dependent formats or uuencoding
or BER integer compression, everything is in network byte order, and floats
are represented as strings so I<pack(3)> is suitable for serialising data to
be written to disk or sent across a network to other hosts. OK, C<v> and
C<w> specifically aren't in network order, but sometimes that's needed too.

C<format> can contain the following type specifiers:

    a   A string with arbitrary binary data
    z   A nul-terminated string, will be nul-padded
    b   A bit-string (rounded out to the nearest byte boundary)
    h   A hexadecimal string (rounded out to the nearest byte boundary)
    c   A char (8 bits)
    s   A short (16 bits)
    i   An int (32 bits)
    l   A long (64 bits - only on some systems)
    f   A single-precision float (length byte + text + nul)
    d   A double-precision float (length byte + text + nul)
    v   A short in "VAX" (little-endian) order (16 bits)
    w   An int in "VAX" (little-endian) order (32 bits)
    p   A pointer (32 or 64 bits)
    x   A nul byte
    X   Back up a byte
    @   Null fill to absolute position

The following rules apply:

Each letter may optionally be followed by a number giving a repeat count or
length, or by C<"*"> or C<"?">. A C<"*"> will obtain the repeat count or
length from the next argument (like I<printf(3)>). The count argument must
appear before the first corresponding data argument. When unpacking C<"a">,
C<"z">, C<"b"> or C<"h">, a C<"?"> will obtain the repeat count or length
from the I<size_t> object pointed to by the next argument, and the size of
the target buffer in the argument after that. These two arguments must
appear before the first corresponding target buffer argument. This enables
unpacking packets that contain length fields without risking target buffer
overflow.

With all types except C<"a">, C<"z">, C<"b"> and C<"h"> the I<pack(3)>
function will gobble up that many arguments.

The C<"a"> and C<"z"> types gobble just one value, but pack it as a string
of length count (specified by the corresponding number), truncating or
padding with C<nul> bytes as necessary. It is the caller's responsibility to
ensure that the data arguments point to sufficient memory. When unpacking,
C<"z"> strips everything after the first C<nul>, and C<"a"> returns data
verbatim.

Likewise, the C<"b"> field packs a string that many bits long.

The C<"h"> field packs a string that many nybbles long.

The C<"p"> type packs a pointer. You are responsible for ensuring the memory
pointed to is not a temporary value (which can potentially get deallocated
before you get around to using the packed result). A C<null> pointer is
unpacked if the corresponding packed value for C<"p"> is C<null>. Of course,
C<"p"> is useless if the packed data is to be sent over a network to another
process.

The integer formats C<"c">, C<"s">, C<"i"> and C<"l"> are all in network
byte order, and so can safely be packed for sending over a network to
another process. However, C<"l"> relies on a non-I<ISO C 89> language
feature (namely, the I<long long int> type which is in I<ISO C 99>), and so
should not be used in portable code, even if it is supported on the local
system. There is no guarantee that a long long packed on one system will be
unpackable on another. At least not until C99 is more widespread. It should
be OK now.

Real numbers (floats and doubles) are packed in text format. Due to the
multiplicity of floating point formats around, this is done to safely
transport real numbers across a network to another process.

It is the caller's responsibility to ensure that there are sufficient
arguments provided to satisfy the requirements of C<format>.

=cut

*/

ssize_t pack(void *buf, size_t size, const char *format, ...)
{
	va_list args;
	int rc;

	va_start(args, format);
	rc = vpack(buf, size, format, args);
	va_end(args);

	return rc;
}

/*

=item C<ssize_t vpack(void *buf, size_t size, const char *format, va_list args)>

Equivalent to I<pack(3)> with the variable argument list specified directly
as for I<vprintf(3)>.

=cut

*/

#define GET_COUNT() \
	count = 1; \
	if (*format == '*') \
		++format, count = (size_t)va_arg(args, int); \
	else if (isdigit((int)(unsigned int)*format)) \
		for (count = 0; isdigit((int)(unsigned int)*format); ++format) \
			count *= 10, count += *format - '0'; \
	if ((ssize_t)count < 1) \
		return set_errno(EINVAL);

#define CHECK_SPACE(required) \
	if (p + (required) > pkt + size) \
		return set_errno(ENOSPC);

ssize_t vpack(void *buf, size_t size, const char *format, va_list args)
{
	size_t count;
	unsigned char *pkt = buf;
	unsigned char *p = pkt;
	char tmp[128];

	if (!pkt || !format)
		return set_errno(EINVAL);

	while (*format)
	{
		char f = *format++;
		if (f == 'p' && sizeof(void *) == 8)
			f = 'P';

		switch (f)
		{
			case 'a': /* A string with arbitrary binary data */
			{
				void *data;
				GET_COUNT()
				CHECK_SPACE(count)
				if (!(data = va_arg(args, void *)))
					return set_errno(EINVAL);
				memcpy(p, data, count);
				p += count;
				break;
			}

			case 'z': /* A nul-terminated string, will be nul padded */
			{
				char *data;
				size_t len;
				GET_COUNT()
				CHECK_SPACE(count)
				if (!(data = va_arg(args, char *)))
					return set_errno(EINVAL);
				len = strlen(data);
				if (len > count)
					len = count;
				memcpy(p, data, len);
				p += len;
				count -= len;
				if (count)
					memset(p, 0, count);
				p += count;
				break;
			}

			case 'b': /* A bit string (rounded out to nearest byte boundary) */
			{
				char *data;
				unsigned char byte;
				int shift;
				GET_COUNT()
				CHECK_SPACE((count + 7) >> 3)
				if (!(data = va_arg(args, char *)))
					return set_errno(EINVAL);
				byte = 0x00;
				shift = 7;
				while (count--)
				{
					switch (*data++)
					{
						case '0':
							break;
						case '1':
							byte |= 1 << shift;
							break;
						default:
							return set_errno(EINVAL);
					}
					if (--shift == -1)
					{
						*p++ = byte;
						byte = 0x00;
						shift = 7;
					}
				}
				if (shift != 7)
					*p++ = byte;
				break;
			}

			case 'h': /* A hex string (rounded out to nearest byte boundary) */
			{
				char *data;
				unsigned char byte;
				int shift;
				GET_COUNT()
				CHECK_SPACE((count + 1) >> 1)
				if (!(data = va_arg(args, char *)))
					return set_errno(EINVAL);
				byte = 0x00;
				shift = 4;
				while (count--)
				{
					unsigned char nybble = *data++;
					switch (nybble)
					{
						case '0': case '1': case '2': case '3': case '4':
						case '5': case '6': case '7': case '8': case '9':
							byte |= (nybble - '0') << shift;
							break;
						case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
							byte |= (nybble - 'a' + 10) << shift;
							break;
						case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
							byte |= (nybble - 'A' + 10) << shift;
							break;
						default:
							return set_errno(EINVAL);
					}
					if ((shift -= 4) == -4)
					{
						*p++ = byte;
						byte = 0x00;
						shift = 4;
					}
				}
				if (shift != 4)
					*p++ = byte;
				break;
			}

			case 'c': /* A char (8 bits) */
			{
				GET_COUNT()
				CHECK_SPACE(count)
				while (count--)
					*p++ = (unsigned char)va_arg(args, int);
				break;
			}

			case 's': /* A short (16 bits) */
			{
				GET_COUNT()
				CHECK_SPACE(count << 1)
				while (count--)
				{
					unsigned short data = (unsigned short)va_arg(args, int);
					*p++ = (data >> 8) & 0xff;
					*p++ = data & 0xff;
				}

				break;
			}

			case 'i': /* An int (32 bits) */
			case 'p': /* A pointer (32 bits) */
			{
				GET_COUNT()
				CHECK_SPACE(count << 2)
				while (count--)
				{
					unsigned long data = (unsigned long)va_arg(args, int);
					*p++ = (data >> 24) & 0xff;
					*p++ = (data >> 16) & 0xff;
					*p++ = (data >> 8) & 0xff;
					*p++ = data & 0xff;
				}

				break;
			}

#ifdef HAVE_LONG_LONG
			case 'l': /* A long (64 bits - only on some systems) */
			case 'P': /* A pointer (64 bits) */
			{
				GET_COUNT()
				CHECK_SPACE(count << 3)
				while (count--)
				{
					unsigned long long data = (unsigned long long)va_arg(args, long long);
					*p++ = (data >> 56) & 0xff;
					*p++ = (data >> 48) & 0xff;
					*p++ = (data >> 40) & 0xff;
					*p++ = (data >> 32) & 0xff;
					*p++ = (data >> 24) & 0xff;
					*p++ = (data >> 16) & 0xff;
					*p++ = (data >> 8) & 0xff;
					*p++ = data & 0xff;
				}

				break;
			}
#else
			case 'l': /* A long (64 bits - only on some systems) */
			case 'P': /* A pointer (64 bits) */
			{
				return set_errno(ENOSYS);
			}
#endif

			case 'f': /* A single-precision float (length byte + text + nul) */
			case 'd': /* A double-precision float (length byte + text + nul) */
			{
				GET_COUNT()
				while (count--)
				{
					double data = va_arg(args, double);
					int rc = snprintf(tmp, 128, "%g", data);
					size_t len;
					if (rc == -1 || rc >= 128)
						return set_errno(ENOSPC);
					len = strlen(tmp) + 1;
					CHECK_SPACE(len + 1)
					*p++ = len & 0xff;
					memcpy(p, tmp, len);
					p += len;
				}

				break;
			}

			case 'v': /* A short in "VAX" (little-endian) order (16 bits) */
			{
				GET_COUNT()
				CHECK_SPACE(count << 1)
				while (count--)
				{
					unsigned short data = (unsigned short)va_arg(args, int);
					*p++ = data & 0xff;
					*p++ = (data >> 8) & 0xff;
				}

				break;
			}

			case 'w': /* An int in "VAX" (little-endian) order (32 bits) */
			{
				GET_COUNT()
				CHECK_SPACE(count << 2)
				while (count--)
				{
					unsigned int data = (unsigned int)va_arg(args, int);
					*p++ = data & 0xff;
					*p++ = (data >> 8) & 0xff;
					*p++ = (data >> 16) & 0xff;
					*p++ = (data >> 24) & 0xff;
				}

				break;
			}

			case 'x': /* A nul byte */
			{
				GET_COUNT()
				CHECK_SPACE(count)
				memset(p, 0, count);
				p += count;
				break;
			}

			case 'X': /* Back up a byte */
			{
				GET_COUNT()
				if (p - count < pkt)
					return set_errno(EINVAL);
				p -= count;
				break;
			}

			case '@': /* Null fill to absolute position */
			{
				GET_COUNT()
				if (count > size)
					return set_errno(ENOSPC);
				if (pkt + count < p)
					return set_errno(EINVAL);
				memset(p, 0, count - (p - pkt));
				p += count - (p - pkt);
				break;
			}

			default:
			{
				return set_errno(EINVAL);
			}
		}
	}

	return p - pkt;
}

/*

=item C<ssize_t unpack(void *buf, size_t size, const char *format, ...)>

Unpacks the data in C<buf> which was packed by I<pack(3)>. C<size> is the
size of C<buf>. C<format> must be equivalent to the C<format> argument to
the call to I<pack(3)> that packed the data. The remaining arguments must be
pointers to variables that will hold the unpacked data or C<null>. If any
are C<null>, the corresponding data will be skipped (i.e. not unpacked).
Unpacked C<"z">, C<"b"> and C<"h"> strings are always C<nul>-terminated. It
is the caller's responsibility to ensure that the pointers into which these
strings are unpacked contain enough memory (count + 1 bytes). It is the
caller's responsibility to ensure that the non-C<null> pointers into which
C<"a"> strings are unpacked also contain enough memory (count bytes). It is
the caller's responsibility to ensure that there are sufficient arguments
supplied to satisfy the requirements of C<format>, even if they are just
C<null> pointers. Returns the number of bytes unpacked on success, or -1 on
error.

=cut

*/

ssize_t unpack(void *buf, size_t size, const char *format, ...)
{
	va_list args;
	int rc;

	va_start(args, format);
	rc = vunpack(buf, size, format, args);
	va_end(args);

	return rc;
}

/*

=item C<ssize_t vunpack(void *buf, size_t size, const char *format, va_list args)>

Equivalent to I<unpack(3)> with the variable argument list specified
directly as for I<vprintf(3)>.

=cut

*/

#define GET_COUNT_LIMIT() \
	limit = count = 1; \
	if (*format == '*') \
		++format, limit = count = (size_t)va_arg(args, int); \
	else if (*format == '?') \
	{ \
		size_t *countp = va_arg(args, size_t *); \
		if (!countp) \
			return set_errno(EINVAL); \
		count = *countp; \
		limit = va_arg(args, size_t); \
		++format; \
	} \
	else if (isdigit((int)(unsigned int)*format)) \
	{ \
		for (count = 0; isdigit((int)(unsigned int)*format); ++format) \
			count *= 10, count += *format - '0'; \
		limit = count; \
	} \
	if ((ssize_t)count < 1 || (ssize_t)limit < 1) \
		return set_errno(EINVAL); \
	if (count > limit) \
		return set_errno(ENOSPC);

#define CHECK_SKIP(count, action) \
	if (!data) \
	{ \
		p += (count); \
		action; \
	}

ssize_t vunpack(void *buf, size_t size, const char *format, va_list args)
{
	unsigned char *pkt = buf;
	unsigned char *p = pkt;
	size_t count, limit;

	if (!pkt || !format)
		return set_errno(EINVAL);

	while (*format)
	{
		char f = *format++;
		if (f == 'p' && sizeof(void *) == 8)
			f = 'P';

		switch (f)
		{
			case 'a': /* A string with arbitrary binary data */
			{
				void *data;
				GET_COUNT_LIMIT()
				CHECK_SPACE(count)
				data = va_arg(args, void *);
				CHECK_SKIP(count, break)
				memcpy(data, p, count);
				p += count;
				break;
			}

			case 'z': /* A nul-terminated string, will be nul padded */
			{
				char *data;
				size_t len;
				GET_COUNT_LIMIT()
				CHECK_SPACE(count)
				data = va_arg(args, char *);
				CHECK_SKIP(count, break)
				for (len = 0; p + len < pkt + size && p[len]; ++len)
					;
				if (len > count)
					len = count;
				memcpy(data, p, len);
				p += len;
				count -= len;
				memset(data + len, 0, count ? count : 1);
				p += count;
				break;
			}

			case 'b': /* A bit string (rounded out to nearest byte boundary) */
			{
				char bin[] = "01";
				char *data;
				int shift;
				GET_COUNT_LIMIT()
				CHECK_SPACE((count + 7) >> 3)
				data = va_arg(args, char *);
				CHECK_SKIP((count + 7) >> 3, break)
				shift = 7;
				while (count--)
				{
					*data++ = bin[(*p & (0x01 << shift)) >> shift];
					if (--shift == -1)
						++p, shift = 7;
				}
				if (shift != 7)
					++p;
				*data = '\0';
				break;
			}

			case 'h': /* A hex string (rounded out to nearest byte boundary) */
			{
				char hex[] = "0123456789abcdef";
				char *data;
				int shift;
				GET_COUNT_LIMIT()
				CHECK_SPACE((count + 1) >> 1)
				data = va_arg(args, char *);
				CHECK_SKIP((count + 1) >> 1, break)
				shift = 4;
				while (count--)
				{
					*data++ = hex[(*p & (0x0f << shift)) >> shift];
					if ((shift -= 4) == -4)
						++p, shift = 4;
				}
				if (shift != 4)
					++p;
				*data = '\0';
				break;
			}

			case 'c': /* A char (8 bits) */
			{
				GET_COUNT()
				CHECK_SPACE(count)
				while (count--)
				{
					signed char *data = va_arg(args, signed char *);
					CHECK_SKIP(1, continue)
					*data = (signed char)*p++;
				}
				break;
			}

			case 's': /* A short (16 bits) */
			{
				GET_COUNT()
				CHECK_SPACE(count << 1)
				while (count--)
				{
					signed short *data = va_arg(args, signed short *);
					CHECK_SKIP(2, continue)
					*data = (signed short)*p++ << 8;
					*data |= *p++;
				}
				break;
			}

			case 'i': /* An int (32 bits) */
			case 'p': /* A pointer (32 bits) */
			{
				GET_COUNT()
				CHECK_SPACE(count << 2)
				while (count--)
				{
					signed int *data = va_arg(args, signed int *);
					CHECK_SKIP(4, continue)
					*data = (signed int)*p++ << 24;
					*data |= (signed int)*p++ << 16;
					*data |= (signed int)*p++ << 8;
					*data |= (signed int)*p++;
				}

				break;
			}

			case 'v': /* A short in "VAX" (little-endian) order (16 bits) */
			{
				GET_COUNT()
				CHECK_SPACE(count << 1)
				while (count--)
				{
					signed short *data = va_arg(args, signed short *);
					CHECK_SKIP(2, continue)
					*data = *p++;
					*data |= (unsigned short)*p++ << 8;
				}
				break;
			}

			case 'w': /* An int in "VAX" (little-endian) order (32 bits) */
			{
				GET_COUNT()
				CHECK_SPACE(count << 2)
				while (count--)
				{
					signed int *data = va_arg(args, signed int *);
					CHECK_SKIP(4, continue)
					*data = (signed long)*p++;
					*data |= (signed long)*p++ << 8;
					*data |= (signed long)*p++ << 16;
					*data |= (signed long)*p++ << 24;
				}

				break;
			}

#ifdef HAVE_LONG_LONG
			case 'l': /* A long (64 bits - only on some systems) */
			case 'P': /* A pointer (64 bits) */
			{
				GET_COUNT()
				CHECK_SPACE(count << 3)
				while (count--)
				{
					signed long long *data = va_arg(args, signed long long *);
					CHECK_SKIP(8, continue)
					*data = (signed long long)*p++ << 56;
					*data |= (signed long long)*p++ << 48;
					*data |= (signed long long)*p++ << 40;
					*data |= (signed long long)*p++ << 32;
					*data |= (signed long long)*p++ << 24;
					*data |= (signed long long)*p++ << 16;
					*data |= (signed long long)*p++ << 8;
					*data |= (signed long long)*p++;
				}

				break;
			}
#else
			case 'l': /* A long (64 bits - only on some systems) */
			case 'P': /* A pointer (64 bits) */
			{
				return set_errno(ENOSYS);
			}
#endif

			case 'f': /* A single-precision float (length byte + text + nul) */
			{
				GET_COUNT()
				while (count--)
				{
					float *data = va_arg(args, float *);
					size_t len;
					CHECK_SPACE(1);
					len = (size_t)*p++;
					CHECK_SPACE(len)
					CHECK_SKIP(len, continue)
					sscanf((const char *)p, "%g", data);
					p += len;
				}

				break;
			}

			case 'd': /* A double-precision float (length byte + text + nul) */
			{
				GET_COUNT()
				while (count--)
				{
					double *data = va_arg(args, double *);
					size_t len;
					CHECK_SPACE(1);
					len = (size_t)*p++;
					CHECK_SPACE(len);
					CHECK_SKIP(len, continue)
					sscanf((const char *)p, "%lg", data);
					p += len;
				}

				break;
			}

			case 'x': /* A nul byte */
			{
				GET_COUNT()
				CHECK_SPACE(count)
				p += count;
				break;
			}

			case 'X': /* Back up a byte */
			{
				GET_COUNT()
				if (p - count < pkt)
					return set_errno(EINVAL);
				p -= count;
				break;
			}

			case '@': /* Null fill to absolute position */
			{
				GET_COUNT()
				if (count > size)
					return set_errno(ENOSPC);
				if (pkt + count < p)
					return set_errno(EINVAL);
				p += count - (p - pkt);
				break;
			}

			default:
			{
				return set_errno(EINVAL);
			}
		}
	}

	return p - pkt;
}

/*

=item C<ssize_t net_read(int sockfd, long timeout, char *buf, size_t count)>

Repeatedly calls I<read(2)> on the connection-oriented socket, C<sockfd>,
until C<count> bytes have been read into C<buf>, or until EOF is
encountered, or until it times out (after C<timeout> seconds). On success,
returns the number of bytes read. On error, returns C<-1> with C<errno> set
appropriately.

=cut

*/

ssize_t net_read(int sockfd, long timeout, char *buf, size_t count)
{
	char *b;
	ssize_t bytes;

	for (b = buf; count; count -= bytes, b += bytes)
	{
		if (read_timeout(sockfd, timeout, 0) == -1)
			return -1;

		if ((bytes = read(sockfd, b, count)) == -1)
			return -1;

		if (bytes == 0)
			break;
	}

	return b - buf;
}

/*

=item C<ssize_t net_write(int sockfd, long timeout, const char *buf, size_t count)>

Repeatedly calls I<write(2)> on the connection-oriented socket, C<sockfd>,
until C<count> bytes from C<buf> have been written, or until it times out
(after C<timeout> seconds). On success, returns the number of bytes written.
On error, returns C<-1>.

=cut

*/

ssize_t net_write(int sockfd, long timeout, const char *buf, size_t count)
{
	const char *b;
	ssize_t bytes;

	for (b = buf; count; count -= bytes, b += bytes)
	{
		if (write_timeout(sockfd, timeout, 0) == -1)
			return -1;

		if ((bytes = write(sockfd, b, count)) <= 0)
			return bytes;
	}

	return b - buf;
}

/*

=item C<ssize_t net_expect(int sockfd, long timeout, const char *format, ...)>

Expects and confirms a formatted text message from a remote connection on
the socket, C<sockfd>. C<timeout> is the number of seconds to wait before
timing out. If C<timeout> is C<0>, times out immediately. On success,
returns the number of conversions performed (see I<scanf(3)>). When the
connection closes, returns C<0>. On error, returns C<-1> with C<errno> set
appropriately.

B<Note:> This is generally unreliable. When TCP segments get lost in
transit, the re-sent bytes can form part of a larger segment so the
"boundaries" that you may expect in your input can fail to appear. This can
lead to lost data (read but not expected). This can only really be used
safely when the application protocol involves each peer reading and writing
alternately, each waiting for the other's response before making their next
response. In short, I<net_expect(3)> should only be used in concert with
I<net_send(3)>.

=cut

*/

ssize_t net_expect(int sockfd, long timeout, const char *format, ...)
{
	va_list args;
	ssize_t rc;

	va_start(args, format);
	rc = net_vexpect(sockfd, timeout, format, args);
	va_end(args);

	return rc;
}

/*

=item C<ssize_t net_vexpect(int sockfd, long timeout, const char *format, va_list args)>

Equivalent to I<net_expect(3)> with the variable argument list specified
directly as for I<vprintf(3)>.

=cut

*/

ssize_t net_vexpect(int sockfd, long timeout, const char *format, va_list args)
{
	char buf[MSG_SIZE + 1];
	ssize_t bytes;

	if (read_timeout(sockfd, timeout, 0) == -1)
		return -1;

	if ((bytes = read(sockfd, buf, MSG_SIZE)) <= 0)
		return bytes;

	buf[bytes] = '\0';

	return vsscanf(buf, format, args);
}

/*

=item C<ssize_t net_send(int sockfd, long timeout, const char *format, ...)>

Sends a formatted string (see I<printf(3)>) to a remote connection on the
socket, C<sockfd>. C<timeout> is the number of seconds to wait before timing
out. On success, returns the number of bytes written. On error, returns
C<-1> with C<errno> set appropriately.

=cut

*/

ssize_t net_send(int sockfd, long timeout, const char *format, ...)
{
	va_list args;
	ssize_t rc;

	va_start(args, format);
	rc = net_vsend(sockfd, timeout, format, args);
	va_end(args);

	return rc;
}

/*

=item C<ssize_t net_vsend(int sockfd, long timeout, const char *format, va_list args)>

Equivalent to I<net_send(3)> with the variable argument list specified
directly as for I<vprintf(3)>.

=cut

*/

ssize_t net_vsend(int sockfd, long timeout, const char *format, va_list args)
{
	char buf[MSG_SIZE + 1];
	ssize_t bytes;

	bytes = vsnprintf(buf, MSG_SIZE + 1, format, args);
	if (bytes == -1 || bytes > MSG_SIZE)
		return set_errno(ENOSPC);

	return net_write(sockfd, timeout, buf, bytes);
}

/*

=item C<ssize_t sendfd(int sockfd, const void *buf, size_t nbytes, int flags, int fd)>

Sends the open file descriptor, C<fd>, to another process (related or
unrelated) on the other end of the UNIX domain socket, C<sockfd>. Equivalent
to I<send(2)> in all other respects. UNIX domain sockets can be created
using I<net_client(3)> or I<net_server(3)> with a first argument of
C<"/unix">, or using I<socketpair(2)> or I<pipe(2)> (under I<SVR4>). It is
safe to I<close(2)> (and even I<unlink(2)>) the file descriptor after
sending it. The kernel won't really close it (or delete it) until the
receiving process closes the descriptor. If the sender doesn't close C<fd>,
both processes share the same file table entry in the kernel. This means
sharing file position if the descriptor refers to a regular file. If the
receiver doesn't receive the file descriptor with I<recvfd(3)> when it is
sent, the descriptor will be closed (in the receiving process). A file
descriptor must always be passed along with some normal data. I<Linux>
doesn't support calling I<recv(2)> with a C<null> buffer or zero length. On
success, returns C<0>. On error, returns C<-1> with C<errno> set
appropriately.

=cut

*/

ssize_t sendfd(int sockfd, const void *buf, size_t nbytes, int flags, int fd)
{
	struct msghdr mesg[1];
	struct iovec iov[1];
	int *intptr;

#ifdef HAVE_MSGHDR_MSG_CONTROL

/* Solaris8 doesn't have these */

#ifndef CMSG_ALIGN
#define CMSG_ALIGN(len) (((len) + sizeof(size_t) - 1) & (size_t)~(sizeof(size_t) - 1))
#endif

#ifndef CMSG_SPACE
#define CMSG_SPACE(len) (CMSG_ALIGN(sizeof(struct cmsghdr)) + CMSG_ALIGN(len))
#endif

#ifndef CMSG_LEN
#define CMSG_LEN(len) (CMSG_ALIGN(sizeof(struct cmsghdr)) + (len))
#endif

	union
	{
		struct cmsghdr align;
		char control[CMSG_SPACE(sizeof(int))];
	}
	control;

	struct cmsghdr *cmsg;

	if (sockfd < 0 || fd < 0)
		return set_errno(EINVAL);

	mesg->msg_control = control.control;
	mesg->msg_controllen = sizeof control.control;

	cmsg = CMSG_FIRSTHDR(mesg);
	cmsg->cmsg_len = CMSG_LEN(sizeof(int));
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SCM_RIGHTS;

	/* Avoid dereferencing type-punned pointer to avoid gcc warning */
	/* *((int *)CMSG_DATA(cmsg)) = fd; */
	intptr = (int *)CMSG_DATA(cmsg);
	*intptr = fd;
#else

	mesg->msg_accrights = (caddr_t)&fd;
	mesg->msg_accrightslen = sizeof(int);

#endif

	mesg->msg_name = NULL;
	mesg->msg_namelen = 0;

	mesg->msg_iov = iov;
	mesg->msg_iovlen = 1;

	iov->iov_base = (void *)buf;
	iov->iov_len = nbytes;

	return sendmsg(sockfd, mesg, flags);
}

/*

=item C<ssize_t recvfd(int sockfd, void *buf, size_t nbytes, int flags, int *fd)>

Receives an open file descriptor (which will be stored in C<*fd>) from
another process (related or unrelated) on the other end of the UNIX domain
socket, C<sockfd>. Equivalent to I<recv(2)> in all other respects. UNIX
domain sockets can be created using I<net_client(3)> or I<net_server(3)>
with a first argument of C<"/unix">, or using I<socketpair(2)> or I<pipe(2)>
(under I<SVR4>). If the sender doesn't close the file descriptor, both
processes share the same file table entry in the kernel. This means sharing
file position if the descriptor refers to a regular file. If the sender
sends the same file descriptor multiple times, all received file descriptors
also share the same file table entry in the kernel. If the receiver doesn't
receive the file descriptor with I<recvfd(3)> when it is sent with
I<sendfd(3)>, the descriptor will be closed (in the receiving process). A
file descriptor must always be passed along with some normal data. I<Linux>
doesn't support calling I<recv(2)> with a C<null> buffer or zero length.
Don't set C<MSG_PEEK> in C<flags> (the results are unpredictable). On
success, returns C<0>. On error, returns C<-1> with C<errno> set
appropriately. If the file descriptor was not passed, C<*fd> is set to
C<-1>.

=cut

*/

ssize_t recvfd(int sockfd, void *buf, size_t nbytes, int flags, int *fd)
{
	struct msghdr mesg[1];
	struct iovec iov[1];
	ssize_t rc;

#ifdef HAVE_MSGHDR_MSG_CONTROL

	union
	{
		struct cmsghdr align;
		char control[CMSG_SPACE(sizeof(int))];
	}
	control;

	struct cmsghdr *cmsg;

	mesg->msg_control = control.control;
	mesg->msg_controllen = sizeof control.control;

#else

	int newfd;

	mesg->msg_accrights = (caddr_t)&newfd;
	mesg->msg_accrightslen = sizeof(int);

#endif

	if (sockfd < 0 || !fd)
		return set_errno(EINVAL);

	mesg->msg_name = NULL;
	mesg->msg_namelen = 0;

	mesg->msg_iov = iov;
	mesg->msg_iovlen = 1;

	iov->iov_base = buf;
	iov->iov_len = nbytes;

	if ((rc = recvmsg(sockfd, mesg, flags)) <= 0)
		return -1;

#ifdef HAVE_MSGHDR_MSG_CONTROL

	*fd = -1;

	if ((cmsg = CMSG_FIRSTHDR(mesg)) && cmsg->cmsg_len == CMSG_LEN(sizeof(int)))
	{
		if (cmsg->cmsg_level == SOL_SOCKET && cmsg->cmsg_type == SCM_RIGHTS)
		{
			/* Avoid dereferencing type-punned pointer to avoid gcc warning */
			/* *fd = *((int *)CMSG_DATA(cmsg)); */
			int *intptr = (int *)CMSG_DATA(cmsg);
			*fd = *intptr;
		}
	}

#else

	*fd = (mesg->msg_accrightslen == sizeof(int)) ? newfd : -1;

#endif

	return rc;
}

#ifdef SO_PASSCRED
#ifdef SCM_CREDENTIALS

/*

=item C<ssize_t recvcred(int sockfd, void *buf, size_t nbytes, int flags, struct ucred *cred)>

Receives the user credentials of the process on the other end of the UNIX
domain socket, C<sockfd>, and stores them in C<*cred>. Equivalent to
I<recv(2)> in all other respects. Requires that the C<SO_PASSCRED> socket
option has been set for C<sockfd> in advance. On datagram sockets, user
credentials accompany every datagram. On stream sockets, user credentials
are sent only once, the first time data is sent. On success, returns the
number of bytes received. On error, returns C<-1> with C<errno> set
appropriately. If the user credentials were not provided by the kernel,
C<cred> is filled with zero bytes (so C<cred[0].pid == 0>).

This function is only available on I<Linux>.

=cut

*/

ssize_t recvcred(int sockfd, void *buf, size_t nbytes, int flags, struct ucred *cred)
{
	return recvfromcred(sockfd, buf, nbytes, flags, NULL, NULL, cred);
}

/*

=item C<ssize_t recvfromcred(int sockfd, void *buf, size_t nbytes, int flags, struct sockaddr *src_addr, socklen_t *src_addrlen, struct ucred *cred)>

Receives the user credentials of the process on the other end of the UNIX
domain socket, C<sockfd>, and stores them in C<*cred>. Equivalent to
I<recvfrom(2)> in all other respects. Requires that the C<SO_PASSCRED>
socket option has been set for C<sockfd> in advance. On datagram sockets,
user credentials accompany every datagram. On stream sockets, user
credentials are sent only once, the first time data is sent. On success,
returns the number of bytes received. On error, returns C<-1> with C<errno>
set appropriately. If the user credentials were not provided by the kernel,
C<cred> is filled with zero bytes (so C<cred[0].pid == 0>).

This function is only available on I<Linux>.

=cut

*/

ssize_t recvfromcred(int sockfd, void *buf, size_t nbytes, int flags, struct sockaddr *src_addr, socklen_t *src_addrlen, struct ucred *cred)
{
	struct msghdr mesg[1];
	struct iovec iov[1];
	ssize_t rc;

	union
	{
		struct cmsghdr align;
		char control[CMSG_SPACE(sizeof(struct ucred))];
	}
	control;

	struct cmsghdr *cmsg = NULL;

	mesg->msg_control = control.control;
	mesg->msg_controllen = sizeof control.control;

	mesg->msg_name = src_addr;
	mesg->msg_namelen = (src_addrlen) ? *src_addrlen : 0;

	mesg->msg_iov = iov;
	mesg->msg_iovlen = 1;

	iov->iov_base = buf;
	iov->iov_len = nbytes;

	if ((rc = recvmsg(sockfd, mesg, flags)) == -1)
		return rc;

	if (src_addrlen)
		*src_addrlen = mesg->msg_namelen;

	if (cred && mesg->msg_controllen >= sizeof(struct cmsghdr))
	{
		int received_credentials = 0;

		for (cmsg = CMSG_FIRSTHDR(mesg); cmsg; cmsg = CMSG_NXTHDR(mesg, cmsg))
		{
			if (cmsg->cmsg_level == SOL_SOCKET && cmsg->cmsg_type == SCM_CREDENTIALS && cmsg->cmsg_len == CMSG_LEN(sizeof(struct ucred)))
			{
				memcpy(cred, CMSG_DATA(cmsg), sizeof(struct ucred));
				received_credentials = 1;
			}
		}

		if (!received_credentials)
			memset(cred, 0, sizeof(struct ucred));
	}

	return rc;
}
#endif
#endif

/*

=item C<int mail(const char *server, const char *sender, const char *recipients, const char *subject, const char *message)>

Sends a mail message consisting of C<subject> and C<message> from C<sender>
to the addresses in C<recipients>. C<recipients> contains mail addresses
separated by sequences of comma and/or space characters. C<message> must not
contain any lines containing only a C<'.'> character. On success, returns
C<0>. On error, returns C<-1> with C<errno> set appropriately.

=cut

*/

static int rcpt(int smtp, const char *recipients)
{
	List *list = split(recipients, ", ");

	if (!list || list_length(list) <= 0)
		return -1;

	while (list_has_next(list) == 1)
	{
		char *recipient = cstr((String *)list_next(list));
		int rc, code;

		if (net_send(smtp, 10, "RCPT TO: <%s>\r\n", recipient) == -1 ||
			(rc = net_expect(smtp, 10, "%d", &code)) == -1)
		{
			list_release(list);
			return -1;
		}

		if (rc != 1 || code != 250)
		{
			list_release(list);
			return set_errno(EPROTO);
		}
	}

	list_release(list);
	return 0;
}

int mail(const char *server, const char *sender, const char *recipients, const char *subject, const char *message)
{
	int smtp;
	int code;
	int rc;
	char c;

	if (!sender || !recipients)
		return set_errno(EINVAL);

	if ((smtp = net_client(server, "smtp", 25, 5, 0, 0, NULL, NULL)) == -1)
		return -1;

#define fail { close(smtp); return -1; }
#define try(action) if ((action) == -1) fail
#define try_send(args) try(net_send args)
#define try_expect(args, cnv, resp) try(rc = net_expect args) \
	if (rc != (cnv) || code != (resp)) { close(smtp); return set_errno(EPROTO); }

	net_tos_lowdelay(smtp);

	try_expect((smtp, 10, "%d%c", &code, &c), 2, 220)

	while (c == '-')
		try_expect((smtp, 10, "%d%c", &code, &c), 2, 220)

	try_send((smtp, 10, "HELO localhost\r\n"))
	try_expect((smtp, 10, "%d", &code), 1, 250)
	try_send((smtp, 10, "MAIL FROM: <%s>\r\n", sender))
	try_expect((smtp, 10, "%d", &code), 1, 250)
	try(rcpt(smtp, recipients))
	try_send((smtp, 10, "DATA\r\n"))
	try_expect((smtp, 10, "%d", &code), 1, 354)

	net_tos_throughput(smtp);

	try_send((smtp, 10, "From: %s\r\n", sender))
	try_send((smtp, 10, "To: %s\r\n", recipients))
	try_send((smtp, 10, "Subject: %s\r\n\r\n", (subject) ? subject : ""))
	try_send((smtp, 10, "%s\r\n.\r\n", (message) ? message : ""))
	try_expect((smtp, 10, "%d", &code), 1, 250)

	net_tos_lowdelay(smtp);

	try_send((smtp, 10, "QUIT\r\n"))
	try_expect((smtp, 10, "%d", &code), 1, 221)
	close(smtp);

	return 0;
}

/*

=back

=head1 SOCKET OPTION NOTES

Here is some vital information about socket options that never made it into
the I<setsockopt(2)> manpage (where it would be most useful). It's from
I<"UNIX Network Programming: Networking APIs: Sockets and XTI (Volume 1)">
by W. Richard Stevens.

=over 4

=item C<SO_LINGER>

Never set this option. There are two (bad) reasons why people set this
option. The first reason is to avoid having to wait until after a socket has
left the C<TIME_WAIT> state before restarting a server that has terminated.
The C<TIME_WAIT> state is your friend. Do not try to avoid it. If you avoid
it, you break TCP. If you break TCP, you will be punished. Set
C<SO_REUSEADDR> instead. This is what it's for.

The other reason is to know when the peer has received all sent data. This
probably doesn't work the way you want. It can only tell you when the peer
TCP has acknowledged the data. It cannot tell you when the peer application
has read the data. To do this, use I<shutdown(2)> with a second argument of
C<SHUT_WR> and then call I<read(2)> until it returns C<0>. This tells you
that the peer application has read all sent data, knows that it has read all
sent data (because it received your FIN) and has closed its half of the
connection with either I<close(2)> or I<shutdown(2)> with a second argument
of C<SHUT_WR> (because you have received the peer's FIN). Then you can
I<close(2)> the socket, safe in the knowledge that no data has been lost.

If you set C<SO_LINGER> with a zero timeout, the peer will think your
application has crashed or aborted the connection (because it receives an
RST). The only time to use C<SO_LINGER> is when this is the behaviour you
want.

=item C<SO_REUSEADDR>

Use this option for every TCP server socket. The net server functions set
this option for every TCP server socket. This means that if your server
dies, the new process that replaces it will be able to bind to the server's
port immediately. This option is also needed when multiple copies of a
multicast application need to run on the same host and C<SO_REUSEPORT> isn't
defined. This option must be set before I<bind(2)>.

=item C<TCP_NODELAY>

Avoid setting this option whenever possible (i.e. most of the time). It
disables the Nagle algorithm. The Nagle algorithm is your friend. It stops
you polluting the network with annoying little packets. If you must set it,
please ensure that the traffic is restricted to your own network and leave
the Internet alone.

Setting this option is often the wrong solution to a bad network programming
practice. If an application protocol involves immediate responses to each
message and exceptionally long delays are experienced, it's probably due to
the message being sent with multiple small I<write(2)>s (e.g. application
header first, then data) instead of a single I<write(2)>.

If a message is sent in small I<write(2)>s, the first I<write(2)> will
result in a small segment being sent. If the data in that segment does not
contain enough information for the peer to respond immediately, the peer TCP
will not ACK the segment until the ACK timer expires (50ms - 200ms). This is
the delayed ACK algorithm. The sending TCP will not send the second small
segment (containing the remainder of the message) until the first small
segment has been acknowledged by the peer TCP. This is the Nagle Algorithm.

The solution to this problem is not to disable the Nagle algorithm, but
rather to modify the application so that the message is sent in a single
call to I<writev(2)>. Avoid copying separate buffers into a single buffer
and then calling I<write(2)> as it is less efficient.

This option should only be set when the peer application does not respond to
each message and there can be no delay in sending the messages (e.g. real
time monitoring systems) or when, even though the peer does respond to each
message, the application can't hang around waiting for the response to the
previous message before sending the next message (e.g. highly interactive
applications like I<The X Window System>).

=item C<SO_SNDBUF>

This option specifies how much unacknowledged data you are willing to have
out in the network before you stop sending data and wait for some
acknowledgement. For bulk transfers, the send and receive buffer sizes need
to be set to the capacity of the pipe (i.e. the bandwidth-delay product)
otherwise throughput will be limited by the buffer sizes rather than by the
network. The bandwidth-delay product is the bandwidth of the network
multiplied by the round trip time. Here are some examples. Note that these
values are for raw bandwidth, not data bandwidth. Actual values will be
smaller due to packet header overhead.

 Network                   | Bandwidth(bps) | RTT(ms) | Buffer(bytes)
 --------------------------+----------------+---------+--------------
 Ethernet LAN (10Mb/s)     |    10,000,000  |    3    |      3,750
 Ethernet LAN (100Mb/s)    |   100,000,000  |    3    |     37,500
 T1, transcontinental      |     1,544,000  |   60    |     11,580
 T1, satellite             |     1,544,000  |  500    |     96,500
 T3, transcontinental      |    45,000,000  |   60    |    337,500
 Gigabit, transcontinental | 1,000,000,000  |   60    |  7,500,000
 Gigabit Satellite Network |   155,520,000  |  500    |  9,720,000
  (SONET OC-3)             |                |         |
 Gigabit Satellite Network |   622,080,000  |  500    | 38,888,000
  (SONET OC-12)            |                |         |

Of course, it's generally impossible to know in advance what the bandwidth
or RTT will be, and they can both change during the life of the connection.
Ideally, the kernel would automatically adjust buffer sizes as needed, but
don't hold your breath. Unless you know exactly what kind of network your
application will be running on, it's best to set buffer sizes to values
obtained from the user via a configuration file or user interface. Bear in
mind that most kernels don't support buffer sizes larger than a few hundred
kilobytes anyway.

Also note that TCP over satellite connections can behave very badly.
Everything is fine provided that there's no congestion. However, if a single
packet is lost, throughput will halve due to congestion avoidance, every
segment sent since the lost packet will have to be retransmitted (that's
38MB!) and it takes five minutes to reach maximum throughput again due to
the long RTT. Selective ACKs are needed in TCP to fix this. Fortunately,
I<Linux> (and probably other) systems support selective ACKs.

This option can also be used to avoid the dreaded interaction between the
Nagle Algorithm and Delayed ACK algorithm during bulk data transfer. This
interaction cannot occur during bulk transfer if the send buffer size is at
least 3 times the Maximum Segment Size (MSS). Having a send buffer this
large means that the sender is always capable of sending two full segments.
If the receiver's receive buffer size isn't large enough to accept both
segments, it will ACK each segment without delay (to indicate that it is
running out of buffer space). If the receiver's receive buffer size is large
enough to accept both segments, it will ACK every second segment without
delay (so as not to disrupt your TCP's RTT calculations). The buffer size
should actually be an even multiple of the MSS (i.e. at least four times the
MSS). Here are some examples.

    Link     | MTU(bytes) | MSS(bytes) | 4*MSS(bytes)
    ---------+------------+------------+--------------
    Ethernet |    1,500   |    1,460   |     5,840
    ATM      |    9,188   |    9,148   |    36,592
    HIPPI    |   65,535   |   65,495   |   261,980

Some TCP implementations automatically round the send and receive buffer
sizes up to an even multiple of the MSS after establishing the connection.
So if you set these options, do so before establishing the connection (i.e.
before I<listen(2)> or I<connect(2)>). The net server and client functions
set these options at the right time if requested.

This option, when set for UDP sockets, limits the maximum datagram size that
can be sent.

=item C<SO_RCVBUF>

Much of what was said about the send buffer size applies to the size of the
peer's receive buffer. If your application is willing to accept large
amounts of data, it needs to advertise the fact by having a large receive
buffer. If the long fat pipe TCP options are required (Window Scale), they
must be negotiated during connection setup (in the SYN packets) so this
option must be set before I<listen(2)> or I<connect(2)>. The net server and
client functions set this option at the right time if requested.

This option, when set for UDP sockets, specifies how many received datagrams
to queue before discarding datagrams.

=item C<SO_KEEPALIVE>

This option causes TCP to send a probe after two hours of inactivity to
check that the connection is still alive. Many people think that two hours
is too long to wait so they implement application level heartbeats instead
(e.g. BGP routing daemons send keepalive packets every 30 seconds). Many
people think that this functionality belongs in the application anyway. The
I<POSIX.1g> standard requires the C<TCP_KEEPALIVE> option which lets you
specify how many seconds to wait before sending the probe but this option
isn't widely implemented yet. Until it is, the C<SO_KEEPALIVE> option is not
very useful.

=back

=head1 PROTOCOL DESIGN NOTES

Here are some things to consider when designing packet headers and
distributed algorithms gleaned from I<"Interconnections: Bridges, Routers,
Switches and Internetworking Protocols"> by Radia Perlman.

=over 4

=item Simplicity versus Flexibility versus Optimality

Simple protocols are more likely to be successfully implemented and
deployed. Various factors complicate a protocol:

=over 4

=item *

Design by committee (multiple ways to do the same thing).

=item *

Backwards Compatibility.

=item *

Flexibility.

=item *

Optimality.

=item *

Underspecification (leaving decisions to the implementer).

=item *

Exotic features.

=back

=item Knowing the problem you're trying to solve

Solve at least one actual problem. Do nothing that is of no use.

=item Overhead and Scaling

Calculate the overhead of algorithms and protocols. Does it scale? How far?
Does it matter?

=item Operation above capacity

If there are assumptions about the size of the problem, either make them
impossibly huge, or cope when the limit is exceeded.

=item Compact IDs versus Object Identifiers

Identifiers take two forms: (1) centrally administered numbers (e.g. port
numbers) which are short, fixed size, fast and easy to locate, but hard to
obtain, and (2) hierarchical identifiers (e.g. MIB names) with decentralised
administration. These are large, variable size, slow, and hard to locate (no
central authority), but easy to obtain.

=item Optimising for the most common or important case

If some information in a packet is rarely needed, make it an option. It is
better for a few packets to be larger and slower than for all other packets
to bear unused overhead.

=item Forward Compatibility

=over 4

=item Large Enough Fields

It is better to overestimate than to underestimate. It makes protocols live
longer.

=item Independence of Layers

Don't assume addresses are IPv4 addresses.

=item Reserved Fields

Spare bits must be transmitted as zero and ignored upon receipt. That way,
they can later be used by future versions to encode features that can safely
be ignored by earlier versions.

=item Version Number Field

Version numbers can be a simple number, or split into major and minor
version components. Minor version increments indicate backwards-compatible
changes. Major version increments indicate incompatible changes. If a node
receives a packet with a version it doesn't know about, it should drop it or
respond with the version it does understand. The other node can switch to
the older protocol when it receives this packet. However, nodes should
occasionally forget that the other node speaks an older version of the
protocol to prevent two nodes from incorrectly thinking that the other can
only speak an old version of the protocol.

Avoid having version numbers wrap around, by making it huge, or by
incrementing versions very rarely. If the version can wrap, make the highest
possible version number indicate that the actual version follows in a larger
field.

=item Options

Another way to provide for future protocol evolutions is to allow options to
be appended. Options should be encoded as I<E<lt>type, length, valueE<gt>>,
and the length must be interpreted in the same way for all options. This
allows unknown options to be skipped. Some options should cause the packet
to be dropped. The type field can be used to specify whether the node should
skip the option or drop the packet: e.g. skip options with odd numbered
types and drop packets when options with even numbered types are
encountered.

=back

=item Migration

When migrating from one protocol to another, incompatible protocol, it's
easiest to keep them separate (e.g. dual IPv4/IPv6 stacks), because
migration can't be done atomically and it can be difficult to translate
between two protocols.

=item Parameters

=over 4

=item *

Have parameters when there are settings that the user may want to control.

=item *

Don't have parameters just because you can't decide on the setting. Who else
will?

=item *

Choose or calculate parameters when possible to reduce human involvement.

=item *

Make it possible to change parameters one at a time throughout a network
without things breaking.

=item *

Nodes can report their parameters to their neighbours so they adjust their
own parameters accordingly or detect misconfiguration.

=back

=item Making Multiprotocol Operation Possible

Have a field that indicates the protocol type. This can allow multiplexing
of mini-protocols within the application if the need ever arises.

=item Robustness

There are three kinds of robustness. I<Simple robustness> is when a node can
cope when other nodes go down. I<Self stabilising robustness> is when, even
though a node may not cope with another node malfunctioning, it will return
to correct behaviour when the malfunctioning node is fixed. I<Byzantine
robustness> is when a node behaves properly even when malicious or
malfunctioning nodes are operating. In this day and age, Byzantine
robustness is a necessity.

=over 4

=item *

Exercise every single line of code, then torture every single line of code.

=item *

Sometimes its better to crash than to malfunction.

=item *

Sometimes you can partition a network to contain a problem.

=item *

Test connectivity, don't assume it.

=item *

Simple checksums can be tricked. Use SHA-2/3 or public key signatures when
practical. Use encryption and authentication when possible (e.g. Transport
Layer Security/Secure Shell tunnels).

=item *

Process packets quickly to avoid denial of service attacks.

=back

=item Determinism versus Stability

Elections can be deterministic (the same node wins every time it is up) or
stable (once a node is elected, it stays elected until it goes down). If
every node is configured with a priority, and the election winner increases
its priority by I<N> after winning an election, then you can achieve
deterministic elections by configuring nodes with priorities that differ by
more than I<N>, and you can achieve stable elections by configuring nodes
with the same priority.

=item Performance for Correctness

Understand the performance requirements that define a "correct"
implementation. For example, processing packets at wire speed is necessary
to avoid denials of service.

=back

=head1 ERRORS

These are the errors generated by the functions that return C<-1> on error.
Additional errors may be generated and returned from the underlying system
calls. See their manual pages.

=over 4

=item C<ENOENT>

I<gethostbyname(3)> failed to identify the C<host> or C<interface> argument
passed to one of the socket functions.

=item C<ENOSYS>

I<gethostbyname(3)> returned an address from an unsupported address family.

The C<"l"> format was used with I<pack(3)> or I<unpack(3)> when the system
doesn't support it or it wasn't compiled into I<libslack>.

=item C<EINVAL>

A string argument is C<null>.

A pack format count is not a positive integer.

An unpack count or limit argument is not a positive integer.

An argument containing C<"a">, C<"z">, C<"b"> or C<"h"> data to be packed is
C<null>.

An argument containing C<"b"> data to be packed contains characters outside
the range [01].

An argument containing C<"h"> data to be packed contains characters outside
the range [0-9a-fA-F].

An C<"X"> pack instruction is trying to go back past the start of the
packet.

The count argument to an C<"@"> pack instruction refers to a location before
that where the instruction was encountered (i.e. it's trying to pack
leftwards).

The C<format> argument to I<pack(3)> or I<unpack(3)> contains an illegal
character.

An unpack C<?> indirect count argument is C<null>.

=item C<ENOSPC>

A message was too large to be sent with I<net_send(3)>.

A packet was too small to store all of the data to be packed or unpacked.

An unpack C<?> indirect count argument points to a number greater than the
subsequent limit argument (not enough space in the target buffer).

=item C<ETIMEDOUT>

I<net_expect(3)> or I<net_send(3)> timed out.

=item C<EPROTO> (or C<EPROTOTYPE> on I<Mac OS X>)

I<mail(3)> encountered an error in the dialogue with the SMTP server. The
most likely cause of this is a missing or inadequate domain name for the
sender address on systems where I<sendmail(8)> requires a real domain name.

=back

=head1 MT-Level

I<MT-Safe>

=head1 EXAMPLES

A TCP server:

    #include <slack/std.h>
    #include <slack/net.h>

    void provide_service(int fd) { write(fd, "ok\n", 3); }

    int main()
    {
        int servfd, clntfd;

        if ((servfd = net_server(NULL, "service", 30000, 0, 0, NULL, NULL)) == -1)
            return 1;

        while ((clntfd = accept(servfd, NULL, NULL)) != -1)
        {
            pid_t pid;

            switch (pid = fork())
            {
                case -1: return 1;
                case  0: provide_service(clntfd); _exit(EXIT_SUCCESS);
                default: close(clntfd); break;
            }
        }

        return EXIT_FAILURE; // unreached
    }

A TCP client:

    #include <slack/std.h>
    #include <slack/net.h>

    void request_service(int fd) {} // Do something here
    void process_response(int fd) {} // Do something here

    int main()
    {
        int sockfd;

        if ((sockfd = net_client("localhost", "service", 30000, 5, 0, 0, NULL, NULL)) == -1)
            return EXIT_FAILURE;

        request_service(sockfd);
        process_response(sockfd);
        close(sockfd);
        return EXIT_SUCCESS;
    }

A UDP server:

    #include <slack/std.h>
    #include <slack/net.h>

    void provide_service(char *pkt) {} // Do something here

    int main()
    {
        char pkt[8];
        sockaddr_any_t addr;
        size_t addrsize;
        int servfd;

        if ((servfd = net_udp_server(NULL, "service", 30000, 0, 0, NULL, NULL)) == -1)
            return EXIT_FAILURE;

        for (;;)
        {
    	    addrsize = sizeof addr;

            if (recvfrom(servfd, pkt, 8, 0, &addr.any, &addrsize) == -1)
                return EXIT_FAILURE;

            provide_service(pkt);

            if (sendto(servfd, pkt, 8, 0, &addr.any, addrsize) == -1)
                return EXIT_FAILURE;
        }

        return EXIT_SUCCESS; // unreached
    }

A UDP client:

    #include <slack/std.h>
    #include <slack/net.h>

    void build_request(char *pkt) {} // Do something here
    void process_response(char *pkt) {} // Do something here

    int main()
    {
        char pkt[8];
        int sockfd = net_udp_client("localhost", "service", 30000, 0, 0, NULL, NULL);
        if (sockfd == -1)
            return EXIT_FAILURE;

        build_request(pkt);

        if (send(sockfd, pkt, 8, 0) == -1)
            return EXIT_FAILURE;

        if (recv(sockfd, pkt, 8, 0) == -1)
            return EXIT_FAILURE;

        process_response(pkt);

        close(sockfd);

        return EXIT_SUCCESS;
    }

A reliable UDP client:

    #include <slack/std.h>
    #include <slack/net.h>

    void build_request(char *pkt) {} // Do something here
    void process_response(char *pkt) {} // Do something here

    int main()
    {
        char opkt[8], ipkt[8];
        int sockfd;
        rudp_t *rudp;

        if ((sockfd = net_udp_client("localhost", "echo", 7, 0, 0, NULL, NULL)) == -1)
            return EXIT_FAILURE;

        if (!(rudp = rudp_create()))
            return EXIT_FAILURE;

        build_request(opkt);

        if (net_rudp_transact(sockfd, rudp, opkt, 8, ipkt, 8) == -1)
            return EXIT_FAILURE;

        process_response(ipkt);

        rudp_release(rudp);
        close(sockfd);

        return EXIT_SUCCESS;
    }

Expect/Send SMTP protocol:

    #include <slack/std.h>
    #include <slack/net.h>

    int tinymail(char *sender, char *recipient, char *subject, char *message)
    {
        int smtp = net_client("localhost", "smtp", 25, 5, 0, 0, NULL, NULL);
        int code;
        int rc =
            smtp != -1 &&
            net_expect(smtp, 10, "%d", &code) == 1 && code == 220 &&
            net_send(smtp, 10, "HELO %s\r\n", "localhost") != -1 &&
            net_expect(smtp, 10, "%d", &code) == 1 && code == 250 &&
            net_send(smtp, 10, "MAIL FROM: <%s>\r\n", sender) != -1 &&
            net_expect(smtp, 10, "%d", &code) == 1 && code == 250 &&
            net_send(smtp, 10, "RCPT TO: <%s>\r\n", recipient) != -1 &&
            net_expect(smtp, 10, "%d", &code) == 1 && code == 250 &&
            net_send(smtp, 10, "DATA\n") != -1 &&
            net_expect(smtp, 10, "%d", &code) == 1 && code == 354 &&
            net_send(smtp, 10, "From: %s\r\n", sender) != -1 &&
            net_send(smtp, 10, "To: %s\r\n", recipient) != -1 &&
            net_send(smtp, 10, "Subject: %s\r\n", subject) != -1 &&
            net_send(smtp, 10, "\n%s\r\n.\r\n", message) != -1 &&
            net_expect(smtp, 10, "%d", &code) == 1 && code == 250 &&
            net_send(smtp, 10, "QUIT\r\n") != -1 &&
            net_expect(smtp, 10, "%d", &code) == 1 && code == 221;

        if (smtp != -1)
            close(smtp);

        return rc;
    }

    int main(int ac, char **av)
    {
        if (tinymail("raf@raf.org", "raf@raf.org", "This is a test", "Are you receiving me?\n") == -1)
            return EXIT_FAILURE;

        return EXIT_SUCCESS;
    }

Unpack the size of a gif image:

    unsigned short width, height;
    unpack(gif, 10, "z6v2", NULL, &width, &height);

Pack and unpack a packet with a length field:

    char pkt[9], data[5] = "4321";
    int packed, unpacked;
    size_t size;

    packed = pack(pkt, sizeof pkt, "ia*", sizeof data, sizeof data, data);
    unpacked = unpack(pkt, packed, "ia?", &size, &size, sizeof data, data);

Pack examples from I<perlfunc(1)>:

    pack(pkt, 4, "cccc", 'A', 'B', 'C', 'D');    // "ABCD"
    pack(pkt, 4, "c4",   'A', 'B', 'C', 'D');    // "ABCD"
    pack(pkt, 6, "ccxxcc", 'A', 'B', 'C', 'D');  // "AB\0\0CD"
    pack(pkt, 4, "s2", 1, 2);                    // "\0\1\0\2"
    pack(pkt, 4, "a4", "abcd", "x", "y", "z");   // "abcd"
    pack(pkt, 4, "aaaa", "abcd", "x", "y", "z"); // "axyz"
    pack(pkt, 14, "z14", "abcdefg");             // "abcdefg\0\0\0\0\0\0\0"

    int binary(const char *binstr)
    {
        char pkt[4], data[33];
        size_t len;
        int ret;

        len = strlen(binstr);
        memset(data, '0', 32 - len);
        strlcpy(data + 32 - len, binstr, 33);
        pack(pkt, 4, "b32", data);
        unpack(pkt, 4, "i", &ret);
        return ret;
    }

    int hexadecimal(const char *hexstr)
    {
        char pkt[4], data[9];
        size_t len;
        int ret;

        len = strlen(hexstr);
        memset(data, '0', 8 - len);
        strlcpy(data + 8 - len, hexstr, 9);
        pack(pkt, 4, "h8", data);
        unpack(pkt, 4, "i", &ret);
        return ret;
    }

=head1 BUGS

The pack functions assume the following: There are 8 bits in a byte. A char
is 1 byte. A short can be stored in 2 bytes. Integers, long integers and
pointers can be stored in 4 bytes. Long long integers can be stored in 8
bytes. If these datatypes are larger on your system, only the least
significant byte(s) will be packed.

Packing long long integers is not portable (in I<ISO C 89>, anyway).

Every effort has been made to use threadsafe, reentrant host and service
name lookups in the net client and server functions. If your system has any
version of I<gethostbyname_r(3)> and I<getservbyname_r(3)>, they will be
used. Some systems (e.g. I<Digital UNIX>, I<HP-UX>, I<Tru64 UNIX>) have a
threadsafe version of I<gethostbyname(3)> that uses thread specific data.
Unfortunately, there's no way to determine whether or not your system's
I<gethostbyname(3)> and I<getservbyname(3)> are threadsafe, so it is
possible (though unlikely) that the net client and servers functions are not
reentrant on your system. This does not apply to I<Linux>, I<Solaris>,
I<Digital UNIX>, I<HP-UX> or I<Tru64 UNIX> (and others, no doubt) since
these systems do have threadsafe versions of the host and service name
lookup functions.

B<Note:> It's possible that the underlying DNS resolver functions on your
system are not threadsafe. Versions of BIND's resolver library prior to BIND
8.2 are not threadsafe. If your system uses such a version, then even
I<gethostbyname_r(3)> isn't threadsafe. Fortunately, I<Solaris> doesn't use
I<libresolv> by default and I<Linux> uses the BIND 8.2 version of
I<libresolv> which has a new threadsafe API and thread specific data for the
old API. It is unlikely that any system that provides I<gethostnyname_r(3)>
would provide a non-threadsafe implementation.

There is a race condition that can cause a failure when creating a UNIX
domain datagram client socket under I<Solaris> and I<OpenBSD> (but not under
I<Linux>). The problem is that UNIX domain datagram sockets must be bound to
a path using I<bind(2)> otherwise they can't receive any replies from the
server (since they have no address to send messages to). I<Linux> lets us
bind to C<""> which is the C<AF_LOCAL> equivalent of C<INADDR_ANY>. This is
great. No actual path is created, each client gets its own address and the
client doesn't need to unlink the path when it's finished. Unfortunately,
systems like I<Solaris> and I<OpenBSD> (and probably many others) don't
support this. You have to bind to an actual file system path and I<bind(2)>
will create an inode for the socket (which the client must unlink when
finished). This means there's a race condition between creating the unique
path and creating the inode with I<bind(2)>. Fortunately, this isn't a
security bug (correct me if I'm wrong) because I<bind(2)> fails if the path
already exists. Nor is it a denial of service, since it only affects
clients. It's more of a denial of request. Also, the names used are not very
predictable. The easy, elegant, portable solution is to never use UNIX
domain datagram sockets. Always use UNIX domain stream sockets instead. They
don't have this problem. If you must use UNIX domain datagram sockets under
I<Solaris>, you have to unlink the socket path when finished.

    sockaddr_any_t addr;
    size_t addrsize = sizeof addr;

    if (getsockname(sockfd, (sockaddr_t *)&addr, &addrsize) != -1)
        if (*addr.un.sun_path)
            unlink(addr.un.sun_path);

This module provides no support for multiple simultaneous TCP connects in a
single thread. Use multiple threads or processes instead.

I<Solaris> (at least 2.6 and 2.7) return C<-1> as the index for all network
interfaces when I<ioctl(2)> is called with a command argument of
C<SIOCGIFINDEX>. I<net_interfaces(3)> guesses the indexes when this happens.
It starts at 1 for the first interface, and increments by 1 for each
subsequent interface which seems to work.

Because I<net_interfaces(3)> under I<Solaris> 2.6 and 2.7 has to guess the
indexes of all interfaces, and because it only returns IPv4 or IPv6
interfaces (but not both), the indexes will probably be wrong on these
systems when there is a mix of IPv4 and IPv6 interfaces. Presumably,
versions of I<Solaris> that actually support IPv6 will have the
I<ioctl(SIOCGIFINDEX)> bug fixed.

I<Solaris> doesn't return hardware addresses when I<ioctl(2)> is called with
a command argument of C<SIOCGIFHWADDR>, so the I<net_interface_t> elements
in the list returned by net_interfaces() always have C<null> hwaddr fields.

I<Linux 2.2> returns C<0.0.0.0> as the address of the outgoing IPv4
multicast interface when I<getsockopt(2)> is called with the
C<IP_MULTICAST_IF> command. This means that
I<net_multicast_get_interface(3)> always returns C<0> under I<Linux 2.2>.
I<Linux 2.4.9> does not have this bug.

The TOS functions are inherently protocol specific. They only work with IPv4
sockets.

=head1 SEE ALSO

I<libslack(3)>,
I<socket(2)>,
I<bind(2)>,
I<listen(2)>,
I<accept(2)>,
I<connect(2)>,
I<shutdown(2)>,
I<select(2)>,
I<read(2)>,
I<write(2)>,
I<readv(2)>,
I<writev(2)>,
I<close(2)>,
I<send(2)>,
I<sendto(2)>,
I<recv(2)>,
I<recvfrom(2)>,
I<gethostbyname(3)>,
I<getservbyname(3)>,
I<perlfunc(1)>,
I<fdopen(3)>,
I<scanf(3)>,
I<printf(3)>

=head1 AUTHOR

20210220 raf <raf@raf.org>

=cut

*/

#endif

#ifdef TEST

#ifndef _GNU_SOURCE
#define _GNU_SOURCE /* For receiving user credentials over UNIX domain sockets */
#endif

#include <fcntl.h>
#include <pwd.h>

#include <sys/utsname.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include <netinet/tcp.h>

static void print_pkt(const char *name, void *buf, size_t size)
{
	unsigned char *pkt = buf;
	size_t i;

	while (size && pkt[size - 1] == '\0')
		--size;

	printf("%s =\n{\n", name);

	for (i = 0; i < size; ++i)
	{
		if (i && i % 10 == 0)
			printf("\n");
		if (i % 10 == 0)
			printf("    ");
		printf("0x%02x%s", pkt[i], (i < size - 1) ? ", " : "");
	}

	printf("%s}\n\n", (i % 10) ? "\n" : "");
}

static int wait_for_child(pid_t pid)
{
	int status;

	if (waitpid(pid, &status, 0) == -1)
	{
		fprintf(stderr, "Failed to waitpid(%d) (%s)\n", (int)pid, strerror(errno));
		exit(EXIT_FAILURE);
	}

	if (WIFSIGNALED(status))
	{
		fprintf(stderr, "Child failed: received signal %d\n", WTERMSIG(status));
		exit(EXIT_FAILURE);
	}

	return WEXITSTATUS(status);
}

int so_broadcast = 1;
int so_keepalive = 1;
int so_debug = 0;
int so_oobinline = 0;
int so_sndbuf = 64 * 1024;
int so_rcvbuf = 64 * 1024;
int so_sndlowat = 1024;
int so_rcvlowat = 1024;
int so_reuseaddr = 1;
struct linger linger = { 1, 0 };
int tcp_nodelay = 1;
int ip_ttl = 32;

#ifdef IPTOS_LOWDELAY
int iptos_lowdelay = IPTOS_LOWDELAY;
#endif

#ifdef IPTOS_THROUGHPUT
int iptos_throughput = IPTOS_THROUGHPUT;
#endif

#ifdef IPTOS_RELIABILITY
int iptos_reliability = IPTOS_RELIABILITY;
#endif

#ifdef IPTOS_LOWCOST
int iptos_lowcost = IPTOS_LOWCOST;
#endif

sockopt_t sockopts[] =
{
	{ SOL_SOCKET,  SO_BROADCAST, &so_broadcast, sizeof(int) },
	{ SOL_SOCKET,  SO_KEEPALIVE, &so_keepalive, sizeof(int) },
	{ SOL_SOCKET,  SO_DEBUG,     &so_debug, sizeof(int) },
	{ SOL_SOCKET,  SO_OOBINLINE, &so_oobinline, sizeof(int) },
	{ SOL_SOCKET,  SO_SNDBUF,    &so_sndbuf, sizeof(int) },
	{ SOL_SOCKET,  SO_RCVBUF,    &so_rcvbuf, sizeof(int) },
	/* { SOL_SOCKET,  SO_SNDLOWAT,  &so_sndlowat, sizeof(int) }, */
	/* { SOL_SOCKET,  SO_RCVLOWAT,  &so_rcvlowat, sizeof(int) }, */
	{ SOL_SOCKET,  SO_REUSEADDR, &so_reuseaddr, sizeof(int) },
	{ SOL_SOCKET,   SO_LINGER,    &linger, sizeof(struct linger) },
	{ IPPROTO_TCP, TCP_NODELAY,  &tcp_nodelay, sizeof(int) },
#if 0 /* Can't do these with IPv6 */
	{ IPPROTO_IP,  IP_TTL,       &ip_ttl, sizeof(int) },
#ifdef IPTOS_LOWDELAY
	{ IPPROTO_IP,  IP_TOS,       &iptos_lowdelay, sizeof(int) },
#endif
#ifdef IPTOS_THROUGHPUT
	{ IPPROTO_IP,  IP_TOS,       &iptos_throughput, sizeof(int) },
#endif
#ifdef IPTOS_RELIABILITY
	{ IPPROTO_IP,  IP_TOS,       &iptos_reliability, sizeof(int) },
#endif
#ifdef IPTOS_LOWCOST
	{ IPPROTO_IP,  IP_TOS,       &iptos_lowcost, sizeof(int) },
#endif
#endif
	{ 0, 0, NULL, 0 }
};

#ifdef AF_INET6
static int inet6_required(void)
{
	char *res_options;
	FILE *resolv_conf;

	if ((res_options = getenv("RES_OPTIONS")) && strstr(res_options, "inet6"))
		return 1;

	if ((resolv_conf = fopen("/etc/resolv.conf", "r")))
	{
		char line[BUFSIZ];

		while (fgets(line, BUFSIZ, resolv_conf))
		{
			if (!strncmp(line, "options", 7) && strstr(line + 8, "inet6"))
			{
				fclose(resolv_conf);
				return 1;
			}
		}

		fclose(resolv_conf);
	}

	return 0;
}
#endif

int main(int ac, char **av)
{
	const char * const unixsock = "/tmp/libslack.net.test";
	pid_t pid;
	int server;
	int client;
	int errors = 0;
	char *format;
	void *a, *a2;
	char *z, *z2;
	char *b, *b2;
	char *h, *h2;
	signed char sc, sc2;
	unsigned char uc, uc2;
	signed short us, us2;
	unsigned short ss, ss2;
	signed int si, si2;
	unsigned int ui = 37, ui2;
	unsigned int uia, uia2;
	unsigned int uib, uib2;
	signed short sv1, sv2;
	unsigned short uv1, uv2;
	signed int sw1, sw2;
	unsigned int uw1, uw2;
	float f, f2;
	double da, da2;
	double db, db2;
	double dc, dc2;
	double dd, dd2;
	void *p, *p2;
#ifdef HAVE_LONG_LONG
	signed long long sl, sl2;
	unsigned long long ul, ul2;
#endif
	char pkt[1024];
	char tstmem[1024];
	ssize_t pkt_len;
	size_t length;
	struct passwd *pwd;
	int no_mailserver = 0;
	int no_multicast = 1;
	int no_rudp = 1;
	sockaddr_any_t addr;
	size_t addrsize = sizeof addr;
	int rc;

	unsigned char pkt_cmp[1024] =
	{
		0x00, 0x01, 0x02, 0x00, 0x03, 0x04, 0x00, 0x05, 0x06, 0x00,
		0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x77, 0x6f, 0x72, 0x6c,
		0x64, 0x00, 0x24, 0x92, 0x40, 0x01, 0x23, 0x45, 0x67, 0x89,
		0xab, 0xcd, 0xef, 0xfd, 0x03, 0xff, 0xfd, 0x00, 0x03, 0xff,
		0xff, 0xff, 0xfd, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
		0x06, 0xfd, 0xff, 0x03, 0x00, 0xfd, 0xff, 0xff, 0xff, 0x03,
		0x00, 0x00, 0x00, 0x06, 0x34, 0x33, 0x2e, 0x32, 0x31, 0x00,
		0x06, 0x31, 0x32, 0x2e, 0x33, 0x34, 0x00, 0x07, 0x2d, 0x31,
		0x32, 0x2e, 0x33, 0x34, 0x00, 0x08, 0x31, 0x2e, 0x35, 0x65,
		0x2b, 0x31, 0x30, 0x00, 0x09, 0x2d, 0x35, 0x2e, 0x31, 0x65,
		0x2d, 0x31, 0x30, 0x00
	};

#ifdef HAVE_LONG_LONG
	unsigned char lpkt_cmp[16] =
	{
		0xfe, 0xfd, 0xfc, 0xfb, 0xfa, 0xf9, 0xf8, 0xf7,
		0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08
	};
#endif

	if (ac == 2 && !strcmp(av[1], "help"))
	{
		printf("usage: %s [multicast|rudp]\n", *av);
		return EXIT_SUCCESS;
	}

	printf("Testing: %s\n", "net");

	/* Test TCP client and server sockets */

	if ((server = net_server(NULL, NULL, 30000, 0, 0, NULL, NULL)) == -1)
		++errors, printf("Test1: net_server(NULL, 30000) failed: %s\n", strerror(errno));
	else
	{
		switch (pid = fork())
		{
			case -1:
			{
				printf("Failed to fork (%s)\n", strerror(errno));
				return 1;
			}

			default:
			{
				int s;
				sockaddr_any_t addr;
				size_t addrsize = sizeof addr;

				if (read_timeout(server, 5, 0) == -1 || (s = accept(server, (sockaddr_t *)&addr, (void *)&addrsize)) == -1)
					++errors, printf("Test2: accept() failed (%s)\n", strerror(errno));
				else
				{
					char test[4];
					int bytes;

					if (read_timeout(s, 5, 0) == -1 || (bytes = read(s, test, 4)) == -1)
						++errors, printf("Test3: read(s, HELO) failed (%s)\n", strerror(errno));
					else if (bytes != 4)
						++errors, printf("Test4: read(s, HELO) failed (read %d bytes, not %d bytes)\n", bytes, 4);
					else if (memcmp(test, "HELO", 4))
						++errors, printf("Test4: read(s, HELO) failed (read \"%4.4s\", not \"%4.4s\")\n", test, "HELO");
					else if (write_timeout(s, 5, 0) == -1 || write(s, "OLEH", 4) == -1)
						++errors, printf("Test5: write(s, OLEH) failed (%s)\n", strerror(errno));
					if (close(s) == -1)
						++errors, printf("Test6: close(s) failed (%s)\n", strerror(errno));
				}

				errors += wait_for_child(pid);
				break;
			}

			case 0:
			{
				errors = 0;

				if ((client = net_client(NULL, NULL, 30000, 5, 0, 0, NULL, NULL)) == -1)
					++errors, printf("Test7: net_client(NULL, 30000) failed (%s)\n", strerror(errno));
				else
				{
					char test[4];

					if (write_timeout(client, 5, 0) == -1 || write(client, "HELO", 4) == -1)
						++errors, printf("Test8: write(client, HELO) failed (%s)\n", strerror(errno));
					else if (read_timeout(client, 5, 0) == -1 || read(client, test, 4) == -1)
						++errors, printf("Test9: read(client, OLEH) failed (%s)\n", strerror(errno));
					else if (memcmp(test, "OLEH", 4))
						++errors, printf("Test10: read(client, OLEH) failed (read \"%4.4s\", not \"%4.4s\")\n", test, "OLEH");
					if (close(client) == -1)
						++errors, printf("Test11: close(client) failed (%s)\n", strerror(errno));
				}

				return errors;
			}
		}

		if (close(server) == -1)
			++errors, printf("Test12: close(server) failed (%s)\n", strerror(errno));
	}

	/* Test UDP client and server sockets */

	if ((server = net_udp_server(NULL, NULL, 30000, 0, 0, NULL, NULL)) == -1)
		++errors, printf("Test13: net_udp_server(NULL, 30000) failed: %s\n", strerror(errno));
	else
	{
		switch (pid = fork())
		{
			case -1:
			{
				printf("Failed to fork (%s)\n", strerror(errno));
				return 1;
			}

			default:
			{
				char test[4];
				sockaddr_any_t addr;
				size_t addrsize = sizeof addr;

				if (read_timeout(server, 5, 0) == -1 || recvfrom(server, test, 4, 0, (sockaddr_t *)&addr, (void *)&addrsize) == -1)
					++errors, printf("Test14: recvfrom(server, HELO) failed (%s)\n", strerror(errno));
				else if (memcmp(test, "HELO", 4))
					++errors, printf("Test15: recvfrom(server, HELO) failed (recv \"%4.4s\", not \"%4.4s\")\n", test, "HELO");
				else if (write_timeout(server, 5, 0) == -1 || sendto(server, "OLEH", 4, 0, (sockaddr_t *)&addr, addrsize) == -1)
					++errors, printf("Test16: sendto(server, OLEH) failed (%s)\n", strerror(errno));

				errors += wait_for_child(pid);
				break;
			}

			case 0:
			{
				errors = 0;

				if ((client = net_udp_client(NULL, NULL, 30000, 0, 0, NULL, NULL)) == -1)
					++errors, printf("Test17: net_udp_client(NULL, 30000) failed (%s)\n", strerror(errno));
				else
				{
					char test[4];

					if (write_timeout(client, 5, 0) == -1 || send(client, "HELO", 4, 0) == -1)
						++errors, printf("Test18: send(client, HELO) failed (%s)\n", strerror(errno));
					else if (read_timeout(client, 5, 0) == -1 || recv(client, test, 4, 0) == -1)
						++errors, printf("Test19: recv(client, OLEH) failed (%s)\n", strerror(errno));
					else if (memcmp(test, "OLEH", 4))
						++errors, printf("Test20: recv(client, OLEH) failed (recv \"%4.4s\", not \"%4.4s\")\n", test, "OLEH");
					if (close(client) == -1)
						++errors, printf("Test21: close(client) failed (%s)\n", strerror(errno));
				}

				return errors;
			}
		}

		if (close(server) == -1)
			++errors, printf("Test22: close(server) failed (%s)\n", strerror(errno));
	}

	/* Test UNIX domain client and server stream sockets */

	if ((server = net_server("/unix", unixsock, 0, 0, 0, NULL, NULL)) == -1)
		++errors, printf("Test23: net_server(\"/unix\", \"%s\") failed: %s\n", unixsock, strerror(errno));
	else
	{
		switch (pid = fork())
		{
			case -1:
			{
				printf("Failed to fork (%s)\n", strerror(errno));
				return 1;
			}

			default:
			{
				int s;
				sockaddr_any_t addr;
				size_t addrsize = sizeof addr;

				if (read_timeout(server, 5, 0) == -1 || (s = accept(server, (sockaddr_t *)&addr, (void *)&addrsize)) == -1)
					++errors, printf("Test24: accept() failed (%s)\n", strerror(errno));
				else
				{
					char test[4];
					int bytes;

					if (read_timeout(s, 5, 0) == -1 || (bytes = read(s, test, 4)) == -1)
						++errors, printf("Test25: read(s, HELO) failed (%s)\n", strerror(errno));
					else if (bytes != 4)
						++errors, printf("Test26: read(s, HELO) failed (read %d bytes, not %d bytes)\n", bytes, 4);
					else if (memcmp(test, "HELO", 4))
						++errors, printf("Test26: read(s, HELO) failed (read \"%4.4s\", not \"%4.4s\")\n", test, "HELO");
					else if (write_timeout(s, 5, 0) == -1 || write(s, "OLEH", 4) == -1)
						++errors, printf("Test27: write(s, OLEH) failed (%s)\n", strerror(errno));
					if (close(s) == -1)
						++errors, printf("Test28: close(s) failed (%s)\n", strerror(errno));
				}

				errors += wait_for_child(pid);
				break;
			}

			case 0:
			{
				errors = 0;

				if ((client = net_client("/unix", unixsock, 0, 5, 0, 0, NULL, NULL)) == -1)
					++errors, printf("Test29: net_client(\"/unix\", \"%s\") failed (%s)\n", unixsock, strerror(errno));
				else
				{
					char test[4];

					if (write_timeout(client, 5, 0) == -1 || write(client, "HELO", 4) == -1)
						++errors, printf("Test30: write(client, HELO) failed (%s)\n", strerror(errno));
					else if (read_timeout(client, 5, 0) == -1 || read(client, test, 4) == -1)
						++errors, printf("Test31: read(client, OLEH) failed (%s)\n", strerror(errno));
					else if (memcmp(test, "OLEH", 4))
						++errors, printf("Test32: read(client, OLEH) failed (read \"%4.4s\", not \"%4.4s\")\n", test, "OLEH");
					if (close(client) == -1)
						++errors, printf("Test33: close(client) failed (%s)\n", strerror(errno));
				}

				return errors;
			}
		}

		if (close(server) == -1)
			++errors, printf("Test34: close(server) failed (%s)\n", strerror(errno));
	}

	unlink(unixsock);

	/* Test UNIX domain client and server datagram sockets */

	if ((server = net_udp_server("/unix", unixsock, 0, 0, 0, NULL, NULL)) == -1)
		++errors, printf("Test35: net_udp_server(\"/unix\", \"%s\") failed: %s\n", unixsock, strerror(errno));
	else
	{
		switch (pid = fork())
		{
			case -1:
			{
				printf("Failed to fork (%s)\n", strerror(errno));
				return 1;
			}

			default:
			{
				char test[4];
				sockaddr_any_t addr;
				size_t addrsize = sizeof addr;

				if (read_timeout(server, 5, 0) == -1 || recvfrom(server, test, 4, 0, (sockaddr_t *)&addr, (void *)&addrsize) == -1)
					++errors, printf("Test36: recvfrom(server, HELO) failed (%s)\n", strerror(errno));
				else if (memcmp(test, "HELO", 4))
					++errors, printf("Test37: recvfrom(server, HELO) failed (recv \"%4.4s\", not \"%4.4s\")\n", test, "HELO");
				else if (write_timeout(server, 5, 0) == -1 || sendto(server, "OLEH", 4, 0, (sockaddr_t *)&addr, addrsize) == -1)
					++errors, printf("Test38: sendto(server, OLEH) failed (%s)\n", strerror(errno));

				errors += wait_for_child(pid);
				break;
			}

			case 0:
			{
				errors = 0;

				if ((client = net_udp_client("/unix", unixsock, 0, 0, 0, NULL, NULL)) == -1)
					++errors, printf("Test39: net_udp_client(\"/unix\", \"%s\") failed (%s)\n", unixsock, strerror(errno));
				else
				{
					char test[4];
					sockaddr_any_t any;
					size_t size = sizeof any;

					if (write_timeout(client, 5, 0) == -1 || send(client, "HELO", 4, 0) == -1)
						++errors, printf("Test40: send(client, HELO) failed (%s)\n", strerror(errno));
					else if (read_timeout(client, 5, 0) == -1 || recv(client, test, 4, 0) == -1)
						++errors, printf("Test41: recv(client, OLEH) failed (%s)\n", strerror(errno));
					else if (memcmp(test, "OLEH", 4))
						++errors, printf("Test42: recv(client, OLEH) failed (recv \"%4.4s\", not \"%4.4s\")\n", test, "OLEH");
					if (getsockname(client, (sockaddr_t *)&any, (void *)&size) != -1)
						if (*any.un.sun_path)
							unlink(any.un.sun_path);
					if (close(client) == -1)
						++errors, printf("Test43: close(client) failed (%s)\n", strerror(errno));
				}

				return errors;
			}
		}

		if (close(server) == -1)
			++errors, printf("Test44: close(server) failed (%s)\n", strerror(errno));
	}

	unlink(unixsock);

	/* Test pack() and unpack() */

	format = "a10z*b*h*c2s2i3v2w2fd4pxX@*";
	a = "\000\001\002\000\003\004\000\005\006\000";
	z = "hello world";
	b = "001001001001001001";
	h = "0123456789abcdef";
	sc = -3; uc = 3;
	ss = -3; us = 3;
	si = -3; uia = 3; uib = 6;
	sv1 = -3; uv1 = 3;
	sw1 = -3; uw1 = 3;
	f = 43.21;
	da = 12.34; db = -12.34; dc = 1.5e10; dd = -5.1e-10;
	p = a;

	pkt_len = pack(pkt, 1024, format,
		a,
		strlen(z) + 1, z,
		strlen(b), b,
		strlen(h), h,
		sc, uc,
		ss, us,
		si, uia, uib,
		sv1, uv1, sw1, uw1,
		f, da, db, dc, dd,
		p,
		1024
	);

	if (pkt_len == -1)
		++errors, printf("Test45: pack(\"%s\") failed (%s)\n", format, strerror(errno));
	else if (pkt_len != 1024)
		++errors, printf("Test45: pack(\"%s\") failed (returned %d, not %d)\n", format, (int)pkt_len, 1024);
	else
	{
		a2 = malloc(10);
		z2 = malloc(strlen(z) + 1);
		b2 = malloc(strlen(b) + 1);
		h2 = malloc(strlen(h) + 1);

		if (a2 && z2 && b2 && h2)
		{
			pkt_len = unpack(pkt, pkt_len, format,
				a2,
				strlen(z) + 1, z2,
				strlen(b), b2,
				strlen(h), h2,
				&sc2, &uc2,
				&ss2, &us2,
				&si2, &uia2, &uib2,
				&sv2, &uv2, &sw2, &uw2,
				&f2, &da2, &db2, &dc2, &dd2,
				&p2,
				1024
			);

			if (pkt_len == -1)
				++errors, printf("Test46: unpack(\"%s\") failed (%s)\n", format, strerror(errno));
			else if (pkt_len != 1024)
				++errors, printf("Test46: unpack(\"%s\") failed (returned %d, not %d)\n", format, (int)pkt_len, 1024);
			else
			{
				if (memcmp(a, a2, 10))
					++errors, printf("Test47: pack(a) failed\n");
				if (strcmp(z, z2))
					++errors, printf("Test48: pack(z) failed (packed %s, unpacked %s)\n", z, z2);
				if (strcmp(b, b2))
					++errors, printf("Test49: pack(b) failed (packed %s, unpacked %s)\n", b, b2);
				if (strcmp(h, h2))
					++errors, printf("Test50: pack(h) failed (packed %s, unpacked %s)\n", h, h2);
				if (sc != sc2)
					++errors, printf("Test51: pack(c) failed (packed %d, unpacked %d)\n", sc, sc2);
				if (uc != uc2)
					++errors, printf("Test52: pack(C) failed (packed %u, unpacked %u)\n", uc, uc2);
				if (ss != ss2)
					++errors, printf("Test53: pack(s) failed (packed %d, unpacked %d)\n", sc, sc2);
				if (us != us2)
					++errors, printf("Test54: pack(S) failed (packed %u, unpacked %u)\n", us, us2);
				if (si != si2)
					++errors, printf("Test55: pack(i) failed (packed %d, unpacked %d)\n", si, si2);
				if (uia != uia2)
					++errors, printf("Test56: pack(I) failed (packed %u, unpacked %u)\n", uia, uia2);
				if (uib != uib2)
					++errors, printf("Test57: pack(I) failed (packed %u, unpacked %u)\n", uib, uib2);
				if (sv1 != sv2)
					++errors, printf("Test58: pack(v) failed (packed %d, unpacked %d)\n", sv1, sv2);
				if (uv1 != uv2)
					++errors, printf("Test59: pack(V) failed (packed %u, unpacked %u)\n", uv1, uv2);
				if (sw1 != sw2)
					++errors, printf("Test60: pack(w) failed (packed %d, unpacked %d)\n", sw1, sw2);
				if (uw1 != uw2)
					++errors, printf("Test61: pack(W) failed (packed %u, unpacked %u)\n", uw1, uw2);
				if (f != f2)
					++errors, printf("Test62: pack(f) failed (packed %g, unpacked %g)\n", (double)f, (double)f2);
				if (da != da2)
					++errors, printf("Test63: pack(d) failed (packed %g, unpacked %g)\n", da, da2);
				if (db != db2)
					++errors, printf("Test64: pack(d) failed (packed %g, unpacked %g)\n", db, db2);
				if (dc != dc2)
					++errors, printf("Test65: pack(d) failed (packed %g, unpacked %g)\n", dc, dc2);
				if (dd != dd2)
					++errors, printf("Test66: pack(d) failed (packed %g, unpacked %g)\n", dd, dd2);
				if (p != p2)
					++errors, printf("Test67: pack(p) failed (packed %p, unpacked %p)\n", p, p2);
			}

			/* Test unpack with skipping */

			memset(z2, 0, strlen(z) + 1);
			sc2 = 0;
			uc2 = 0;
			ss2 = 0;
			si2 = 0;
			uib2 = 0;
			da2 = 0.0;
			dc2 = 0.0;
			p2 = NULL;

			pkt_len = unpack(pkt, pkt_len, format,
				NULL,
				strlen(z) + 1, z2,
				strlen(b), NULL,
				strlen(h), NULL,
				&sc2, &uc2,
				&ss2, NULL,
				&si2, NULL, &uib2,
				NULL, NULL, NULL, NULL,
				NULL, &da2, NULL, &dc2, NULL,
				&p2,
				1024
			);

			if (pkt_len == -1)
				++errors, printf("Test68: unpack(\"%s\", NULL) failed (%s)\n", format, strerror(errno));
			else if (pkt_len != 1024)
				++errors, printf("Test68: unpack(\"%s\", NULL) failed (returned %d, not %d)\n", format, (int)pkt_len, 1024);
			else
			{
				if (strcmp(z, z2))
					++errors, printf("Test69: pack(z, NULL) failed (packed %s, unpacked %s)\n", z, z2);
				if (sc != sc2)
					++errors, printf("Test70: pack(c, NULL) failed (packed %d, unpacked %d)\n", sc, sc2);
				if (uc != uc2)
					++errors, printf("Test71: pack(C, NULL) failed (packed %u, unpacked %u)\n", uc, uc2);
				if (ss != ss2)
					++errors, printf("Test72: pack(s, NULL) failed (packed %d, unpacked %d)\n", sc, sc2);
				if (si != si2)
					++errors, printf("Test73: pack(i, NULL) failed (packed %d, unpacked %d)\n", si, si2);
				if (uib != uib2)
					++errors, printf("Test74: pack(I, NULL) failed (packed %u, unpacked %u)\n", uib, uib2);
				if (da != da2)
					++errors, printf("Test75: pack(d, NULL) failed (packed %g, unpacked %g)\n", da, da2);
				if (dc != dc2)
					++errors, printf("Test76: pack(d, NULL) failed (packed %g, unpacked %g)\n", dc, dc2);
				if (p != p2)
					++errors, printf("Test77: pack(p, NULL) failed (packed %p, unpacked %p)\n", p, p2);
			}
		}
		else
			++errors, printf("Test46: failed to run (%s)\n", strerror(errno));

		free(a2);
		free(z2);
		free(b2);
		free(h2);

		/* Test binary compatibility (ignoring the packed pointer) */

		if (memcmp(pkt, pkt_cmp, 104) || memcmp(pkt + 104 + sizeof(void *), pkt_cmp + 104 + sizeof(void *), 1024 - (104 + sizeof(void *))))
		{
			++errors, printf("Test78: pack(\"%s\") failed (packed data looks wrong)\n", format);
			print_pkt("good packet", pkt_cmp, 1024);
			print_pkt("bad packet", pkt, 1024);
		}
	}

#ifdef HAVE_LONG_LONG
	sl = ~(((signed long long)0x01020304 << 32) | 0x05060708);
	ul = ((unsigned long long)0x01020304 << 32) | 0x05060708;

	pkt_len = pack(pkt, 1024, "l2", sl, ul);

	if (pkt_len == -1)
		++errors, printf("Test79: pack(\"l2\") failed (%s)\n", strerror(errno));
	else if (pkt_len != 16)
		++errors, printf("Test79: pack(\"l2\") failed (returned %d, not %d)\n", (int)pkt_len, 16);
	else
	{
		pkt_len = unpack(pkt, 16, "l2", &sl2, &ul2);

		if (pkt_len == -1)
			++errors, printf("Test80: unpack(\"l2\") failed (%s)\n", strerror(errno));
		else if (pkt_len != 16)
			++errors, printf("Test80: unpack(\"l2\") failed (returned %d, not %d)\n", (int)pkt_len, 16);
		else
		{
			if (sl != sl2)
				++errors, printf("Test81: pack(\"l2\") failed\n");
			if (ul != ul2)
				++errors, printf("Test82: pack(\"l2\") failed\n");
		}

		/* Test long long binary compatibility */

		if (memcmp(pkt, lpkt_cmp, 16))
		{
			++errors, printf("Test83: pack(\"l2\") failed (packed data looks wrong)\n");
			print_pkt("good packet", lpkt_cmp, 16);
			print_pkt("bad packet", pkt, 16);
		}
	}
#endif

	/* Test packing sizes */

#define TEST_SIZE(i, format, size, pformat, cast_type, data1, data2, data2ref, test) \
	pkt_len = pack(pkt, (size), (format), (data1)); \
	if (pkt_len == -1) \
		++errors, printf("Test%d: pack(%d, \"%s\") failed (%s)\n", (i), (size), (format), strerror(errno)); \
	else if ((size) && pkt_len != (size)) \
		++errors, printf("Test%d: pack(%d, \"%s\") failed (size = %d, not %d)\n", (i), (size), (format), (int)pkt_len, (size)); \
	else \
	{ \
		pkt_len = unpack(pkt, (size), (format), (data2ref)); \
		if (pkt_len == -1) \
			++errors, printf("Test%d: unpack(%d, \"%s\") failed (%s)\n", (i), (size), (format), strerror(errno)); \
		else if ((size) && pkt_len != (size)) \
			++errors, printf("Test%d: unpack(%d, \"%s\") failed (size = %d, not %d)\n", (i), (size), (format), (int)pkt_len, (size)); \
		else if (test) \
		{ \
			char a[128], b[128]; \
			snprintf(a, 128, pformat, (cast_type) data1); \
			snprintf(b, 128, pformat, (cast_type) data2); \
			++errors, printf("Test%d: unpack(%d, \"%s\") failed (%s != %s)\n", (i), (size), (format), a, b); \
		} \
	}

#define TEST_SINT(i, format, size, data1, data2) TEST_SIZE(i, format, size, "%ld", signed   long, data1, data2, &data2, data2 != data1)
#define TEST_UINT(i, format, size, data1, data2) TEST_SIZE(i, format, size, "%lu", unsigned long, data1, data2, &data2, data2 != data1)
#define TEST_STR(i, format, size, pformat, len, data1, data2) TEST_SIZE(i, format, size, pformat, char *, data1, data2, data2, memcmp(data1, data2, len))

	TEST_SINT(84, "c", 1, sc, sc2)
	TEST_UINT(85, "c", 1, uc, uc2)
	TEST_SINT(86, "s", 2, ss, ss2)
	TEST_UINT(87, "s", 2, us, us2)
	TEST_SINT(88, "i", 4, si, si2)
	TEST_UINT(89, "i", 4, ui, ui2)
	TEST_UINT(90, "v", 2, sv1, sv2)
	TEST_UINT(91, "v", 2, uv1, uv2)
	TEST_SINT(92, "w", 4, sw1, sw2)
	TEST_SINT(93, "w", 4, uw1, uw2)
#ifdef HAVE_LONG_LONG
	TEST_SINT(94, "l", 8, sl, sl2)
	TEST_UINT(95, "l", 8, ul, ul2)
#endif

	TEST_STR(96, "b1", 1, "%1.1s", 1, b, tstmem)
	TEST_STR(97, "b2", 1, "%2.2s", 2, b, tstmem)
	TEST_STR(98, "b3", 1, "%3.3s", 3, b, tstmem)
	TEST_STR(99, "b4", 1, "%4.4s", 4, b, tstmem)
	TEST_STR(100, "b5", 1, "%5.5s", 5, b, tstmem)
	TEST_STR(101, "b6", 1, "%6.6s", 6, b, tstmem)
	TEST_STR(102, "b7", 1, "%7.7s", 7, b, tstmem)
	TEST_STR(103, "b8", 1, "%8.8s", 8, b, tstmem)
	TEST_STR(104, "b9", 2, "%9.9s", 9, b, tstmem)
	TEST_STR(105, "b10", 2, "%10.10s", 10, b, tstmem)
	TEST_STR(106, "b11", 2, "%11.11s", 11, b, tstmem)
	TEST_STR(107, "b12", 2, "%12.12s", 12, b, tstmem)
	TEST_STR(108, "b13", 2, "%13.13s", 13, b, tstmem)
	TEST_STR(109, "b14", 2, "%14.14s", 14, b, tstmem)
	TEST_STR(110, "b15", 2, "%15.15s", 15, b, tstmem)
	TEST_STR(111, "b16", 2, "%16.16s", 16, b, tstmem)
	TEST_STR(112, "b17", 3, "%17.17s", 17, b, tstmem)
	TEST_STR(113, "b18", 3, "%18.18s", 18, b, tstmem)

	TEST_STR(114, "h1", 1, "%1.1s", 1, h, tstmem)
	TEST_STR(115, "h2", 1, "%2.2s", 2, h, tstmem)
	TEST_STR(116, "h3", 2, "%3.3s", 3, h, tstmem)
	TEST_STR(117, "h4", 2, "%4.4s", 4, h, tstmem)
	TEST_STR(118, "h5", 3, "%5.5s", 5, h, tstmem)
	TEST_STR(119, "h6", 3, "%6.6s", 6, h, tstmem)
	TEST_STR(120, "h7", 4, "%7.7s", 7, h, tstmem)
	TEST_STR(121, "h8", 4, "%8.8s", 8, h, tstmem)
	TEST_STR(122, "h9", 5, "%9.9s", 9, h, tstmem)

	/* Test unpacking with '?' */

#define TEST_VARSIZE(i, pformat, uformat, size, len1, data1, len2ref, data2limit, data2) \
	pkt_len = pack(pkt, (size), (pformat), (len1), (len1), (data1)); \
	if (pkt_len == -1) \
		++errors, printf("Test%d: pack(%d, \"%s\") failed (%s)\n", (i), (size), (pformat), strerror(errno)); \
	else if ((size) && pkt_len != (size)) \
		++errors, printf("Test%d: pack(%d, \"%s\") failed (size = %d, not %d)\n", (i), (size), (pformat), (int)pkt_len, (size)); \
	else \
	{ \
		pkt_len = unpack(pkt, (size), (uformat), (len2ref), (len2ref), (len1), (data2)); \
		if (pkt_len == -1) \
			++errors, printf("Test%d: unpack(%d, \"%s\") failed (%s)\n", (i), (size), (uformat), strerror(errno)); \
		else if ((size) && pkt_len != (size)) \
			++errors, printf("Test%d: unpack(%d, \"%s\") failed (size = %d, not %d)\n", (i), (size), (uformat), (int)pkt_len, (size)); \
		else if (*(len2ref) != (len1)) \
			++errors, printf("Test%d: unpack(%d, \"%s\") failed (unpacked length field = %d, not %d\n", (i), (size), (pformat), *(len2ref), (len1)); \
		else if (memcmp((data1), (data2), (len1))) \
			++errors, printf("Test%d: unpack(%d, \"%s\") failed (%.*s != %.*s)\n", (i), (size), (format), (len1), (char *)(data1), (len1), (char *)(data2)); \
	}

	a2 = malloc(10);
	z2 = malloc(strlen(z) + 1);
	b2 = malloc(strlen(b) + 1);
	h2 = malloc(strlen(h) + 1);
	length = 10; TEST_VARSIZE(123, "ia*", "ia?", 14, 10, a, (int *)&length, 10, a2)
	length = 11; TEST_VARSIZE(124, "iz*", "iz?", 15, 11, z, (int *)&length, 11, z2)
	length = 18; TEST_VARSIZE(125, "ib*", "ib?", 7, 18, b, (int *)&length, 18, b2)
	length = 16; TEST_VARSIZE(126, "ih*", "ih?", 12, 16, h, (int *)&length, 16, h2)
	free(a2);
	free(z2);
	free(b2);
	free(h2);

	/* Test error reporting */

#define TEST_FAILURE(i, test, error) \
	pkt_len = test; \
	if (pkt_len != -1) \
		++errors, printf("Test%d: %s error failed (size %d, no error, not %s)\n", (i), #test, (int)pkt_len, strerror(error)); \
	else if (errno != error) \
		++errors, printf("Test%d: %s error failed (%s, not %s)\n", (i), #test, strerror(errno), strerror(error));

	TEST_FAILURE(127, pack(NULL, 1, "a", a), EINVAL)
	TEST_FAILURE(128, pack(pkt, 1, NULL, a), EINVAL)

	TEST_FAILURE(129, pack(pkt, 1, "a0", a), EINVAL)
	TEST_FAILURE(130, pack(pkt, 1, "a*", 0, a), EINVAL)
	TEST_FAILURE(131, pack(pkt, 1, "a*", -1, a), EINVAL)
	TEST_FAILURE(132, pack(pkt, 0, "a", a), ENOSPC)
	TEST_FAILURE(133, pack(pkt, 1, "a2", a), ENOSPC)
	TEST_FAILURE(134, pack(pkt, 1, "a*", 2, a), ENOSPC)
	TEST_FAILURE(135, pack(pkt, 1, "a", NULL), EINVAL)

	TEST_FAILURE(136, pack(pkt, 1, "z0", z), EINVAL)
	TEST_FAILURE(137, pack(pkt, 1, "z*", 0, z), EINVAL)
	TEST_FAILURE(138, pack(pkt, 1, "z*", -1, z), EINVAL)
	TEST_FAILURE(139, pack(pkt, 0, "z", z), ENOSPC)
	TEST_FAILURE(140, pack(pkt, 1, "z2", z), ENOSPC)
	TEST_FAILURE(141, pack(pkt, 1, "z*", 2, z), ENOSPC)
	TEST_FAILURE(142, pack(pkt, 1, "z", NULL), EINVAL)

	TEST_FAILURE(143, pack(pkt, 1, "b0", b), EINVAL)
	TEST_FAILURE(144, pack(pkt, 1, "b*", 0, b), EINVAL)
	TEST_FAILURE(145, pack(pkt, 1, "b*", -1, b), EINVAL)
	TEST_FAILURE(146, pack(pkt, 0, "b", b), ENOSPC)
	TEST_FAILURE(147, pack(pkt, 1, "b9", b), ENOSPC)
	TEST_FAILURE(148, pack(pkt, 1, "b*", 9, b), ENOSPC)
	TEST_FAILURE(149, pack(pkt, 1, "b", NULL), EINVAL)
	TEST_FAILURE(150, pack(pkt, 1, "b8", a), EINVAL)
	TEST_FAILURE(151, pack(pkt, 1, "b8", h), EINVAL)

	TEST_FAILURE(152, pack(pkt, 1, "h0", h), EINVAL)
	TEST_FAILURE(153, pack(pkt, 1, "h*", 0, h), EINVAL)
	TEST_FAILURE(154, pack(pkt, 1, "h*", -1, h), EINVAL)
	TEST_FAILURE(155, pack(pkt, 0, "h", h), ENOSPC)
	TEST_FAILURE(156, pack(pkt, 1, "h3", h), ENOSPC)
	TEST_FAILURE(157, pack(pkt, 1, "h*", 3, h), ENOSPC)
	TEST_FAILURE(158, pack(pkt, 1, "h", NULL), EINVAL)
	TEST_FAILURE(159, pack(pkt, 1, "h", a), EINVAL)

	TEST_FAILURE(160, pack(pkt, 1, "c0", sc), EINVAL)
	TEST_FAILURE(161, pack(pkt, 1, "c*", 0, sc), EINVAL)
	TEST_FAILURE(162, pack(pkt, 1, "c*", -1, sc), EINVAL)
	TEST_FAILURE(163, pack(pkt, 0, "c", sc), ENOSPC)
	TEST_FAILURE(164, pack(pkt, 1, "c2", sc, sc), ENOSPC)
	TEST_FAILURE(165, pack(pkt, 1, "c*", 2, sc, sc), ENOSPC)

	TEST_FAILURE(166, pack(pkt, 1, "c0", uc), EINVAL)
	TEST_FAILURE(167, pack(pkt, 1, "c*", 0, uc), EINVAL)
	TEST_FAILURE(168, pack(pkt, 1, "c*", -1, uc), EINVAL)
	TEST_FAILURE(169, pack(pkt, 0, "c", uc), ENOSPC)
	TEST_FAILURE(170, pack(pkt, 1, "c2", uc, uc), ENOSPC)
	TEST_FAILURE(171, pack(pkt, 1, "c*", 2, uc, uc), ENOSPC)

	TEST_FAILURE(172, pack(pkt, 1, "s0", ss), EINVAL)
	TEST_FAILURE(173, pack(pkt, 1, "s*", 0, ss), EINVAL)
	TEST_FAILURE(174, pack(pkt, 1, "s*", -1, ss), EINVAL)
	TEST_FAILURE(175, pack(pkt, 0, "s", ss), ENOSPC)
	TEST_FAILURE(176, pack(pkt, 1, "s", ss), ENOSPC)
	TEST_FAILURE(177, pack(pkt, 2, "s2", ss, ss), ENOSPC)
	TEST_FAILURE(178, pack(pkt, 3, "s*", 2, ss, ss), ENOSPC)

	TEST_FAILURE(179, pack(pkt, 1, "s0", us), EINVAL)
	TEST_FAILURE(180, pack(pkt, 1, "s*", 0, us), EINVAL)
	TEST_FAILURE(181, pack(pkt, 1, "s*", -1, us), EINVAL)
	TEST_FAILURE(182, pack(pkt, 0, "s", us), ENOSPC)
	TEST_FAILURE(183, pack(pkt, 1, "s", us), ENOSPC)
	TEST_FAILURE(184, pack(pkt, 2, "s2", us, us), ENOSPC)
	TEST_FAILURE(185, pack(pkt, 3, "s*", 2, us, us), ENOSPC)

	TEST_FAILURE(186, pack(pkt, 1, "i0", si), EINVAL)
	TEST_FAILURE(187, pack(pkt, 1, "i*", 0, si), EINVAL)
	TEST_FAILURE(188, pack(pkt, 1, "i*", -1, si), EINVAL)
	TEST_FAILURE(189, pack(pkt, 0, "i", si), ENOSPC)
	TEST_FAILURE(190, pack(pkt, 1, "i", si), ENOSPC)
	TEST_FAILURE(191, pack(pkt, 2, "i", si), ENOSPC)
	TEST_FAILURE(192, pack(pkt, 3, "i", si), ENOSPC)
	TEST_FAILURE(193, pack(pkt, 4, "i2", si, si), ENOSPC)
	TEST_FAILURE(194, pack(pkt, 5, "i*", 2, si, si), ENOSPC)

	TEST_FAILURE(195, pack(pkt, 1, "i0", ui), EINVAL)
	TEST_FAILURE(196, pack(pkt, 1, "i*", 0, ui), EINVAL)
	TEST_FAILURE(197, pack(pkt, 1, "i*", -1, ui), EINVAL)
	TEST_FAILURE(198, pack(pkt, 0, "i", ui), ENOSPC)
	TEST_FAILURE(199, pack(pkt, 1, "i", ui), ENOSPC)
	TEST_FAILURE(200, pack(pkt, 2, "i", ui), ENOSPC)
	TEST_FAILURE(201, pack(pkt, 3, "i", ui), ENOSPC)
	TEST_FAILURE(202, pack(pkt, 4, "i2", ui, ui), ENOSPC)
	TEST_FAILURE(203, pack(pkt, 5, "i*", 2, ui, ui), ENOSPC)

	TEST_FAILURE(204, pack(pkt, 1, "v0", sv1), EINVAL)
	TEST_FAILURE(205, pack(pkt, 1, "v*", 0, sv1), EINVAL)
	TEST_FAILURE(206, pack(pkt, 1, "v*", -1, sv1), EINVAL)
	TEST_FAILURE(207, pack(pkt, 0, "v", sv1), ENOSPC)
	TEST_FAILURE(208, pack(pkt, 1, "v", sv1), ENOSPC)
	TEST_FAILURE(209, pack(pkt, 2, "v2", sv1, sv2), ENOSPC)
	TEST_FAILURE(210, pack(pkt, 3, "v*", 2, sv1, sv2), ENOSPC)

	TEST_FAILURE(211, pack(pkt, 1, "v0", uv1), EINVAL)
	TEST_FAILURE(212, pack(pkt, 1, "v*", 0, uv1), EINVAL)
	TEST_FAILURE(213, pack(pkt, 1, "v*", -1, uv1), EINVAL)
	TEST_FAILURE(214, pack(pkt, 0, "v", uv1), ENOSPC)
	TEST_FAILURE(215, pack(pkt, 1, "v", uv1), ENOSPC)
	TEST_FAILURE(216, pack(pkt, 2, "v2", uv1, uv2), ENOSPC)
	TEST_FAILURE(217, pack(pkt, 3, "v*", 2, uv1, uv2), ENOSPC)

	TEST_FAILURE(218, pack(pkt, 1, "w0", sw1), EINVAL)
	TEST_FAILURE(219, pack(pkt, 1, "w*", 0, sw1), EINVAL)
	TEST_FAILURE(220, pack(pkt, 1, "w*", -1, sw1), EINVAL)
	TEST_FAILURE(221, pack(pkt, 0, "w", sw1), ENOSPC)
	TEST_FAILURE(222, pack(pkt, 1, "w", sw1), ENOSPC)
	TEST_FAILURE(223, pack(pkt, 2, "w", uv1), ENOSPC)
	TEST_FAILURE(224, pack(pkt, 3, "w", uv1), ENOSPC)
	TEST_FAILURE(225, pack(pkt, 4, "w2", sw1, sw2), ENOSPC)
	TEST_FAILURE(226, pack(pkt, 5, "w*", 2, sw1, sw2), ENOSPC)

	TEST_FAILURE(227, pack(pkt, 1, "w0", uw1), EINVAL)
	TEST_FAILURE(228, pack(pkt, 1, "w*", 0, uw1), EINVAL)
	TEST_FAILURE(229, pack(pkt, 1, "w*", -1, uw1), EINVAL)
	TEST_FAILURE(230, pack(pkt, 0, "w", uw1), ENOSPC)
	TEST_FAILURE(231, pack(pkt, 1, "w", uw1), ENOSPC)
	TEST_FAILURE(232, pack(pkt, 2, "w", uw1), ENOSPC)
	TEST_FAILURE(233, pack(pkt, 3, "w", uw1), ENOSPC)
	TEST_FAILURE(234, pack(pkt, 4, "w2", uw1, uw2), ENOSPC)
	TEST_FAILURE(235, pack(pkt, 5, "w*", 2, uw1, uw2), ENOSPC)

	TEST_FAILURE(236, pack(pkt, 1, "p0", p), EINVAL)
	TEST_FAILURE(237, pack(pkt, 1, "p*", 0, p), EINVAL)
	TEST_FAILURE(238, pack(pkt, 1, "p*", -1, p), EINVAL)
	TEST_FAILURE(239, pack(pkt, 0, "p", p), ENOSPC)
	TEST_FAILURE(240, pack(pkt, 1, "p", p), ENOSPC)
	TEST_FAILURE(241, pack(pkt, 2, "p", p), ENOSPC)
	TEST_FAILURE(242, pack(pkt, 3, "p", p), ENOSPC)
	TEST_FAILURE(243, pack(pkt, 4, "p2", p, p), ENOSPC)
	TEST_FAILURE(244, pack(pkt, 5, "p*", 2, p, p), ENOSPC)

#ifdef HAVE_LONG_LONG
	TEST_FAILURE(245, pack(pkt, 1, "l0", sl), EINVAL)
	TEST_FAILURE(246, pack(pkt, 1, "l*", 0, sl), EINVAL)
	TEST_FAILURE(247, pack(pkt, 1, "l*", -1, sl), EINVAL)
	TEST_FAILURE(248, pack(pkt, 0, "l", sl), ENOSPC)
	TEST_FAILURE(249, pack(pkt, 1, "l", sl), ENOSPC)
	TEST_FAILURE(250, pack(pkt, 2, "l", sl), ENOSPC)
	TEST_FAILURE(251, pack(pkt, 3, "l", sl), ENOSPC)
	TEST_FAILURE(252, pack(pkt, 4, "l", sl), ENOSPC)
	TEST_FAILURE(253, pack(pkt, 5, "l", sl), ENOSPC)
	TEST_FAILURE(254, pack(pkt, 6, "l", sl), ENOSPC)
	TEST_FAILURE(255, pack(pkt, 7, "l", sl), ENOSPC)
	TEST_FAILURE(256, pack(pkt, 8, "l2", sl, sl), ENOSPC)
	TEST_FAILURE(257, pack(pkt, 9, "l*", 2, sl, sl), ENOSPC)

	TEST_FAILURE(258, pack(pkt, 1, "l0", ul), EINVAL)
	TEST_FAILURE(259, pack(pkt, 1, "l*", 0, ul), EINVAL)
	TEST_FAILURE(260, pack(pkt, 1, "l*", -1, ul), EINVAL)
	TEST_FAILURE(261, pack(pkt, 0, "l", ul), ENOSPC)
	TEST_FAILURE(262, pack(pkt, 1, "l", ul), ENOSPC)
	TEST_FAILURE(263, pack(pkt, 2, "l", ul), ENOSPC)
	TEST_FAILURE(264, pack(pkt, 3, "l", ul), ENOSPC)
	TEST_FAILURE(265, pack(pkt, 4, "l", ul), ENOSPC)
	TEST_FAILURE(266, pack(pkt, 5, "l", ul), ENOSPC)
	TEST_FAILURE(267, pack(pkt, 6, "l", ul), ENOSPC)
	TEST_FAILURE(268, pack(pkt, 7, "l", ul), ENOSPC)
	TEST_FAILURE(269, pack(pkt, 8, "l2", ul, ul), ENOSPC)
	TEST_FAILURE(270, pack(pkt, 9, "l*", 2, ul, ul), ENOSPC)
#else
	TEST_FAILURE(271, pack(pkt, 8, "l", 0), ENOSYS)
	TEST_FAILURE(272, pack(pkt, 8, "l", 0), ENOSYS)
#endif

	f = -1.5; /* packed length = 6 */
	TEST_FAILURE(273, pack(pkt, 1, "f0", f), EINVAL)
	TEST_FAILURE(274, pack(pkt, 1, "f*", 0, f), EINVAL)
	TEST_FAILURE(275, pack(pkt, 1, "f*", -1, f), EINVAL)
	TEST_FAILURE(276, pack(pkt, 0, "f", f), ENOSPC)
	TEST_FAILURE(277, pack(pkt, 1, "f", f), ENOSPC)
	TEST_FAILURE(278, pack(pkt, 2, "f", f), ENOSPC)
	TEST_FAILURE(279, pack(pkt, 3, "f", f), ENOSPC)
	TEST_FAILURE(280, pack(pkt, 4, "f", f), ENOSPC)
	TEST_FAILURE(281, pack(pkt, 5, "f", f), ENOSPC)
	TEST_FAILURE(282, pack(pkt, 6, "f2", f, f), ENOSPC)
	TEST_FAILURE(283, pack(pkt, 7, "f*", 2, f, f), ENOSPC)

	da = db = 1.5; /* packed length = 5 */
	TEST_FAILURE(284, pack(pkt, 1, "d0", da), EINVAL)
	TEST_FAILURE(285, pack(pkt, 1, "d*", 0, da), EINVAL)
	TEST_FAILURE(286, pack(pkt, 1, "d*", -1, da), EINVAL)
	TEST_FAILURE(287, pack(pkt, 0, "d", da), ENOSPC)
	TEST_FAILURE(288, pack(pkt, 1, "d", da), ENOSPC)
	TEST_FAILURE(289, pack(pkt, 2, "d", da), ENOSPC)
	TEST_FAILURE(290, pack(pkt, 3, "d", da), ENOSPC)
	TEST_FAILURE(291, pack(pkt, 4, "d", da), ENOSPC)
	TEST_FAILURE(292, pack(pkt, 5, "d2", da, db), ENOSPC)
	TEST_FAILURE(293, pack(pkt, 6, "d*", 2, da, db), ENOSPC)

	TEST_FAILURE(294, pack(pkt, 1, "x0"), EINVAL)
	TEST_FAILURE(295, pack(pkt, 1, "x*", 0), EINVAL)
	TEST_FAILURE(296, pack(pkt, 1, "x*", -1), EINVAL)
	TEST_FAILURE(297, pack(pkt, 0, "x"), ENOSPC)
	TEST_FAILURE(298, pack(pkt, 1, "x2"), ENOSPC)
	TEST_FAILURE(299, pack(pkt, 1, "x*", 2), ENOSPC)

	TEST_FAILURE(300, pack(pkt, 1, "X0"), EINVAL)
	TEST_FAILURE(301, pack(pkt, 1, "X*", 0), EINVAL)
	TEST_FAILURE(302, pack(pkt, 1, "X*", -1), EINVAL)
	TEST_FAILURE(303, pack(pkt, 0, "X"), EINVAL)
	TEST_FAILURE(304, pack(pkt, 1, "X"), EINVAL)
	TEST_FAILURE(305, pack(pkt, 1, "X2"), EINVAL)
	TEST_FAILURE(306, pack(pkt, 1024, "X*", 2), EINVAL)

	TEST_FAILURE(307, pack(pkt, 1, "@0"), EINVAL)
	TEST_FAILURE(308, pack(pkt, 1, "@*", 0), EINVAL)
	TEST_FAILURE(309, pack(pkt, 1, "@*", -1), EINVAL)
	TEST_FAILURE(310, pack(pkt, 0, "@"), ENOSPC)
	TEST_FAILURE(311, pack(pkt, 0, "@1"), ENOSPC)
	TEST_FAILURE(312, pack(pkt, 1, "@2"), ENOSPC)
	TEST_FAILURE(313, pack(pkt, 1, "@*", 2), ENOSPC)

	TEST_FAILURE(314, pack(pkt, 1, "?", 0), EINVAL)

	length = 11;
	TEST_FAILURE(315, unpack(NULL, 1, "a", a), EINVAL)
	TEST_FAILURE(316, unpack(pkt, 1, NULL, a), EINVAL)

	TEST_FAILURE(317, unpack(pkt, 1, "a0", a), EINVAL)
	TEST_FAILURE(318, unpack(pkt, 1, "a*", 0, a), EINVAL)
	TEST_FAILURE(319, unpack(pkt, 1, "a*", -1, a), EINVAL)
	TEST_FAILURE(320, unpack(pkt, 0, "a", a), ENOSPC)
	TEST_FAILURE(321, unpack(pkt, 1, "a2", a), ENOSPC)
	TEST_FAILURE(322, unpack(pkt, 1, "a*", 2, a), ENOSPC)
	TEST_FAILURE(323, unpack(pkt, 0, "a", NULL), ENOSPC)
	TEST_FAILURE(324, unpack(pkt, 10, "a?", NULL, 10, a), EINVAL)
	TEST_FAILURE(325, unpack(pkt, 10, "a?", &length, 10, a), ENOSPC)

	TEST_FAILURE(326, unpack(pkt, 1, "z0", z), EINVAL)
	TEST_FAILURE(327, unpack(pkt, 1, "z*", 0, z), EINVAL)
	TEST_FAILURE(328, unpack(pkt, 1, "z*", -1, z), EINVAL)
	TEST_FAILURE(329, unpack(pkt, 0, "z", z), ENOSPC)
	TEST_FAILURE(330, unpack(pkt, 1, "z2", z), ENOSPC)
	TEST_FAILURE(331, unpack(pkt, 1, "z*", 2, z), ENOSPC)
	TEST_FAILURE(332, unpack(pkt, 0, "z", NULL), ENOSPC)
	TEST_FAILURE(333, unpack(pkt, 10, "z?", NULL, 10, z), EINVAL)
	TEST_FAILURE(334, unpack(pkt, 10, "z?", &length, 10, z), ENOSPC)

	TEST_FAILURE(335, unpack(pkt, 1, "b0", b), EINVAL)
	TEST_FAILURE(336, unpack(pkt, 1, "b*", 0, b), EINVAL)
	TEST_FAILURE(337, unpack(pkt, 1, "b*", -1, b), EINVAL)
	TEST_FAILURE(338, unpack(pkt, 0, "b", b), ENOSPC)
	TEST_FAILURE(339, unpack(pkt, 1, "b9", b), ENOSPC)
	TEST_FAILURE(340, unpack(pkt, 1, "b*", 9, b), ENOSPC)
	TEST_FAILURE(341, unpack(pkt, 0, "b", NULL), ENOSPC)
	TEST_FAILURE(342, unpack(pkt, 10, "b?", NULL, 10, b), EINVAL)
	TEST_FAILURE(343, unpack(pkt, 10, "b?", &length, 10, b), ENOSPC)

	TEST_FAILURE(344, unpack(pkt, 1, "h0", h), EINVAL)
	TEST_FAILURE(345, unpack(pkt, 1, "h*", 0, h), EINVAL)
	TEST_FAILURE(346, unpack(pkt, 1, "h*", -1, h), EINVAL)
	TEST_FAILURE(347, unpack(pkt, 0, "h", h), ENOSPC)
	TEST_FAILURE(348, unpack(pkt, 1, "h3", h), ENOSPC)
	TEST_FAILURE(349, unpack(pkt, 1, "h*", 3, h), ENOSPC)
	TEST_FAILURE(350, unpack(pkt, 0, "h", NULL), ENOSPC)
	TEST_FAILURE(351, unpack(pkt, 10, "h?", NULL, 10, h), EINVAL)
	TEST_FAILURE(352, unpack(pkt, 10, "h?", &length, 10, h), ENOSPC)

	TEST_FAILURE(353, unpack(pkt, 1, "c0", &sc), EINVAL)
	TEST_FAILURE(354, unpack(pkt, 1, "c*", 0, &sc), EINVAL)
	TEST_FAILURE(355, unpack(pkt, 1, "c*", -1, &sc), EINVAL)
	TEST_FAILURE(356, unpack(pkt, 0, "c", &sc), ENOSPC)
	TEST_FAILURE(357, unpack(pkt, 1, "c2", &sc, &sc), ENOSPC)
	TEST_FAILURE(358, unpack(pkt, 1, "c*", 2, &sc, &sc), ENOSPC)
	TEST_FAILURE(359, unpack(pkt, 1, "c?", &length, 1, &sc), EINVAL)

	TEST_FAILURE(360, unpack(pkt, 1, "c0", &uc), EINVAL)
	TEST_FAILURE(361, unpack(pkt, 1, "c*", 0, &uc), EINVAL)
	TEST_FAILURE(362, unpack(pkt, 1, "c*", -1, &uc), EINVAL)
	TEST_FAILURE(363, unpack(pkt, 0, "c", &uc), ENOSPC)
	TEST_FAILURE(364, unpack(pkt, 1, "c2", &uc, &uc), ENOSPC)
	TEST_FAILURE(365, unpack(pkt, 1, "c*", 2, &uc, &uc), ENOSPC)

	TEST_FAILURE(366, unpack(pkt, 1, "s0", &ss), EINVAL)
	TEST_FAILURE(367, unpack(pkt, 1, "s*", 0, &ss), EINVAL)
	TEST_FAILURE(368, unpack(pkt, 1, "s*", -1, &ss), EINVAL)
	TEST_FAILURE(369, unpack(pkt, 0, "s", &ss), ENOSPC)
	TEST_FAILURE(370, unpack(pkt, 1, "s", &ss), ENOSPC)
	TEST_FAILURE(371, unpack(pkt, 2, "s2", &ss, &ss), ENOSPC)
	TEST_FAILURE(372, unpack(pkt, 3, "s*", 2, &ss, &ss), ENOSPC)
	TEST_FAILURE(373, unpack(pkt, 2, "s?", &length, 1, &ss), EINVAL)

	TEST_FAILURE(374, unpack(pkt, 1, "s0", &us), EINVAL)
	TEST_FAILURE(375, unpack(pkt, 1, "s*", 0, &us), EINVAL)
	TEST_FAILURE(376, unpack(pkt, 1, "s*", -1, &us), EINVAL)
	TEST_FAILURE(377, unpack(pkt, 0, "s", &us), ENOSPC)
	TEST_FAILURE(378, unpack(pkt, 1, "s", &us), ENOSPC)
	TEST_FAILURE(379, unpack(pkt, 2, "s2", &us, &us), ENOSPC)
	TEST_FAILURE(380, unpack(pkt, 3, "s*", 2, &us, &us), ENOSPC)

	TEST_FAILURE(381, unpack(pkt, 1, "i0", &si), EINVAL)
	TEST_FAILURE(382, unpack(pkt, 1, "i*", 0, &si), EINVAL)
	TEST_FAILURE(383, unpack(pkt, 1, "i*", -1, &si), EINVAL)
	TEST_FAILURE(384, unpack(pkt, 0, "i", &si), ENOSPC)
	TEST_FAILURE(385, unpack(pkt, 1, "i", &si), ENOSPC)
	TEST_FAILURE(386, unpack(pkt, 2, "i", &si), ENOSPC)
	TEST_FAILURE(387, unpack(pkt, 3, "i", &si), ENOSPC)
	TEST_FAILURE(388, unpack(pkt, 4, "i2", &si, &si), ENOSPC)
	TEST_FAILURE(389, unpack(pkt, 5, "i*", 2, &si, &si), ENOSPC)
	TEST_FAILURE(390, unpack(pkt, 4, "i?", &length, 1, &si), EINVAL)

	TEST_FAILURE(391, unpack(pkt, 1, "i0", &ui), EINVAL)
	TEST_FAILURE(392, unpack(pkt, 1, "i*", 0, &ui), EINVAL)
	TEST_FAILURE(393, unpack(pkt, 1, "i*", -1, &ui), EINVAL)
	TEST_FAILURE(394, unpack(pkt, 0, "i", &ui), ENOSPC)
	TEST_FAILURE(395, unpack(pkt, 1, "i", &ui), ENOSPC)
	TEST_FAILURE(396, unpack(pkt, 2, "i", &ui), ENOSPC)
	TEST_FAILURE(397, unpack(pkt, 3, "i", &ui), ENOSPC)
	TEST_FAILURE(398, unpack(pkt, 4, "i2", &ui, &ui), ENOSPC)
	TEST_FAILURE(399, unpack(pkt, 5, "i*", 2, &ui, &ui), ENOSPC)

	TEST_FAILURE(400, unpack(pkt, 1, "p0", &p), EINVAL)
	TEST_FAILURE(401, unpack(pkt, 1, "p*", 0, &p), EINVAL)
	TEST_FAILURE(402, unpack(pkt, 1, "p*", -1, &p), EINVAL)
	TEST_FAILURE(403, unpack(pkt, 0, "p", &p), ENOSPC)
	TEST_FAILURE(404, unpack(pkt, 1, "p", &p), ENOSPC)
	TEST_FAILURE(405, unpack(pkt, 2, "p", &p), ENOSPC)
	TEST_FAILURE(406, unpack(pkt, 3, "p", &p), ENOSPC)
	TEST_FAILURE(407, unpack(pkt, 4, "p2", &p, &p2), ENOSPC)
	TEST_FAILURE(408, unpack(pkt, 5, "p*", 2, &p, &p2), ENOSPC)
#ifndef HAVE_LONG_LONG
	TEST_FAILURE(409, unpack(pkt, 4, "p?", &length, 1, &p), EINVAL)
#else
	TEST_FAILURE(409, unpack(pkt, 8, "p?", &length, 1, &p), EINVAL)
#endif

#ifdef HAVE_LONG_LONG
	TEST_FAILURE(410, unpack(pkt, 1, "l0", &sl), EINVAL)
	TEST_FAILURE(411, unpack(pkt, 1, "l*", 0, &sl), EINVAL)
	TEST_FAILURE(412, unpack(pkt, 1, "l*", -1, &sl), EINVAL)
	TEST_FAILURE(413, unpack(pkt, 0, "l", &sl), ENOSPC)
	TEST_FAILURE(414, unpack(pkt, 1, "l", &sl), ENOSPC)
	TEST_FAILURE(415, unpack(pkt, 2, "l", &sl), ENOSPC)
	TEST_FAILURE(416, unpack(pkt, 3, "l", &sl), ENOSPC)
	TEST_FAILURE(417, unpack(pkt, 4, "l", &sl), ENOSPC)
	TEST_FAILURE(418, unpack(pkt, 5, "l", &sl), ENOSPC)
	TEST_FAILURE(419, unpack(pkt, 6, "l", &sl), ENOSPC)
	TEST_FAILURE(420, unpack(pkt, 7, "l", &sl), ENOSPC)
	TEST_FAILURE(421, unpack(pkt, 8, "l2", &sl, &sl), ENOSPC)
	TEST_FAILURE(422, unpack(pkt, 9, "l*", 2, &sl, &sl), ENOSPC)
	TEST_FAILURE(423, unpack(pkt, 8, "l?", &length, 1, &sl), EINVAL)

	TEST_FAILURE(424, unpack(pkt, 1, "l0", &ul), EINVAL)
	TEST_FAILURE(425, unpack(pkt, 1, "l*", 0, &ul), EINVAL)
	TEST_FAILURE(426, unpack(pkt, 1, "l*", -1, &ul), EINVAL)
	TEST_FAILURE(427, unpack(pkt, 0, "l", &ul), ENOSPC)
	TEST_FAILURE(428, unpack(pkt, 1, "l", &ul), ENOSPC)
	TEST_FAILURE(429, unpack(pkt, 2, "l", &ul), ENOSPC)
	TEST_FAILURE(430, unpack(pkt, 3, "l", &ul), ENOSPC)
	TEST_FAILURE(431, unpack(pkt, 4, "l", &ul), ENOSPC)
	TEST_FAILURE(432, unpack(pkt, 5, "l", &ul), ENOSPC)
	TEST_FAILURE(433, unpack(pkt, 6, "l", &ul), ENOSPC)
	TEST_FAILURE(434, unpack(pkt, 7, "l", &ul), ENOSPC)
	TEST_FAILURE(435, unpack(pkt, 8, "l2", &ul, &ul), ENOSPC)
	TEST_FAILURE(436, unpack(pkt, 9, "l*", 2, &ul, &ul), ENOSPC)
#else
	TEST_FAILURE(437, unpack(pkt, 8, "l", NULL), ENOSYS)
	TEST_FAILURE(438, unpack(pkt, 8, "l", NULL), ENOSYS)
#endif

	f = -1.5; /* packed length = 6 */
	pack(pkt, 1024, "ff", f, f); /* needs this to find length */
	TEST_FAILURE(439, unpack(pkt, 1, "f0", &f), EINVAL)
	TEST_FAILURE(440, unpack(pkt, 1, "f*", 0, &f), EINVAL)
	TEST_FAILURE(441, unpack(pkt, 1, "f*", -1, &f), EINVAL)
	TEST_FAILURE(442, unpack(pkt, 0, "f", &f), ENOSPC)
	TEST_FAILURE(443, unpack(pkt, 1, "f", &f), ENOSPC)
	TEST_FAILURE(444, unpack(pkt, 2, "f", &f), ENOSPC)
	TEST_FAILURE(445, unpack(pkt, 3, "f", &f), ENOSPC)
	TEST_FAILURE(446, unpack(pkt, 4, "f", &f), ENOSPC)
	TEST_FAILURE(447, unpack(pkt, 5, "f", &f), ENOSPC)
	TEST_FAILURE(448, unpack(pkt, 6, "f2", &f, &f), ENOSPC)
	TEST_FAILURE(449, unpack(pkt, 7, "f*", 2, &f, &f), ENOSPC)
	TEST_FAILURE(450, unpack(pkt, 10, "f?", &length, 1, &f), EINVAL)

	da = db = 1.5; /* packed length = 5 */
	pack(pkt, 1024, "dd", da, db); /* needs this to find length */
	TEST_FAILURE(451, unpack(pkt, 1, "d0", &da), EINVAL)
	TEST_FAILURE(452, unpack(pkt, 1, "d*", 0, &da), EINVAL)
	TEST_FAILURE(453, unpack(pkt, 1, "d*", -1, &da), EINVAL)
	TEST_FAILURE(454, unpack(pkt, 0, "d", &da), ENOSPC)
	TEST_FAILURE(455, unpack(pkt, 1, "d", &da), ENOSPC)
	TEST_FAILURE(456, unpack(pkt, 2, "d", &da), ENOSPC)
	TEST_FAILURE(457, unpack(pkt, 3, "d", &da), ENOSPC)
	TEST_FAILURE(458, unpack(pkt, 4, "d", &da), ENOSPC)
	TEST_FAILURE(459, unpack(pkt, 5, "d2", &da, &db), ENOSPC)
	TEST_FAILURE(460, unpack(pkt, 6, "d*", 2, &da, &db), ENOSPC)
	TEST_FAILURE(461, unpack(pkt, 10, "d?", &length, 1, &da), EINVAL)

	TEST_FAILURE(462, unpack(pkt, 1, "x0"), EINVAL)
	TEST_FAILURE(463, unpack(pkt, 1, "x*", 0), EINVAL)
	TEST_FAILURE(464, unpack(pkt, 1, "x*", -1), EINVAL)
	TEST_FAILURE(465, unpack(pkt, 0, "x"), ENOSPC)
	TEST_FAILURE(466, unpack(pkt, 1, "x2"), ENOSPC)
	TEST_FAILURE(467, unpack(pkt, 1, "x*", 2), ENOSPC)
	TEST_FAILURE(468, unpack(pkt, 5, "x?", &length, 10, &a), EINVAL)

	TEST_FAILURE(469, unpack(pkt, 1, "X0"), EINVAL)
	TEST_FAILURE(470, unpack(pkt, 1, "X*", 0), EINVAL)
	TEST_FAILURE(471, unpack(pkt, 1, "X*", -1), EINVAL)
	TEST_FAILURE(472, unpack(pkt, 0, "X"), EINVAL)
	TEST_FAILURE(473, unpack(pkt, 1, "X"), EINVAL)
	TEST_FAILURE(474, unpack(pkt, 1, "X2"), EINVAL)
	TEST_FAILURE(475, unpack(pkt, 1024, "X*", 2), EINVAL)
	TEST_FAILURE(476, unpack(pkt, 1024, "X?", &length, 10, &a), EINVAL)

	TEST_FAILURE(477, unpack(pkt, 1, "@0"), EINVAL)
	TEST_FAILURE(478, unpack(pkt, 1, "@*", 0), EINVAL)
	TEST_FAILURE(479, unpack(pkt, 1, "@*", -1), EINVAL)
	TEST_FAILURE(480, unpack(pkt, 0, "@"), ENOSPC)
	TEST_FAILURE(481, unpack(pkt, 0, "@1"), ENOSPC)
	TEST_FAILURE(482, unpack(pkt, 1, "@2"), ENOSPC)
	TEST_FAILURE(483, unpack(pkt, 1, "@*", 2), ENOSPC)
	TEST_FAILURE(484, unpack(pkt, 1, "@?", &length, 10, a), EINVAL)

	TEST_FAILURE(485, unpack(pkt, 1, "?", 0), EINVAL)

	/* Test truncation of string data */

#define TEST_TRUNC(i, format, size, pformat, len, data1, data2, init) \
	memset(data2, init, 1024); \
	TEST_STR(i, format, size, pformat, len, data1, data2) \
	if (data2[len] != 0) \
		++errors, printf("Test%d: unpack(%s, trunc) failed (%s[%d] == %d, not %d)\n", i, format, #format, len, ((char *)data1)[len], 0);

	TEST_TRUNC(486, "a3", 3, "%3.3s", 3, a, tstmem, '\0')
	TEST_TRUNC(487, "z3", 3, "%s", 3, z, tstmem, ' ')
	TEST_TRUNC(488, "b3", 1, "%s", 3, b, tstmem, ' ')
	TEST_TRUNC(489, "h3", 2, "%s", 3, h, tstmem, ' ')

	/* Test net_expect() and net_send() */

	if ((server = net_server(NULL, NULL, 30001, 0, 0, NULL, NULL)) == -1)
		++errors, printf("Test490: net_server(NULL, 30001) failed: %s\n", strerror(errno));
	else
	{
		switch (pid = fork())
		{
			case -1:
			{
				printf("Failed to fork (%s)\n", strerror(errno));
				return 1;
			}

			default:
			{
				int s;
				sockaddr_any_t addr;
				size_t addrsize = sizeof addr;

				if (read_timeout(server, 5, 0) == -1 || (s = accept(server, (sockaddr_t *)&addr, (void *)&addrsize)) == -1)
					++errors, printf("Test491: accept() failed (%s)\n", strerror(errno));
				else
				{
					char test[4];
					ssize_t rc;

					if ((rc = net_expect(s, 5, "%s", test)) != 1)
						++errors, printf("Test492: net_expect(s, 5, HELO) failed (%s)\n", (rc == 0) ? "eof" : strerror(errno));
					else if (memcmp(test, "HELO", 4))
						++errors, printf("Test493: net_expect(s, 5, HELO) failed (received \"%4.4s\", not \"%4.4s\")\n", test, "HELO");
					else if (net_send(s, 5, "OLEH") == -1)
						++errors, printf("Test494: net_send(s, 5, OLEH) failed (%s)\n", strerror(errno));
					if (close(s) == -1)
						++errors, printf("Test495: close(s) failed (%s)\n", strerror(errno));
				}

				errors += wait_for_child(pid);
				break;
			}

			case 0:
			{
				errors = 0;

				if ((client = net_client(NULL, NULL, 30001, 5, 0, 0, NULL, NULL)) == -1)
					++errors, printf("Test496: net_client(NULL, 30001) failed (%s)\n", strerror(errno));
				else
				{
					char test[4];
					ssize_t rc;

					if (net_send(client, 5, "HELO") == -1)
						++errors, printf("Test497: net_send(client, 5, HELO) failed (%s)\n", strerror(errno));
					else if ((rc = net_expect(client, 5, "%s", test)) != 1)
						++errors, printf("Test498: net_expect(client, OLEH) failed (%s)\n", (rc == 0) ? "eof" : strerror(errno));
					else if (memcmp(test, "OLEH", 4))
						++errors, printf("Test499: net_expect(client, OLEH) failed (received \"%4.4s\", not \"%4.4s\")\n", test, "OLEH");
					if (close(client) == -1)
						++errors, printf("Test500: close(client) failed (%s)\n", strerror(errno));
				}

				return errors;
			}
		}

		if (close(server) == -1)
			++errors, printf("Test501: close(server) failed (%s)\n", strerror(errno));
	}

	/* Test net_pack(), net_packto(), net_unpack() and net_unpackfrom()*/

	if ((server = net_udp_server(NULL, NULL, 30001, 0, 0, NULL, NULL)) == -1)
		++errors, printf("Test502: net_udp_server(NULL, 30001) failed: %s\n", strerror(errno));
	else
	{
		switch (pid = fork())
		{
			case -1:
			{
				printf("Failed to fork (%s)\n", strerror(errno));
				return 1;
			}

			default:
			{
				int neti;
				char netz[5];
				sockaddr_any_t addr;
				size_t addrsize = sizeof addr;

				if (net_unpackfrom(server, 5, 0, (sockaddr_t *)&addr, &addrsize, "iz4", &neti, netz) == -1)
					++errors, printf("Test503: net_unpackfrom(server, \"iz4\", 37, HELO) failed (%s)\n", strerror(errno));
				else if (neti != 37)
					++errors, printf("Test504: net_unpackfrom(server, \"iz4\", 37, HELO) failed (%d != %d)\n", neti, 37);
				else if (strcmp(netz, "HELO"))
					++errors, printf("Test505: net_unpackfrom(server, \"iz4\", 37, HELO) failed (recv \"%4.4s\", not \"%4.4s\")\n", netz, "HELO");
				else if (net_packto(server, 5, 0, (sockaddr_t *)&addr, addrsize, "iz4", neti + 1, "OLEH") == -1)
					++errors, printf("Test506: net_packto(server, \"iz4\", 38, OLEH) failed (%s)\n", strerror(errno));

				errors += wait_for_child(pid);
				break;
			}

			case 0:
			{
				errors = 0;

				if ((client = net_udp_client(NULL, NULL, 30001, 0, 0, NULL, NULL)) == -1)
					++errors, printf("Test507: net_udp_client(NULL, 30001) failed (%s)\n", strerror(errno));
				else
				{
					int neti;
					char netz[5];

					if (net_pack(client, 5, 0, "iz4", 37, "HELO") == -1)
						++errors, printf("Test508: net_pack(client, \"iz4\", 37, HELO) failed (%s)\n", strerror(errno));
					else if (net_unpack(client, 5, 0, "iz4", &neti, netz) == -1)
						++errors, printf("Test509: net_unpack(client, 38, OLEH) failed (%s)\n", strerror(errno));
					else if (neti != 38)
						++errors, printf("Test510: net_unpack(client, 38, OLEH) failed (%d != %d)\n", neti, 38);
					else if (strcmp(netz, "OLEH"))
						++errors, printf("Test511: net_unpack(client, 38, OLEH) failed (recv \"%4.4s\", not \"%4.4s\")\n", netz, "OLEH");
					if (close(client) == -1)
						++errors, printf("Test512: close(client) failed (%s)\n", strerror(errno));
				}

				return errors;
			}
		}

		if (close(server) == -1)
			++errors, printf("Test513: close(server) failed (%s)\n", strerror(errno));
	}

	/* Test mail() */

	if (!(pwd = getpwuid(getuid())))
		++errors, printf("Test514: failed to test mail() (getpwuid() failed))\n");
	else
	{
		struct utsname utsbuf[1];
		char addr[128];

		if (uname(utsbuf) == -1)
			++errors, printf("Test514: failed to test mail() (uname() failed %s)\n", strerror(errno));
		else
		{
			snprintf(addr, 128, "%s@%s", pwd->pw_name, utsbuf->nodename);
			rc = mail(NULL, addr, addr, "subject", "message");
			if (rc == -1 && errno == ECONNREFUSED)
				no_mailserver = 1;
			else if (rc != 0)
				++errors, printf("Test514: mail(NULL, \"%s\", \"%s\", \"%s\", \"%s\") failed: returned %d, not %d (%s)\n", addr, addr, "subject", "message", rc, 0, strerror(errno));
		}

		TEST_FAILURE(515, mail(NULL, addr, NULL, NULL, NULL), EINVAL)
		TEST_FAILURE(516, mail(NULL, NULL, addr, NULL, NULL), EINVAL)
	}

	/* Test net_options() */

	if ((server = net_server(NULL, NULL, 30001, 0, 0, NULL, NULL)) == -1)
		++errors, printf("Test517: net_server(NULL, 30001) failed: %s\n", strerror(errno));
	else
	{
		if (net_options(server, sockopts) == -1)
			++errors, printf("Test517: net_options(server, sockopts) failed: %s\n", strerror(errno));

		if ((rc = net_options(server, NULL)) != -1)
			++errors, printf("Test518: net_options(server, null) failed (returned %d, not %d)\n", rc, -1);
		else if (errno != EINVAL)
			++errors, printf("Test518: net_options(server, null) failed (error was %s, not %s)\n", strerror(errno), "Invalid argument");

		close(server);
	}

	if ((rc = net_options(-1, sockopts)) != -1)
		++errors, printf("Test519: net_options(-1, sockopts) failed (returned %d, not %d)\n", rc, -1);
	else if (errno != EBADF)
		++errors, printf("Test519: net_options(-1, sockopts) failed (error was %s, not %s)\n", strerror(errno), "Bad file descriptor");

	/* Test sendfd() and recvfd() */

	if ((server = net_server("/unix", unixsock, 0, 0, 0, NULL, NULL)) == -1)
		++errors, printf("Test520: failed to perform test: net_server(\"/unix\", \"%s\") failed: %s\n", unixsock, strerror(errno));
	else
	{
		switch (pid = fork())
		{
			case -1:
			{
				printf("Failed to fork (%s)\n", strerror(errno));
				return 1;
			}

			default:
			{
				int s;
				sockaddr_any_t addr;
				size_t addrsize = sizeof addr;

				if (read_timeout(server, 5, 0) == -1 || (s = accept(server, (sockaddr_t *)&addr, (void *)&addrsize)) == -1)
					++errors, printf("Test520: failed to perform test: accept() failed (%s)\n", strerror(errno));
				else
				{
					const char * const text = "file descriptor passing test\n";
					char test[BUFSIZ];
					int bytes;
					int fd;

					if (read_timeout(s, 5, 0) == -1 || (bytes = recvfd(s, test, BUFSIZ, 0, &fd)) == -1)
						++errors, printf("Test521: recvfd(s, test, BUFSIZ, 0, &fd) failed (%s)\n", strerror(errno));
					else if (bytes != strlen(text))
						++errors, printf("Test522: recvfd(s, test, BUFSIZ, 0, &fd) failed (read %d bytes, not %d bytes)\n", bytes, (int)strlen(text));
					else if (memcmp(test, text, strlen(text)))
						++errors, printf("Test523: recvfd(s, test, BUFSIZ, 0, &fd) failed (read \"%*.*s\", not \"%s\")\n", bytes, bytes, test, text);
					else if (fd == -1)
						++errors, printf("Test524: recvfd(s, test, BUFSIZ, 0, &fd) failed (fd not passed)\n");
					else if (read_timeout(fd, 5, 0) == -1 || (bytes = read(fd, test, BUFSIZ)) == -1)
						++errors, printf("Test525: recvfd(s, test, BUFSIZ, 0, &fd) failed: read(fd) failed (%s)\n", strerror(errno));
					else if (bytes != strlen(text))
						++errors, printf("Test526: recvfd(s, test, BUFSIZ, 0, &fd) failed (read %d bytes, not %d bytes)\n", bytes, (int)strlen(text));
					else if (memcmp(test, text, strlen(text)))
						++errors, printf("Test527: recvfd(s, test, BUFSIZ, 0, &fd) failed: read(fd) failed (read \"%*.*s\", not \"%s\")\n", bytes, bytes, test, text);
					else if (close(fd) == -1)
						++errors, printf("Test528: recvfd(s, test, BUFSIZ, 0, &fd) failed: close(fd) failed (%s)\n", strerror(errno));

					close(s);
				}

				errors += wait_for_child(pid);
				break;
			}

			case 0:
			{
				errors = 0;

				if ((client = net_client("/unix", unixsock, 0, 5, 0, 0, NULL, NULL)) == -1)
					++errors, printf("Test529: failed to perform test: net_client(\"/unix\", \"%s\") failed (%s)\n", unixsock, strerror(errno));
				else
				{
					const char * const pass = "/tmp/libslack.net.pass";
					const char * const text = "file descriptor passing test\n";
					int fd;

					if ((fd = open(pass, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR)) == -1 || write(fd, text, strlen(text)) == -1 || close(fd) == -1)
						++errors, printf("Test530: failed to perform test: open/write/close(\"%s\") failed (%s)\n", pass, strerror(errno));
					else if ((fd = open(pass, O_RDONLY)) == -1)
						++errors, printf("Test531: failed to perform test: open(\"%s\") failed (%s)\n", pass, strerror(errno));
					else if (write_timeout(client, 5, 0) == -1 || sendfd(client, text, strlen(text), 0, fd) == -1)
						++errors, printf("Test532: sendfd(client, text, %d, fd) failed (%s)\n", (int)strlen(text), strerror(errno));

					unlink(pass);
					close(fd);
					close(client);
				}

				return errors;
			}
		}

		close(server);
	}

	unlink(unixsock);

	/* Test multicasting */

	if (av[1] && !strcmp(av[1], "multicast"))
	{
#ifdef AF_INET6
		const char *allhosts = inet6_required() ? "ff02::1" : "224.0.0.1";
#else
		const char *allhosts = "224.0.0.1";
#endif

		no_multicast = 0;

		/* Should have created the receiver first */

		if ((server = net_multicast_sender(allhosts, NULL, 30000, NULL, NULL, NULL, NULL, 0, 0, 0)) == -1)
			++errors, printf("Test533: net_multicast_sender(\"%s\", 30000) failed: %s\n", allhosts, strerror(errno));
		else
		{
			int index, ttl, loopback;
			int sync[2];
#define RD 0
#define WR 1

			if (pipe(sync) == -1)
				++errors, printf("Test534: failed to run test: pipe() failed (%s)\n", strerror(errno));
			else
			{
				switch (pid = fork())
				{
					case -1:
					{
						printf("Failed to fork (%s)\n", strerror(errno));
						return 1;
					}

					default:
					{
						char test[BUFSIZ], ack;

						strlcpy(test, "MCAST", BUFSIZ);

						close(sync[WR]);

						if (read_timeout(sync[RD], 5, 0) == -1 || read(sync[RD], &ack, 1) != 1)
							++errors, printf("Test535: failed to perform test: read_timeout() or read() failed (%s)\n", strerror(errno));
						else if (write_timeout(server, 5, 0) == -1 || send(server, test, strlen(test), 0) == -1)
							++errors, printf("Test536: send(multicast) failed (%s)\n", strerror(errno));

						close(sync[RD]);

						errors += wait_for_child(pid);
						break;
					}

					case 0:
					{
						sockaddr_any_t addr[1];
						size_t addrsize = sizeof addr;
						int client = 0;
						errors = 0;

						close(sync[RD]);

						if ((client = net_multicast_receiver(allhosts, NULL, 30000, NULL, NULL, NULL, NULL, 0)) == -1)
							++errors, printf("Test537: net_multicast_receiver(\"%s\", 30000) failed (%s)\n", allhosts, strerror(errno));
						else
						{
							char test[BUFSIZ];
							ssize_t bytes;

							if (write_timeout(sync[WR], 5, 0) == -1 || write(sync[WR], "", 1) != 1)
								++errors, printf("Test538: failed to perform test: write() failed (%s)\n", strerror(errno));
							else if (read_timeout(client, 5, 0) == -1 || (bytes = recvfrom(client, test, BUFSIZ, 0, &addr->any, (void *)&addrsize)) == -1)
								++errors, printf("Test539: recvfrom(multicast) failed (%s)\n", strerror(errno));
							else if (bytes != 5)
								++errors, printf("Test540: recvfrom(multicast) failed (read %d bytes, not %d)\n", (int)bytes, 5);
							else if (memcmp(test, "MCAST", 5))
								++errors, printf("Test541: recvfrom(multicast) failed (recv \"%5.5s\", not \"%5.5s\")\n", test, "MCAST");
							if (close(client) == -1)
								++errors, printf("Test542: close(client) failed (%s)\n", strerror(errno));
						}

						close(sync[WR]);

						return errors;
					}
				}
			}

			/* Test get/set interface/ttl/loopback */

			if ((index = net_multicast_get_interface(server)) == -1)
				++errors, printf("Test543: net_multicast_get_interface() failed (%s)\n", strerror(errno));
			else if (net_multicast_set_interface(server, NULL, index) == -1)
				++errors, printf("Test544: net_multicast_set_interface() failed (%s)\n", strerror(errno));

			if (net_multicast_set_ttl(server, 42) == -1)
				++errors, printf("Test545: net_multicast_set_ttl() failed (%s)\n", strerror(errno));
			else if ((ttl = net_multicast_get_ttl(server)) == -1)
				++errors, printf("Test546: net_multicast_get_ttl() failed (%s)\n", strerror(errno));
			else if (ttl != 42)
				++errors, printf("Test547: net_multicast_get_ttl() failed (returned %d, not %d)\n", ttl, 42);

			if (net_multicast_set_loopback(server, 0) == -1)
				++errors, printf("Test548: net_multicast_set_loopback() failed (%s)\n", strerror(errno));
			else if ((loopback = net_multicast_get_loopback(server)) == -1)
				++errors, printf("Test549: net_multicast_get_loopback() failed (%s)\n", strerror(errno));
			else if (loopback != 0)
				++errors, printf("Test550: net_multicast_get_loopback() failed (returned %d, not %d)\n", loopback, 0);

			if (close(server) == -1)
				++errors, printf("Test551: close(server) failed (%s)\n", strerror(errno));
		}
	}

	/* Test net_rudp_transact() */

	if ((server = net_udp_server(NULL, NULL, 30000, 0, 0, NULL, NULL)) == -1)
		++errors, printf("Test552: net_udp_server(NULL, 30000) failed: %s\n", strerror(errno));
	else
	{
		switch (pid = fork())
		{
			case -1:
			{
				printf("Failed to fork (%s)\n", strerror(errno));
				return 1;
			}

			default:
			{
				char test[12];
				sockaddr_any_t addr;
				size_t addrsize = sizeof addr;

				/* Respond immediately */

				if (read_timeout(server, 5, 0) == -1 || recvfrom(server, test, 12, 0, (sockaddr_t *)&addr, (void *)&addrsize) == -1)
					++errors, printf("Test553: recvfrom(server, HELO) failed (%s)\n", strerror(errno));
				else if (memcmp(test + 8, "HELO", 4))
					++errors, printf("Test554: recvfrom(server, HELO) failed (recv \"%4.4s\", not \"%4.4s\")\n", test, "HELO");
				else if (write_timeout(server, 5, 0) == -1 || sendto(server, test, 12, 0, (sockaddr_t *)&addr, addrsize) == -1)
					++errors, printf("Test555: sendto(server, HELO) failed (%s)\n", strerror(errno));

				if (av[1] && !strcmp(av[1], "rudp"))
				{
					no_rudp = 0;

					/* Respond after 1 retransmission */

					if (read_timeout(server, 5, 0) == -1 || recvfrom(server, test, 12, 0, (sockaddr_t *)&addr, (void *)&addrsize) == -1)
						++errors, printf("Test556: recvfrom(server, HELO) failed (%s)\n", strerror(errno));
					else if (memcmp(test + 8, "HELO", 4))
						++errors, printf("Test577: recvfrom(server, HELO) failed (recv \"%4.4s\", not \"%4.4s\")\n", test, "HELO");
					else if (read_timeout(server, 10, 0) == -1 || recvfrom(server, test, 12, 0, (sockaddr_t *)&addr, (void *)&addrsize) == -1)
						++errors, printf("Test558: recvfrom(server, HELO) failed (%s)\n", strerror(errno));
					else if (memcmp(test + 8, "HELO", 4))
						++errors, printf("Test559: recvfrom(server, HELO) failed (recv \"%4.4s\", not \"%4.4s\")\n", test, "HELO");
					else if (write_timeout(server, 5, 0) == -1 || sendto(server, test, 12, 0, (sockaddr_t *)&addr, addrsize) == -1)
						++errors, printf("Test560: sendto(server, HELO) failed (%s)\n", strerror(errno));

					/* Respond after 2 retransmissions */

					if (read_timeout(server, 5, 0) == -1 || recvfrom(server, test, 12, 0, (sockaddr_t *)&addr, (void *)&addrsize) == -1)
						++errors, printf("Test561: recvfrom(server, HELO) failed (%s)\n", strerror(errno));
					else if (memcmp(test + 8, "HELO", 4))
						++errors, printf("Test562: recvfrom(server, HELO) failed (recv \"%4.4s\", not \"%4.4s\")\n", test, "HELO");
					else if (read_timeout(server, 10, 0) == -1 || recvfrom(server, test, 12, 0, (sockaddr_t *)&addr, (void *)&addrsize) == -1)
						++errors, printf("Test563: recvfrom(server, HELO) failed (%s)\n", strerror(errno));
					else if (memcmp(test + 8, "HELO", 4))
						++errors, printf("Test564: recvfrom(server, HELO) failed (recv \"%4.4s\", not \"%4.4s\")\n", test, "HELO");
					else if (read_timeout(server, 20, 0) == -1 || recvfrom(server, test, 12, 0, (sockaddr_t *)&addr, (void *)&addrsize) == -1)
						++errors, printf("Test565: recvfrom(server, HELO) failed (%s)\n", strerror(errno));
					else if (memcmp(test + 8, "HELO", 4))
						++errors, printf("Test566: recvfrom(server, HELO) failed (recv \"%4.4s\", not \"%4.4s\")\n", test, "HELO");
					else if (write_timeout(server, 5, 0) == -1 || sendto(server, test, 12, 0, (sockaddr_t *)&addr, addrsize) == -1)
						++errors, printf("Test567: sendto(server, HELO) failed (%s)\n", strerror(errno));

					/* Respond after 3 retransmissions */

					if (read_timeout(server, 5, 0) == -1 || recvfrom(server, test, 12, 0, (sockaddr_t *)&addr, (void *)&addrsize) == -1)
						++errors, printf("Test568: recvfrom(server, HELO) failed (%s)\n", strerror(errno));
					else if (memcmp(test + 8, "HELO", 4))
						++errors, printf("Test569: recvfrom(server, HELO) failed (recv \"%4.4s\", not \"%4.4s\")\n", test, "HELO");
					else if (read_timeout(server, 10, 0) == -1 || recvfrom(server, test, 12, 0, (sockaddr_t *)&addr, (void *)&addrsize) == -1)
						++errors, printf("Test570: recvfrom(server, HELO) failed (%s)\n", strerror(errno));
					else if (memcmp(test + 8, "HELO", 4))
						++errors, printf("Test571: recvfrom(server, HELO) failed (recv \"%4.4s\", not \"%4.4s\")\n", test, "HELO");
					else if (read_timeout(server, 20, 0) == -1 || recvfrom(server, test, 12, 0, (sockaddr_t *)&addr, (void *)&addrsize) == -1)
						++errors, printf("Test572: recvfrom(server, HELO) failed (%s)\n", strerror(errno));
					else if (memcmp(test + 8, "HELO", 4))
						++errors, printf("Test573: recvfrom(server, HELO) failed (recv \"%4.4s\", not \"%4.4s\")\n", test, "HELO");
					else if (read_timeout(server, 40, 0) == -1 || recvfrom(server, test, 12, 0, (sockaddr_t *)&addr, (void *)&addrsize) == -1)
						++errors, printf("Test574: recvfrom(server, HELO) failed (%s)\n", strerror(errno));
					else if (memcmp(test + 8, "HELO", 4))
						++errors, printf("Test575: recvfrom(server, HELO) failed (recv \"%4.4s\", not \"%4.4s\")\n", test, "HELO");
					else if (write_timeout(server, 5, 0) == -1 || sendto(server, test, 12, 0, (sockaddr_t *)&addr, addrsize) == -1)
						++errors, printf("Test576: sendto(server, HELO) failed (%s)\n", strerror(errno));

					/* Don't respond at all */

					if (read_timeout(server, 5, 0) == -1 || recvfrom(server, test, 12, 0, (sockaddr_t *)&addr, (void *)&addrsize) == -1)
						++errors, printf("Test577: recvfrom(server, HELO) failed (%s)\n", strerror(errno));
					else if (memcmp(test + 8, "HELO", 4))
						++errors, printf("Test578: recvfrom(server, HELO) failed (recv \"%4.4s\", not \"%4.4s\")\n", test, "HELO");
					else if (read_timeout(server, 10, 0) == -1 || recvfrom(server, test, 12, 0, (sockaddr_t *)&addr, (void *)&addrsize) == -1)
						++errors, printf("Test579: recvfrom(server, HELO) failed (%s)\n", strerror(errno));
					else if (memcmp(test + 8, "HELO", 4))
						++errors, printf("Test580: recvfrom(server, HELO) failed (recv \"%4.4s\", not \"%4.4s\")\n", test, "HELO");
					else if (read_timeout(server, 20, 0) == -1 || recvfrom(server, test, 12, 0, (sockaddr_t *)&addr, (void *)&addrsize) == -1)
						++errors, printf("Test581: recvfrom(server, HELO) failed (%s)\n", strerror(errno));
					else if (memcmp(test + 8, "HELO", 4))
						++errors, printf("Test582: recvfrom(server, HELO) failed (recv \"%4.4s\", not \"%4.4s\")\n", test, "HELO");
					else if (read_timeout(server, 40, 0) == -1 || recvfrom(server, test, 12, 0, (sockaddr_t *)&addr, (void *)&addrsize) == -1)
						++errors, printf("Test583: recvfrom(server, HELO) failed (%s)\n", strerror(errno));
					else if (memcmp(test + 8, "HELO", 4))
						++errors, printf("Test584: recvfrom(server, HELO) failed (recv \"%4.4s\", not \"%4.4s\")\n", test, "HELO");
				}

				errors += wait_for_child(pid);
				break;
			}

			case 0:
			{
				errors = 0;

				if ((client = net_udp_client(NULL, NULL, 30000, 0, 0, NULL, NULL)) == -1)
					++errors, printf("Test585: net_udp_client(NULL, 30000) failed (%s)\n", strerror(errno));
				else
				{
					rudp_t *rudp;
					char test[4];
					int rc = -1;

					if (!(rudp = rudp_create()))
						++errors, printf("Test586: rudp_create() failed (%s)\n", strerror(errno));
					else
					{
						/* Receive a response immediately */

						if (write_timeout(client, 5, 0) == -1 || net_rudp_transact(client, rudp, "HELO", 4, test, 4) == -1)
							++errors, printf("Test587: net_rudp_transact(client, HELO) failed%s (%s)\n", (errno == ETIMEDOUT) ? " but this might be ok" : "", strerror(errno));
						else if (memcmp(test, "HELO", 4))
							++errors, printf("Test588: net_rudp_transact(client, HELO) failed (received \"%4.4s\", not \"%4.4s\")\n", test, "HELO");

						if (av[1] && !strcmp(av[1], "rudp"))
						{
							/* Receive a response after 1 retransmission */

							if (write_timeout(client, 5, 0) == -1 || net_rudp_transact(client, rudp, "HELO", 4, test, 4) == -1)
								++errors, printf("Test589: net_rudp_transact(client, HELO) failed%s (%s)\n", (errno == ETIMEDOUT) ? " but this might be ok" : "", strerror(errno));
							else if (memcmp(test, "HELO", 4))
								++errors, printf("Test590: net_rudp_transact(client, HELO) failed (received \"%4.4s\", not \"%4.4s\")\n", test, "HELO");

							/* Receive a response after 2 retransmissions */

							if (write_timeout(client, 5, 0) == -1 || net_rudp_transact(client, rudp, "HELO", 4, test, 4) == -1)
								++errors, printf("Test591: net_rudp_transact(client, HELO) failed%s (%s)\n", (errno == ETIMEDOUT) ? " but this might be ok" : "", strerror(errno));
							else if (memcmp(test, "HELO", 4))
								++errors, printf("Test592: net_rudp_transact(client, HELO) failed (received \"%4.4s\", not \"%4.4s\")\n", test, "HELO");

							/* Receive a response after 3 retransmissions */

							if (write_timeout(client, 5, 0) == -1 || net_rudp_transact(client, rudp, "HELO", 4, test, 4) == -1)
								++errors, printf("Test593: net_rudp_transact(client, HELO) failed%s (%s)\n", (errno == ETIMEDOUT) ? " but this might be ok" : "", strerror(errno));
							else if (memcmp(test, "HELO", 4))
								++errors, printf("Test594: net_rudp_transact(client, HELO) failed (received \"%4.4s\", not \"%4.4s\")\n", test, "HELO");

							/* Receive no response at all */

							if (write_timeout(client, 5, 0) == -1 || (rc = net_rudp_transact(client, rudp, "HELO", 4, test, 4)) != -1)
								++errors, printf("Test595: net_rudp_transact(client, HELO) failed (should have timed out, returned %d)\n", rc);
							else if (errno != ETIMEDOUT)
								++errors, printf("Test596: net_rudp_transact(client, HELO) failed (errno %s, not %s)\n", strerror(errno), strerror(ETIMEDOUT));
						}

						if (close(client) == -1)
							++errors, printf("Test597: close(client) failed (%s)\n", strerror(errno));

						if (rudp_destroy(&rudp))
							++errors, printf("Test598: rudp_destroy() failed\n");
					}
				}

				return errors;
			}
		}

		if (close(server) == -1)
			++errors, printf("Test599: close(server) failed (%s)\n", strerror(errno));
	}

	/* Test net_rudp_transactwith() */

	if ((server = net_udp_server(NULL, NULL, 30000, 0, 0, (sockaddr_t *)&addr, &addrsize)) == -1)
		++errors, printf("Test600: net_udp_server(NULL, 30000) failed: %s\n", strerror(errno));
	else
	{
		switch (pid = fork())
		{
			case -1:
			{
				printf("Failed to fork (%s)\n", strerror(errno));
				return 1;
			}

			default:
			{
				char test[12];

				/* Respond immediately */

				if (read_timeout(server, 5, 0) == -1 || recvfrom(server, test, 12, 0, (sockaddr_t *)&addr, (void *)&addrsize) == -1)
					++errors, printf("Test601: recvfrom(server, HELO) failed (%s)\n", strerror(errno));
				else if (memcmp(test + 8, "HELO", 4))
					++errors, printf("Test602: recvfrom(server, HELO) failed (recv \"%4.4s\", not \"%4.4s\")\n", test, "HELO");
				else if (write_timeout(server, 5, 0) == -1 || sendto(server, test, 12, 0, (sockaddr_t *)&addr, addrsize) == -1)
					++errors, printf("Test603: sendto(server, HELO) failed (%s)\n", strerror(errno));

				if (av[1] && !strcmp(av[1], "rudp"))
				{
					/* Respond after 1 retransmission */

					if (read_timeout(server, 5, 0) == -1 || recvfrom(server, test, 12, 0, (sockaddr_t *)&addr, (void *)&addrsize) == -1)
						++errors, printf("Test604: recvfrom(server, HELO) failed (%s)\n", strerror(errno));
					else if (memcmp(test + 8, "HELO", 4))
						++errors, printf("Test625: recvfrom(server, HELO) failed (recv \"%4.4s\", not \"%4.4s\")\n", test, "HELO");
					else if (read_timeout(server, 10, 0) == -1 || recvfrom(server, test, 12, 0, (sockaddr_t *)&addr, (void *)&addrsize) == -1)
						++errors, printf("Test606: recvfrom(server, HELO) failed (%s)\n", strerror(errno));
					else if (memcmp(test + 8, "HELO", 4))
						++errors, printf("Test607: recvfrom(server, HELO) failed (recv \"%4.4s\", not \"%4.4s\")\n", test, "HELO");
					else if (write_timeout(server, 5, 0) == -1 || sendto(server, test, 12, 0, (sockaddr_t *)&addr, addrsize) == -1)
						++errors, printf("Test608: sendto(server, HELO) failed (%s)\n", strerror(errno));

					/* Respond after 2 retransmissions */

					if (read_timeout(server, 5, 0) == -1 || recvfrom(server, test, 12, 0, (sockaddr_t *)&addr, (void *)&addrsize) == -1)
						++errors, printf("Test609: recvfrom(server, HELO) failed (%s)\n", strerror(errno));
					else if (memcmp(test + 8, "HELO", 4))
						++errors, printf("Test610: recvfrom(server, HELO) failed (recv \"%4.4s\", not \"%4.4s\")\n", test, "HELO");
					else if (read_timeout(server, 10, 0) == -1 || recvfrom(server, test, 12, 0, (sockaddr_t *)&addr, (void *)&addrsize) == -1)
						++errors, printf("Test611: recvfrom(server, HELO) failed (%s)\n", strerror(errno));
					else if (memcmp(test + 8, "HELO", 4))
						++errors, printf("Test612: recvfrom(server, HELO) failed (recv \"%4.4s\", not \"%4.4s\")\n", test, "HELO");
					else if (read_timeout(server, 20, 0) == -1 || recvfrom(server, test, 12, 0, (sockaddr_t *)&addr, (void *)&addrsize) == -1)
						++errors, printf("Test613: recvfrom(server, HELO) failed (%s)\n", strerror(errno));
					else if (memcmp(test + 8, "HELO", 4))
						++errors, printf("Test614: recvfrom(server, HELO) failed (recv \"%4.4s\", not \"%4.4s\")\n", test, "HELO");
					else if (write_timeout(server, 5, 0) == -1 || sendto(server, test, 12, 0, (sockaddr_t *)&addr, addrsize) == -1)
						++errors, printf("Test615: sendto(server, HELO) failed (%s)\n", strerror(errno));

					/* Respond after 3 retransmissions */

					if (read_timeout(server, 5, 0) == -1 || recvfrom(server, test, 12, 0, (sockaddr_t *)&addr, (void *)&addrsize) == -1)
						++errors, printf("Test616: recvfrom(server, HELO) failed (%s)\n", strerror(errno));
					else if (memcmp(test + 8, "HELO", 4))
						++errors, printf("Test617: recvfrom(server, HELO) failed (recv \"%4.4s\", not \"%4.4s\")\n", test, "HELO");
					else if (read_timeout(server, 10, 0) == -1 || recvfrom(server, test, 12, 0, (sockaddr_t *)&addr, (void *)&addrsize) == -1)
						++errors, printf("Test618: recvfrom(server, HELO) failed (%s)\n", strerror(errno));
					else if (memcmp(test + 8, "HELO", 4))
						++errors, printf("Test619: recvfrom(server, HELO) failed (recv \"%4.4s\", not \"%4.4s\")\n", test, "HELO");
					else if (read_timeout(server, 20, 0) == -1 || recvfrom(server, test, 12, 0, (sockaddr_t *)&addr, (void *)&addrsize) == -1)
						++errors, printf("Test620: recvfrom(server, HELO) failed (%s)\n", strerror(errno));
					else if (memcmp(test + 8, "HELO", 4))
						++errors, printf("Test621: recvfrom(server, HELO) failed (recv \"%4.4s\", not \"%4.4s\")\n", test, "HELO");
					else if (read_timeout(server, 40, 0) == -1 || recvfrom(server, test, 12, 0, (sockaddr_t *)&addr, (void *)&addrsize) == -1)
						++errors, printf("Test622: recvfrom(server, HELO) failed (%s)\n", strerror(errno));
					else if (memcmp(test + 8, "HELO", 4))
						++errors, printf("Test623: recvfrom(server, HELO) failed (recv \"%4.4s\", not \"%4.4s\")\n", test, "HELO");
					else if (write_timeout(server, 5, 0) == -1 || sendto(server, test, 12, 0, (sockaddr_t *)&addr, addrsize) == -1)
						++errors, printf("Test624: sendto(server, HELO) failed (%s)\n", strerror(errno));

					/* Don't respond at all */

					if (read_timeout(server, 5, 0) == -1 || recvfrom(server, test, 12, 0, (sockaddr_t *)&addr, (void *)&addrsize) == -1)
						++errors, printf("Test625: recvfrom(server, HELO) failed (%s)\n", strerror(errno));
					else if (memcmp(test + 8, "HELO", 4))
						++errors, printf("Test626: recvfrom(server, HELO) failed (recv \"%4.4s\", not \"%4.4s\")\n", test, "HELO");
					else if (read_timeout(server, 10, 0) == -1 || recvfrom(server, test, 12, 0, (sockaddr_t *)&addr, (void *)&addrsize) == -1)
						++errors, printf("Test627: recvfrom(server, HELO) failed (%s)\n", strerror(errno));
					else if (memcmp(test + 8, "HELO", 4))
						++errors, printf("Test628: recvfrom(server, HELO) failed (recv \"%4.4s\", not \"%4.4s\")\n", test, "HELO");
					else if (read_timeout(server, 20, 0) == -1 || recvfrom(server, test, 12, 0, (sockaddr_t *)&addr, (void *)&addrsize) == -1)
						++errors, printf("Test629: recvfrom(server, HELO) failed (%s)\n", strerror(errno));
					else if (memcmp(test + 8, "HELO", 4))
						++errors, printf("Test630: recvfrom(server, HELO) failed (recv \"%4.4s\", not \"%4.4s\")\n", test, "HELO");
					else if (read_timeout(server, 40, 0) == -1 || recvfrom(server, test, 12, 0, (sockaddr_t *)&addr, (void *)&addrsize) == -1)
						++errors, printf("Test631: recvfrom(server, HELO) failed (%s)\n", strerror(errno));
					else if (memcmp(test + 8, "HELO", 4))
						++errors, printf("Test632: recvfrom(server, HELO) failed (recv \"%4.4s\", not \"%4.4s\")\n", test, "HELO");
				}

				errors += wait_for_child(pid);
				break;
			}

			case 0:
			{
				errors = 0;

				if ((client = net_udp_server(NULL, NULL, 30001, 0, 0, NULL, NULL)) == -1)
					++errors, printf("Test633: net_udp_server(NULL, 30001) failed (%s)\n", strerror(errno));
				else
				{
					rudp_t *rudp;
					char test[4];
					int rc = -1;

					if (!(rudp = rudp_create()))
						++errors, printf("Test634: rudp_create() failed (%s)\n", strerror(errno));
					else
					{
						/* Receive a response immediately */

						if (write_timeout(client, 5, 0) == -1 || net_rudp_transactwith(client, rudp, "HELO", 4, 0, test, 4, 0, &addr, addrsize) == -1)
							++errors, printf("Test635: net_rudp_transactwith(client, HELO) failed%s (%s)\n", (errno == ETIMEDOUT) ? " but this might be ok" : "", strerror(errno));
						else if (memcmp(test, "HELO", 4))
							++errors, printf("Test636: net_rudp_transactwith(client, HELO) failed (received \"%4.4s\", not \"%4.4s\")\n", test, "HELO");

						if (av[1] && !strcmp(av[1], "rudp"))
						{
							/* Receive a response after 1 retransmission */

							if (write_timeout(client, 5, 0) == -1 || net_rudp_transactwith(client, rudp, "HELO", 4, 0, test, 4, 0, &addr, addrsize) == -1)
								++errors, printf("Test637: net_rudp_transactwith(client, HELO) failed%s (%s)\n", (errno == ETIMEDOUT) ? " but this might be ok" : "", strerror(errno));
							else if (memcmp(test, "HELO", 4))
								++errors, printf("Test638: net_rudp_transactwith(client, HELO) failed (received \"%4.4s\", not \"%4.4s\")\n", test, "HELO");

							/* Receive a response after 2 retransmissions */

							if (write_timeout(client, 5, 0) == -1 || net_rudp_transactwith(client, rudp, "HELO", 4, 0, test, 4, 0, &addr, addrsize) == -1)
								++errors, printf("Test639: net_rudp_transactwith(client, HELO) failed%s (%s)\n", (errno == ETIMEDOUT) ? " but this might be ok" : "", strerror(errno));
							else if (memcmp(test, "HELO", 4))
								++errors, printf("Test640: net_rudp_transactwith(client, HELO) failed (received \"%4.4s\", not \"%4.4s\")\n", test, "HELO");

							/* Receive a response after 3 retransmissions */

							if (write_timeout(client, 5, 0) == -1 || net_rudp_transactwith(client, rudp, "HELO", 4, 0, test, 4, 0, &addr, addrsize) == -1)
								++errors, printf("Test641: net_rudp_transactwith(client, HELO) failed%s (%s)\n", (errno == ETIMEDOUT) ? " but this might be ok" : "", strerror(errno));
							else if (memcmp(test, "HELO", 4))
								++errors, printf("Test642: net_rudp_transactwith(client, HELO) failed (received \"%4.4s\", not \"%4.4s\")\n", test, "HELO");

							/* Receive no response at all */

							if (write_timeout(client, 5, 0) == -1 || (rc = net_rudp_transactwith(client, rudp, "HELO", 4, 0, test, 4, 0, &addr, addrsize)) != -1)
								++errors, printf("Test643: net_rudp_transactwith(client, HELO) failed (should have timed out, returned %d)\n", rc);
							else if (errno != ETIMEDOUT)
								++errors, printf("Test644: net_rudp_transactwith(client, HELO) failed (errno %s, not %s)\n", strerror(errno), strerror(ETIMEDOUT));
						}

						if (close(client) == -1)
							++errors, printf("Test645: close(client) failed (%s)\n", strerror(errno));

						if (rudp_destroy(&rudp))
							++errors, printf("Test646: rudp_destroy() failed\n");
					}
				}

				return errors;
			}
		}

		if (close(server) == -1)
			++errors, printf("Test647: close(server) failed (%s)\n", strerror(errno));
	}

	/* Test tos (requires tcpdump) */

	/*
	** Tcpdump should show something like:
	**
	** localhost.49060 > localhost.50505: S 4067041016:4067041016(0) win 32767 <mss 16396,sackOK,timestamp 3424009 0,nop,wscale 0> (DF)
	** localhost.50505 > localhost.49060: S 4062100663:4062100663(0) ack 4067041017 win 32767 <mss 16396,sackOK,timestamp 3424009 3424009,nop,wscale 0> (DF)
	** localhost.49060 > localhost.50505: . ack 1 win 32767 <nop,nop,timestamp 3424009 3424009> (DF)
	** localhost.49060 > localhost.50505: P 1:5(4) ack 1 win 32767 <nop,nop,timestamp 3424009 3424009> (DF) [tos 0x10]
	** localhost.50505 > localhost.49060: . ack 5 win 32767 <nop,nop,timestamp 3424009 3424009> (DF)
	** localhost.49060 > localhost.50505: P 5:9(4) ack 1 win 32767 <nop,nop,timestamp 3424009 3424009> (DF) [tos 0x8]
	** localhost.50505 > localhost.49060: . ack 9 win 32767 <nop,nop,timestamp 3424009 3424009> (DF)
	** localhost.49060 > localhost.50505: P 9:13(4) ack 1 win 32767 <nop,nop,timestamp 3424009 3424009> (DF) [tos 0x4]
	** localhost.50505 > localhost.49060: . ack 13 win 32767 <nop,nop,timestamp 3424009 3424009> (DF)
	** localhost.49060 > localhost.50505: P 13:17(4) ack 1 win 32767 <nop,nop,timestamp 3424009 3424009> (DF) [tos 0x2]
	** localhost.50505 > localhost.49060: . ack 17 win 32767 <nop,nop,timestamp 3424009 3424009> (DF)
	** localhost.49060 > localhost.50505: P 17:21(4) ack 1 win 32767 <nop,nop,timestamp 3424009 3424009> (DF)
	** localhost.50505 > localhost.49060: . ack 21 win 32767 <nop,nop,timestamp 3424009 3424009> (DF)
	** localhost.49060 > localhost.50505: F 21:21(0) ack 1 win 32767 <nop,nop,timestamp 3424009 3424009> (DF)
	** localhost.50505 > localhost.49060: F 1:1(0) ack 22 win 32767 <nop,nop,timestamp 3424009 3424009> (DF)
	** localhost.49060 > localhost.50505: . ack 2 win 32767 <nop,nop,timestamp 3424009 3424009> (DF)
	*/

#ifdef AF_INET6
	if (!inet6_required())
	{
		if ((server = net_server(NULL, NULL, 50505, 0, 0, NULL, NULL)) == -1)
			++errors, printf("Test648: net_server(NULL, 50505) failed: %s\n", strerror(errno));
		else
		{
			switch (pid = fork())
			{
				case -1:
				{
					printf("Failed to fork (%s)\n", strerror(errno));
					return 1;
				}

				default:
				{
					int s;
					sockaddr_any_t addr;
					size_t addrsize = sizeof addr;

					if (read_timeout(server, 5, 0) == -1 || (s = accept(server, (sockaddr_t *)&addr, (void *)&addrsize)) == -1)
						++errors, printf("Test649: accept() failed (%s)\n", strerror(errno));
					else
					{
						char test[4];
						int bytes;

						if (read_timeout(s, 5, 0) == -1 || (bytes = read(s, test, 4)) == -1)
							++errors, printf("Test650: read(s, HELO) failed (%s)\n", strerror(errno));
						else if (bytes != 4)
							++errors, printf("Test651: read(s, HELO) failed (read %d bytes, not %d)\n", bytes, 4);
						else if (read_timeout(s, 5, 0) == -1 || (bytes = read(s, test, 4)) == -1)
							++errors, printf("Test652: read(s, HELO) failed (%s)\n", strerror(errno));
						else if (bytes != 4)
							++errors, printf("Test653: read(s, HELO) failed (read %d bytes, not %d)\n", bytes, 4);
						else if (read_timeout(s, 5, 0) == -1 || (bytes = read(s, test, 4)) == -1)
							++errors, printf("Test654: read(s, HELO) failed (%s)\n", strerror(errno));
						else if (bytes != 4)
							++errors, printf("Test655: read(s, HELO) failed (read %d bytes, not %d)\n", bytes, 4);
						else if (read_timeout(s, 5, 0) == -1 || (bytes = read(s, test, 4)) == -1)
							++errors, printf("Test656: read(s, HELO) failed (%s)\n", strerror(errno));
						else if (bytes != 4)
							++errors, printf("Test647: read(s, HELO) failed (read %d bytes, not %d)\n", bytes, 4);
						else if (read_timeout(s, 5, 0) == -1 || (bytes = read(s, test, 4)) == -1)
							++errors, printf("Test657: read(s, HELO) failed (%s)\n", strerror(errno));
						else if (bytes != 4)
							++errors, printf("Test658: read(s, HELO) failed (read %d bytes, not %d)\n", bytes, 4);

						if (close(s) == -1)
							++errors, printf("Test659: close(s) failed (%s)\n", strerror(errno));
					}

					errors += wait_for_child(pid);
					break;
				}

				case 0:
				{
					errors = 0;

					if ((client = net_client(NULL, NULL, 50505, 5, 0, 0, NULL, NULL)) == -1)
						++errors, printf("Test660: net_client(NULL, 50505) failed (%s)\n", strerror(errno));
					else
					{
						if (net_tos_lowdelay(client) == -1)
							++errors, printf("Test661: net_tos_lowdelay() failed (%s)\n", strerror(errno));
						else if (write_timeout(client, 5, 0) == -1 || write(client, "HELO", 4) == -1)
							++errors, printf("Test662: write(client, HELO) failed (%s)\n", strerror(errno));

						if (net_tos_throughput(client) == -1)
							++errors, printf("Test663: net_tos_throughput() failed (%s)\n", strerror(errno));
						else if (write_timeout(client, 5, 0) == -1 || write(client, "HELO", 4) == -1)
							++errors, printf("Test664: write(client, HELO) failed (%s)\n", strerror(errno));

						if (net_tos_reliability(client) == -1)
							++errors, printf("Test665: net_tos_reliability() failed (%s)\n", strerror(errno));
						else if (write_timeout(client, 5, 0) == -1 || write(client, "HELO", 4) == -1)
							++errors, printf("Test666: write(client, HELO) failed (%s)\n", strerror(errno));

						if (net_tos_lowcost(client) == -1)
							++errors, printf("Test667: net_tos_lowcost() failed (%s)\n", strerror(errno));
						else if (write_timeout(client, 5, 0) == -1 || write(client, "HELO", 4) == -1)
							++errors, printf("Test668: write(client, HELO) failed (%s)\n", strerror(errno));

						if (net_tos_normal(client) == -1)
							++errors, printf("Test669: net_tos_normal() failed (%s)\n", strerror(errno));
						else if (write_timeout(client, 5, 0) == -1 || write(client, "HELO", 4) == -1)
							++errors, printf("Test670: write(client, HELO) failed (%s)\n", strerror(errno));

						if (close(client) == -1)
							++errors, printf("Test671: close(client) failed (%s)\n", strerror(errno));
					}

					return errors;
				}
			}

			if (close(server) == -1)
				++errors, printf("Test672: close(server) failed (%s)\n", strerror(errno));
		}
	}
#endif

#ifdef SO_PASSCRED
#ifdef SCM_CREDENTIALS
	/* Test UNIX domain stream client and server stream sockets with recvcred() */

	if ((server = net_server("/unix", unixsock, 0, 0, 0, NULL, NULL)) == -1)
		++errors, printf("Test673: net_server(\"/unix\", \"%s\") failed: %s\n", unixsock, strerror(errno));
	else
	{
		/* Turn on passing of user credentials */
		int on = 1;

		if (setsockopt(server, SOL_SOCKET, SO_PASSCRED, &on, sizeof(on)) == -1)
			++errors, printf("Test674: setsocketopt(SO_PASSCRED) for %s failed: %s\n", unixsock, strerror(errno));

		switch (pid = fork())
		{
			case -1:
			{
				printf("Failed to fork (%s)\n", strerror(errno));
				return 1;
			}

			default:
			{
				int s;
				sockaddr_any_t addr;
				size_t addrsize = sizeof addr;

				if (read_timeout(server, 5, 0) == -1 || (s = accept(server, (sockaddr_t *)&addr, (void *)&addrsize)) == -1)
					++errors, printf("Test675: accept() failed (%s)\n", strerror(errno));
				else
				{
					struct ucred cred[1];
					char test[4];
					ssize_t bytes;

					if (read_timeout(s, 5, 0) == -1 || (bytes = recvcred(s, test, 4, 0, cred)) == -1)
						++errors, printf("Test676: recvcred(s, HELO) failed (%s)\n", strerror(errno));
					else if (cred->pid != pid)
						++errors, printf("Test677: recvcred(s, HELO) failed (cred->pid == %d, expected %d)\n", (int)cred->pid, (int)pid);
					else if (cred->uid != getuid())
						++errors, printf("Test678: recvcred(s, HELO) failed (cred->uid == %d, expected %d)\n", (int)cred->uid, (int)getuid());
					else if (cred->gid != getgid())
						++errors, printf("Test679: recvcred(s, HELO) failed (cred->gid == %d, expected %d)\n", (int)cred->gid, (int)getgid());
					else if (bytes != 4)
						++errors, printf("Test680: recvcred(s, HELO) failed (read %d bytes, not %d bytes)\n", bytes, 4);
					else if (memcmp(test, "HELO", 4))
						++errors, printf("Test681: recvcred(s, HELO) failed (read \"%4.4s\", not \"%4.4s\")\n", test, "HELO");
					else if (write_timeout(s, 5, 0) == -1 || write(s, "OLEH", 4) == -1)
						++errors, printf("Test2682: write(s, OLEH) failed (%s)\n", strerror(errno));
					if (close(s) == -1)
						++errors, printf("Test683: close(s) failed (%s)\n", strerror(errno));
				}

				errors += wait_for_child(pid);
				break;
			}

			case 0:
			{
				errors = 0;

				if ((client = net_client("/unix", unixsock, 0, 5, 0, 0, NULL, NULL)) == -1)
					++errors, printf("Test684: net_client(\"/unix\", \"%s\") failed (%s)\n", unixsock, strerror(errno));
				else
				{
					char test[4];

					if (write_timeout(client, 5, 0) == -1 || write(client, "HELO", 4) == -1)
						++errors, printf("Test685: write(client, HELO) failed (%s)\n", strerror(errno));
					else if (read_timeout(client, 5, 0) == -1 || read(client, test, 4) == -1)
						++errors, printf("Test686: read(client, OLEH) failed (%s)\n", strerror(errno));
					else if (memcmp(test, "OLEH", 4))
						++errors, printf("Test687: read(client, OLEH) failed (read \"%4.4s\", not \"%4.4s\")\n", test, "OLEH");
					if (close(client) == -1)
						++errors, printf("Test688: close(client) failed (%s)\n", strerror(errno));
				}

				return errors;
			}
		}

		if (close(server) == -1)
			++errors, printf("Test689: close(server) failed (%s)\n", strerror(errno));
	}

	unlink(unixsock);

	/* Test UNIX domain datagram client and server datagram sockets with recvfromcred() */

	if ((server = net_udp_server("/unix", unixsock, 0, 0, 0, NULL, NULL)) == -1)
		++errors, printf("Test690: net_udp_server(\"/unix\", \"%s\") failed: %s\n", unixsock, strerror(errno));
	else
	{
		/* Turn on passing of user credentials */
		int on = 1;

		if (setsockopt(server, SOL_SOCKET, SO_PASSCRED, &on, sizeof(on)) == -1)
			++errors, printf("Test691: setsocketopt(SO_PASSCRED) for %s failed: %s\n", unixsock, strerror(errno));

		switch (pid = fork())
		{
			case -1:
			{
				printf("Failed to fork (%s)\n", strerror(errno));
				return 1;
			}

			default:
			{
				struct ucred cred[1];
				char test[4];
				ssize_t bytes;
				sockaddr_any_t addr;
				size_t addrsize = sizeof addr;

				if (read_timeout(server, 5, 0) == -1 || (bytes = recvfromcred(server, test, 4, 0, (sockaddr_t *)&addr, (socklen_t *)&addrsize, cred)) == -1)
					++errors, printf("Test692: recvcred(server, HELO) failed (%s)\n", strerror(errno));
				else if (cred->pid != pid)
					++errors, printf("Test693: recvcred(s, HELO) failed (cred->pid == %d, expected %d)\n", (int)cred->pid, (int)pid);
				else if (cred->uid != getuid())
					++errors, printf("Test694: recvcred(s, HELO) failed (cred->uid == %d, expected %d)\n", (int)cred->uid, (int)getuid());
				else if (cred->gid != getgid())
					++errors, printf("Test695: recvcred(s, HELO) failed (cred->gid == %d, expected %d)\n", (int)cred->gid, (int)getgid());
				else if (bytes != 4)
					++errors, printf("Test696: recvcred(s, HELO) failed (read %d bytes, not %d bytes)\n", bytes, 4);
				else if (memcmp(test, "HELO", 4))
					++errors, printf("Test697: recvcred(server, HELO) failed (recv \"%4.4s\", not \"%4.4s\")\n", test, "HELO");
				else if (write_timeout(server, 5, 0) == -1 || sendto(server, "OLEH", 4, 0, (sockaddr_t *)&addr, addrsize) == -1)
					++errors, printf("Test699: sendto(server, OLEH) failed (%s)\n", strerror(errno));

				errors += wait_for_child(pid);
				break;
			}

			case 0:
			{
				errors = 0;

				if ((client = net_udp_client("/unix", unixsock, 0, 0, 0, NULL, NULL)) == -1)
					++errors, printf("Test700: net_udp_client(\"/unix\", \"%s\") failed (%s)\n", unixsock, strerror(errno));
				else
				{
					char test[4];
					sockaddr_any_t addr;
					size_t addrsize = sizeof addr;

					if (write_timeout(client, 5, 0) == -1 || send(client, "HELO", 4, 0) == -1)
						++errors, printf("Test701: send(client, HELO) failed (%s)\n", strerror(errno));
					else if (read_timeout(client, 5, 0) == -1 || recv(client, test, 4, 0) == -1)
						++errors, printf("Test702: recv(client, OLEH) failed (%s)\n", strerror(errno));
					else if (memcmp(test, "OLEH", 4))
						++errors, printf("Test703: recv(client, OLEH) failed (recv \"%4.4s\", not \"%4.4s\")\n", test, "OLEH");
					if (getsockname(client, (sockaddr_t *)&addr, (void *)&addrsize) != -1)
						if (*addr.un.sun_path)
							unlink(addr.un.sun_path);
					if (close(client) == -1)
						++errors, printf("Test704: close(client) failed (%s)\n", strerror(errno));
				}

				return errors;
			}
		}

		if (close(server) == -1)
			++errors, printf("Test705: close(server) failed (%s)\n", strerror(errno));
	}

	unlink(unixsock);
#endif
#endif

	if (errors)
		printf("%d/705 tests failed\n", errors);
	else
		printf("All tests passed\n");

	if (no_mailserver)
	{
		printf("\n");
		printf("    Note: You can also perform mail tests.\n");
		printf("    Rerun the test on a system with an SMTP server.\n");
	}
	else
	{
		printf("\n");
		printf("    Note: Can't verify mail delivery.\n");
		printf("    Look for mail consisting of: \"subject\" and \"message\"\n");
	}

	if (no_multicast)
	{
		printf("\n");
		printf("    Note: You can also perform multicast tests.\n");
		printf("    Rerun the test with \"%s multicast\" if your system supports multicast.\n", *av);
	}

	if (no_rudp)
	{
		printf("\n");
		printf("    Note: You can also perform rudp tests with retransmissions.\n");
		printf("    Rerun the test with \"%s rudp\" (takes about 105 seconds).\n", *av);
	}

#ifdef AF_INET6
	if (!inet6_required())
	{
		printf("\n");
		printf("    Note: To verify the type of service (tos) tests, run the test after\n");
		printf("    starting tcpdump to monitor the loopback interface (look for port 50505).\n");
	}
#endif

	return (errors == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

#endif

/* vi:set ts=4 sw=4: */
