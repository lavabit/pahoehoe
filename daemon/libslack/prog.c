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

I<libslack(prog)> - program framework module

=head1 SYNOPSIS

    #include <slack/std.h>
    #include <slack/prog.h>

    typedef struct option option;
    typedef struct Option Option;
    typedef struct Options Options;

    typedef void opt_action_int_t(int arg);
    typedef void opt_action_optional_int_t(int *arg);
    typedef void opt_action_string_t(const char *arg);
    typedef void opt_action_optional_string_t(const char *arg);
    typedef void opt_action_none_t(void);
    typedef void func_t(void);

    enum OptionArgument
    {
        OPT_NONE,
        OPT_INTEGER,
        OPT_STRING
    };

    enum OptionAction
    {
        OPT_NOTHING,
        OPT_VARIABLE,
        OPT_FUNCTION
    };

    typedef enum OptionArgument OptionArgument;
    typedef enum OptionAction OptionAction;

    struct Option
    {
        const char *name;
        char short_name;
        const char *argname;
        const char *desc;
        int has_arg;
        OptionArgument arg_type;
        OptionAction action;
        void *object;
        func_t *function;
    };

    struct Options
    {
        Options *parent;
        Option *options;
    };

    void prog_init(void);
    const char *prog_set_name(const char *name);
    Options *prog_set_options(Options *options);
    const char *prog_set_syntax(const char *syntax);
    const char *prog_set_desc(const char *desc);
    const char *prog_set_version(const char *version);
    const char *prog_set_date(const char *date);
    const char *prog_set_author(const char *author);
    const char *prog_set_contact(const char *contact);
    const char *prog_set_vendor(const char *vendor);
    const char *prog_set_url(const char *url);
    const char *prog_set_legal(const char *legal);
    Msg *prog_set_out(Msg *out);
    Msg *prog_set_err(Msg *err);
    Msg *prog_set_dbg(Msg *dbg);
    Msg *prog_set_alert(Msg *alert);
    ssize_t prog_set_debug_level(size_t debug_level);
    ssize_t prog_set_verbosity_level(size_t verbosity_level);
    int prog_set_locker(Locker *locker);
    const char *prog_name(void);
    const Options *prog_options(void);
    const char *prog_syntax(void);
    const char *prog_desc(void);
    const char *prog_version(void);
    const char *prog_date(void);
    const char *prog_author(void);
    const char *prog_contact(void);
    const char *prog_vendor(void);
    const char *prog_url(void);
    const char *prog_legal(void);
    Msg *prog_out(void);
    Msg *prog_err(void);
    Msg *prog_dbg(void);
    Msg *prog_alert(void);
    size_t prog_debug_level(void);
    size_t prog_verbosity_level(void);
    int prog_out_fd(int fd);
    int prog_out_stdout(void);
    int prog_out_file(const char *path);
    int prog_out_syslog(const char *ident, int option, int facility, int priority);
    int prog_out_push_filter(msg_filter_t *filter);
    int prog_out_none(void);
    int prog_err_fd(int fd);
    int prog_err_stderr(void);
    int prog_err_file(const char *path);
    int prog_err_syslog(const char *ident, int option, int facility, int priority);
    int prog_err_push_filter(msg_filter_t *filter);
    int prog_err_none(void);
    int prog_dbg_fd(int fd);
    int prog_dbg_stdout(void);
    int prog_dbg_stderr(void);
    int prog_dbg_file(const char *path);
    int prog_dbg_syslog(const char *id, int option, int facility, int priority);
    int prog_dbg_push_filter(msg_filter_t *filter);
    int prog_dbg_none(void);
    int prog_alert_fd(int fd);
    int prog_alert_stdout(void);
    int prog_alert_stderr(void);
    int prog_alert_file(const char *path);
    int prog_alert_syslog(const char *id, int option, int facility, int priority);
    int prog_alert_push_filter(msg_filter_t *filter);
    int prog_alert_none(void);
    int prog_opt_process(int ac, char **av);
    void prog_usage_msg(const char *format, ...);
    void prog_help_msg(void);
    void prog_version_msg(void);
    const char *prog_basename(const char *path);
    extern Options prog_options_table[1];
    int opt_process(int argc, char **argv, Options *options, char *msgbuf, size_t bufsize);
    char *opt_usage(char *buf, size_t size, Options *options);

=head1 DESCRIPTION

This module provides administrative services for arbitrary programs. The
services include program identification; flexible, complete command line
option processing; help, usage and version messages; flexible debug,
verbose, error and normal messaging (simple call syntax with arbitrary
message destinations including multiplexing).

This module exposes an alternate interface to I<GNU getopt_long(3)>. It
defines a way to specify command line option syntax, semantics and
descriptions in multiple, discrete chunks. The I<getopt> functions require
that the client specify the syntax and partial semantics for all options in
the same place (if it is to be done statically). This can be annoying when
library modules require their own command line options. This module allows
various parts of a program to (statically) specify their own command line
options independently, and link them together via C<parent> pointers.

Option syntax is specified in much the same way as for I<GNU
getopt_long(3)>. Option semantics are specified by an action
(C<OPT_NOTHING>, C<OPT_VARIABLE> or C<OPT_FUNCTION>), an argument type
(C<OPT_NONE>, C<OPT_INTEGER> or C<OPT_STRING>), and either an object
(C<int *>, C<char **>) or function (C<func()>, C<func(int)> or C<func(char *)>).

The I<opt_process(3)> and I<opt_usage(3)> functions are used by the I<prog>
functions and needn't be used directly. Instead, use I<prog_opt_process(3)>
to execute options and I<prog_usage_msg(3)> and I<prog_help_msg(3)> to
construct usage and help messages directly from the supplied option data.
They are exposed in case you don't want to use any other part of this
module.

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

#include "msg.h"
#include "err.h"
#include "mem.h"
#include "prog.h"

#ifndef HAVE_SNPRINTF
#include "snprintf.h"
#endif

typedef struct Prog Prog;

struct Prog
{
	const char *name;
	Options *options;
	const char *syntax;
	const char *desc;
	const char *version;
	const char *date;
	const char *author;
	const char *contact;
	const char *vendor;
	const char *url;
	const char *legal;
	Msg *out;
	Msg *err;
	Msg *dbg;
	Msg *log;
	size_t debug_level;
	size_t verbosity_level;
	Locker *locker;
};

#ifndef TEST

static Prog g =
{
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, NULL
};

/*

=item C<void prog_init(void)>

Initialises the message, error, debug, and alert destinations to C<stdout>,
C<stderr>, C<stderr>, and C<stderr>, respectively. These are all C<null> by
default so this function must be called before any messages are emitted.

=cut

*/

void prog_init(void)
{
	prog_out_stdout();
	prog_err_stderr();
	prog_dbg_stderr();
	prog_alert_stderr();
}

/*

=item C<const char *prog_set_name(const char *name)>

Sets the program's name to C<name>. This is used when composing usage, help,
version, and error messages. On success, returns C<name>. On error, returns
C<null> with C<errno> set appropriately.

=cut

*/

#define RDLOCK(ret) { int rc; if ((rc = locker_rdlock(g.locker))) { set_errno(rc); return (ret); } }
#define WRLOCK(ret) { int rc; if ((rc = locker_wrlock(g.locker))) { set_errno(rc); return (ret); } }
#define UNLOCK(ret) { int rc; if ((rc = locker_unlock(g.locker))) { set_errno(rc); return (ret); } }

#define PROG_SET_STR_AND_RETURN(name, value) \
	WRLOCK(NULL) \
	name = value; \
	UNLOCK(NULL) \
	return value

#define PROG_SET_MSG_AND_RETURN(name, value) \
	WRLOCK(NULL) \
	if (name && name != value) \
		msg_release(name); \
	name = value; \
	UNLOCK(NULL) \
	return value

#define PROG_POP_MSG(name, lvalue) \
	WRLOCK(-1) \
	lvalue = name; \
	name = NULL; \
	UNLOCK(-1) \

#define PROG_SET_INT_AND_RETURN_PREVIOUS(name, value) \
	size_t prev; \
	WRLOCK(-1) \
	prev = name; \
	name = value; \
	UNLOCK(-1) \
	return prev

#define PROG_GET_PTR_AND_RETURN(name) \
	void *value; \
	RDLOCK(NULL) \
	value = (void *)name; \
	UNLOCK(NULL) \
	return value

#define PROG_GET_INT_AND_RETURN(name) \
	int value; \
	RDLOCK(0) \
	value = name; \
	UNLOCK(0) \
	return value

const char *prog_set_name(const char *name)
{
	PROG_SET_STR_AND_RETURN(g.name, name);
}

/*

=item C<Options *prog_set_options(Options *options)>

Sets the program's options to C<options>. This is used when processing the
command line options with I<prog_opt_process(3)>. On success, returns
C<options>. On error, returns C<null> with C<errno> set appropriately.

=cut

*/

Options *prog_set_options(Options *options)
{
	PROG_SET_STR_AND_RETURN(g.options, options);
}

/*

=item C<const char *prog_set_syntax(const char *syntax)>

Sets the program's command line syntax summary to C<syntax>. This is used
when composing usage and help messages. It must contain a one line summary
of the command line arguments, excluding the program name (e.g. C<"[options]
arg...">). On success, returns C<syntax>. On error, returns C<null> with
C<errno> set appropriately.

=cut

*/

const char *prog_set_syntax(const char *syntax)
{
	PROG_SET_STR_AND_RETURN(g.syntax, syntax);
}

/*

=item C<const char *prog_set_desc(const char *desc)>

Sets the program's description to C<desc>. This is used when composing help
messages. On success, returns C<desc>. On error, returns C<null> with
C<errno> set appropriately.

=cut

*/

const char *prog_set_desc(const char *desc)
{
	PROG_SET_STR_AND_RETURN(g.desc, desc);
}

/*

=item C<const char *prog_set_version(const char *version)>

Sets the program's version string to C<version>. This is used when composing
help and version messages. On success, returns C<version>. On error, returns
C<null> with C<errno> set appropriately.

=cut

*/

const char *prog_set_version(const char *version)
{
	PROG_SET_STR_AND_RETURN(g.version, version);
}

/*

=item C<const char *prog_set_date(const char *date)>

Sets the program's release date to C<date>. This is used when composing help
messages. On success, returns C<date>. On error, returns C<null> with
C<errno> set appropriately.

=cut

*/

const char *prog_set_date(const char *date)
{
	PROG_SET_STR_AND_RETURN(g.date, date);
}

/*

=item C<const char *prog_set_author(const char *author)>

Sets the program's author to C<author>. This is used when composing help
messages. It must contain the (free format) name of the author. Returns
C<author>. On error, returns C<null> with C<errno> set appropriately.

=cut

*/

const char *prog_set_author(const char *author)
{
	PROG_SET_STR_AND_RETURN(g.author, author);
}

/*

=item C<const char *prog_set_contact(const char *contact)>

Sets the program's contact address to C<contact>. This is used when
composing help messages. It must contain the email address to which bug
reports should be sent. On success, returns C<contact>. On error, returns
C<null> with C<errno> set appropriately.

=cut

*/

const char *prog_set_contact(const char *contact)
{
	PROG_SET_STR_AND_RETURN(g.contact, contact);
}

/*

=item C<const char *prog_set_vendor(const char *vendor)>

Sets the program's vendor to C<vendor>. This is used when composing help
messages. It must contain the (free format) name of the vendor. Returns
C<vendor>. On error, returns C<null> with C<errno> set appropriately.

=cut

*/

const char *prog_set_vendor(const char *vendor)
{
	PROG_SET_STR_AND_RETURN(g.vendor, vendor);
}

/*

=item C<const char *prog_set_url(const char *url)>

Sets the program's URL to C<url>. This is used when composing help messages.
It must contain the URL where the program can be downloaded. On success,
returns C<url>. On error, returns C<null> with C<errno> set appropriately.

=cut

*/

const char *prog_set_url(const char *url)
{
	PROG_SET_STR_AND_RETURN(g.url, url);
}

/*

=item C<const char *prog_set_legal(const char *legal)>

Sets the program's legal notice to C<legal>. This is used when composing
help messages. It is assumed that the legal notice may contain multiple
lines and so must contain its own newline characters. On success, returns
C<legal>. On error, returns C<null> with C<errno> set appropriately.

=cut

*/

const char *prog_set_legal(const char *legal)
{
	PROG_SET_STR_AND_RETURN(g.legal, legal);
}

/*

=item C<Msg *prog_set_out(Msg *out)>

Sets the program's message destination to C<out>. This is used by I<msg(3)>
and I<vmsg(3)> which are, in turn, used to emit usage, version and help
messages. The program message destination is set to standard output by
I<prog_init(3)>, but it can be anything. However, it is probably best to
leave it as standard output at least until after command line option
processing is complete. See I<msg(3)> for details. On success, returns
C<out>. On error, returns C<null> with C<errno> set appropriately.

=cut

*/

Msg *prog_set_out(Msg *out)
{
	PROG_SET_MSG_AND_RETURN(g.out, out);
}

/*

=item C<Msg *prog_set_err(Msg *err)>

Sets the program's error message destination to C<err>. This is used by
I<error(3)>, I<errorsys(3)>, I<fatal(3)>, I<fatalsys(3)>, I<dump(3)> and
I<dumpsys(3)>. The program error message destination is set to standard
error by I<prog_init(3)>, but it can be anything. See I<msg(3)> for details.
On success, returns C<err>. On error, returns C<null> with C<errno> set
appropriately.

=cut

*/

Msg *prog_set_err(Msg *err)
{
	PROG_SET_MSG_AND_RETURN(g.err, err);
}

/*

=item C<Msg *prog_set_dbg(Msg *dbg)>

Sets the program's debug message destination to C<dbg>. This is set to
standard error by I<prog_init(3)>, but it can be set to anything. See
I<msg(3)> for details. On success, returns C<dbg>. On error, returns C<null>
with C<errno> set appropriately.

=cut

*/

Msg *prog_set_dbg(Msg *dbg)
{
	PROG_SET_MSG_AND_RETURN(g.dbg, dbg);
}

/*

=item C<Msg *prog_set_alert(Msg *alert)>

Sets the program's alert message destination to C<alert>. This is set to
standard error by I<prog_init(3)> but it can be set to anything. See
I<msg(3)> for details. On success, returns C<alert>. On error, returns
C<null> with C<errno> set appropriately.

=cut

*/

Msg *prog_set_alert(Msg *alert)
{
	PROG_SET_MSG_AND_RETURN(g.log, alert);
}

/*

=item C<ssize_t prog_set_debug_level(size_t debug_level)>

Sets the program's debug level to C<debug_level>. This is used when
determining whether or not to emit a debug message. The debug level
comprises two parts, the I<section> and the I<level>. The I<level> occupies
the low byte of C<debug_level>. The I<section> occupies the next three
bytes. This enables debugging to be partitioned into sections, allowing
users to turn on debugging at any level (from 0 up to 255) for particular
sections of a program (at most 24). Debug messages with a section value
whose bits overlap those of the program's current debug section and with a
level that is less than or equal to the program's current debug level are
emitted. As a convenience, if the program debug section is zero, debug
messages with a sufficiently small level are emitted regardless of the
message section. On success, returns the previous debug level. On error,
returns C<-1> with C<errno> set appropriately.

Example:

    #define LEXER_SECTION  (1 << 8)
    #define PARSER_SECTION (2 << 8)
    #define INTERP_SECTION (4 << 8)

    prog_set_debug_level(LEXER_SECTION | PARSER_SECTION | 1);
    debug((LEXER_SECTION  | 1, "lexer debugmsg"))  // yes
    debug((LEXER_SECTION  | 4, "lexer debugmsg"))  // no (level too high)
    debug((PARSER_SECTION | 1, "parser debugmsg")) // yes
    debug((INTERP_SECTION | 1, "interp debugmsg")) // no (wrong section)
    debug((1, "global debug"))                  // no (no section to match)

    prog_set_debug_level(1);
    debug((LEXER_SECTION  | 1, "lexer debugmsg"))  // yes
    debug((LEXER_SECTION  | 4, "lexer debugmsg"))  // no (level too high)
    debug((PARSER_SECTION | 1, "parser debugmsg")) // yes
    debug((INTERP_SECTION | 1, "interp debugmsg")) // yes
    debug((1, "global debugmsg"))                  // yes
    debug((4, "global debugmsg"))                  // no (level too high)

=cut

*/

ssize_t prog_set_debug_level(size_t debug_level)
{
	PROG_SET_INT_AND_RETURN_PREVIOUS(g.debug_level, debug_level);
}

/*

=item C<ssize_t prog_set_verbosity_level(size_t verbosity_level)>

Sets the program's verbosity level to C<verbosity_level>. This is used to
determine whether or not to emit verbose messages. Verbose messages with a
level that is less than or equal to the program's current verbosity level
are emitted. On success, returns the previous verbosity level. On error,
returns C<-1> with C<errno> set appropriately.

=cut

*/

ssize_t prog_set_verbosity_level(size_t verbosity_level)
{
	PROG_SET_INT_AND_RETURN_PREVIOUS(g.verbosity_level, verbosity_level);
}

/*

=item C<int prog_set_locker(Locker *locker)>

Sets the locker (multiple thread synchronisation strategy) for this module.
This is only needed in multi-threaded programs. See I<locker(3)> for
details. On success, returns C<0>. On error, returns C<-1> with C<errno> set
appropriately.

=cut

*/

int prog_set_locker(Locker *locker)
{
	if (g.locker)
		return -1;

	g.locker = locker;

	return 0;
}

/*

=item C<const char *prog_name(void)>

Returns the program's name as set by I<prog_set_name(3)>. On error, returns
C<null> with C<errno> set appropriately.

=cut

*/

const char *prog_name(void)
{
	PROG_GET_PTR_AND_RETURN(g.name);
}

/*

=item C<const Options *prog_options(void)>

Returns the program's options as set by I<prog_set_options(3)>. On error,
returns C<null> with C<errno> set appropriately.

=cut

*/

const Options *prog_options(void)
{
	PROG_GET_PTR_AND_RETURN(g.options);
}

/*

=item C<const char *prog_syntax(void)>

Returns the program's command line syntax summary as set by
I<prog_set_syntax(3)>. On error, returns C<null> with C<errno> set
appropriately.

=cut

*/

const char *prog_syntax(void)
{
	PROG_GET_PTR_AND_RETURN(g.syntax);
}

/*

=item C<const char *prog_desc(void)>

Returns the program's description as set by I<prog_set_desc(3)>. On error,
returns C<null> with C<errno> set appropriately.

=cut

*/

const char *prog_desc(void)
{
	PROG_GET_PTR_AND_RETURN(g.desc);
}

/*

=item C<const char *prog_version(void)>

Returns the program's version string as set by I<prog_set_version(3)>. On
error, returns C<null> with C<errno> set appropriately.

=cut

*/

const char *prog_version(void)
{
	PROG_GET_PTR_AND_RETURN(g.version);
}

/*

=item C<const char *prog_date(void)>

Returns the program's release date as set by I<prog_set_date(3)>. On error,
returns C<null> with C<errno> set appropriately.

=cut

*/

const char *prog_date(void)
{
	PROG_GET_PTR_AND_RETURN(g.date);
}

/*

=item C<const char *prog_author(void)>

Returns the program's author as set by I<prog_set_author(3)>. On error,
returns C<null> with C<errno> set appropriately.

=cut

*/

const char *prog_author(void)
{
	PROG_GET_PTR_AND_RETURN(g.author);
}

/*

=item C<const char *prog_contact(void)>

Returns the program's contact address as set by I<prog_set_contact(3)>. On
error, returns C<null> with C<errno> set appropriately.

=cut

*/

const char *prog_contact(void)
{
	PROG_GET_PTR_AND_RETURN(g.contact);
}

/*

=item C<const char *prog_vendor(void)>

Returns the program's vendor as set by I<prog_set_vendor(3)>. On error,
returns C<null> with C<errno> set appropriately.

=cut

*/

const char *prog_vendor(void)
{
	PROG_GET_PTR_AND_RETURN(g.vendor);
}

/*

=item C<const char *prog_url(void)>

Returns the program's URL as set by I<prog_set_url(3)>. On error, returns
C<null> with C<errno> set appropriately.

=cut

*/

const char *prog_url(void)
{
	PROG_GET_PTR_AND_RETURN(g.url);
}

/*

=item C<const char *prog_legal(void)>

Returns the program's legal notice as set by I<prog_set_legal(3)>. On error,
returns C<null> with C<errno> set appropriately.

=cut

*/

const char *prog_legal(void)
{
	PROG_GET_PTR_AND_RETURN(g.legal);
}

/*

=item C<Msg *prog_out(void)>

Returns the program's message destination as set by I<prog_set_out(3)>. On
error, returns C<null> with C<errno> set appropriately.

=cut

*/

Msg *prog_out(void)
{
	PROG_GET_PTR_AND_RETURN(g.out);
}

/*

=item C<Msg *prog_err(void)>

Returns the program's error message destination as set by
I<prog_set_err(3)>. On error, returns C<null> with C<errno> set
appropriately.

=cut

*/

Msg *prog_err(void)
{
	PROG_GET_PTR_AND_RETURN(g.err);
}

/*

=item C<Msg *prog_dbg(void)>

Returns the program's debug message destination as set by
I<prog_set_dbg(3)>. On error, returns C<null> with C<errno> set
appropriately.

=cut

*/

Msg *prog_dbg(void)
{
	PROG_GET_PTR_AND_RETURN(g.dbg);
}

/*

=item C<Msg *prog_alert(void)>

Returns the program's alert message destination as set by
I<prog_set_alert(3)>. On error, returns C<null> with C<errno> set
appropriately.

=cut

*/

Msg *prog_alert(void)
{
	PROG_GET_PTR_AND_RETURN(g.log);
}

/*

=item C<size_t prog_debug_level(void)>

Returns the program's debug level as set by I<prog_set_debug_level(3)>. On
error, returns C<0> with C<errno> set appropriately.

=cut

*/

size_t prog_debug_level(void)
{
	PROG_GET_INT_AND_RETURN(g.debug_level);
}

/*

=item C<size_t prog_verbosity_level(void)>

Returns the program's verbosity level as set by
I<prog_set_verbosity_level(3)>. On error, returns C<0> with C<errno> set
appropriately.

=cut

*/

size_t prog_verbosity_level(void)
{
	PROG_GET_INT_AND_RETURN(g.verbosity_level);
}

/*

=item C<int prog_out_fd(int fd)>

Sets the program's normal message destination to be the file descriptor,
C<fd>. On success, returns C<0>. On error, returns C<-1> with C<errno> set
appropriately.

=cut

*/

int prog_out_fd(int fd)
{
	Msg *mesg;

	if (!(mesg = msg_create_fd_with_locker(g.locker, fd)))
		return -1;

	if (!prog_set_out(mesg))
	{
		msg_release(mesg);
		return -1;
	}

	return 0;
}

/*

=item C<int prog_out_stdout(void)>

Sets the program's normal message destination to be standard output. On
success, returns C<0>. On error, returns C<-1> with C<errno> set
appropriately.

=cut

*/

int prog_out_stdout(void)
{
	return prog_out_fd(STDOUT_FILENO);
}

/*

=item C<int prog_out_file(const char *path)>

Sets the program's normal message destination to be the file specified by
C<path>. On success, returns C<0>. On error, returns C<-1> with C<errno> set
appropriately.

=cut

*/

int prog_out_file(const char *path)
{
	Msg *mesg;

	if (!(mesg = msg_create_file_with_locker(g.locker, path)))
		return -1;

	if (!prog_set_out(mesg))
	{
		msg_release(mesg);
		return -1;
	}

	return 0;
}

/*

=item C<int prog_out_syslog(const char *ident, int option, int facility, int priority)>

Sets the program's normal message destination to be I<syslog> initialised
with C<ident>, C<option>, C<facility>, and C<priority>. On success, returns
C<0>. On error, returns C<-1> with C<errno> set appropriately.

=cut

*/

int prog_out_syslog(const char *ident, int option, int facility, int priority)
{
	Msg *mesg;

	if (!(mesg = msg_create_syslog_with_locker(g.locker, ident, option, facility, priority)))
		return -1;

	if (!prog_set_out(mesg))
	{
		msg_release(mesg);
		return -1;
	}

	return 0;
}

/*

=item C<int prog_out_push_filter(msg_filter_t *filter)>

Pushes the message filter function, I<filter>, onto the program's normal
message destination. On success, returns C<0>. On error, returns C<-1> with
C<errno> set appropriately.

=cut

*/

int prog_out_push_filter(msg_filter_t *filter)
{
	Msg *mesg, *top;

	PROG_POP_MSG(g.out, top);

	if (!(mesg = msg_create_filter_with_locker(g.locker, filter, top)))
		return -1;

	if (!prog_set_out(mesg))
	{
		msg_release(mesg);
		return -1;
	}

	return 0;
}

/*

=item C<int prog_out_none(void)>

Sets the program's normal message destination to C<null>. This disables all
normal messages. On success, returns C<0>. On error, returns C<-1> with
C<errno> set appropriately.

=cut

*/

int prog_out_none(void)
{
	errno = 0;
	prog_set_out(NULL);

	return (errno) ? -1 : 0;
}

/*

=item C<int prog_err_fd(int fd)>

Sets the program's error message destination to be the file descriptor,
C<fd>. On success, returns C<0>. On error, returns C<-1> with C<errno> set
appropriately.

=cut

*/

int prog_err_fd(int fd)
{
	Msg *mesg;

	if (!(mesg = msg_create_fd_with_locker(g.locker, fd)))
		return -1;

	if (!prog_set_err(mesg))
	{
		msg_release(mesg);
		return -1;
	}

	return 0;
}

/*

=item C<int prog_err_stderr(void)>

Sets the program's error message destination to be standard error. On
success, returns C<0>. On error, returns C<-1> with C<errno> set
appropriately.

=cut

*/

int prog_err_stderr(void)
{
	return prog_err_fd(STDERR_FILENO);
}

/*

=item C<int prog_err_file(const char *path)>

Sets the program's error message destination to be the file specified by
C<path>. On success, returns C<0>. On error, returns C<-1> with C<errno> set
appropriately.

=cut

*/

int prog_err_file(const char *path)
{
	Msg *mesg;

	if (!(mesg = msg_create_file_with_locker(g.locker, path)))
		return -1;

	if (!prog_set_err(mesg))
	{
		msg_release(mesg);
		return -1;
	}

	return 0;
}

/*

=item C<int prog_err_syslog(const char *ident, int option, int facility, int priority)>

Sets the program's error message destination to be I<syslog> initialised
with C<ident>, C<option>, C<facility>, and C<priority>. On success, returns
C<0>. On error, returns C<-1> with C<errno> set appropriately.

=cut

*/

int prog_err_syslog(const char *ident, int option, int facility, int priority)
{
	Msg *mesg;

	if (!(mesg = msg_create_syslog_with_locker(g.locker, ident, option, facility, priority)))
		return -1;

	if (!prog_set_err(mesg))
	{
		msg_release(mesg);
		return -1;
	}

	return 0;
}

/*

=item C<int prog_err_push_filter(msg_filter_t *filter)>

Pushes the message filter function, I<filter>, onto the program's error
message destination. On success, returns C<0>. On error, returns C<-1> with
C<errno> set appropriately.

=cut

*/

int prog_err_push_filter(msg_filter_t *filter)
{
	Msg *mesg, *top;

	PROG_POP_MSG(g.err, top);

	if (!(mesg = msg_create_filter_with_locker(g.locker, filter, top)))
		return -1;

	if (!prog_set_err(mesg))
	{
		msg_release(mesg);
		return -1;
	}

	return 0;
}

/*

=item C<int prog_err_none(void)>

Sets the program's error message destination to C<null>. This disables all
error messages. On success, returns C<0>. On error, returns C<-1> with
C<errno> set appropriately.

=cut

*/

int prog_err_none(void)
{
	errno = 0;
	prog_set_err(NULL);

	return (errno) ? -1 : 0;
}

/*

=item C<int prog_dbg_fd(int fd)>

Sets the program's debug message destination to be the file descriptor,
C<fd>. On success, returns C<0>. On error, returns C<-1> with C<errno> set
appropriately.

=cut

*/

int prog_dbg_fd(int fd)
{
	Msg *mesg;

	if (!(mesg = msg_create_fd_with_locker(g.locker, fd)))
		return -1;

	if (!prog_set_dbg(mesg))
	{
		msg_release(mesg);
		return -1;
	}

	return 0;
}

/*

=item C<int prog_dbg_stdout(void)>

Sets the program's debug message destination to be standard output. On
success, returns C<0>. On error, returns C<-1> with C<errno> set
appropriately.

=cut

*/

int prog_dbg_stdout(void)
{
	return prog_dbg_fd(STDOUT_FILENO);
}

/*

=item C<int prog_dbg_stderr(void)>

Sets the program's debug message destination to be standard error. On
success, returns C<0>. On error, returns C<-1> with C<errno> set
appropriately.

=cut

*/

int prog_dbg_stderr(void)
{
	return prog_dbg_fd(STDERR_FILENO);
}

/*

=item C<int prog_dbg_file(const char *path)>

Sets the program's debug message destination to be the file specified by
C<path>. On success, returns C<0>. On error, returns C<-1> with C<errno> set
appropriately.

=cut

*/

int prog_dbg_file(const char *path)
{
	Msg *dbg;

	if (!(dbg = msg_create_file_with_locker(g.locker, path)))
		return -1;

	if (!prog_set_dbg(dbg))
	{
		msg_release(dbg);
		return -1;
	}

	return 0;
}

/*

=item C<int prog_dbg_syslog(const char *id, int option, int facility, int priority)>

Sets the program's debug message destination to be I<syslog> initialised
with C<ident>, C<option>, C<facility>, and C<priority>. On success, returns
C<0>. On error, returns C<-1> with C<errno> set appropriately.

=cut

*/

int prog_dbg_syslog(const char *id, int option, int facility, int priority)
{
	Msg *dbg;

	if (!(dbg = msg_create_syslog_with_locker(g.locker, id, option, facility, priority)))
		return -1;

	if (!prog_set_dbg(dbg))
	{
		msg_release(dbg);
		return -1;
	}

	return 0;
}

/*

=item C<int prog_dbg_push_filter(msg_filter_t *filter)>

Pushes the message filter function, I<filter>, onto the program's debug
message destination. On success, returns C<0>. On error, returns C<-1> with
C<errno> set appropriately.

=cut

*/

int prog_dbg_push_filter(msg_filter_t *filter)
{
	Msg *mesg, *top;

	PROG_POP_MSG(g.dbg, top);

	if (!(mesg = msg_create_filter_with_locker(g.locker, filter, top)))
		return -1;

	if (!prog_set_dbg(mesg))
	{
		msg_release(mesg);
		return -1;
	}

	return 0;
}

/*

=item C<int prog_dbg_none(void)>

Sets the program's debug message destination to C<null>. This disables all
debug messages. On success, returns C<0>. On error, returns C<-1> with
C<errno> set appropriately.

=cut

*/

int prog_dbg_none(void)
{
	errno = 0;
	prog_set_dbg(NULL);

	return (errno) ? -1 : 0;
}

/*

=item C<int prog_alert_fd(int fd)>

Sets the program's alert message destination to be the file descriptor
specified by C<fd>. On success, returns C<0>. On error, returns C<-1> with
C<errno> set appropriately.

=cut

*/

int prog_alert_fd(int fd)
{
	Msg *mesg;

	if (!(mesg = msg_create_fd_with_locker(g.locker, fd)))
		return -1;

	if (!prog_set_alert(mesg))
	{
		msg_release(mesg);
		return -1;
	}

	return 0;
}

/*

=item C<int prog_alert_stdout(void)>

Sets the program's alert message destination to be standard output. On
success, returns C<0>. On error, returns C<-1> with C<errno> set
appropriately.

=cut

*/

int prog_alert_stdout(void)
{
	return prog_alert_fd(STDOUT_FILENO);
}

/*

=item C<int prog_alert_stderr(void)>

Sets the program's alert message destination to be standard error. On
success, returns C<0>. On error, returns C<-1> with C<errno> set
appropriately.

=cut

*/

int prog_alert_stderr(void)
{
	return prog_alert_fd(STDERR_FILENO);
}

/*

=item C<int prog_alert_file(const char *path)>

Sets the program's alert message destination to be the file specified by
C<path>. On success, returns C<0>. On error, returns C<-1> with C<errno> set
appropriately.

=cut

*/

int prog_alert_file(const char *path)
{
	Msg *alert;

	if (!(alert = msg_create_file_with_locker(g.locker, path)))
		return -1;

	if (!prog_set_alert(alert))
	{
		msg_release(alert);
		return -1;
	}

	return 0;
}

/*

=item C<int prog_alert_syslog(const char *id, int option, int facility, int priority)>

Sets the program's alert message destination to be I<syslog> initialised
with C<ident>, C<option>, C<facility>, and C<priority>. On success, returns
C<0>. On error, returns C<-1> with C<errno> set appropriately.

=cut

*/

int prog_alert_syslog(const char *id, int option, int facility, int priority)
{
	Msg *alert;

	if (!(alert = msg_create_syslog_with_locker(g.locker, id, option, facility, priority)))
		return -1;

	if (!prog_set_alert(alert))
	{
		msg_release(alert);
		return -1;
	}

	return 0;
}

/*

=item C<int prog_alert_push_filter(msg_filter_t *filter)>

Pushes the message filter function, I<filter>, onto the program's alert
message destination. On success, returns C<0>. On error, returns C<-1> with
C<errno> set appropriately.

=cut

*/

int prog_alert_push_filter(msg_filter_t *filter)
{
	Msg *mesg, *top;

	PROG_POP_MSG(g.log, top);

	if (!(mesg = msg_create_filter_with_locker(g.locker, filter, top)))
		return -1;

	g.log = NULL;

	if (!prog_set_alert(mesg))
	{
		msg_release(mesg);
		return -1;
	}

	return 0;
}

/*

=item C<int prog_alert_none(void)>

Sets the program's alert message destination to C<null>. This disables all
alert messages. On success, returns C<0>. On error, returns C<-1> with
C<errno> set appropriately.

=cut

*/

int prog_alert_none(void)
{
	errno = 0;
	prog_set_alert(NULL);

	return (errno) ? -1 : 0;
}

/*

=item C<int prog_opt_process(int ac, char **av)>

Parses and processes the command line options in C<av>. If there is an
error, a usage message is emitted and the program terminates. This function
is just an interface to I<GNU getopt_long(3)> that provides easier, more
flexible, and more complete option handling. As well as supplying the syntax
for options, this function requires their semantics and descriptions. The
descriptions allow usage and help messages to be automatically composed by
I<prog_usage_msg(3)> and I<prog_help_msg(3)>. The semantics (which may be
either a variable assignment or a function invocation) allow complete
command line option processing to be performed with a single call to this
function. On success, returns C<optind>. On error (i.e. invalid option or
option argument), calls I<prog_usage_msg(3)> which terminates the program
with a return code of C<EXIT_FAILURE>. See the I<EXAMPLE> section for
details on specifying option data. See I<opt_process(3)> for details on the
processing of each option. On error, returns C<-1> with C<errno> set
appropriately.

=cut

*/

int prog_opt_process(int ac, char **av)
{
	char msgbuf[256] = "";
	int err;
	int rc;

	if ((err = locker_rdlock(g.locker)))
		return set_errno(err);

	rc = opt_process(ac, av, g.options, msgbuf, 256);

	if ((err = locker_unlock(g.locker)))
		return set_errno(err);

	if (rc == -1)
		prog_usage_msg("%s", msgbuf);

	return rc;
}

/*

=item C<void prog_usage_msg(const char *format, ...)>

Emits a program usage error message, then terminates the program with a
return code of C<EXIT_FAILURE>. The usage message consists of the program's
name, command line syntax, options and their descriptions (if they have been
supplied), and the given message. C<format> is a I<printf(3)>-like format
string. Any remaining arguments are processed as in I<printf(3)>.

B<Warning: Do not under any circumstances ever pass a non-literal string as
the format argument unless you know exactly how many conversions will take
place. Being careless with this is a very good way to build potential
security vulnerabilities into your programs. The same is true for all
functions that take a printf()-like format string as an argument.>

    prog_usage_msg(buf);       // EVIL
    prog_usage_msg("%s", buf); // GOOD

=cut

*/

void prog_usage_msg(const char *format, ...)
{
	char msg_buf[MSG_SIZE];
	char opt_buf[MSG_SIZE];
	int msg_length;
	va_list args;
	va_start(args, format);
	vsnprintf(msg_buf, MSG_SIZE, format, args);
	va_end(args);

	if (locker_rdlock(g.locker))
		exit(EXIT_FAILURE);

	opt_usage(opt_buf, MSG_SIZE, g.options);
	fflush(stderr);

	msg_length = strlen(msg_buf);
	msg_out(g.err, "%s%susage: %s%s%s\n%s%s",
		msg_buf,
		(msg_length && msg_buf[msg_length - 1] != '\n') ? "\n" : "",
		(g.name) ? g.name : "",
		(g.name) ? " " : "",
		(g.syntax) ? g.syntax : "",
		(*opt_buf) ? "options:\n" : "",
		opt_buf
	);

	locker_unlock(g.locker);

	exit(EXIT_FAILURE);
}

/*

=item C<void prog_help_msg(void)>

Emits a program help message, then terminates the program with a return code
of C<EXIT_SUCCESS>. This message consists of the program's usage message,
description, name, version, release date, author, vendor, URL, legal notice
and contact address (if they have been supplied).

=cut

*/

void prog_help_msg(void)
{
	char buf[MSG_SIZE];
	size_t length = 0;

	if (locker_rdlock(g.locker))
		exit(EXIT_FAILURE);

	snprintf(buf, MSG_SIZE, "usage: %s %s\n",
		g.name ? g.name : "",
		g.syntax ? g.syntax : ""
	);

	if (g.options)
	{
		length = strlen(buf);
		snprintf(buf + length, MSG_SIZE - length, "options:\n");
		length = strlen(buf);
		opt_usage(buf + length, MSG_SIZE - length, g.options);
	}

	if (g.desc)
	{
		length = strlen(buf);
		snprintf(buf + length, MSG_SIZE - length, "\n%s\n", g.desc);
	 }

	if (g.name)
	{
		length = strlen(buf);
		snprintf(buf + length, MSG_SIZE - length, "Name: %s\n", g.name);
	}

	if (g.version)
	{
		length = strlen(buf);
		snprintf(buf + length, MSG_SIZE - length, "Version: %s\n", g.version);
	}

	if (g.date)
	{
		length = strlen(buf);
		snprintf(buf + length, MSG_SIZE - length, "Date: %s\n", g.date);
	}

	if (g.author)
	{
		length = strlen(buf);
		snprintf(buf + length, MSG_SIZE - length, "Author: %s\n", g.author);
	}

	if (g.vendor)
	{
		length = strlen(buf);
		snprintf(buf + length, MSG_SIZE - length, "Vendor: %s\n", g.vendor);
	}

	if (g.url)
	{
		length = strlen(buf);
		snprintf(buf + length, MSG_SIZE - length, "URL: %s\n", g.url);
	}

	if (g.legal)
	{
		length = strlen(buf);
		snprintf(buf + length, MSG_SIZE - length, "\n%s\n", g.legal);
	}

	if (g.contact)
	{
		length = strlen(buf);
		snprintf(buf + length, MSG_SIZE - length, "Report bugs to %s\n", g.contact);
	}

	if (locker_unlock(g.locker))
		exit(EXIT_FAILURE);

	msg("%s", buf);
	exit(EXIT_SUCCESS);
}

/*

=item C<void prog_version_msg(void)>

Emits a program version message, then terminates the program with a return
code of C<EXIT_SUCCESS>. This message consists of the program's name and
version (if they have been supplied).

=cut

*/

void prog_version_msg(void)
{
	char buf[MSG_SIZE];

	if (locker_rdlock(g.locker))
		exit(EXIT_FAILURE);

	if (g.name && g.version)
		snprintf(buf, MSG_SIZE, "%s-%s\n", g.name, g.version);
	else if (g.name)
		snprintf(buf, MSG_SIZE, "%s\n", g.name);
	else if (g.version)
		snprintf(buf, MSG_SIZE, "%s\n", g.version);
	else
		*buf = nul;

	if (locker_unlock(g.locker))
		exit(EXIT_FAILURE);

	msg("%s", buf);
	exit(EXIT_SUCCESS);
}

/*

=item C<const char *prog_basename(const char *path)>

Returns the filename part of C<path>. On error, returns C<null> with
C<errno> set appropriately.

=cut

*/

const char *prog_basename(const char *path)
{
	const char *name;

	if (!path)
		return set_errnull(EINVAL);

	return (name = strrchr(path, PATH_SEP)) ? (name + 1) : path;
}

/*

=item C<extern Options prog_options_table[1]>

Contains the syntax, semantics and descriptions of some options that are
available to all programs that use I<libslack>. These options are:

=over 4

=item C<-h>, C<--help>

Print a help message then exit

=item C<-V>, C<--version>

Print a version message then exit

=item C<-v>I<[level]>, C<--verbose>I<[=level]>

Set the verbosity level (Defaults to 1 if I<level> is not supplied)

=item C<-d>I<[level]>, C<--debug>I<[=level]>

Set the debugging level (Defaults to 1 if I<level> is not supplied)

=back

If your program supports no other options than these, C<prog_options_table>
can be passed directly to I<prog_set_options(3)>. Otherwise,
C<prog_options_table> should be assigned to the C<parent> field of the
C<Options> structure that will be passed to I<prog_set_options(3)>.

=cut

*/

static void handle_verbose_option(int *arg)
{
	prog_set_verbosity_level(arg ? *arg : 1);
}

static void handle_debug_option(int *arg)
{
	prog_set_debug_level(arg ? *arg : 1);
}

static Option prog_optab[] =
{
	{
		"help", 'h', NULL, "Print a help message then exit",
		no_argument, OPT_NONE, OPT_FUNCTION, NULL, (func_t *)prog_help_msg
	},
	{
		"version", 'V', NULL, "Print a version message then exit",
		no_argument, OPT_NONE, OPT_FUNCTION, NULL, (func_t *)prog_version_msg
	},
	{
		"verbose", 'v', "level", "Set the verbosity level",
		optional_argument, OPT_INTEGER, OPT_FUNCTION, NULL, (func_t *)handle_verbose_option
	},
#ifndef NDEBUG
	{
		"debug", 'd', "level", "Set the debugging level",
		optional_argument, OPT_INTEGER, OPT_FUNCTION, NULL, (func_t *)handle_debug_option
	},
#endif
	{
		NULL, nul, NULL, NULL, 0, 0, 0, NULL, NULL
	}
};

Options prog_options_table[1] = {{ NULL, prog_optab }};

/*

C<opt_convert(Options *options)>

Creates and returns a flat table of option structs from C<options>. The
resulting array is for use with I<GNU getopt_long(3)>. It is the caller's
responsibility to deallocate the returned memory with I<free(3)>,
I<mem_release(3)>, or I<mem_destroy(3)>. It is strongly recommended to use
I<mem_destroy(3)>, because it also sets the pointer variable to C<null>. On
error, returns C<null> with C<errno> set appropriately.

*/

static option *opt_convert(Options *options)
{
	Options *opts;
	size_t size = 0;
	size_t i;
	option *ret;
	int index = 0;

	for (opts = options; opts; opts = opts->parent)
		for (i = 0; opts->options[i].name; ++i)
			++size;

	if (!(ret = mem_create(size + 1, option)))
		return NULL;

	for (opts = options; opts; opts = opts->parent)
	{
		int i;

		for (i = 0; opts->options[i].name; ++i, ++index)
		{
			ret[index].name = (char *)opts->options[i].name;
			ret[index].has_arg = opts->options[i].has_arg;
			ret[index].flag = NULL;
			ret[index].val = 0;
		}
	}

	memset(ret + index, 0, sizeof(option));

	return ret;
}

/*

C<opt_optstring(Options *options)>

Creates and returns a string containing all of the short option names from
C<options>. The resulting string is for use with I<GNU getopt_long(3)>. It
is the caller's responsibility to deallocate the returned memory with
I<free(3)>, I<mem_release(3)>, or I<mem_destroy(3)>. It is strongly
recommended to use I<mem_destroy(3)>, because it also sets the pointer
variable to C<null>. On error, returns C<null> with C<errno> set
appropriately.

*/

static char *opt_optstring(Options *options)
{
	Options *opts;
	size_t size = 0;
	size_t i;
	char *optstring;
	char *p;

	for (opts = options; opts; opts = opts->parent)
		for (i = 0; opts->options[i].name; ++i)
			if (opts->options[i].short_name)
				++size;

	if (!(p = optstring = mem_create((size * 3) + 1, char)))
		return NULL;

	for (opts = options; opts; opts = opts->parent)
	{
		int i;

		for (i = 0; opts->options[i].name; ++i)
		{
			if (opts->options[i].short_name)
			{
				char short_name = opts->options[i].short_name;

				if (short_name == '?')
					short_name = '\001';

				*p++ = short_name;

				switch (opts->options[i].has_arg)
				{
					case optional_argument: *p++ = ':';
					case required_argument: *p++ = ':';
				}
			}
		}
	}

	*p = nul;

	return optstring;
}

/*

C<int int_arg(const char *argument)>

Parse and return the number in C<argument>. Uses I<strtol(3)> with base
C<0>. Also, if C<argument> is not a number at all, or it contains trailing
text, sets C<errno = EDOM> and returns C<0>. If C<argument> is out of
integer range, sets C<errno = ERANGE> and returns C<INT_MAX> or C<INT_MIN>.

*/

static int int_arg(const char *argument)
{
	char *endptr = NULL;
	long val = strtol(argument, &endptr, 0);

	if (val > INT_MAX)
		return errno = ERANGE, INT_MAX;

	if (val < INT_MIN)
		return errno = ERANGE, INT_MIN;

	if (endptr == argument || *endptr != '\0')
		return errno = EDOM, 0;

	return (int)val;
}

/*

C<void opt_action(Options *options, int rc, int longindex, const char *argument)>

Performs the action associated with the option in C<options> when I<GNU
getopt_long(3)> returned C<rc> or C<longindex>. C<argument> is a pointer to
an C<int> or a C<char *>. See I<opt_process(3)> for details.

*/

static void opt_action(Options *options, int rc, int longindex, const char *argument)
{
	Option *option;
	int i = -1;

	if (rc != 0 && longindex == -1) /* Short option */
	{
		for (; options; options = options->parent)
		{
			for (i = 0; options->options[i].name; ++i)
				if (options->options[i].short_name == rc)
					break;

			if (options->options[i].short_name == rc)
				break;
		}
	}
	else if (rc == 0 && longindex != -1) /* Long option */
	{
		for (i = 0; options; options = options->parent)
		{
			for (i = 0; longindex && options->options[i].name; ++i)
				--longindex;

			if (!options->options[i].name)
				continue;

			if (longindex == 0)
				break;
		}
	}
	else
		return;

	if (!options || i == -1)
		return;

	option = options->options + i;

	if (option->has_arg == required_argument && !argument)
		return;

	if (option->has_arg == no_argument && argument)
		return;

	if (argument)
	{
		switch (option->arg_type)
		{
			case OPT_NONE:
				break;

			case OPT_INTEGER:
			{
				switch (option->action)
				{
					case OPT_NOTHING:
						break;

					case OPT_VARIABLE:
						*(int *)option->object = int_arg(argument);
						break;

					case OPT_FUNCTION:
					{
						int arg = int_arg(argument);

						if (option->has_arg == required_argument)
							((opt_action_int_t *)option->function)(arg);
						else
							((opt_action_optional_int_t *)option->function)(&arg);

						break;
					}
				}

				break;
			}

			case OPT_STRING:
			{
				switch (option->action)
				{
					case OPT_NOTHING:
						break;

					case OPT_VARIABLE:
						*(const char **)option->object = argument;
						break;

					case OPT_FUNCTION:
						((opt_action_string_t *)option->function)(argument);
						break;
				}

				break;
			}
		}
	}
	else
	{
		switch (option->action)
		{
			case OPT_NOTHING:
				break;

			case OPT_VARIABLE:
				if (option->arg_type != OPT_STRING)
					++*(int *)option->object;
				break;

			case OPT_FUNCTION:
				if (option->action == optional_argument)
					((opt_action_optional_int_t *)option->function)(NULL);
				else
					((opt_action_none_t *)option->function)();
				break;
		}
	}
}

/*

=item C<int opt_process(int argc, char **argv, Options *options, char *msgbuf, size_t bufsize)>

Parses C<argv> for options specified in C<options>. Uses I<GNU
getopt_long(3)>. As each option is encountered, its corresponding action is
performed. On success, returns C<optind>. On error, returns C<-1> with
C<errno> set appropriately.

The following table shows the actions that are applied to an option's
C<object> or C<function> based on its C<has_arg>, C<arg_type> and
C<arg_action> attributes and whether or not an argument is present.

 has_arg           arg_type    arg_action   optarg action
 ~~~~~~~~~~~~~~~~~ ~~~~~~~~~~~ ~~~~~~~~~~~~ ~~~~~~ ~~~~~~
 required_argument OPT_INTEGER OPT_VARIABLE yes    *object = strtol(argument)
 required_argument OPT_STRING  OPT_VARIABLE yes    *object = argument
 required_argument OPT_INTEGER OPT_FUNCTION yes    function(strtol(argument))
 required_argument OPT_STRING  OPT_FUNCTION yes    function(argument)

 optional_argument OPT_INTEGER OPT_VARIABLE yes    *object = strtol(argument)
 optional_argument OPT_STRING  OPT_VARIABLE yes    *object = argument
 optional_argument OPT_INTEGER OPT_FUNCTION yes    function(&strtol(argument))
 optional_argument OPT_STRING  OPT_FUNCTION yes    function(argument)

 optional_argument OPT_INTEGER OPT_VARIABLE no     ++*object
 optional_argument OPT_STRING  OPT_VARIABLE no     nothing
 optional_argument OPT_INTEGER OPT_FUNCTION no     function(NULL)
 optional_argument OPT_STRING  OPT_FUNCTION no     function(NULL)

 no_argument       OPT_NONE    OPT_VARIABLE no     ++*object
 no_argument       OPT_NONE    OPT_FUNCTION no     function()

Note that integer option arguments may be expressed in octal, decimal or
hexadecimal. There may be leading whitespace but no trailing text of any
kind. Overflow and underflow are also treated as errors.

=cut

*/

int opt_process(int argc, char **argv, Options *options, char *msgbuf, size_t bufsize)
{
	option *long_options;
	char *optstring;

	if (!(long_options = opt_convert(options)))
		return optind;

	if (!(optstring = opt_optstring(options)))
	{
		mem_release(long_options);
		return optind;
	}

	for (;;)
	{
		int longindex = -1;
		int rc;

		if ((rc = getopt_long(argc, argv, optstring, long_options, &longindex)) == EOF)
			break;

		if (rc == '?' || rc == ':')
		{
			mem_release(long_options);
			mem_release(optstring);
			return set_errno(EINVAL);
		}

		if (rc == '\001')
			rc = '?';

		errno = 0;
		opt_action(options, rc, longindex, optarg);
		if (errno == ERANGE || errno == EDOM)
		{
			if (msgbuf)
			{
				char optname[256];

				if (rc != 0 && longindex == -1) /* Short option */
					snprintf(optname, 256, "-%c", rc);
				else if (rc == 0 && longindex != -1) /* Long option */
					snprintf(optname, 256, "--%s", long_options[longindex].name);

				snprintf(msgbuf, bufsize, "%s: invalid %s argument: %s", *argv, optname, (errno == EDOM) ? "not an integer" : "integer out of range");
			}

			mem_release(long_options);
			mem_release(optstring);
			return -1;
		}
	}

	mem_release(long_options);
	mem_release(optstring);
	return optind;
}

/*

=item C<char *opt_usage(char *buf, size_t size, Options *options)>

Writes a usage message into C<buf> that displays the names, syntax and
descriptions of all options in C<options>. C<options> is traversed depth
first so the chunk with the C<null> C<parent> appears first. Each chunk of
options is preceded by a blank line. No more than C<size> bytes are
written, including the terminating C<nul> character. The string returned
will look like:

      -a, --aaa       -- no-arg/var option
      -b, --bbb       -- no-arg/func option
      -c, --ccc=arg   -- int-arg/var option
      -d, --ddd=arg   -- int-arg/func option
      -e, --eee=arg   -- str-arg/var option
      -f, --fff=arg   -- str-arg/func option
      -g, --ggg[=arg] -- opt-int-arg/var option
      -h, --hhh[=arg] -- opt-str-arg/func option with one of those really,
                         really, really, long descriptions that goes on and on
                         and even contains a really long url:
                         http://www.zip.com.au/~joe/fairly/long/url/index.html
                         would you believe? Here it is again!
                         http://www.zip.com.au/~joe/fairly/long/url/index.html#just_kidding

=cut

*/

char *opt_usage(char *buf, size_t size, Options *options)
{
	const int total_width = 80;
	const char * const indent = "      ";
	const size_t indent_width = strlen(indent);
	const char * const leader = " - ";
	const size_t leader_width = strlen(leader);

	Options *opts;
	Options **stack;
	size_t max_width = 0;
	size_t depth = 0;
	size_t length = 0;
	size_t remainder = 0;
	int i;

	/* Determine the room needed by the longest option */

	for (opts = options; opts; opts = opts->parent, ++depth)
	{
		for (i = 0; opts->options[i].name; ++i)
		{
			Option *opt = opts->options + i;
			size_t width = strlen(opt->name);

			if (opt->argname)
			{
				width += 1 + strlen(opt->argname);

				if (opt->has_arg == optional_argument)
					width += 2;
			}

			if (width > max_width)
				max_width = width;
		}
	}

	/* Include room for "-o, --" */

	max_width += 6;

	/* Remember all options for reverse traversal */

	if (!(stack = mem_create(depth, Options *)))
		return NULL;

	for (opts = options, i = 0; opts; opts = opts->parent)
		stack[i++] = opts;

	/* Process options parent first */

	while (depth--)
	{
		opts = stack[depth];

		snprintf(buf + length, size - length, "\n");
		length = strlen(buf);

		for (i = 0; opts->options[i].name; ++i)
		{
			Option *opt = opts->options + i;
			char help[BUFSIZ];
			const char *desc;
			const char *next = NULL;
			size_t desc_length;
			size_t help_length;

			/* Produce the left hand side: syntax */

			snprintf(help, BUFSIZ, "%s%c%c%c --%s",
				indent,
				opt->short_name ? '-' : ' ',
				opt->short_name ? opt->short_name : ' ',
				opt->short_name ? ',' : ' ',
				opt->name
			);

			help_length = strlen(help);

			if (opt->argname)
			{
				int optional = (opt->has_arg == optional_argument);

				snprintf(help + help_length, BUFSIZ - help_length, "%s%s%s",
					optional ? "[=" : "=",
					opt->argname,
					optional ? "]" : ""
				);

				help_length = strlen(help);
			}

			snprintf(help + help_length, BUFSIZ - help_length, "%*s%s", (int)(max_width - help_length + indent_width), "", leader);
			help_length = strlen(help);
			remainder = total_width - help_length;

			/* Produce the right hand side: descriptions */

			for (desc = opt->desc; (desc_length = strlen(desc)) > remainder; desc = next)
			{
				/* Indent subsequent description lines */
				if (desc != opt->desc)
				{
					snprintf(help + help_length, BUFSIZ - help_length, "%*s%*.*s", (int)(indent_width + max_width), "", (int)leader_width, (int)leader_width, "");
					help_length = strlen(help);
				}

				/* Look for last space that will fit on this line */

				next = desc + remainder;

				for (; next > desc && !isspace((int)(unsigned int)*next); --next)
				{}

				/* If none (word too long), look forward for end of word */

				if (next == desc)
				{
					while (isspace((int)(unsigned int)*next))
						++next;

					next = strchr(desc, ' ');

					if (!next)
						next = desc + desc_length;
				}

				/* Ignore any extra whitespace to the left */

				while (next != desc && isspace((int)(unsigned int)next[-1]))
					--next;

				/* Add one line of description */

				snprintf(help + help_length, BUFSIZ - help_length, "%*.*s\n", (int)(next - desc), (int)(next - desc), desc);
				help_length = strlen(help);

				/* Ignore any extra whitespace to the right */

				while (isspace((int)(unsigned int)*next))
					++next;
			}

			/* Add the last line of description */

			if (desc_length)
			{
				/* Indent the last line if it's not also the first line */

				if (desc != opt->desc)
				{
					snprintf(help + help_length, BUFSIZ - help_length, "%*s%*.*s", (int)(indent_width + max_width), "", (int)leader_width, (int)leader_width, "");
					help_length = strlen(help);
				}

				snprintf(help + help_length, BUFSIZ - help_length, "%s\n", desc);
			}

			/* Add this option's help to the whole usage message */

			snprintf(buf + length, size - length, "%s", help);
			length = strlen(buf);
		}
	}

	mem_release(stack);

	return buf;
}

/*

=back

=head1 ERRORS

On error, C<errno> is set either by an underlying function, or as follows:

=over 4

=item C<EINVAL>

Arguments are C<null> or invalid.

=item C<EDOM>

An integer option argument string failed to be parsed completely.

=item C<ERANGE>

An integer option argument string was out of integer range. In this case,
C<INT_MAX> or C<INT_MIN> is used.

=back

=head1 MT-Level

I<MT-Disciplined> - prog functions

By default, this module is not threadsafe, because most programs are
single-threaded, and synchronisation doesn't come for free. For
multi-threaded programs, use I<prog_set_locker(3)> to synchronise access to
this module's data, before creating the threads that will access it.

Unsafe - opt functions

I<opt_process(3)> and I<opt_usage(3)> must only be used in the main thread.
They should not be needed anywhere else. Normally, they would not be called
directly at all.

=head1 EXAMPLE

The following program:

 #include <slack/std.h>
 #include <slack/prog.h>

 char *name = NULL;
 int minimum = 0;
 int reverse = 0;

 void setup_syslog(char *facility) {}
 void parse_config(char *path) {}

 Option example_optab[] =
 {
     {
         "name", 'n', "name", "Provide a name",
         required_argument, OPT_STRING, OPT_VARIABLE, &name, NULL
     },
     {
         "minimum", 'm', "minval", "Ignore everything below minimum",
         required_argument, OPT_INTEGER, OPT_VARIABLE, &minimum, NULL
     },
     {
         "syslog", 's', "facility.priority", "Send client's output to syslog (defaults to local0.debug)",
         optional_argument, OPT_STRING, OPT_FUNCTION, NULL, (func_t *)setup_syslog
     },
     {
         "reverse", 'r', NULL, "Reverse direction",
         no_argument, OPT_NONE, OPT_VARIABLE, &reverse, NULL
     },
     {
         "config", 'c', "path", "Specify the configuration file",
         required_argument, OPT_STRING, OPT_FUNCTION, NULL, (func_t *)parse_config
     },
     {
         NULL, '\0', NULL, NULL, 0, 0, 0, NULL
     }
 };

 Options options[1] = {{ prog_options_table, example_optab }};

 int main(int ac, char **av)
 {
     int a;
     prog_init();
     prog_set_name("example");
     prog_set_syntax("[options] arg...");
     prog_set_options(options);
     prog_set_version("1.0");
     prog_set_date("20210220");
     prog_set_author("raf <raf@raf.org>");
     prog_set_contact(prog_author());
     prog_set_url("http://libslack.org/");
     prog_set_legal("This software is released under the terms of the GPL.\n");
     prog_set_desc("This program is an example of the prog module.\n");

     for (a = prog_opt_process(ac, av); a < ac; ++a)
         msg("av[%d] = \"%s\"\n", a, av[a]);

     return EXIT_SUCCESS;
 }

will behave like:

 $ example --version # to stdout
 example-1.0

 $ example --help # to stdout
 usage: example [options] arg...
 options:

       -h, --help                       - Print a help message then exit
       -V, --version                    - Print a version message then exit
       -v, --verbose[=level]            - Set the verbosity level
       -d, --debug[=level]              - Set the debugging level

       -n, --name=name                  - Provide a name
       -m, --minimum=minval             - Ignore everything below minimum
       -s, --syslog[=facility.priority] - Send client's output to syslog
                                          (defaults to local0.debug)
       -r, --reverse                    - Reverse direction
       -c, --config=path                - Specify the configuration file

 This program is an example of the prog module.

 Name: example
 Version: 1.0
 Date: 20210220
 Author: raf <raf@raf.org>
 URL: http://libslack.org/

 This software is released under the terms of the GPL.

 Report bugs to raf <raf@raf.org>

 $ example -x # to stderr
 ./example: invalid option -- x
 usage: example [options] arg...
 options:

       -h, --help                       - Print a help message then exit
       -V, --version                    - Print a version message then exit
       -v, --verbose[=level]            - Set the verbosity level
       -d, --debug[=level]              - Set the debugging level

       -n, --name=name                  - Provide a name
       -m, --minimum=minval             - Ignore everything below minimum
       -s, --syslog[=facility.priority] - Send client's output to syslog
                                          (defaults to local0.debug)
       -r, --reverse                    - Reverse direction
       -c, --config=path                - Specify the configuration file

 $ example a b c # to stdout
 av[1] = "a"
 av[2] = "b"
 av[3] = "c"

=head1 SEE ALSO

I<libslack(3)>,
I<getopt_long(3)>,
I<err(3)>,
I<msg(3)>,
I<prop(3)>,
I<sig(3)>,
I<locker(3)>

=head1 AUTHOR

20210220 raf <raf@raf.org>

=cut

*/

#endif

#ifdef TEST

#include <fcntl.h>
#include <sys/wait.h>

int verify(int i, const char *name, const char *result, const char *prog_name, const char *type)
{
	char buf[BUFSIZ];
	char result_buf[BUFSIZ];
	int fd;
	ssize_t bytes;
	char *q;

	if ((fd = open(name, O_RDONLY)) == -1)
	{
		printf("Test%d: failed to output message\n", i);
		return 1;
	}

	memset(buf, 0, BUFSIZ);
	bytes = read(fd, buf, BUFSIZ);
	close(fd);
	unlink(name);

	if (bytes == -1)
	{
		printf("Test%d: failed to read output (%s)\n", i, strerror(errno));
		return 1;
	}

	snprintf(result_buf, BUFSIZ, result, prog_name, prog_name);
	/* Replace ` with ' as getopt seems to use either */
	while ((q = strchr(buf, '`')))
		*q = '\'';

	if (strcmp(buf, result_buf))
	{
		printf("Test%d: incorrect %s:\nshould be:\n%s\nwas:\n%s\n", i, type, result_buf, buf);
		return 1;
	}

	return 0;
}

int intvar_a;
int intvar_b;
void nonefunc_b(void) { ++intvar_b; }
int intvar_c;
int intvar_d;
void intfunc_d(int arg) { intvar_d = arg; }
const char *strvar_e;
const char *strvar_f;
void strfunc_f(const char *arg) { strvar_f = arg; }
int optintvar_g;
int optintvar_h;
void optintfunc_h(int *arg) { if (arg) optintvar_h = *arg; else ++optintvar_h; }
const char *optstrvar_i;
const char *optstrvar_j;
void optstrfunc_j(const char *arg) { if (arg) optstrvar_j = arg; }

static Option optab[] =
{
	{ "aaa", 'a', NULL,  "no-arg/var option", no_argument, OPT_NONE, OPT_VARIABLE, &intvar_a, NULL },
	{ "bbb", 'b', NULL,  "no-arg/func option", no_argument, OPT_NONE, OPT_FUNCTION, NULL, (func_t *)nonefunc_b },
	{ "ccc", nul, "int", "int-arg/var option", required_argument, OPT_INTEGER, OPT_VARIABLE, &intvar_c, NULL },
	{ "ddd", 'd', "int", "int-arg/func option", required_argument, OPT_INTEGER, OPT_FUNCTION, NULL, (func_t *)intfunc_d },
	{ "eee", 'e', "str", "str-arg/var option", required_argument, OPT_STRING, OPT_VARIABLE, &strvar_e, NULL },
	{ "fff", nul, "str", "str-arg/func option", required_argument, OPT_STRING, OPT_FUNCTION, NULL, (func_t *)strfunc_f },
	{ "ggg", 'g', "int", "opt-int-arg/var option", optional_argument, OPT_INTEGER, OPT_VARIABLE, &optintvar_g, NULL },
	{ "hhh", 'h', "int", "opt-int-arg/func option", optional_argument, OPT_INTEGER, OPT_FUNCTION, NULL, (func_t *)optintfunc_h },
	{ "iii", 'i', "str", "opt-str-arg/var option", optional_argument, OPT_STRING, OPT_VARIABLE, &optstrvar_i, NULL },
	{ "jjj", 'j', "str", "opt-str-arg/func option with one of those really, really, really, long descriptions that goes on and on and even contains a really long url: http://www.zip.com.au/~joe/fairly/long/url/index.html would you believe? Here it is again! http://www.zip.com.au/~joe/fairly/long/url/index.html#just_kidding", optional_argument, OPT_STRING, OPT_FUNCTION, NULL, (func_t *)optstrfunc_j },
	{ NULL, nul, NULL, NULL, 0, 0, 0, NULL, NULL }
};

static Options options[1] = {{ NULL, optab }};

int main(int ac, char **av)
{
	int oargc = 26;
	char *oargv[] = /* Note: optstrvar with no argument (i.e. -i) does nothing  */
	{
		"test.opt", "-ab", "--aaa", "--bbb", "--ccc", "42",
		"-d", "37", "--ddd=51", "-e", "eee", "--eee", "123",
		"--fff", "fff", "--ggg=4", "-g8", "-h3", "-h", "--hhh",
		"-ifish", "--iii=carp", "-i", "--jjj=jjj",
		"remaining", "arguments", NULL
	};
	char buf[BUFSIZ];
	int rc;
	const char * const usage =
		"\n"
		"      -a, --aaa       - no-arg/var option\n"
		"      -b, --bbb       - no-arg/func option\n"
		"          --ccc=int   - int-arg/var option\n"
		"      -d, --ddd=int   - int-arg/func option\n"
		"      -e, --eee=str   - str-arg/var option\n"
		"          --fff=str   - str-arg/func option\n"
		"      -g, --ggg[=int] - opt-int-arg/var option\n"
		"      -h, --hhh[=int] - opt-int-arg/func option\n"
		"      -i, --iii[=str] - opt-str-arg/var option\n"
		"      -j, --jjj[=str] - opt-str-arg/func option with one of those really,\n"
		"                        really, really, long descriptions that goes on and on\n"
		"                        and even contains a really long url:\n"
		"                        http://www.zip.com.au/~joe/fairly/long/url/index.html\n"
		"                        would you believe? Here it is again!\n"
		"                        http://www.zip.com.au/~joe/fairly/long/url/index.html#just_kidding\n";

	const char *prog_name;
	char out_name[32];
	char err_name[32];
	int tests = 3;
	int argc = 3;
	char *argv[3][4] =
	{
		{ "prog.test", "--debug=4", "--help", NULL },
		{ "prog.test", "--debug=1", "--version", NULL },
		{ "prog.test", "--debug=1", "--invalid", NULL }
	};
	char *results[3][2] =
	{
		/* --help output */
		{
			/* stdout */
			"usage: %s [options]\n"
			"options:\n"
			"\n"
			"      -h, --help            - Print a help message then exit\n"
			"      -V, --version         - Print a version message then exit\n"
			"      -v, --verbose[=level] - Set the verbosity level\n"
			"      -d, --debug[=level]   - Set the debugging level\n"
			"\n"
			"This program tests the prog module.\n"
			"\n"
			"Name: %s\n"
			"Version: 1.0\n"
			"Date: 20210220\n"
			"Author: raf <raf@raf.org>\n"
			"Vendor: A Software Vendor\n"
			"URL: http://libslack.org/test/\n"
			"\n"
			"This software is released under the terms of the GPL.\n"
			"\n"
			"Report bugs to raf <raf@raf.org>\n",

			/* stderr */
			""
		},

		/* --version output */

		{
			/* stdout */
			"%s-1.0\n",

			/* stderr */
			""
		},

		/* --invalid output */
		{
			/* stdout */
			"",

			/* stderr */
			"%s: unrecognized option '--invalid'\n"
			"usage: %s [options]\n"
			"options:\n"
			"\n"
			"      -h, --help            - Print a help message then exit\n"
			"      -V, --version         - Print a version message then exit\n"
			"      -v, --verbose[=level] - Set the verbosity level\n"
			"      -d, --debug[=level]   - Set the debugging level\n"
		}
	};

	int errors = 0;
	int i;

	if (ac == 2 && !strcmp(av[1], "help"))
	{
		printf("usage: %s\n", *av);
		return EXIT_SUCCESS;
	}

	printf("Testing: %s\n", "prog");

	rc = opt_process(oargc, oargv, options, NULL, 0);
	if (rc != 24)
		++errors, printf("Test1: rc = %d (not 24, fail)\n", rc);
	if (intvar_a != 2)
		++errors, printf("Test2: intvar_a = %d (not 2, fail)\n", intvar_a);
	if (intvar_b != 2)
		++errors, printf("Test3: intvar_b = %d (not 2, fail)\n", intvar_b);
	if (intvar_c != 42)
		++errors, printf("Test4: intvar_c = %d (not 42, fail)\n", intvar_c);
	if (intvar_d != 51)
		++errors, printf("Test5: intvar_d = %d (not 51, fail)\n", intvar_d);
	if (strcmp(strvar_e ? strvar_e : "NULL", "123"))
		++errors, printf("Test6: strvar_e = '%s' (not '123', fail)\n", strvar_e ? strvar_e : "NULL");
	if (strcmp(strvar_f ? strvar_f : "NULL", "fff"))
		++errors, printf("Test7: strvar_f = '%s' (not 'fff', fail)\n", strvar_f ? strvar_f : "NULL");
	if (optintvar_g != 8)
		++errors, printf("Test8: optintvar_g = %d (not 8, fail)\n", optintvar_g);
	if (optintvar_h != 5)
		++errors, printf("Test9: optintvar_h = %d (not 5, fail)\n", optintvar_h);
	if (!optstrvar_i)
		optstrvar_i = "NULL";
	if (strcmp(optstrvar_i, "carp"))
		++errors, printf("Test10: optstrvar_i = '%s' (not 'carp', fail)\n", optstrvar_i);
	if (!optstrvar_j)
		optstrvar_j = "NULL";
	if (strcmp(optstrvar_j, "jjj"))
		++errors, printf("Test11: optstrvar_j = '%s' (not 'jjj', fail)\n", optstrvar_j);

	opt_usage(buf, BUFSIZ, options);
	if (strcmp(buf, usage))
		++errors, printf("Test12: opt_usage() produced incorrect output:\nshould be:\n%s\nwas:\n%s\n", usage, buf);

	optind = 0;

	prog_name = prog_basename(*av);
	argv[0][0] = argv[1][0] = argv[2][0] = (char *)prog_name;

	prog_init();
	prog_set_name(prog_name);
	prog_set_syntax("[options]");
	prog_set_options(prog_options_table);
	prog_set_version("1.0");
	prog_set_date("20210220");
	prog_set_author("raf <raf@raf.org>");
	prog_set_contact("raf <raf@raf.org>");
	prog_set_vendor("A Software Vendor");
	prog_set_url("http://libslack.org/test/");
	prog_set_legal("This software is released under the terms of the GPL.\n");
	prog_set_desc("This program tests the prog module.\n");

	for (i = 0; i < tests; ++i)
	{
		pid_t pid;

		snprintf(out_name, 32, "prog.out.%d", i);
		snprintf(err_name, 32, "prog.err.%d", i);

		switch (pid = fork())
		{
			case 0:
			{
				freopen(out_name, "a", stdout);
				freopen(err_name, "a", stderr);

				return (prog_opt_process(argc, argv[i]) != argc);
			}

			case -1:
			{
				++errors, printf("Test%d: failed to perform test - fork() failed (%s)\n", 12 + i + 1, strerror(errno));
				continue;
			}

			default:
			{
				int status;

				if (waitpid(pid, &status, 0) == -1)
				{
					++errors, printf("Test%d: failed to wait for test - waitpid(%d) failed (%s)\n", 12 + i + 1, (int)pid, strerror(errno));
					continue;
				}

				if (WIFSIGNALED(status))
					++errors, printf("Test%d: failed: received signal %d\n", 12 + i + 1, WTERMSIG(status));

				if (i != 2 && WIFEXITED(status) && WEXITSTATUS(status) != EXIT_SUCCESS)
					++errors, printf("Test%d: failed: exit status %d\n", 12 + i + 1, WEXITSTATUS(status));
			}
		}

		errors += verify(12 + i + 1, out_name, results[i][0], prog_name, "stdout");
		errors += verify(12 + i + 1, err_name, results[i][1], prog_name, "stderr");
	}

	if (errors)
		printf("%d/%d tests failed\n", errors, 12 + tests * 2);
	else
		printf("All tests passed\n");

	return (errors == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

#endif

/* vi:set ts=4 sw=4: */
