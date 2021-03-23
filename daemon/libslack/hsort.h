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

#ifndef LIBSLACK_HSORT_H
#define LIBSLACK_HSORT_H

#include <stdlib.h>

#include <slack/hdr.h>

typedef int hsort_cmp_t(const void *a, const void *b);
typedef int hsort_closure_cmp_t(const void *a, const void *b, const void *data);

_begin_decls
void hsort(void *base, size_t n, size_t size, hsort_cmp_t *cmp);
void hsort_closure(void *base, size_t n, size_t size, hsort_closure_cmp_t *cmp, const void *data);
_end_decls

#endif

/* vi:set ts=4 sw=4: */
