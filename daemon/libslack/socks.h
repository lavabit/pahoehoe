/*
* libslack - http://libslack.org/
*
* Copyright (C) 1999-2002, 2004, 2010, 2020 raf <raf@raf.org>
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
* 20201111 raf <raf@raf.org>
*/

#ifndef LIBSLACK_SOCKS_H
#define LIBSLACK_SOCKS_H

#ifdef SOCKS
#ifdef SOCKS4
#define SOCKS_PREFIX R
#else
#define SOCKS_PREFIX SOCKS
#endif
#define connect         SOCKS_PREFIX ## connect
#define getsockname     SOCKS_PREFIX ## getsockname
#define getpeername     SOCKS_PREFIX ## getpeername
#define bind            SOCKS_PREFIX ## bind
#define accept          SOCKS_PREFIX ## accept
#define listen          SOCKS_PREFIX ## listen
#define select          SOCKS_PREFIX ## select
#define recvfrom        SOCKS_PREFIX ## recvfrom
#define sendto          SOCKS_PREFIX ## sendto
#define recv            SOCKS_PREFIX ## recv
#define send            SOCKS_PREFIX ## send
#define read            SOCKS_PREFIX ## read
#define write           SOCKS_PREFIX ## write
#define rresvport       SOCKS_PREFIX ## rresvport
#define shutdown        SOCKS_PREFIX ## shutdown
#define listen          SOCKS_PREFIX ## listen
#define close           SOCKS_PREFIX ## close
#define dup             SOCKS_PREFIX ## dup
#define dup2            SOCKS_PREFIX ## dup2
#define fclose          SOCKS_PREFIX ## fclose
#define gethostbyname   SOCKS_PREFIX ## gethostbyname
#undef SOCKS_PREFIX
#endif

#endif

/* vi:set ts=4 sw=4: */
