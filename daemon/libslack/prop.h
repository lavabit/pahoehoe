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

#ifndef LIBSLACK_PROP_H
#define LIBSLACK_PROP_H

#include <slack/hdr.h>
#include <slack/locker.h>

_begin_decls
const char *prop_get(const char *name);
const char *prop_get_or(const char *name, const char *default_value);
const char *prop_set(const char *name, const char *value);
int prop_get_int(const char *name);
int prop_get_int_or(const char *name, int default_value);
int prop_set_int(const char *name, int value);
double prop_get_double(const char *name);
double prop_get_double_or(const char *name, double default_value);
double prop_set_double(const char *name, double value);
int prop_get_bool(const char *name);
int prop_get_bool_or(const char *name, int default_value);
int prop_set_bool(const char *name, int value);
int prop_unset(const char *name);
int prop_save(void);
int prop_clear(void);
int prop_locker(Locker *locker);
_end_decls

#endif

/* vi:set ts=4 sw=4: */
