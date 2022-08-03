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

#ifndef LIBSLACK_STD_H
#define LIBSLACK_STD_H

#ifndef NO_STDC
#ifndef __STDC__
#define __STDC__
#endif
#endif

#ifndef NO_STRICT_ANSI
#ifndef __STRICT_ANSI__
#define __STRICT_ANSI__
#endif
#endif

#ifndef NO_POSIX_SOURCE
#ifndef _POSIX_SOURCE
#define _POSIX_SOURCE
#endif
#endif

#ifndef NO_POSIX_C_SOURCE
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809
#endif
#endif

#ifndef NO_XOPEN_SOURCE
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 700
#endif
#endif

#ifndef NO_ISOC9X_SOURCE
#ifndef _ISOC9X_SOURCE
#define _ISOC9X_SOURCE
#endif
#endif

#ifndef NO_EXTENSIONS
#ifndef __EXTENSIONS__
#define __EXTENSIONS__
#endif
#endif

#ifndef NO_REENTRANT
#ifndef _REENTRANT
#define _REENTRANT
#endif
#endif

#ifndef NO_THREAD_SAFE
#ifndef _THREAD_SAFE
#define _THREAD_SAFE
#endif
#endif

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <locale.h>
#include <math.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

#include <unistd.h>
#include <pthread.h>

#endif

/* vi:set ts=4 sw=4: */
