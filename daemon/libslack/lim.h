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

#ifndef LIBSLACK_LIM_H
#define LIBSLACK_LIM_H

#include <slack/hdr.h>

_begin_decls
long limit_arg(void);
long limit_child(void);
long limit_tick(void);
long limit_group(void);
long limit_open(void);
long limit_stream(void);
long limit_tzname(void);
long limit_job(void);
long limit_save_ids(void);
long limit_version(void);
long limit_pcanon(const char *path);
long limit_fcanon(int fd);
long limit_canon(void);
long limit_pinput(const char *path);
long limit_finput(int fd);
long limit_input(void);
long limit_pvdisable(const char *path);
long limit_fvdisable(int fd);
long limit_vdisable(void);
long limit_plink(const char *path);
long limit_flink(int fd);
long limit_link(void);
long limit_pname(const char *path);
long limit_fname(int fd);
long limit_name(void);
long limit_ppath(const char *path);
long limit_fpath(int fd);
long limit_path(void);
long limit_ppipe(const char *path);
long limit_fpipe(int fd);
long limit_pnotrunc(const char *path);
long limit_fnotrunc(int fd);
long limit_notrunc(void);
long limit_pchown(const char *path);
long limit_fchown(int fd);
long limit_chown(void);
_end_decls

#endif

/* vi:set ts=4 sw=4: */
