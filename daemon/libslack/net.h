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

#ifndef LIBSLACK_NET_H
#define LIBSLACK_NET_H

#include <stdarg.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>

#include <slack/list.h>

#include <slack/hdr.h>

typedef struct sockaddr sockaddr_t;
typedef unsigned short sockport_t;
typedef struct sockopt_t sockopt_t;

typedef union sockaddr_any_t sockaddr_any_t;
typedef struct sockaddr_un sockaddr_un_t;
typedef struct sockaddr_in sockaddr_in_t;
#ifdef AF_INET6
typedef struct sockaddr_in6 sockaddr_in6_t;
#endif

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
#ifdef AF_INET6
	sockaddr_in6_t in6;
#endif
};

#ifndef IFNAMSIZ
#define IFNAMSIZ 16
#endif

#ifndef IFHWADDRLEN
#define IFHWADDRLEN 6
#endif

struct net_interface_t
{
	char name[IFNAMSIZ];      /* network interface name, null terminated */
	unsigned int index;       /* network interface index */
	short flags;              /* IFF_ constants from <net/if.h> */
	int mtu;                  /* Maximum Transmission Unit */
	sockaddr_any_t *addr;       /* address */
	sockaddr_any_t *brdaddr;    /* broadcast address, if any */
	sockaddr_any_t *dstaddr;    /* destination address, if any */
	sockaddr_any_t *hwaddr;     /* hardware address, if any (not on Solaris) */
};

_begin_decls
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
_end_decls

#endif

/* vi:set ts=4 sw=4: */
