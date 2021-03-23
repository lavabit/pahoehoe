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

#ifndef LIBSLACK_MEM_H
#define LIBSLACK_MEM_H

#include <stdlib.h>

#include <slack/hdr.h>
#include <slack/locker.h>

#ifndef null
#define null NULL
#endif

#ifndef nul
#define nul '\0'
#endif

typedef struct Pool Pool;

_begin_decls
#define mem_new(type) malloc(sizeof(type))
#define mem_create(size, type) malloc((size) * sizeof(type))
#define mem_resize(mem, size) mem_resize_fn((void **)(mem), (size) * sizeof(**(mem)))
void *mem_resize_fn(void **mem, size_t size);
#define mem_release(mem) free(mem)
void *mem_destroy(void **mem);
#define mem_destroy(mem) (mem_destroy)((void **)(mem))
void *mem_create_secure(size_t size);
void mem_release_secure(void *mem);
void *mem_destroy_secure(void **mem);
#define mem_destroy_secure(mem) (mem_destroy_secure)((void **)(mem))
char *mem_strdup(const char *str);
#define mem_create2d(i, j, type) ((type **)mem_create_space(sizeof(type), (i), (j), 0))
#define mem_create3d(i, j, k, type) ((type ***)mem_create_space(sizeof(type), (i), (j), (k), 0))
#define mem_create4d(i, j, k, l, type) ((type ****)mem_create_space(sizeof(type), (i), (j), (k), (l), 0))
void *mem_create_space(size_t size, ...);
size_t mem_space_start(size_t size, ...);
#define mem_release2d(space) mem_release_space(space)
#define mem_release3d(space) mem_release_space(space)
#define mem_release4d(space) mem_release_space(space)
#define mem_release_space(space) mem_release(space)
#define mem_destroy2d(space) mem_destroy_space(space)
#define mem_destroy3d(space) mem_destroy_space(space)
#define mem_destroy4d(space) mem_destroy_space(space)
#define mem_destroy_space(space) mem_destroy(space)
Pool *pool_create(size_t size);
Pool *pool_create_with_locker(Locker *locker, size_t size);
void pool_release(Pool *pool);
void *pool_destroy(Pool **pool);
Pool *pool_create_secure(size_t size);
Pool *pool_create_secure_with_locker(Locker *locker, size_t size);
void pool_release_secure(Pool *pool);
void *pool_destroy_secure(Pool **pool);
void pool_clear_secure(Pool *pool);
#define pool_new(pool, type) pool_alloc((pool), sizeof(type))
#define pool_newsz(pool, size, type) pool_alloc((pool), (size) * sizeof(type))
void *pool_alloc(Pool *pool, size_t size);
void pool_clear(Pool *pool);
_end_decls

#endif

/* vi:set ts=4 sw=4: */
