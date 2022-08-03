/*
* daemon - http://libslack.org/daemon/
*
* Copyright (C) 1999-2004, 2004, 2010, 2020-2021 raf <raf@raf.org>
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
* 20210304 raf <raf@raf.org>
*/

#ifndef DAEMON_CONFIG_H
#define DAEMON_CONFIG_H

/* Define if we have systemd's logind or elogind (i.e. libsystemd or libelogind) */
/* #undef HAVE_LOGIND */

/* Define if we have <sys/ttydefaults.h> and need it for CEOF in musl libc (Linux only)  */
#define HAVE_SYS_TTYDEFAULTS_H 1

#endif

/* vi:set ts=4 sw=4: */
