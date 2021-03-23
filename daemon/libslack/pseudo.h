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
 * Author: Tatu Ylonen <ylo@cs.hut.fi>
 * Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
 *                    All rights reserved
 * Functions for allocating a pseudo-terminal and making it the controlling
 * tty.
 *
 * As far as I am concerned, the code I have written for this software
 * can be used freely for any purpose.  Any derived versions of this
 * software must be clearly marked as such, and if the derived work is
 * incompatible with the protocol description in the RFC file, it must be
 * called by a name other than "ssh" or "Secure Shell".
 */

#ifndef LIBSLACK_PSEUDO_H
#define LIBSLACK_PSEUDO_H

#include <termios.h>

#include <sys/ioctl.h>
#include <sys/types.h>

#include <slack/hdr.h>

_begin_decls
int pty_open(int *pty_user_fd, int *pty_process_fd, char *pty_device_name, size_t pty_device_name_size, const struct termios *pty_device_termios, const struct winsize *pty_device_winsize);
int pty_release(const char *pty_device_name);
int pty_set_owner(const char *pty_device_name, uid_t uid);
int pty_make_controlling_tty(int *pty_process_fd, const char *pty_device_name);
int pty_change_window_size(int pty_user_fd, int row, int col, int xpixel, int ypixel);
pid_t pty_fork(int *pty_user_fd, char *pty_device_name, size_t pty_device_name_size, const struct termios *pty_device_termios, const struct winsize *pty_device_winsize);
_end_decls

#endif
