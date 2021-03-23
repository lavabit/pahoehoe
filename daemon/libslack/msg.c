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

/*

=head1 NAME

I<libslack(msg)> - message module

=head1 SYNOPSIS

    #include <slack/std.h>
    #include <slack/msg.h>

    typedef struct Msg Msg;
    typedef void msg_out_t(void *data, const void *mesg, size_t mesglen);
    typedef int msg_filter_t(void **mesgp, const void *mesg, size_t mesglen);
    typedef void msg_release_t(void *data);

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

=head1 DESCRIPTION

This module provides general messaging functions. Message channels can be
created that send messages to a file descriptor, a file, I<syslog> or a
client defined message handler or that multiplexes messages to any
combination of the above. Messages sent to files are timestamped using (by
default) the I<strftime(3)> format: C<"%Y%m%d %H:%M:%S">.

It also provides functions for parsing I<syslog> targets, converting between
I<syslog> facility names and codes, and converting between I<syslog>
priority names and codes.

=over 4

=cut

*/

#ifndef _BSD_SOURCE
#define _BSD_SOURCE /* For snprintf() on OpenBSD-4.7 */
#endif

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE /* New name for _BSD_SOURCE */
#endif

#include "config.h"
#include "std.h"

#include <syslog.h>
#include <fcntl.h>
#include <time.h>

#include <sys/stat.h>

#include "msg.h"
#include "mem.h"
#include "err.h"
#include "str.h"

#ifndef HAVE_SNPRINTF
#include "snprintf.h"
#endif

typedef int MsgFDData;
typedef struct MsgFileData MsgFileData;
typedef struct MsgSyslogData MsgSyslogData;
typedef struct MsgFilterData MsgFilterData;
typedef struct MsgPlexData MsgPlexData;

#define MSG_FD 1
#define MSG_FILE 2
#define MSG_SYSLOG 3
#define MSG_PLEX 4
#define MSG_FILTER 5

struct Msg
{
	int type;               /* subtype */
	msg_out_t *out;         /* message handling function */
	void *data;             /* subtype specific data */
	msg_release_t *destroy; /* destructor function for data */
	Locker *locker;         /* locking strategy for this structure */
};

struct MsgFileData
{
	int fd;       /* file descriptor (-1 if open failed) */
};

struct MsgSyslogData
{
	int facility; /* syslog(3) priority */
	int priority; /* syslog(3) priority */
};

struct MsgPlexData
{
	size_t size;   /* elements allocated */
	size_t length; /* length of Msg list */
	Msg **list;    /* list of Msg objects */
};

struct MsgFilterData
{
	msg_filter_t *filter; /* filter function */
	Msg *mesg;            /* destination Msg */
};

typedef struct syslog_map_t syslog_map_t;

struct syslog_map_t
{
	char *name;
	int val;
};

/*
** The following masks may be wrong on some systems.
*/

#ifndef LOG_PRIMASK
#define LOG_PRIMASK 0x0007
#endif

#ifndef LOG_FACMASK
#define LOG_FACMASK 0x03f8
#endif

static const syslog_map_t syslog_facility_map[] =
{
	{ "kern",   LOG_KERN },
	{ "user",   LOG_USER },
	{ "mail",   LOG_MAIL },
	{ "daemon", LOG_DAEMON },
	{ "auth",   LOG_AUTH },
	{ "syslog", LOG_SYSLOG },
	{ "lpr",    LOG_LPR },
	{ "news",   LOG_NEWS },
	{ "uucp",   LOG_UUCP },
	{ "cron",   LOG_CRON },
	{ "local0", LOG_LOCAL0 },
	{ "local1", LOG_LOCAL1 },
	{ "local2", LOG_LOCAL2 },
	{ "local3", LOG_LOCAL3 },
	{ "local4", LOG_LOCAL4 },
	{ "local5", LOG_LOCAL5 },
	{ "local6", LOG_LOCAL6 },
	{ "local7", LOG_LOCAL7 },
	{ NULL,     -1 }
};

static const syslog_map_t syslog_priority_map[] =
{
	{ "emerg",   LOG_EMERG },
	{ "alert",   LOG_ALERT },
	{ "crit",    LOG_CRIT },
	{ "err",     LOG_ERR },
	{ "warning", LOG_WARNING },
#ifdef LOG_NOTICE
	{ "notice",  LOG_NOTICE },
#endif
	{ "info",    LOG_INFO },
	{ "debug",   LOG_DEBUG },
	{ NULL,      -1 }
};

#ifndef TEST

static const char *timestamp_format = "%Y%m%d %H:%M:%S ";
static Locker *timestamp_format_locker = NULL;

/*

=item C<Msg *msg_create(int type, msg_out_t *out, void *data, msg_release_t *destroy)>

Creates a I<Msg> object initialised with C<type>, C<out>, C<data> and
C<destroy>. Client defined message handlers must specify a C<type> greater
than C<5>. It is the caller's responsibility to deallocate the new I<Msg>
with I<msg_release(3)> or I<msg_destroy>. On success, returns the new I<Msg>
object. On error, returns C<null>.

=cut

*/

Msg *msg_create(int type, msg_out_t *out, void *data, msg_release_t *destroy)
{
	return msg_create_with_locker(NULL, type, out, data, destroy);
}

/*

=item C<Msg *msg_create_with_locker(Locker *locker, int type, msg_out_t *out, void *data, msg_release_t *destroy)>

Equivalent to I<msg_create(3)> except that multiple threads accessing the
new I<Msg> will be synchronised by C<locker>.

=cut

*/

Msg *msg_create_with_locker(Locker *locker, int type, msg_out_t *out, void *data, msg_release_t *destroy)
{
	Msg *mesg;

	if (!(mesg = mem_new(Msg)))
		return NULL;

	mesg->type = type;
	mesg->out = out;
	mesg->data = data;
	mesg->destroy = destroy;
	mesg->locker = locker;

	return mesg;
}

/*

=item C<int msg_rdlock(Msg *mesg)>

Claims a read lock on C<mesg> (if C<mesg> was created with a I<Locker>).
This is needed when multiple read only I<msg(3)> module functions
need to be called atomically. It is the caller's responsibility to call
I<msg_unlock(3)> after the atomic operation. The only functions that may be
called on C<mesg> between calls to I<msg_rdlock(3)> and I<msg_unlock(3)> are
any read only I<msg(3)> module functions whose name ends with
C<_unlocked>. On success, returns C<0>. On error, returns an error code.

=cut

*/

#define msg_rdlock(mesg) ((mesg) ? locker_rdlock((mesg)->locker) : EINVAL)
#define msg_wrlock(mesg) ((mesg) ? locker_wrlock((mesg)->locker) : EINVAL)
#define msg_unlock(mesg) ((mesg) ? locker_unlock((mesg)->locker) : EINVAL)

int (msg_rdlock)(Msg *mesg)
{
	return msg_rdlock(mesg);
}

/*

=item C<int msg_wrlock(Msg *mesg)>

Claims a write lock on C<mesg>.

Claims a write lock on C<mesg> (if C<mesg> was created with a I<Locker>).
This is needed when multiple read/write I<msg(3)> module functions
need to be called atomically. It is the caller's responsibility to call
I<msg_unlock(3)> after the atomic operation. The only functions that may be
called on C<mesg> between calls to I<msg_rdlock(3)> and I<msg_unlock(3)> are
any I<msg(3)> module functions whose name ends with C<_unlocked>. On
success, returns C<0>. On error, returns an error code.

=cut

*/

int (msg_wrlock)(Msg *mesg)
{
	return msg_wrlock(mesg);
}

/*

=item C<int msg_unlock(Msg *mesg)>

Unlocks a read or write lock on C<mesg> obtained with I<msg_rdlock(3)> or
I<msg_wrlock(3)> (if C<mesg> was created with a I<Locker>).  On success,
returns C<0>. On error, returns an error code.

=cut

*/

int (msg_unlock)(Msg *mesg)
{
	return msg_unlock(mesg);
}

/*

=item C<void msg_release(Msg *mesg)>

Releases (deallocates) C<mesg> and its internal data.

=cut

*/

void msg_release(Msg *mesg)
{
	if (!mesg)
		return;

	if (mesg->destroy)
		mesg->destroy(mesg->data);

	mem_release(mesg);
}

/*

=item C<void *msg_destroy(Msg **mesg)>

Destroys (deallocates and sets to C<null>) C<*mesg>. Returns C<null>.

=cut

*/

void *msg_destroy(Msg **mesg)
{
	if (mesg && *mesg)
	{
		msg_release(*mesg);
		*mesg = NULL;
	}

	return NULL;
}

/*

=item C<void msg_out(Msg *dst, const char *format, ...)>

Sends a message to C<dst>. C<format> is a I<printf(3)>-like format string.
Any remaining arguments are processed as in I<printf(3)>.

B<Warning: Do not under any circumstances ever pass a non-literal string as
the format argument unless you know exactly how many conversions will take
place. Being careless with this is a very good way to build potential
security holes into your programs. The same is true for all functions that
take a printf()-like format string as an argument.>

    msg_out(dst, buf);       // EVIL
    msg_out(dst, "%s", buf); // GOOD

=cut

*/

void msg_out(Msg *dst, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	vmsg_out(dst, format, args);
	va_end(args);
}

/*

=item C<void msg_out_unlocked(Msg *dst, const char *format, ...)>

Equivalent to I<msg_out(3)> except that C<dst> is not read locked.

=cut

*/

void msg_out_unlocked(Msg *dst, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	vmsg_out_unlocked(dst, format, args);
	va_end(args);
}

/*

=item C<void vmsg_out(Msg *dst, const char *format, va_list args)>

Sends a message to C<dst>. C<format> is a I<printf(3)>-like format string.
C<args> is processed as in I<vprintf(3)>.

=cut

*/

void vmsg_out(Msg *dst, const char *format, va_list args)
{
	int err;

	if (!dst)
		return;

	if ((err = msg_rdlock(dst)))
	{
		set_errno(err);
		return;
	}

	vmsg_out_unlocked(dst, format, args);

	if ((err = msg_unlock(dst)))
		set_errno(err);
}

/*

=item C<void vmsg_out_unlocked(Msg *dst, const char *format, va_list args)>

Equivalent to I<vmsg_out(3)> except that C<dst> is not read locked.

=cut

*/

void vmsg_out_unlocked(Msg *dst, const char *format, va_list args)
{
	if (!dst)
		return;

	if (dst->out)
	{
		char mesg[MSG_SIZE];
		vsnprintf(mesg, MSG_SIZE, format, args);
		dst->out(dst->data, mesg, strlen(mesg));
	}
}

/*

C<MsgFDData *msg_fddata_create(int fd)>

Creates and initialises the internal data needed by a I<Msg> object that
sends messages to file descriptor C<fd>. On success, returns the data. On
error, returns C<null>.

*/

static MsgFDData *msg_fddata_create(int fd)
{
	MsgFDData *data;

	if (!(data = mem_new(MsgFDData)))
		return NULL;

	*data = fd;

	return data;
}

/*

C<void msg_fddata_release(MsgFDData *data)>

Releases (deallocates) the internal data needed by a I<Msg> object that
sends messages to a file descriptor. The file descriptor is not closed.

*/

static void msg_fddata_release(MsgFDData *data)
{
	mem_release(data);
}

/*

C<void msg_out_fd(void *data, const void *mesg, size_t mesglen)>

Sends a message to a file descriptor. C<data> is a pointer to the file
descriptor. C<mesg> is the message. C<mesglen> is it's length.

*/

static void msg_out_fd(void *data, const void *mesg, size_t mesglen)
{
	if (data && mesg)
		if (write(*(MsgFDData *)data, mesg, mesglen) == -1)
			/* Avoid gcc warning */;
}

/*

=item C<Msg *msg_create_fd(int fd)>

Creates a I<Msg> object that sends messages to file descriptor C<fd>. It is
the caller's responsibility to deallocate the new I<Msg> with
I<msg_release(3)> or I<msg_destroy(3)>. On success, returns the new I<Msg>
object. On error, returns C<null>.

=cut

*/

Msg *msg_create_fd(int fd)
{
	return msg_create_fd_with_locker(NULL, fd);
}

/*

=item C<Msg *msg_create_fd_with_locker(Locker *locker, int fd)>

Equivalent to I<msg_create_fd(3)> except that multiple threads accessing the
new I<Msg> will be synchronised by C<locker>.

=cut

*/

Msg *msg_create_fd_with_locker(Locker *locker, int fd)
{
	MsgFDData *data;
	Msg *mesg;

	if (!(data = msg_fddata_create(fd)))
		return NULL;

	if (!(mesg = msg_create_with_locker(locker, MSG_FD, msg_out_fd, data, (msg_release_t *)msg_fddata_release)))
	{
		msg_fddata_release(data);
		return NULL;
	}

	return mesg;
}

/*

=item C<Msg *msg_create_stderr(void)>

Creates a I<Msg> object that sends messages to standard error. It is the
caller's responsibility to deallocate the new I<Msg> with I<msg_release(3)>
or I<msg_destroy(3)>. On success, returns the new I<Msg> object. On error,
returns C<null>.

=cut

*/

Msg *msg_create_stderr(void)
{
	return msg_create_fd_with_locker(NULL, STDERR_FILENO);
}

/*

=item C<Msg *msg_create_stderr_with_locker(Locker *locker)>

Equivalent to I<msg_create_stderr(3)> except that multiple threads accessing
the new I<Msg> will be synchronised by C<locker>.

=cut

*/

Msg *msg_create_stderr_with_locker(Locker *locker)
{
	return msg_create_fd_with_locker(locker, STDERR_FILENO);
}

/*

=item C<Msg *msg_create_stdout(void)>

Creates a I<Msg> object that sends messages to standard output. It is the
caller's responsibility to deallocate the new I<Msg> with I<msg_release(3)>
or I<msg_destroy(3)>. On success, returns the new I<Msg> object. On error,
returns C<null>.

=cut

*/

Msg *msg_create_stdout(void)
{
	return msg_create_fd_with_locker(NULL, STDOUT_FILENO);
}

/*

=item C<Msg *msg_create_stdout_with_locker(Locker *locker)>

Equivalent to I<msg_create_stdout(3)> except that multiple threads accessing
the new I<Msg> will be synchronised by C<locker>.

=cut

*/

Msg *msg_create_stdout_with_locker(Locker *locker)
{
	return msg_create_fd_with_locker(locker, STDOUT_FILENO);
}

/*

C<int msg_filedata_init(MsgFileData *data, const char *path)>

Initialises the internal data needed by a I<Msg> object that sends messages
to the file specified by C<path>. This data consists of a copy of C<path>
and an open file descriptor to the file. The file descriptor is opened with
the C<O_WRONLY>, C<O_CREAT> and C<O_APPEND> flags. On success, returns C<0>.
On error, returns C<-1> with C<errno> set appropriately.

*/

static int msg_filedata_init(MsgFileData *data, const char *path)
{
	mode_t mode;

	if (!data || !path)
		return set_errno(EINVAL);

	mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;

	if ((data->fd = open(path, O_WRONLY | O_CREAT | O_APPEND, mode)) == -1)
		return -1;

	return 0;
}

/*

C<MsgFileData *msg_filedata_create(const char *path)>

Creates the internal data needed by a I<Msg> object that sends messages to
the file specified by C<path>. On success, returns the data. On error,
returns C<null> with C<errno> set appropriately.

*/

static MsgFileData *msg_filedata_create(const char *path)
{
	MsgFileData *data;

	if (!(data = mem_new(MsgFileData)))
		return NULL;

	if (msg_filedata_init(data, path) == -1)
	{
		mem_release(data);
		return NULL;
	}

	return data;
}

/*

C<void msg_filedata_release(MsgFileData *data)>

Releases (deallocates) the internal data needed by a I<Msg> object that
sends messages to a file. The file descriptor is closed first.

*/

static void msg_filedata_release(MsgFileData *data)
{
	if (!data)
		return;

	if (data->fd != -1)
		close(data->fd);

	mem_release(data);
}

/*

C<void msg_out_file(void *data, const void *mesg, size_t mesglen)>

Sends a message to a file. C<data> contains the file descriptor. C<mesg> is
the message. C<mesglen> is it's length. On error, sets C<errno>
appropriately.

*/

static void msg_out_file(void *data, const void *mesg, size_t mesglen)
{
	MsgFileData *dst = data;
	char buf[MSG_SIZE];
	size_t buflen;
	int err;

	time_t t = time(NULL);

	if ((err = locker_rdlock(timestamp_format_locker)))
	{
		set_errno(err);
		return;
	}

	strftime(buf, MSG_SIZE, timestamp_format, localtime(&t));

	if ((err = locker_unlock(timestamp_format_locker)))
	{
		set_errno(err);
		return;
	}

	buflen = strlen(buf);
	if (buflen + mesglen >= MSG_SIZE)
		mesglen -= MSG_SIZE - buflen;
	memmove(buf + buflen, mesg, mesglen);

	if (mesg && dst && dst->fd != -1)
		if (write(dst->fd, buf, buflen + mesglen) == -1)
			/* Avoid gcc warning */;
}

/*

=item C<Msg *msg_create_file(const char *path)>

Creates a I<Msg> object that sends messages to the file specified by
C<path>. It is the caller's responsibility to deallocate the new I<Msg> with
I<msg_release(3)> or I<msg_destroy(3)>. On success, returns the new I<Msg>
object. On error, returns C<null> with C<errno> set appropriately.

=cut

*/

Msg *msg_create_file(const char *path)
{
	return msg_create_file_with_locker(NULL, path);
}

/*

=item C<Msg *msg_create_file_with_locker(Locker *locker, const char *path)>

Equivalent to I<msg_create_file(3)> except that multiple threads accessing
the new I<Msg> will be synchronised by C<locker>.

=cut

*/

Msg *msg_create_file_with_locker(Locker *locker, const char *path)
{
	MsgFileData *data;
	Msg *mesg;

	if (!(data = msg_filedata_create(path)))
		return NULL;

	if (!(mesg = msg_create_with_locker(locker, MSG_FILE, msg_out_file, data, (msg_release_t *)msg_filedata_release)))
	{
		msg_filedata_release(data);
		return NULL;
	}

	return mesg;
}

/*

C<int msg_sysdata_init(MsgSyslogData *data, const char *ident, int option, int facility, int priority)>

Initialises the internal data needed by a I<Msg> object that sends messages
to I<syslog>. I<openlog(3)> is called with C<ident> and C<option>.
C<facility> and C<priority> are stored to be used when sending messages. On
success, returns C<0>. On error, returns C<-1> with C<errno> set
appropriately.

*/

static int msg_sysdata_init(MsgSyslogData *data, const char *ident, int option, int facility, int priority)
{
	if (!data || facility == -1)
		return set_errno(EINVAL);

	data->facility = facility & LOG_FACMASK;
	data->priority = priority & LOG_PRIMASK;

	openlog(ident, option, 0);

	return 0;
}

/*

C<MsgSyslogData *msg_sysdata_create(const char *ident, int option, int facility, int priority)>

Creates the internal data needed by a I<Msg> object that sends messages to
I<syslog>. C<ident>, C<option>, C<facility> and C<priority> are used to
initialise the connection to I<syslog>. On success, returns the data. On
error, returns C<null> with C<errno> set appropriately.

*/

static MsgSyslogData *msg_sysdata_create(const char *ident, int option, int facility, int priority)
{
	MsgSyslogData *data;

	if (!(data = mem_new(MsgSyslogData)))
		return NULL;

	if (msg_sysdata_init(data, ident, option, facility, priority) == -1)
	{
		mem_release(data);
		return NULL;
	}

	return data;
}

/*

C<void msg_sysdata_release(MsgSyslogData *data)>

Releases (deallocates) the internal data needed by a I<Msg> object that
sends messages to I<syslog>. Calls I<closelog(3)>.

*/

static void msg_sysdata_release(MsgSyslogData *data)
{
	if (!data)
		return;

	mem_release(data);
	closelog();
}

/*

C<void msg_out_syslog(void *data, const void *mesg, size_t mesglen)>

Sends a message to I<syslog>. C<data> contains the facility to use. C<mesg>
is the message. C<mesglen> is it's length.

*/

static void msg_out_syslog(void *data, const void *mesg, size_t mesglen)
{
	MsgSyslogData *dst = data;

	if (mesg && dst && dst->facility != -1)
		syslog(dst->facility | dst->priority, "%*.*s", (int)mesglen, (int)mesglen, (char *)mesg);
}

/*

=item C<Msg *msg_create_syslog(const char *ident, int option, int facility, int priority)>

Creates a I<Msg> object that sends messages to I<syslog> initialised with
C<ident>, C<option>, C<facility> and C<priority>. It is the caller's
responsibility to deallocate the new I<Msg> with I<msg_release(3)> or
I<msg_destroy(3)>. On success, returns the new I<Msg> object. On error,
returns C<null> with C<errno> set appropriately.

=cut

*/

Msg *msg_create_syslog(const char *ident, int option, int facility, int priority)
{
	return msg_create_syslog_with_locker(NULL, ident, option, facility, priority);
}

/*

=item C<Msg *msg_create_syslog_with_locker(Locker *locker, const char *ident, int option, int facility, int priority)>

Equivalent to I<msg_create_syslog(3)> except that multiple threads accessing
the new I<Msg> will be synchronised by C<locker>.

=cut

*/

Msg *msg_create_syslog_with_locker(Locker *locker, const char *ident, int option, int facility, int priority)
{
	MsgSyslogData *data;
	Msg *mesg;

	if (!(data = msg_sysdata_create(ident, option, facility, priority)))
		return NULL;

	if (!(mesg = msg_create_with_locker(locker, MSG_SYSLOG, msg_out_syslog, data, (msg_release_t *)msg_sysdata_release)))
	{
		msg_sysdata_release(data);
		return NULL;
	}

	return mesg;
}

/*

=item C<Msg *msg_syslog_set_facility(Msg *mesg, int facility)>

Sets the facility field in C<mesg>'s data to C<facility>. On success,
returns C<mesg>. On error, returns C<null> with C<errno> set appropriately.

=cut

*/

Msg *msg_syslog_set_facility(Msg *mesg, int facility)
{
	Msg *ret;
	int err;

	if (!mesg)
		return set_errnull(EINVAL);

	if ((err = msg_wrlock(mesg)))
		return set_errnull(err);

	ret = msg_syslog_set_facility_unlocked(mesg, facility);

	if ((err = msg_unlock(mesg)))
		return set_errnull(err);

	return ret;
}

/*

=item C<Msg *msg_syslog_set_facility_unlocked(Msg *mesg, int facility)>

Equivalent to I<msg_syslog_set_facility(3)> except that C<mesg> is not write
locked.

=cut

*/

Msg *msg_syslog_set_facility_unlocked(Msg *mesg, int facility)
{
	MsgSyslogData *data;

	if (!mesg || mesg->type != MSG_SYSLOG)
		return set_errnull(EINVAL);

	data = (MsgSyslogData *)mesg->data;
	data->facility = facility;

	return mesg;
}

/*

=item C<Msg *msg_syslog_set_priority(Msg *mesg, int priority)>

Sets the priority field in C<mesg>'s data to C<priority>. On success,
returns C<mesg>. On error, returns C<null> with C<errno> set appropriately.

=cut

*/

Msg *msg_syslog_set_priority(Msg *mesg, int priority)
{
	Msg *ret;
	int err;

	if (!mesg)
		return set_errnull(EINVAL);

	if ((err = msg_wrlock(mesg)))
		return set_errnull(err);

	ret = msg_syslog_set_priority_unlocked(mesg, priority);

	if ((err = msg_unlock(mesg)))
		return set_errnull(err);

	return ret;
}

/*

=item C<Msg *msg_syslog_set_priority_unlocked(Msg *mesg, int priority)>

Equivalent to I<msg_syslog_set_priority(3)> except that C<mesg> is not write
locked.

=cut

*/

Msg *msg_syslog_set_priority_unlocked(Msg *mesg, int priority)
{
	MsgSyslogData *data;

	if (!mesg || mesg->type != MSG_SYSLOG)
		return set_errnull(EINVAL);

	data = (MsgSyslogData *)mesg->data;
	data->priority = priority;

	return mesg;
}

/*

C<int msg_plexdata_init(Msg *msg1, Msg *msg2)>

Initialises the internal data needed by a I<Msg> object that multiplexes
messages to several I<Msg> objects. On success, returns C<0>. On error,
returns C<-1> with C<errno> set appropriately.

*/

static int msg_plexdata_init(MsgPlexData *data, Msg *msg1, Msg *msg2)
{
	data->length = data->size = 2;

	if (!(data->list = mem_create(data->size, Msg *)))
		return -1;

	data->list[0] = msg1;
	data->list[1] = msg2;

	return 0;
}

/*

C<int msg_plexdata_add(MsgPlexData *data, Msg *mesg)>

Adds C<mesg> to a list of multiplexed I<Msg> objects. On success, returns
C<0>. On error, returns C<-1> with C<errno> set appropriately.

*/

static int msg_plexdata_add(MsgPlexData *data, Msg *mesg)
{
	if (data->length == data->size)
	{
		size_t new_size = data->size << 1;
		Msg **new_list = mem_resize(&data->list, new_size);
		if (!new_list)
			return -1;

		data->size = new_size;
		data->list = new_list;
	}

	data->list[data->length++] = mesg;

	return 0;
}

/*

C<MsgPlexData *msg_plexdata_create(Msg *msg1, Msg * msg2)>

Creates the internal data needed by a I<Msg> object that multiplexes
messages to several I<Msg> objects. Further I<Msg> objects can be added to
the list with I<msg_plexdata_add(3)>. On success, returns the data. On
error, returns C<null> with C<errno> set appropriately.

*/

static MsgPlexData *msg_plexdata_create(Msg *msg1, Msg *msg2)
{
	MsgPlexData *data;

	if (!(data = mem_new(MsgPlexData)))
		return NULL;

	if (msg_plexdata_init(data, msg1, msg2) == -1)
	{
		mem_release(data);
		return NULL;
	}

	return data;
}

/*

C<void msg_plexdata_release(MsgPlexData *data)>

Releases (deallocates) the internal data needed by a I<Msg> object that
multiplexes messages to several I<Msg> objects.

*/

static void msg_plexdata_release(MsgPlexData *data)
{
	size_t i;

	if (!data)
		return;

	for (i = 0; i < data->length; ++i)
		msg_destroy(data->list + i);

	mem_release(data->list);
	mem_release(data);
}

/*

C<void msg_out_plex(void *data, const void *mesg, size_t mesglen)>

Multiplexes a message to several I<Msg> objects. C<data> contains the list
of I<Msg> objects. C<mesg> is the message. C<mesglen> is it's length.

*/

static void msg_out_plex(void *data, const void *mesg, size_t mesglen)
{
	MsgPlexData *dst = data;
	size_t i;

	if (mesg && dst)
	{
		for (i = 0; i < dst->length; ++i)
		{
			Msg *out = dst->list[i];
			if (out && out->out)
				out->out(out->data, mesg, mesglen);
		}
	}
}

/*

=item C<Msg *msg_create_plex(Msg *msg1, Msg *msg2)>

Creates a I<Msg> object that multiplexes messages to C<msg1> and C<msg2>.
Further I<Msg> objects may be added to its list using I<msg_add_plex(3)>. It
is the caller's responsibility to deallocate the new I<Msg> with
I<msg_release(3)> or I<msg_destroy(3)>. On success, returns the new I<Msg>
object. On error, returns C<null> with C<errno> set appropriately.

=cut

*/

Msg *msg_create_plex(Msg *msg1, Msg *msg2)
{
	return msg_create_plex_with_locker(NULL, msg1, msg2);
}

/*

=item C<Msg *msg_create_plex_with_locker(Locker *locker, Msg *msg1, Msg *msg2)>

Equivalent to I<msg_create_plex(3)> except that multiple threads accessing
the new I<Msg> will be synchronised by C<locker>.

=cut

*/

Msg *msg_create_plex_with_locker(Locker *locker, Msg *msg1, Msg *msg2)
{
	MsgPlexData *data;
	Msg *mesg;

	if (!(data = msg_plexdata_create(msg1, msg2)))
		return NULL;

	if (!(mesg = msg_create_with_locker(locker, MSG_PLEX, msg_out_plex, data, (msg_release_t *)msg_plexdata_release)))
	{
		msg_plexdata_release(data);
		return NULL;
	}

	return mesg;
}

/*

=item C<int msg_add_plex(Msg *mesg, Msg *item)>

Adds C<item> to the list of I<Msg> objects multiplexed by C<mesg>. On
success, returns C<0>. On error, returns C<-1> with C<errno> set
appropriately.

=cut

*/

int msg_add_plex(Msg *mesg, Msg *item)
{
	int ret;
	int err;

	if (!mesg)
		return set_errno(EINVAL);

	if ((err = msg_wrlock(mesg)))
		return set_errno(err);

	ret = msg_add_plex_unlocked(mesg, item);

	if ((err = msg_unlock(mesg)))
		return set_errno(err);

	return ret;
}

/*

=item C<int msg_add_plex_unlocked(Msg *mesg, Msg *item)>

Equivalent to I<msg_add_plex(3)> except that C<mesg> is not write locked.

=cut

*/

int msg_add_plex_unlocked(Msg *mesg, Msg *item)
{
	if (!mesg || mesg->type != MSG_PLEX)
		return set_errno(EINVAL);

	return msg_plexdata_add((MsgPlexData *)mesg->data, item);
}

/*

C<MsgFilterData *msg_filterdata_create(msg_filter_t *filter, Msg *mesg)>

Creates the internal data needed by a I<Msg> object that sends filtered
messages to another I<Msg> object, I<mesg>. C<filter> and C<mesg> are used
to initialise a I<msg> object. On success, returns the data. On error,
returns C<null> with C<errno> set appropriately.

*/

static MsgFilterData *msg_filterdata_create(msg_filter_t *filter, Msg *mesg)
{
	MsgFilterData *data;

	if (!filter || !mesg)
		return set_errnull(EINVAL);

	if (!(data = mem_new(MsgFilterData)))
		return NULL;

	data->filter = filter;
	data->mesg = mesg;

	return data;
}

/*

C<void msg_filterdata_release(MsgFilterData *data)>

Releases (deallocates) the internal data needed by a I<Msg> object that
sends filtered messages to another I<Msg> object.

*/

static void msg_filterdata_release(MsgFilterData *data)
{
	if (!data)
		return;

	msg_release(data->mesg);
	mem_release(data);
}

/*

C<void msg_out_filter(void *data, const void *mesg, size_t mesglen)>

Filters and sends a message to another I<Msg> object. C<data> contains the
filter function and the destination I<Msg> object. C<mesg> is the unfiltered
message. C<mesglen> is it's length.

*/

static void msg_out_filter(void *data, const void *mesg, size_t mesglen)
{
	MsgFilterData *filter_data = data;
	void *filtered_mesg;
	int filtered_mesglen;

	if (!filter_data || !mesg || !mesglen)
		return;

	if ((filtered_mesglen = filter_data->filter(&filtered_mesg, mesg, mesglen)) == -1)
		return;

	filter_data->mesg->out(filter_data->mesg->data, filtered_mesg, filtered_mesglen);
	mem_release(filtered_mesg);
}

/*

=item C<Msg *msg_create_filter(msg_filter_t *filter, Msg *mesg)>

Creates a I<Msg> object that sends messages to I<mesg> after filtering
messages through the I<filter> function which must dynamically create a
modified version of its input message which will be deallocated by its
caller. It is the caller's responsibility to deallocate the new I<Msg> with
I<msg_release(3)> or I<msg_destroy(3)>. On success, returns the new I<Msg>
object. On error, returns C<null> with C<errno> set appropriately.

=cut

*/

Msg *msg_create_filter(msg_filter_t *filter, Msg *mesg)
{
	return msg_create_filter_with_locker(NULL, filter, mesg);
}

/*

=item C<Msg *msg_create_filter_with_locker(Locker *locker, msg_filter_t *filter, Msg *mesg)>

Equivalent to I<msg_create_filter(3)> except that multiple threads accessing
the new I<Msg> will be synchronised by C<locker>.

=cut

*/

Msg *msg_create_filter_with_locker(Locker *locker, msg_filter_t *filter, Msg *mesg)
{
	MsgFilterData *data;
	Msg *newmesg;

	if (!(data = msg_filterdata_create(filter, mesg)))
		return NULL;

	if (!(newmesg = msg_create_with_locker(locker, MSG_FILTER, msg_out_filter, data, (msg_release_t *)msg_filterdata_release)))
	{
		msg_filterdata_release(data);
		return NULL;
	}

	return newmesg;
}

/*

=item C<const char *msg_set_timestamp_format(const char *format)>

Sets the I<strftime(3)> format string used when sending messages to a file.
By default, it is C<"%Y%m%d %H:%M:%S ">. On success, returns the previous
format string. On error, returns C<null> with C<errno> set appropriately.

=cut

*/

const char *msg_set_timestamp_format(const char *format)
{
	const char *save;
	int err;

	if (!format)
		return set_errnull(EINVAL);

	if ((err = locker_wrlock(timestamp_format_locker)))
		return set_errnull(err);

	save = timestamp_format;
	timestamp_format = format;

	if ((err = locker_unlock(timestamp_format_locker)))
		return set_errnull(err);

	return save;
}

/*

=item C<int msg_set_timestamp_format_locker(Locker *locker)>

Sets the locking strategy for changing the timestamp format used when
sending messages to a file. This is only needed if the timestamp format will
be modified in multiple threads. On success, returns C<0>. On error, returns
C<-1> with C<errno> set appropriately.

=cut

*/

int msg_set_timestamp_format_locker(Locker *locker)
{
	if (timestamp_format_locker)
		return set_errno(EINVAL);

	timestamp_format_locker = locker;

	return 0;
}

/*

C<int syslog_lookup(const syslog_map_t *map, const char *name)>

Looks for C<name> (a facility or priority name) in C<map>. If found, returns
its corresponding code. If not found, returns C<-1>.

*/

static int syslog_lookup(const syslog_map_t *map, const char *name)
{
	int i;

	for (i = 0; map[i].name; ++i)
		if (!strcmp(name, map[i].name))
			break;

	return map[i].val;
}

/*

C<const char *syslog_lookup_str(const syslog_map_t *map, int spec)>

Looks for C<spec> (a facility or priority code) in C<map>. If found, returns
its corresponding name. If not found, returns C<null>.

*/

static const char *syslog_lookup_str(const syslog_map_t *map, int spec, int mask)
{
	int i;

	for (i = 0; map[i].name; ++i)
		if ((spec & mask) == map[i].val)
			break;

	return map[i].name;
}

/*

=item C<int syslog_lookup_facility(const char *facility)>

Returns the code corresponding to C<facility>. If not found, returns C<-1>.

=cut

*/

int syslog_lookup_facility(const char *facility)
{
	return syslog_lookup(syslog_facility_map, facility);
}

/*

=item C<int syslog_lookup_priority(const char *priority)>

Returns the code corresponding to C<priority>. If not found, returns C<-1>.

=cut

*/

int syslog_lookup_priority(const char *priority)
{
	return syslog_lookup(syslog_priority_map, priority);
}

/*

=item C<const char *syslog_facility_str(int spec)>

Returns the name corresponding to the facility part of C<spec>. If not
found, returns C<null>.

=cut

*/

const char *syslog_facility_str(int spec)
{
	return syslog_lookup_str(syslog_facility_map, spec, LOG_FACMASK);
}

/*

=item C<const char *syslog_priority_str(int spec)>

Returns the name corresponding to the priority part of C<spec>. If not
found, returns C<null>.

=cut

*/

const char *syslog_priority_str(int spec)
{
	return syslog_lookup_str(syslog_priority_map, spec, LOG_PRIMASK);
}

/*

=item C<int syslog_parse(const char *spec, int *facility, int *priority)>

Parses C<spec> as a I<facility.priority> string. If C<facility> is
non-C<null>, the parsed facility is stored in the location pointed to by
C<facility>. If C<priority> is non-C<null> the parsed priority is stored in
the location pointed to by C<priority>. On success, returns C<0>. On error,
returns C<-1> with C<errno> set appropriately.

    syslog facilities          syslog priorities
    ----------------------     -----------------------
    "kern"      LOG_KERN       "emerg"       LOG_EMERG
    "user"      LOG_USER       "alert"       LOG_ALERT
    "mail"      LOG_MAIL       "crit"        LOG_CRIT
    "daemon"    LOG_DAEMON     "err"         LOG_ERR
    "auth"      LOG_AUTH       "warning"     LOG_WARNING
    "syslog"    LOG_SYSLOG     "info"        LOG_INFO
    "lpr"       LOG_LPR        "debug"       LOG_DEBUG
    "news"      LOG_NEWS
    "uucp"      LOG_UUCP
    "cron"      LOG_CRON
    "local0"    LOG_LOCAL0
    "local1"    LOG_LOCAL1
    "local2"    LOG_LOCAL2
    "local3"    LOG_LOCAL3
    "local4"    LOG_LOCAL4
    "local5"    LOG_LOCAL5
    "local6"    LOG_LOCAL6
    "local7"    LOG_LOCAL7

=cut

*/

int syslog_parse(const char *spec, int *facility, int *priority)
{
	char fac[64], *pri;
	int f, p;

	if (!spec)
		return set_errno(EINVAL);

	strlcpy(fac, spec, 64);

	if (!(pri = strchr(fac, '.')))
		return set_errno(EINVAL);

	*pri++ = '\0';

	if ((f = syslog_lookup_facility(fac)) == -1)
		return set_errno(EINVAL);

	if ((p = syslog_lookup_priority(pri)) == -1)
		return set_errno(EINVAL);

	if (facility)
		*facility = f;

	if (priority)
		*priority = p;

	return 0;
}

/*

=back

=head1 ERRORS

On error, C<errno> is set by underlying functions or as follows:

=over 4

=item C<EINVAL>

An argument was C<null> or could not be parsed.

=back

=head1 MT-Level

MT-Disciplined - msg functions - See I<locker(3)> for details.

MT-Safe - syslog functions

=head1 EXAMPLE

Parse syslog facility priority pair:

    #include <slack/std.h>
    #include <slack/msg.h>

    int main(int ac, char **av)
    {
        int facility, priority;

        if (syslog_parse(av[1], &facility, &priority) != -1)
            syslog(facility | priority, "syslog(%s)", av[1]);

        return EXIT_SUCCESS;
    }

Multiplex a message to several locations:

    #include <slack/std.h>
    #include <slack/msg.h>

    int main(int ac, char **av)
    {
        Msg *stdout_msg = msg_create_stdout();
        Msg *stderr_msg = msg_create_stderr();
        Msg *file_msg = msg_create_file("/tmp/junk");
        Msg *syslog_msg = msg_create_syslog("ident", 0, LOG_DAEMON, LOG_ERR);
        Msg *plex_msg = msg_create_plex(stdout_msg, stderr_msg);

        msg_add_plex(plex_msg, file_msg);
        msg_add_plex(plex_msg, syslog_msg);

        msg_out(plex_msg, "Multiplex message\n");
        unlink("/tmp/junk");

        return EXIT_SUCCESS;
    }

=head1 SEE ALSO

I<libslack(3)>,
I<err(3)>,
I<prog(3)>,
I<openlog(3)>,
I<syslog(3)>,
I<locker(3)>

=head1 AUTHOR

20201111 raf <raf@raf.org>

=cut

*/

#endif

#ifdef TEST

static int verify(int test, const char *name, const char *mesg)
{
	char buf[MSG_SIZE];
	int fd;
	ssize_t bytes;

	if ((fd = open(name, O_RDONLY)) == -1)
	{
		printf("Test%d: failed to create msg file: %s (%s)\n", test, name, strerror(errno));
		return 1;
	}

	memset(buf, 0, MSG_SIZE);
	bytes = read(fd, buf, MSG_SIZE);
	close(fd);
	unlink(name);

	if (bytes == -1)
	{
		printf("Test%d: failed to read msg file: %s (%s)\n", test, name, strerror(errno));
		return 1;
	}

	if (!strstr(buf, mesg))
	{
		printf("Test%d: msg file produced incorrect input:\nshould contain:\n%s\nwas:\n%s\n", test, mesg, buf);
		return 1;
	}

	return 0;
}

static int prefix_filter(void **mesgp, const void *mesg, size_t mesglen)
{
	if (!mesgp || !mesg)
		return -1;

	return asprintf((char **)mesgp, "[%d] %.*s", 12345, (int)mesglen, (char *)mesg);
}

int main(int ac, char **av)
{
	const char *msg_file_name = "./msg.file";
	const char *msg_stdout_name = "./msg.stdout";
	const char *msg_stderr_name = "./msg.stderr";
	const char *msg_filter_name = "./msg.filter";
	const char *mesg = "multiplexed msg to stdout, stderr, ./msg.file, syslog local0.debug and ./msg.filtered\n";
	const char *note = "\n    Note: Can't verify syslog local0.debug. Look for:";
	void *filtered_mesg = null;
	int filtered_mesglen = 0;

	Msg *msg_stdout = msg_create_stdout();
	Msg *msg_stderr = msg_create_stderr();
	Msg *msg_file = msg_create_file(msg_file_name);
	Msg *msg_syslog = msg_create_syslog(NULL, 0, LOG_LOCAL0, LOG_DEBUG);
	Msg *msg_filter = msg_create_filter(prefix_filter, msg_create_file(msg_filter_name));
	Msg *msg_plex = msg_create_plex(msg_stdout, msg_stderr);
	int errors = 0;
	int tests = 0;
	int out, i, j, rc;

	if (ac == 2 && !strcmp(av[1], "help"))
	{
		printf("usage: %s\n", *av);
		return EXIT_SUCCESS;
	}

	printf("Testing: %s\n", "msg");

	++tests;
	if (!msg_stdout)
		++errors, printf("Test%d: failed to create msg_stdout\n", tests);
	++tests;
	if (!msg_stderr)
		++errors, printf("Test%d: failed to create msg_stderr\n", tests);
	++tests;
	if (!msg_file)
		++errors, printf("Test%d: failed to create msg_file\n", tests);
	++tests;
	if (!msg_syslog)
		++errors, printf("Test%d: failed to create msg_syslog\n", tests);
	++tests;
	if (!msg_filter)
		++errors, printf("Test%d: failed to create msg_filter\n", tests);
	++tests;
	if (!msg_plex)
		++errors, printf("Test%d: failed to create msg_plex\n", tests);
	++tests;
	if (msg_add_plex(msg_plex, msg_file) == -1)
		++errors, printf("Test%d: failed to add msg_file to msg_plex\n", tests);
	++tests;
	if (msg_add_plex(msg_plex, msg_syslog) == -1)
		++errors, printf("Test%d: failed to add msg_syslog to msg_plex\n", tests);
	++tests;
	if (msg_add_plex(msg_plex, msg_filter) == -1)
		++errors, printf("Test%d: failed to add msg_filter to msg_plex\n", tests);

	out = dup(STDOUT_FILENO);
	freopen(msg_stdout_name, "w", stdout);
	freopen(msg_stderr_name, "w", stderr);
	msg_out(msg_plex, mesg);
	msg_destroy(&msg_plex);
	dup2(out, STDOUT_FILENO);
	close(out);

	errors += verify(++tests, msg_stdout_name, mesg);
	errors += verify(++tests, msg_stderr_name, mesg);
	errors += verify(++tests, msg_file_name, mesg);

	filtered_mesglen = prefix_filter(&filtered_mesg, mesg, strlen(mesg));
	if (filtered_mesg && filtered_mesglen > 0)
	{
		errors += verify(++tests, msg_filter_name, filtered_mesg);
		mem_destroy(&filtered_mesg);
	}

	for (i = 0; syslog_facility_map[i].name; ++i)
	{
		for (j = 0; syslog_priority_map[j].name; ++j)
		{
			char buf[64];
			int facility = 0;
			int priority = 0;

			snprintf(buf, 64, "%s.%s", syslog_facility_map[i].name, syslog_priority_map[j].name);
			++tests;

			rc = syslog_parse(buf, &facility, &priority);
			if (rc == -1)
				++errors, printf("Test%d: syslog_parse(%s) failed\n", tests, buf);
			else if (facility != syslog_facility_map[i].val)
				++errors, printf("Test%d: syslog_parse(%s) failed: facility = %d (not %d)\n", tests, buf, facility, syslog_facility_map[i].val);
			else if (priority != syslog_priority_map[j].val)
				++errors, printf("Test%d: syslog_parse(%s) failed: priority = %d (not %d)\n", tests, buf, priority, syslog_priority_map[j].val);
		}
	}

	for (i = 0; syslog_facility_map[i].name; ++i)
	{
		const char *fac = syslog_facility_str(syslog_facility_map[i].val);

		++tests;
		if (strcmp(fac, syslog_facility_map[i].name))
			++errors, printf("Test%d: syslog_facility_str(%d) failed: %s (not %s)\n", tests, syslog_facility_map[i].val, fac, syslog_facility_map[i].name);
	}

	for (i = 0; syslog_priority_map[i].name; ++i)
	{
		const char *pri = syslog_priority_str(syslog_priority_map[i].val);

		++tests;
		if (strcmp(pri, syslog_priority_map[i].name))
			++errors, printf("Test%d: syslog_priority_str(%d) failed: %s (not %s)\n", tests, syslog_priority_map[i].val, pri, syslog_priority_map[i].name);
	}

	++tests;
	if (syslog_parse(NULL, NULL, NULL) != -1)
		++errors, printf("Test%d: syslog_parse(NULL) failed\n", tests);

	++tests;
	if (syslog_parse("gibberish", NULL, NULL) != -1)
		++errors, printf("Test%d: syslog_parse(\"gibberish\") failed\n", tests);

	if (errors)
		printf("%d/%d tests failed\n%s\n    %s", errors, tests, note, mesg);
	else
		printf("All tests passed\n%s\n    %s", note, mesg);

	return (errors == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

#endif

/* vi:set ts=4 sw=4: */
