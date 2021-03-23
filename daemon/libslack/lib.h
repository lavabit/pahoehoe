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

#ifndef LIBSLACK_LIB_H
#define LIBSLACK_LIB_H

#include <slack/std.h>
#include <slack/agent.h>
#include <slack/coproc.h>
#include <slack/daemon.h>
#include <slack/err.h>
#include <slack/fio.h>
#include <slack/hsort.h>
#include <slack/lim.h>
#include <slack/link.h>
#include <slack/list.h>
#include <slack/locker.h>
#include <slack/map.h>
#include <slack/mem.h>
#include <slack/msg.h>
#include <slack/net.h>
#include <slack/prog.h>
#include <slack/prop.h>
#include <slack/pseudo.h>
#include <slack/sig.h>
#include <slack/str.h>

#ifndef HAVE_SNPRINTF
#include <slack/snprintf.h>
#endif

#ifndef HAVE_VSSCANF
#include <slack/vsscanf.h>
#endif

#endif

/* vi:set ts=4 sw=4: */
