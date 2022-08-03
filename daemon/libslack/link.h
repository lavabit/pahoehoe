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

#ifndef LIBSLACK_LINK_H
#define LIBSLACK_LINK_H

#include <slack/hdr.h>

typedef struct slink_t slink_t;
typedef struct dlink_t dlink_t;

struct slink_t
{
	void *next;
};

struct dlink_t
{
	void *next;
	void *prev;
};

_begin_decls
int slink_has_next(void *link);
void *slink_next(void *link);
int dlink_has_next(void *link);
void *dlink_next(void *link);
int dlink_has_prev(void *link);
void *dlink_prev(void *link);
void *slink_insert(void *link, void *item);
void *dlink_insert(void *link, void *item);
void *slink_remove(void *link);
void *dlink_remove(void *link);
void *slink_freelist_init(void *freelist, size_t nelem, size_t size);
void *dlink_freelist_init(void *freelist, size_t nelem, size_t size);
void *slink_freelist_attach(void *freelist1, void *freelist2);
void *dlink_freelist_attach(void *freelist1, void *freelist2);
void *slink_alloc(void **freelist);
void *dlink_alloc(void **freelist);
void *slink_free(void **freelist, void *item);
void *dlink_free(void **freelist, void *item);
_end_decls

#endif

/* vi:set ts=4 sw=4: */
