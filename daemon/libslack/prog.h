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

#ifndef LIBSLACK_PROG_H
#define LIBSLACK_PROG_H

#include <stdlib.h>

#include <slack/hdr.h>
#include <slack/msg.h>

#ifndef PATH_SEP
#define PATH_SEP '/'
#endif

#ifndef HAVE_GETOPT_LONG
#include <slack/getopt.h>
#else
#include <getopt.h>
#endif

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

_begin_decls
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
_end_decls

#endif

/* vi:set ts=4 sw=4: */
