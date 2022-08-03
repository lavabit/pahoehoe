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

#ifndef LIBSLACK_ERR_H
#define LIBSLACK_ERR_H

#include <stdlib.h>
#include <stdarg.h>

#include <slack/hdr.h>

#if __cplusplus
#define void_cast static_cast<void>
#else
#define void_cast (void)
#endif

#undef debug
#undef vdebug
#undef debugsys
#undef vdebugsys
#undef check

#ifdef NDEBUG
#define debug(args)
#define vdebug(args)
#define debugsys(args)
#define vdebugsys(args)
#define check(cond, mesg) (void_cast(0))
#else
#define debug(args) debugf args;
#define vdebug(args) vdebugf args;
#define debugsys(args) debugsysf args;
#define vdebugsys(args) vdebugsysf args;
#define check(cond, mesg) ((cond) ? void_cast(0) : (dump("Internal Error: %s: %s [%s:%d]", (#cond), (mesg), __FILE__, __LINE__)))
#endif

_begin_decls
void msg(const char *format, ...);
void vmsg(const char *format, va_list args);
void verbose(size_t level, const char *format, ...);
void vverbose(size_t level, const char *format, va_list args);
void debugf(size_t level, const char *format, ...);
void vdebugf(size_t level, const char *format, va_list args);
int error(const char *format, ...);
int verror(const char *format, va_list args);
void fatal(const char *format, ...);
void vfatal(const char *format, va_list args);
void dump(const char *format, ...);
void vdump(const char *format, va_list args);
void alert(int priority, const char *format, ...);
void valert(int priority, const char *format, va_list args);
void debugsysf(size_t level, const char *format, ...);
void vdebugsysf(size_t level, const char *format, va_list args);
int errorsys(const char *format, ...);
int verrorsys(const char *format, va_list args);
void fatalsys(const char *format, ...);
void vfatalsys(const char *format, va_list args);
void dumpsys(const char *format, ...);
void vdumpsys(const char *format, va_list args);
void alertsys(int priority, const char *format, ...);
void valertsys(int priority, const char *format, va_list args);
int set_errno(int errnum);
void *set_errnull(int errnum);
void (*set_errnullf(int errnum))();
_end_decls

/* Don't look below here - optimisations only */
/* And they produce lots of warnings with gcc-4.1 */

/* #define set_errno(errnum) (errno = (errnum), -1) */
/* #define set_errnull(errnum) ((void *)((void *)(errno = (errnum)), NULL)) */

#endif

/* vi:set ts=4 sw=4: */
