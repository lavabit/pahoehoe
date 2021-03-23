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

#ifndef LIBSLACK_MSG_H
#define LIBSLACK_MSG_H

#include <stdarg.h>

#include <sys/syslog.h>

#include <slack/hdr.h>
#include <slack/locker.h>

#ifndef MSG_SIZE
#define MSG_SIZE 8192
#endif

typedef struct Msg Msg;
typedef void msg_out_t(void *data, const void *mesg, size_t mesglen);
typedef int msg_filter_t(void **mesgp, const void *mesg, size_t mesglen);
typedef void msg_release_t(void *data);

_begin_decls
Msg *msg_create(int type, msg_out_t *out, void *data, msg_release_t *destroy);
Msg *msg_create_with_locker(Locker *locker, int type, msg_out_t *out, void *data, msg_release_t *destroy);
int msg_rdlock(Msg *mesg);
int msg_wrlock(Msg *mesg);
int msg_unlock(Msg *mesg);
void msg_release(Msg *mesg);
void *msg_destroy(Msg **mesg);
void msg_out(Msg *dst, const char *format, ...);
void msg_out_unlocked(Msg *dst, const char *format, ...);
void vmsg_out(Msg *dst, const char *format, va_list args);
void vmsg_out_unlocked(Msg *dst, const char *format, va_list args);
Msg *msg_create_fd(int fd);
Msg *msg_create_fd_with_locker(Locker *locker, int fd);
Msg *msg_create_stderr(void);
Msg *msg_create_stderr_with_locker(Locker *locker);
Msg *msg_create_stdout(void);
Msg *msg_create_stdout_with_locker(Locker *locker);
Msg *msg_create_file(const char *path);
Msg *msg_create_file_with_locker(Locker *locker, const char *path);
Msg *msg_create_syslog(const char *ident, int option, int facility, int priority);
Msg *msg_create_syslog_with_locker(Locker *locker, const char *ident, int option, int facility, int priority);
Msg *msg_syslog_set_facility(Msg *mesg, int facility);
Msg *msg_syslog_set_facility_unlocked(Msg *mesg, int facility);
Msg *msg_syslog_set_priority(Msg *mesg, int priority);
Msg *msg_syslog_set_priority_unlocked(Msg *mesg, int priority);
Msg *msg_create_plex(Msg *msg1, Msg *msg2);
Msg *msg_create_plex_with_locker(Locker *locker, Msg *msg1, Msg *msg2);
int msg_add_plex(Msg *mesg, Msg *item);
int msg_add_plex_unlocked(Msg *mesg, Msg *item);
Msg *msg_create_filter(msg_filter_t *filter, Msg *mesg);
Msg *msg_create_filter_with_locker(Locker *locker, msg_filter_t *filter, Msg *mesg);
const char *msg_set_timestamp_format(const char *format);
int msg_set_timestamp_format_locker(Locker *locker);
int syslog_lookup_facility(const char *facility);
int syslog_lookup_priority(const char *priority);
const char *syslog_facility_str(int spec);
const char *syslog_priority_str(int spec);
int syslog_parse(const char *spec, int *facility, int *priority);
_end_decls

#endif

/* vi:set ts=4 sw=4: */
