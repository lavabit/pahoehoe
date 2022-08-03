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

#ifndef LIBSLACK_FIO_H
#define LIBSLACK_FIO_H

#include <fcntl.h>
#include <sys/types.h>

#include <slack/hdr.h>

_begin_decls
char *fgetline(char *line, size_t size, FILE *stream);
char *fgetline_unlocked(char *line, size_t size, FILE *stream);
int read_timeout(int fd, long sec, long usec);
int write_timeout(int fd, long sec, long usec);
int rw_timeout(int fd, long sec, long usec);
int nap(long sec, long usec);
int fcntl_set_flag(int fd, int flag);
int fcntl_clear_flag(int fd, int flag);
int fcntl_set_fdflag(int fd, int flag);
int fcntl_clear_fdflag(int fd, int flag);
int fcntl_lock(int fd, int cmd, int type, int whence, int start, int len);
int nonblock_set(int fd, int arg);
int nonblock_on(int fd);
int nonblock_off(int fd);
int fifo_exists(const char *path, int prepare);
int fifo_has_reader(const char *path, int prepare);
int fifo_open(const char *path, mode_t mode, int lock, int *writefd);
_end_decls

#endif

/* vi:set ts=4 sw=4: */
