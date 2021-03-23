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

#ifndef H_COPROC_H
#define H_COPROC_H

#include <termios.h>

#include <sys/ioctl.h>
#include <sys/types.h>

#include <slack/hdr.h>

_begin_decls
pid_t coproc_open(int *to, int *from, int *err, const char *cmd, char * const *argv, char * const *envv, void (*action)(void *data), void *data);
int coproc_close(pid_t pid, int *to, int *from, int *err);
pid_t coproc_pty_open(int *pty_user_fd, char *pty_device_name, size_t pty_device_name_size, const struct termios *pty_device_termios, const struct winsize *pty_device_winsize, const char *cmd, char * const *argv, char * const *envv, void (*action)(void *data), void *data);
int coproc_pty_close(pid_t pid, int *pty_user_fd, const char *pty_device_name);
_end_decls

#endif

/* vi:set ts=4 sw=4: */
