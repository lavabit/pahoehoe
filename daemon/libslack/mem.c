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

=head1 NAME

I<libslack(mem)> - memory module

=head1 SYNOPSIS

    #include <slack/std.h>
    #include <slack/mem.h>

    typedef struct Pool Pool;

    #define null NULL
    #define nul '\0'

    #define mem_new(type)
    #define mem_create(size, type)
    #define mem_resize(mem, size)
    void *mem_resize_fn(void **mem, size_t size);
    #define mem_release(mem)
    void *mem_destroy(void **mem);
    void *mem_create_secure(size_t size);
    void mem_release_secure(void *mem);
    void *mem_destroy_secure(void **mem);
    char *mem_strdup(const char *str);
    #define mem_create2d(type, x, y)
    #define mem_create3d(type, x, y, z)
    #define mem_create4d(type, x, y, z, a)
    void *mem_create_space(size_t size, ...);
    size_t mem_space_start(size_t size, ...);
    #define mem_release2d(space)
    #define mem_release3d(space)
    #define mem_release4d(space)
    #define mem_release_space(space)
    #define mem_destroy2d(space)
    #define mem_destroy3d(space)
    #define mem_destroy4d(space)
    #define mem_destroy_space(space)
    Pool *pool_create(size_t size);
    Pool *pool_create_with_locker(Locker *locker, size_t size);
    void pool_release(Pool *pool);
    void *pool_destroy(Pool **pool);
    Pool *pool_create_secure(size_t size);
    Pool *pool_create_secure_with_locker(Locker *locker, size_t size);
    void pool_release_secure(Pool *pool);
    void *pool_destroy_secure(Pool **pool);
    void pool_clear_secure(Pool *pool);
    #define pool_new(pool, type)
    #define pool_newsz(pool, size, type)
    void *pool_alloc(Pool *pool, size_t size);
    void pool_clear(Pool *pool);

=head1 DESCRIPTION

This module is mostly just an interface to I<malloc(3)>, I<realloc(3)> and
I<free(3)> that tries to ensure that pointers that don't point to anything
get set to C<null>. It also provides dynamically allocated multi-dimensional
arrays, memory pools and secure memory for the more adventurous.

=over 4

=cut

*/

#include "config.h"
#include "std.h"

#ifdef HAVE_MLOCK
#include <sys/mman.h>
#endif

#include "err.h"
#include "mem.h"

struct Pool
{
	size_t size;    /* number of bytes in the pool */
	size_t used;    /* number of bytes allocated from the pool */
	char *pool;     /* address of the pool */
	Locker *locker; /* locking strategy for the pool */
};

#ifndef TEST

/*

=item C< #define null NULL>

Easier to type. Easier to read.

=item C< #define nul '\0'>

A name for the C<nul> character.

=item C< #define mem_new(type)>

Allocates enough memory (with I<malloc(3)>) to store an object of type
C<type>. It is the caller's responsibility to deallocate the allocated
memory with I<mem_release(3)>, I<mem_destroy(3)> or I<free(3)>. On success,
returns the address of the allocated memory. On error, returns C<null>.

=item C< #define mem_create(size, type)>

Allocates enough memory (with I<malloc(3)>) to store C<size> objects of type
C<type>. It is the caller's responsibility to deallocate the allocated
memory with I<mem_release(3)>, I<mem_destroy(3)> or I<free(3)>. On success,
returns the address of the allocated memory. On error, returns C<null>.

=item C< #define mem_resize(mem, num)>

Alters the amount of memory pointed to by C<*mem>. If C<*mem> is C<null>,
new memory is allocated and assigned to C<*mem>. If size is zero, C<*mem> is
deallocated and C<null> is assigned to C<*mem>. Otherwise, C<*mem> is
reallocated and assigned back to C<*mem>. On success, returns C<*mem> (which
will be C<null> if C<size> is zero). On error, returns C<null> with C<errno>
set appropriately and C<*mem> is not altered.

=item C<void *mem_resize_fn(void **mem, size_t size)>

An interface to I<realloc(3)> that also assigns to a pointer variable unless
an error occurred. C<mem> points to the pointer to be affected. C<size> is
the requested size in bytes. If C<size> is zero, C<*mem> is deallocated and
set to C<null>. This function is exposed as an implementation side effect.
Don't call it directly. Call I<mem_resize(3)> instead. On error, returns
C<null> with C<errno> set appropriately.

=cut

*/

#ifdef HAVE_ISOC_REALLOC

#define isoc_realloc realloc

#else

static void *isoc_realloc(void *ptr, size_t size)
{
	void *p;

	if (size)
	{
		if (!(p = (ptr) ? realloc(ptr, size) : malloc(size)))
			errno = ENOMEM; /* Not required by ISO C but handy */
	}
	else
	{
		free(ptr);
		p = NULL;
	}

	return p;
}

#endif

void *mem_resize_fn(void **mem, size_t size)
{
	void *ptr;

	if (!mem)
		return set_errnull(EINVAL);

	ptr = isoc_realloc(*mem, size);
	if (size && !ptr)
		return NULL;

	return *mem = ptr;
}

/*

=item C< #define mem_release(mem)>

Releases (deallocates) C<mem>. Same as I<free(3)>. Only to be used in
destructor functions. In other cases, use I<mem_destroy(3)> which also sets
C<mem> to C<null>.

=item C<void *mem_destroy(void **mem)>

Calls I<free(3)> on the pointer, C<*mem>. Then assigns C<null> to this
pointer. Returns C<null>.

=cut

*/

void *(mem_destroy)(void **mem)
{
	if (mem && *mem)
	{
		free(*mem);
		*mem = NULL;
	}

	return NULL;
}

/*

=item C<void *mem_create_secure(size_t size)>

Allocates C<size> bytes of memory (with I<malloc(3)>) and then locks it into
RAM with I<mlock(2)> so that it can't be paged to disk where some nefarious
local user with root access might read its contents. It is the caller's
responsibility to deallocate the secure memory with I<mem_release_secure(3)>
or I<mem_destroy_secure(3)> which will clear the memory and unlock it before
deallocating it. On success, returns the address of the secure allocated
memory. On error, returns C<null> with C<errno> set appropriately.

Note that entire pages are locked by I<mlock(2)> so don't create many, small
pieces of secure memory or many entire pages will be locked. Use a secure
memory pool instead. Also note that on old systems, secure memory requires
root privileges.

On some systems (e.g. Solaris), memory locks must start on page boundaries.
So we need to I<malloc(3)> enough memory to extend from whatever address
I<malloc(3)> may return to the next page boundary (worst case: C<pagesize -
sizeof(int)>) and then the actual number of bytes requested. We need an
additional 8 bytes to store the address returned by I<malloc(3)> (so we can
I<free(3)> it later) and the size passed to I<mlock(2)> so we can pass it to
I<munlock(2)> later. Unfortunately, we need to store the address and size
after the page boundary and not before it because I<malloc(3)> may return a
page boundary or an address less than 8 bytes to the left of a page
boundary.

It will look like:

   for free()
   +-------+       +- size+8 for munlock()
   v       |       v
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |       |* * * *|# # # #|       |       |       |       |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   ^       ^               ^ . . . size bytes . . . . . .
   |       +- next page    |
   +- malloc()             +- address returned

If your system doesn't require page boundaries (e.g. Linux), the address
returned by I<malloc(3)> is locked and returned and only the size is stored.

=cut

*/

void *mem_create_secure(size_t size)
{
#ifdef HAVE_MLOCK

	char *addr = NULL, *lock = NULL;
#ifdef MLOCK_REQUIRES_PAGE_BOUNDARY
	long pagesize;

	if ((pagesize = sysconf(_SC_PAGESIZE)) == -1)
		return set_errnull(EINVAL);

	size += sizeof(void *) + sizeof(size_t);
	addr = malloc(pagesize - sizeof(int) + size);
#else
	size += sizeof(size_t);
	addr = malloc(size);
#endif

	if (!addr)
		return NULL;

#ifdef MLOCK_REQUIRES_PAGE_BOUNDARY
	if ((long)addr & (pagesize - 1)) /* addr not on page boundary */
		lock = (void *)(((long)addr & ~(pagesize - 1)) + pagesize);
	else
		lock = addr;
#else
	lock = addr;
#endif

	if (mlock(lock, size) == -1)
	{
		free(addr);
		return NULL;
	}

#ifdef MLOCK_REQUIRES_PAGE_BOUNDARY
	*(void **)lock = addr;
	lock += sizeof(void *);
#endif
	*(size_t *)lock = size;
	lock += sizeof(size_t);

	return lock;
#else
	errno = ENOSYS;
	return NULL;
#endif
}

/*

=item C<void mem_release_secure(void *mem)>

Sets the memory pointed to by C<mem> to C<0xff> bytes, then to C<0xaa>
bytes, then to C<0x55> bytes, then to C<nul> bytes, then unlocks and
releases (deallocates) C<mem>. Only to be used on memory returned by
I<mem_create_secure(3)>. Only to be used in destructor functions. In other
cases, use I<mem_destroy(3)> which also sets C<mem> to C<null>.

=cut

*/

void mem_release_secure(void *mem)
{
#ifdef HAVE_MLOCK
	char *addr, *lock;
	size_t size;

	if (!mem)
		return;

	lock = mem;
	lock -= sizeof(size_t);
	size = *(size_t *)lock;
#ifdef MLOCK_REQUIRES_PAGE_BOUNDARY
	lock -= sizeof(void *);
	addr = *(void **)lock;
#else
	addr = lock;
#endif

	memset(lock, 0xff, size);
	memset(lock, 0xaa, size);
	memset(lock, 0x55, size);
	memset(lock, 0x00, size);
	munlock(lock, size);
	free(addr);
#endif
}

/*

=item C<void *mem_destroy_secure(void **mem)>

Sets the memory pointed to by C<*mem> to C<nul> bytes, then unlocks and
destroys (deallocates and sets to C<null>) C<*mem>. Only to be used on
memory returned by I<mem_create_secure(3)>. Returns C<null>.

=cut

*/

void *(mem_destroy_secure)(void **mem)
{
	if (mem && *mem)
	{
		mem_release_secure(*mem);
		*mem = NULL;
	}

	return NULL;
}

/*

=item C<char *mem_strdup(const char *str)>

Returns a dynamically allocated copy of C<str>. It is the caller's
responsibility to deallocate the new string with I<mem_release(3)>,
I<mem_destroy(3)> or I<free(3)>. This function exists because I<strdup(3)>
is not part of the ISO C standard. On error, returns C<null> with C<errno>
set appropriately.

=cut

*/

char *mem_strdup(const char *str)
{
	size_t size;
	char *copy;

	if (!str)
		return set_errnull(EINVAL);

	if (!(copy = mem_create(size = strlen(str) + 1, char)))
		return NULL;

	return memcpy(copy, str, size);
}

/*

=item C< #define mem_create2d(i, j, type)>

Alias for allocating a 2-dimensional array. See I<mem_create_space(3)>.

=item C< #define mem_create3d(i, j, k, type)>

Alias for allocating a 3-dimensional array. See I<mem_create_space(3)>.

=item C< #define mem_create4d(i, j, k, l, type)>

Alias for allocating a 4-dimensional array. See I<mem_create_space(3)>.

=item C<void *mem_create_space(size_t size, ...)>

Allocates a multi-dimensional array of elements of size C<size> and sets the
memory to C<nul> bytes. The remaining arguments specify the sizes of each
dimension. The last argument must be zero. There is an arbitrary limit of 32
dimensions. The memory returned is set to zero. The memory returned needs to
be cast or assigned into the appropriate pointer type. You can then set and
access elements exactly like a real multi-dimensional C array. Finally, it
must be deallocated with I<mem_destroy_space(3)> or I<mem_release_space(3)>
or I<mem_destroy(3)> or I<mem_release(3)> or I<free(3)>.

Note: You must not use I<memset(3)> on all of the returned memory because
the start of this memory contains pointers into the remainder. The exact
amount of this overhead depends on the number and size of dimensions. The
memory is allocated with I<calloc(3)> to reduce the need to I<memset(3)> the
elements but if you need to know where the elements begin, use
I<mem_space_start(3)>.

The memory returned looks like (e.g.):

  char ***a = mem_create3d(2, 2, 3, char);

                                          +-------------------------+
                                  +-------|-------------------+     |
  a                       +-------|-------|-------------+     |     |
  |               +-------|-------|-------|-------+     |     |     |
  v               |       |       |       |       V     V     V     V
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 | a[0]  | a[1]  |a[0][0]|a[0][1]|a[1][0]|a[1][1]| | | | | | | | | | | | |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |       |       ^               ^               a a a a a a a a a a a a
  +-------|-------+               |               0 0 0 0 0 0 1 1 1 1 1 1
          +-----------------------+               0 0 0 1 1 1 0 0 0 1 1 1
                                                  0 1 2 0 1 2 0 1 2 0 1 2

=cut

*/

#ifndef MEM_MAX_DIM
#define MEM_MAX_DIM 32
#endif

void *mem_create_space(size_t size, ...)
{
	size_t dim[MEM_MAX_DIM], d, i, j;
	size_t lengths[MEM_MAX_DIM];
	size_t starts[MEM_MAX_DIM];
	size_t sizes[MEM_MAX_DIM];
	char *space;
	size_t arg, length;
	va_list args;

	va_start(args, size);
	for (d = 0; d < MEM_MAX_DIM && (arg = va_arg(args, size_t)); ++d)
		dim[d] = arg;
	va_end(args);

	for (length = i = 0; i < d; ++i)
	{
		starts[i] = length;
		lengths[i] = sizes[i] = (i == d - 1) ? size : sizeof(void *);
		for (j = 0; j <= i; ++j)
			lengths[i] *= dim[j];
		length += lengths[i];
	}

	if (!(space = calloc(length, 1)))
		return NULL;

	for (i = 0; i < d - 1; ++i)
	{
		size_t num = dim[i];

		for (j = 0; j < i; ++j)
			num *= dim[j];

		for (j = 0; j < num; ++j)
			*(char **)(space + starts[i] + j * sizes[i]) = space + starts[i + 1] + j * dim[i + 1] * sizes[i + 1];
	}

	return space;
}

/*

=item C<size_t mem_space_start(size_t size, ...)>

Calculates the amount of overhead required for a multi-dimensional array
created by a call to I<mem_create_space(3)> with the same arguments. If you
need reset all elements in such an array to zero:

    int ****space = mem_create_space(sizeof(int), 2, 3, 4, 5, 0);
    size_t start = mem_space_start(sizeof(int), 2, 3, 4, 5, 0);
    memset((char *)space + start, '\0', sizeof(int) * 2 * 3 * 4 * 5);

=cut

*/

size_t mem_space_start(size_t size, ...)
{
	size_t dim[MEM_MAX_DIM], d, i, j;
	size_t lengths[MEM_MAX_DIM];
	size_t arg, length;
	va_list args;

	va_start(args, size);
	for (d = 0; d < MEM_MAX_DIM && (arg = va_arg(args, size_t)); ++d)
		dim[d] = arg;
	va_end(args);

	for (length = i = 0; i < d; ++i)
	{
		lengths[i] = (i == d - 1) ? size : sizeof(void *);
		for (j = 0; j <= i; ++j)
			lengths[i] *= dim[j];
		length += lengths[i];
	}

	return length - lengths[d - 1];
}

/*

=item C< #define mem_release2d(space)>

Alias for releasing (deallocating) a 2-dimensional array.
See I<mem_release_space(3)>.

=item C< #define mem_release3d(space)>

Alias for releasing (deallocating) a 3-dimensional array.
See I<mem_release_space(3)>.

=item C< #define mem_release4d(space)>

Alias for releasing (deallocating) a 4-dimensional array.
See I<mem_release_space(3)>.

=item C< #define mem_release_space(space)>

Releases (deallocates) a multi-dimensional array, C<space>, allocated with
I<mem_create_space>. Same as I<free(3)>. Only to be used in destructor
functions. In other cases, use I<mem_destroy_space(3)> which also sets
C<space> to C<null>.

=item C< #define mem_destroy2d(space)>

Alias for destroying (deallocating and setting to C<null>) a
2-dimensional array. See I<mem_destroy_space(3)>.

=item C< #define mem_destroy3d(space)>

Alias for destroying (deallocating and setting to C<null>) a
3-dimensional array. See I<mem_destroy_space(3)>.

=item C< #define mem_destroy4d(space)>

Alias for destroying (deallocating and setting to C<null>) a
4-dimensional array. See I<mem_destroy_space(3)>.

=item C< #define mem_destroy_space(mem)>

Destroys (deallocates and sets to C<null>) the multi-dimensional array
pointed to by C<space>.

=cut

*/

/*

=item C<Pool *pool_create(size_t size)>

Creates a memory pool of size C<size> from which smaller chunks of memory
may be subsequently allocated (with I<pool_alloc(3)>) without resorting to
the use of I<malloc(3)>. Useful when you have many small objects to allocate
but I<malloc(3)> is slowing your program down too much. It is the caller's
responsibility to deallocate the new pool with I<pool_release(3)> or
I<pool_destroy(3)>. On success, returns the pool. On error, returns C<null>.

The size of a pool can't be changed after it is created and the individual
chunks of memory allocated from within a pool can't be separately
deallocated. The entire pool can be emptied with I<pool_clear(3)>.

=cut

*/

Pool *pool_create(size_t size)
{
	return pool_create_with_locker(NULL, size);
}

/*

=item C<Pool *pool_create_with_locker(Locker *locker, size_t size)>

Equivalent to I<pool_create(3)> except that multiple threads accessing the
new pool will be synchronised by C<locker>.

=cut

*/

Pool *pool_create_with_locker(Locker *locker, size_t size)
{
	Pool *pool = mem_create(1, Pool);

	if (!pool)
		return NULL;

	if (!(pool->pool = malloc(size)))
	{
		mem_release(pool);
		return NULL;
	}

	pool->size = size;
	pool->used = 0;
	pool->locker = locker;

	return pool;
}

/*

C<int pool_lock(Pool *pool)>

Claims a write lock on C<pool>. On success, returns C<0>. On error, returns
an error code.

C<int pool_unlock(Pool *pool)>

Unlocks a write lock on C<pool>. On success, returns C<0>. On error, returns
an error code.

*/

#define pool_lock(pool) locker_wrlock((pool)->locker)
#define pool_unlock(pool) locker_unlock((pool)->locker)

/*

=item C<void pool_release(Pool *pool)>

Releases (deallocates) C<pool>. Only to be used in destructor functions. In
other cases, use I<pool_destroy(3)> which also sets C<pool> to C<null>.

=cut

*/

void pool_release(Pool *pool)
{
	Locker *locker;
	int err;

	if (!pool)
		return;

	if ((err = pool_lock(pool)))
	{
		set_errno(err);
		return;
	}

	locker = pool->locker;
	mem_release(pool->pool);
	mem_release(pool);

	if ((err = locker_unlock(locker)))
		set_errno(err);
}

/*

=item C<void *pool_destroy(Pool **pool)>

Destroys (deallocates and sets to C<null>) C<*pool>. Returns C<null>.
B<Note:> pools shared by multiple threads must not be destroyed until after
all threads have finished with it.

=cut

*/

void *pool_destroy(Pool **pool)
{
	if (pool && *pool)
	{
		pool_release(*pool);
		*pool = NULL;
	}

	return NULL;
}

/*

=item C<Pool *pool_create_secure(size_t size)>

Creates a memory pool of size C<size> just like I<pool_create(3)> except
that the memory pool is locked into RAM with I<mlock(2)> so that it can't be
paged to disk where some nefarious local user with root access might read
its contents. It is the caller's responsibility to deallocate the new pool
with I<pool_release_secure(3)> or I<pool_destroy_secure(3)> which will clear
the memory pool and unlock it before deallocating it. In all other ways, the
pool returned is exactly like a pool returned by I<pool_create(3)>. On
success, returns the pool. On error, returns C<null> with C<errno> set
appropriately. Note that on old systems, secure memory requires root
privileges.

=cut

*/

Pool *pool_create_secure(size_t size)
{
	return pool_create_secure_with_locker(NULL, size);
}

/*

=item C<Pool *pool_create_secure_with_locker(Locker *locker, size_t size)>

Equivalent to I<pool_create_secure(3)> except that multiple threads accessing
the new pool will be synchronised by C<locker>.

=cut

*/

Pool *pool_create_secure_with_locker(Locker *locker, size_t size)
{
#ifdef HAVE_MLOCK
	Pool *pool = mem_create(1, Pool);

	if (!pool)
		return NULL;

	if (!(pool->pool = mem_create_secure(size)))
	{
		mem_release(pool);
		return NULL;
	}

	pool->size = size;
	pool->used = 0;
	pool->locker = locker;

	return pool;
#else
	errno = ENOSYS;
	return NULL;
#endif
}

/*

=item C<void pool_release_secure(Pool *pool)>

Sets the contents of the memory pool to C<nul> bytes, then unlocks and
releases (deallocates) C<pool>. Only to be used on pools returned by
I<pool_create_secure(3)>. Only to be used in destructor functions. In other
cases, use I<pool_destroy_secure(3)> which also sets C<pool> to C<null>.

=cut

*/

void pool_release_secure(Pool *pool)
{
#ifdef HAVE_MLOCK
	Locker *locker;
	int err;

	if (!pool)
		return;

	if ((err = pool_lock(pool)))
	{
		set_errno(err);
		return;
	}

	locker = pool->locker;
	mem_release_secure(pool->pool);
	mem_release(pool);

	if ((err = locker_unlock(locker)))
		set_errno(err);
#endif
}

/*

=item C<void *pool_destroy_secure(Pool **pool)>

Sets the contents of the memory pool to C<nul> bytes, then unlocks and
destroys (deallocates and sets to C<null>) C<*pool>. Returns C<null>.
B<Note:> secure pools shared by multiple threads must not be destroyed until
after all threads have finished with it.

=cut

*/

void *pool_destroy_secure(Pool **pool)
{
	if (pool && *pool)
	{
		pool_release_secure(*pool);
		*pool = NULL;
	}

	return NULL;
}

/*

=item C<void pool_clear_secure(Pool *pool)>

Fills the secure C<pool> with C<nul> bytes and deallocates all of the chunks
of secure memory previously allocated from C<pool> so that it can be reused.
Does not use I<free(3)>.

=cut

*/

static void pool_clear_unlocked(Pool *pool);

void pool_clear_secure(Pool *pool)
{
	int err;

	if (!pool)
		return;

	if ((err = pool_lock(pool)))
	{
		set_errno(err);
		return;
	}

	pool_clear_unlocked(pool);
	memset(pool->pool, 0xff, pool->size);
	memset(pool->pool, 0xaa, pool->size);
	memset(pool->pool, 0x55, pool->size);
	memset(pool->pool, 0x00, pool->size);

	if ((err = pool_unlock(pool)))
		set_errno(err);
}

/*

=item C< #define pool_new(pool, type)>

Allocates enough memory from C<pool> to store an object of type C<type>. On
success, returns the address of the allocated memory. On error, returns
C<null> with C<errno> set appropriately.

=item C< #define pool_newsz(pool, size, type)>

Allocates enough memory from C<pool> to store C<size> objects of type
C<type>. On success, returns the address of the allocated memory. On error,
returns C<null> with C<errno> set appropriately.

=item C<void *pool_alloc(Pool *pool, size_t size)>

Allocates a chunk of memory of C<size> bytes from C<pool>. Does not use
I<malloc(3)>. The pointer returned must not be passed to I<free(3)> or
I<realloc(3)>. Only the entire pool can be deallocated with
I<pool_release(3)> or I<pool_destroy(3)>. All of the chunks can be
deallocated in one go with I<pool_clear(3)> without deallocating the pool
itself.

On success, returns the pointer to the allocated pool memory. On error,
returns C<null> with C<errno> set appropriately (i.e. C<EINVAL> if C<pool>
is C<null>, C<ENOSPC> if C<pool> does not have enough unused memory to
allocate C<size> bytes).

It is the caller's responsibility to ensure the correct alignment if
necessary by allocating the right numbers of bytes. The easiest way to do
ensure is to use separate pools for each specific data type that requires
specific alignment.

=cut

*/

void *pool_alloc(Pool *pool, size_t size)
{
	void *addr;
	int err;

	if (!pool)
		return set_errnull(EINVAL);

	if ((err = pool_lock(pool)))
		return set_errnull(err);

	if (pool->used + size > pool->size)
	{
		pool_unlock(pool);
		return set_errnull(ENOSPC);
	}

	addr = pool->pool + pool->used;
	pool->used += size;

	if ((err = pool_unlock(pool)))
		return set_errnull(err);

	return addr;
}

/*

=item C<void pool_clear(Pool *pool)>

Deallocates all of the chunks of memory previously allocated from C<pool> so
that it can be reused. Does not use I<free(3)>.

=cut

*/

static void pool_clear_with_locker(Pool *pool, int lock_pool)
{
	int err;

	if (!pool)
		return;

	if (lock_pool && (err = pool_lock(pool)))
	{
		set_errno(err);
		return;
	}

	pool->used = 0;

	if (lock_pool && (err = pool_unlock(pool)))
		set_errno(err);
}

static void pool_clear_unlocked(Pool *pool)
{
	pool_clear_with_locker(pool, 0);
}

void pool_clear(Pool *pool)
{
	pool_clear_with_locker(pool, 1);
}

/*

=back

=head1 ERRORS

On error, C<errno> is set by underlying functions or as follows:

=over 4

=item C<EINVAL>

When arguments are invalid.

=item C<ENOSPC>

When there is insufficient available space in a pool for I<pool_alloc(3)> to
satisfy a request.

=item C<ENOSYS>

Returned by I<mem_create_secure(3)> and I<pool_create_secure(3)> when
I<mlock(2)> is not supported (e.g. Mac OS X).

=back

=head1 MT-Level

MT-Safe (mem)

MT-Disciplined (pool) man I<locker(3)> for details.

=head1 EXAMPLES

1D array of longs:

    long *mem = mem_create(100, long);
    mem_resize(&mem, 200);
    mem_destroy(&mem);

3D array of ints:

    int ***space = mem_create3d(10, 20, 30, int);
    int i, j, k;

    for (i = 0; i < 10; ++i)
        for (j = 0; j < 20; ++j)
            for (k = 0; k < 30; ++k)
                space[i][j][k] = i + j + j;

    mem_destroy3d(&space);

A pool of a million integers:

    void pool()
    {
        Pool *pool;
        int i, *p;

        if (!(pool = pool_create(1024 * 1024 * sizeof(int))))
            return;

        for (i = 0; i < 1024 * 1024; ++i)
        {
            if (!(p = pool_new(pool, int)))
                break;

            *p = i;
        }

        pool_destroy(&pool);
    }

Secure memory:

    char *secure_passwd = mem_create_secure(32);

    if (!secure_passwd)
        exit(EXIT_FAILURE);

    get_passwd(secure_passwd, 32);
    use_passwd(secure_passwd);
    mem_destroy_secure(&secure_passwd);

Secure memory pool:

    Pool *secure_pool;
    char *secure_passwd;

    if (!(secure_pool = pool_create_secure(1024 * 1024)))
        exit(EXIT_FAILURE);

    secure_passwd = pool_alloc(secure_pool, 32);
    get_passwd(secure_passwd, 32);
    use_passwd(secure_passwd);
    pool_destroy_secure(&secure_pool);

=head1 SEE ALSO

I<libslack(3)>,
I<malloc(3)>,
I<realloc(3)>,
I<calloc(3)>,
I<free(3)>,
I<mlock(2)>,
I<munlock(2)>,
I<locker(3)>

=head1 AUTHOR

20201111 raf <raf@raf.org>

=cut

*/

#endif

#ifdef TEST

#include <time.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <slack/net.h>

int main(int ac, char **av)
{
	int *mem1 = NULL;
	char *mem2 = NULL;
	char *str = NULL;
	int **space2 = NULL;
	int ***space3 = NULL;
	int ****space4 = NULL;
	int *****space5 = NULL;
	char **space2d = NULL;
	double ***space3d = NULL;
	int i, j, k, l, m;
	Pool *pool;
	clock_t start_clock;
	clock_t end_clock;
	long malloc_time;
	long pool_time;
	int errors = 0;
	int no_secure_mem;
	pid_t pid;

	if (ac == 2 && !strcmp(av[1], "help"))
	{
		printf("usage: %s [pool]\n", *av);
		return EXIT_SUCCESS;
	}

	printf("Testing: %s\n", "mem");

	/* Test create, resize and destroy */

	if (!(mem1 = mem_create(100, int)))
		++errors, printf("Test1: mem_create(100, int) failed\n");
	if (!(mem_resize(&mem1, 50)))
		++errors, printf("Test2: mem_resize(100 -> 50) failed\n");
	if (mem_resize(&mem1, 0))
		++errors, printf("Test3: mem_resize(50 -> 0) failed\n");
	if (!(mem_resize(&mem1, 50)))
		++errors, printf("Test4: mem_resize(0 -> 50) failed\n");
	if (mem_destroy(&mem1))
		++errors, printf("Test5: mem_destroy() failed\n");
	if (!(mem2 = mem_create(0, char)))
		++errors, printf("Test6: mem_create(0, char) failed\n");
	if (!(mem_resize(&mem2, 10)))
		++errors, printf("Test7: mem_resize(0 -> 10) failed\n");
	if (mem_resize(&mem2, 0))
		++errors, printf("Test8: mem_resize(10 -> 0) failed\n");
	if (mem_destroy(&mem2))
		++errors, printf("Test9: mem_destroy() failed\n");

	/* This used to segfault a broken version of mem_resize() */

	switch (pid = fork())
	{
		case -1:
		{
			fprintf(stderr, "Failed to perform test - fork() failed (%s)\n", strerror(errno));
			break;
		}

		case 0:
		{
			size_t size = 16;
			sockopt_t *so = mem_create(size, sockopt_t);
			sockaddr_any_t *sa = mem_create(size, sockaddr_any_t);
			mem_resize(&so, size << 1);
			memset(so + size, 0, size * sizeof(sockopt_t));
			mem_resize(&sa, size << 1);
			memset(sa + size, 0, size * sizeof(sockaddr_any_t));
			mem_destroy(&so);
			mem_destroy(&sa);
			_exit(EXIT_SUCCESS);
			break;
		}

		default:
		{
			int status;

			if (waitpid(pid, &status, 0) == -1)
			{
				fprintf(stderr, "Failed to evaluate test - waitpid(%d) failed (%s)\n", (int)pid, strerror(errno));
				break;
			}

			if (WIFSIGNALED(status) && WTERMSIG(status) != SIGABRT)
				++errors, printf("Test10: mem_resize() failed - killed by signal %d\n", WTERMSIG(status));
			else if (WIFEXITED(status) && WEXITSTATUS(status) != EXIT_SUCCESS)
				++errors, printf("Test10: mem_resize() failed - exit status %d\n", WEXITSTATUS(status));

			break;
		}
	}

	/* Test strdup */

	if (!(str = mem_strdup("test")))
		++errors, printf("Test11: mem_strdup() failed (returned NULL)\n");
	else
	{
		if (strcmp(str, "test"))
			++errors, printf("Test12: mem_strdup() failed (equals \"%s\", not \"test\")\n", str);

		mem_release(str);
	}

	/* Test 2D space allocation and deallocation */

	if (!(space2 = mem_create_space(sizeof(int), (size_t)1, (size_t)1, (size_t)0)))
		++errors, printf("Test13: mem_create_space(1, 1) failed (returned NULL)\n");
	else
	{
		space2[0][0] = 37;
		if (space2[0][0] != 37)
			++errors, printf("Test14: mem_create_space(1, 1) failed (space2[%d][%d] = %d, not %d)\n", 0, 0, space2[0][0], 37);

		mem_destroy_space(&space2);
		if (space2)
			++errors, printf("Test15: mem_destroy_space(1, 1) failed\n");
	}

	if (!(space2 = mem_create_space(sizeof(int), (size_t)10, (size_t)10, (size_t)0)))
		++errors, printf("Test16: mem_create_space(10, 10) failed (returned NULL)\n");
	else
	{
		int error = 0;

		for (i = 0; i < 10; ++i)
			for (j = 0; j < 10; ++j)
				space2[i][j] = i + j;

		for (i = 0; i < 10; ++i)
			for (j = 0; j < 10; ++j)
				if (space2[i][j] != i + j)
					++error, printf("Test17: mem_create_space(10, 10) failed (space2[%d][%d] = %d, not %d)\n", i, j, space2[i][j], i + j);

		if (error)
			++errors;

		mem_destroy_space(&space2);
		if (space2)
			++errors, printf("Test18: mem_destroy_space(10, 10) failed\n");
	}

	/* Test 3D space allocation and deallocation */

	if (!(space3 = mem_create_space(sizeof(int), (size_t)1, (size_t)1, (size_t)1, (size_t)0)))
		++errors, printf("Test19: mem_create_space(1, 1, 1) failed (returned NULL)\n");
	else
	{
		space3[0][0][0] = 37;
		if (space3[0][0][0] != 37)
			++errors, printf("Test20: mem_create_space(1, 1, 1) failed (space3[%d][%d][%d] = %d, not %d)\n", 0, 0, 0, space3[0][0][0], 37);

		mem_destroy_space(&space3);
		if (space3)
			++errors, printf("Test21: mem_destroy_space(1, 1, 1) failed\n");
	}

	if (!(space3 = mem_create_space(sizeof(int), (size_t)10, (size_t)10, (size_t)10, (size_t)0)))
		++errors, printf("Test22: mem_create_space(10, 10, 10) failed (returned NULL)\n");
	else
	{
		int error = 0;

		for (i = 0; i < 10; ++i)
			for (j = 0; j < 10; ++j)
				for (k = 0; k < 10; ++k)
					space3[i][j][k] = i + j + k;

		for (i = 0; i < 10; ++i)
			for (j = 0; j < 10; ++j)
				for (k = 0; k < 10; ++k)
					if (space3[i][j][k] != i + j + k)
						++error, printf("Test23: mem_create_space(10, 10, 10) failed (space3[%d][%d][%d] = %d, not %d)\n", i, j, k, space3[i][j][k], i + j + k);

		if (error)
			++errors;

		mem_destroy_space(&space3);
		if (space3)
			++errors, printf("Test24: mem_destroy_space(10, 10, 10) failed\n");
	}

	/* Test 4D space allocation and deallocation */

	if (!(space4 = mem_create_space(sizeof(int), (size_t)1, (size_t)1, (size_t)1, (size_t)1, (size_t)0)))
		++errors, printf("Test25: mem_create_space(1, 1, 1, 1) failed (returned NULL)\n");
	else
	{
		space4[0][0][0][0] = 37;
		if (space4[0][0][0][0] != 37)
			++errors, printf("Test26: mem_create_space(1, 1, 1, 1) failed (space4[%d][%d][%d][%d] = %d, not %d)\n", 0, 0, 0, 0, space4[0][0][0][0], 37);

		mem_destroy_space(&space4);
		if (space4)
			++errors, printf("Test27: mem_destroy_space(1, 1, 1, 1) failed\n");
	}

	if (!(space4 = mem_create_space(sizeof(int), (size_t)10, (size_t)10, (size_t)10, (size_t)10, (size_t)0)))
		++errors, printf("Test28: mem_create_space(10, 10, 10, 10) failed (returned NULL)\n");
	else
	{
		int error = 0;

		for (i = 0; i < 10; ++i)
			for (j = 0; j < 10; ++j)
				for (k = 0; k < 10; ++k)
					for (l = 0; l < 10; ++l)
						space4[i][j][k][l] = i + j + k + l;

		for (i = 0; i < 10; ++i)
			for (j = 0; j < 10; ++j)
				for (k = 0; k < 10; ++k)
					for (l = 0; l < 10; ++l)
						if (space4[i][j][k][l] != i + j + k + l)
							++error, printf("Test29: mem_create_space(10, 10, 10, 10) failed (space4[%d][%d][%d][%d] = %d, not %d)\n", i, j, k, l, space4[i][j][k][l], i + j + k + l);

		if (error)
			++errors;

		mem_destroy_space(&space4);
		if (space4)
			++errors, printf("Test30: mem_destroy_space(10, 10, 10, 10) failed\n");
	}

	/* Test 5D space allocation and deallocation */

	if (!(space5 = mem_create_space(sizeof(int), (size_t)1, (size_t)1, (size_t)1, (size_t)1, (size_t)1, (size_t)0)))
		++errors, printf("Test31: mem_create_space(1, 1, 1, 1, 1) failed (returned NULL)\n");
	else
	{
		space5[0][0][0][0][0] = 37;
		if (space5[0][0][0][0][0] != 37)
			++errors, printf("Test32: mem_create_space(1, 1, 1, 1, 1) failed (space5[%d][%d][%d][%d][%d] = %d, not %d)\n", 0, 0, 0, 0, 0, space5[0][0][0][0][0], 37);

		mem_destroy_space(&space5);
		if (space5)
			++errors, printf("Test33: mem_destroy_space(1, 1, 1, 1, 1) failed\n");
	}

	if (!(space5 = mem_create_space(sizeof(int), (size_t)10, (size_t)10, (size_t)10, (size_t)10, (size_t)10, (size_t)0)))
		++errors, printf("Test34: mem_create_space(10, 10, 10, 10, 10) failed (returned NULL)\n");
	else
	{
		int error = 0;

		for (i = 0; i < 10; ++i)
			for (j = 0; j < 10; ++j)
				for (k = 0; k < 10; ++k)
					for (l = 0; l < 10; ++l)
						for (m = 0; m < 10; ++m)
							space5[i][j][k][l][m] = i + j + k + l + m;

		for (i = 0; i < 10; ++i)
			for (j = 0; j < 10; ++j)
				for (k = 0; k < 10; ++k)
					for (l = 0; l < 10; ++l)
						for (m = 0; m < 10; ++m)
							if (space5[i][j][k][l][m] != i + j + k + l + m)
								++error, printf("Test35: mem_create_space(10, 10, 10, 10, 10) failed (space5[%d][%d][%d][%d][%d] = %d, not %d)\n", i, j, k, l, m, space5[i][j][k][l][m], i + j + k + l + m);

		if (error)
			++errors;

		mem_destroy_space(&space5);
		if (space5)
			++errors, printf("Test36: mem_destroy_space(10, 10, 10, 10, 10) failed\n");
	}

	/* Test element sizes smaller than sizeof(void *) */

	if (!(space2d = mem_create_space(sizeof(char), (size_t)1, (size_t)1, (size_t)0)))
		++errors, printf("Test37: mem_create_space(char, 1, 1) failed (returned NULL)\n");
	else
	{
		space2d[0][0] = 'a';
		if (space2d[0][0] != 'a')
			++errors, printf("Test38: mem_create_space(char, 1, 1) failed (space2d[%d][%d] = '%c', not '%c')\n", 0, 0, space2d[0][0], 'a');

		mem_destroy_space(&space2d);
		if (space2d)
			++errors, printf("Test39: mem_destroy_space(char, 1, 1) failed\n");
	}

	if (!(space2d = mem_create_space(sizeof(char), (size_t)10, (size_t)10, (size_t)0)))
		++errors, printf("Test40: mem_create_space(char, 10, 10) failed (returned NULL)\n");
	else
	{
		int error = 0;

		for (i = 0; i < 10; ++i)
			for (j = 0; j < 10; ++j)
				space2d[i][j] = 'a' + (i + j) % 26;

		for (i = 0; i < 10; ++i)
			for (j = 0; j < 10; ++j)
				if (space2d[i][j] != 'a' + (i + j) % 26)
					++error, printf("Test41: mem_create_space(char, 10, 10) failed (space2d[%d][%d] = '%c', not '%c')\n", i, j, space2d[i][j], 'a' + (i + j) % 26);

		if (error)
			++errors;

		mem_destroy_space(&space2d);
		if (space2d)
			++errors, printf("Test42: mem_destroy_space(char, 10, 10) failed\n");
	}

	/* Test element sizes larger than sizeof(void *) */

	if (!(space3d = mem_create_space(sizeof(double), (size_t)1, (size_t)1, (size_t)1, (size_t)0)))
		++errors, printf("Test43: mem_create_space(double, 1, 1, 1) failed (returned NULL)\n");
	else
	{
		space3d[0][0][0] = 37.5;
		if (space3d[0][0][0] != 37.5)
			++errors, printf("Test44: mem_create_space(double, 1, 1, 1) failed (space3d[%d][%d][%d] = %g, not %g)\n", 0, 0, 0, space3d[0][0][0], 37.5);

		mem_destroy_space(&space3d);
		if (space3d)
			++errors, printf("Test45: mem_destroy_space(double, 1, 1, 1) failed\n");
	}

	if (!(space3d = mem_create_space(sizeof(double), (size_t)10, (size_t)10, (size_t)10, (size_t)0)))
		++errors, printf("Test46: mem_create_space(double, 10, 10, 10) failed (returned NULL)\n");
	else
	{
		int error = 0;

		for (i = 0; i < 10; ++i)
			for (j = 0; j < 10; ++j)
				for (k = 0; k < 10; ++k)
					space3d[i][j][k] = (double)(i + j + k);

		for (i = 0; i < 10; ++i)
			for (j = 0; j < 10; ++j)
				for (k = 0; k < 10; ++k)
					if (space3d[i][j][k] != (double)(i + j + k))
						++error, printf("Test47: mem_create_space(double, 10, 10, 10) failed (space3[%d][%d][%d] = %g, not %g)\n", i, j, k, space3d[i][j][k], (double)(i + j + k));

		if (error)
			++errors;

		mem_destroy_space(&space3d);
		if (space3d)
			++errors, printf("Test48: mem_destroy_space(double, 10, 10, 10) failed\n");
	}

	/* Test mem_space_start() */

	if (!(space4 = mem_create_space(sizeof(int), (size_t)2, (size_t)3, (size_t)4, (size_t)5, (size_t)0)))
		++errors, printf("Test49: mem_create_space(int, 2, 3, 4, 5) failed (returned NULL)\n");
	else
	{
		int error = 0;
		size_t start;

		for (i = 0; i < 2; ++i)
			for (j = 0; j < 3; ++j)
				for (k = 0; k < 4; ++k)
					for (l = 0; l < 5; ++l)
						space4[i][j][k][l] = i + j + k + l;

		for (i = 0; i < 2; ++i)
			for (j = 0; j < 3; ++j)
				for (k = 0; k < 4; ++k)
					for (l = 0; l < 5; ++l)
						if (space4[i][j][k][l] != i + j + k + l)
							++error, printf("Test50: mem_create_space(int, 2, 3, 4, 5) failed (space4[%d][%d][%d][%d] = %d, not %d)\n", i, j, k, l, space4[i][j][k][l], i + j + k + l);

		if (error)
			++errors;

		start = mem_space_start(sizeof(int), (size_t)2, (size_t)3, (size_t)4, (size_t)5, (size_t)0);
		memset((char *)space4 + start, 0, sizeof(int) * 2 * 3 * 4 * 5);

		for (i = 0; i < 2; ++i)
			for (j = 0; j < 3; ++j)
				for (k = 0; k < 4; ++k)
					for (l = 0; l < 4; ++l)
						if (space4[i][j][k][l] != 0)
							++error, printf("Test51: mem_space_start(int, 2, 3, 4, 5) failed (space4[%d][%d][%d][%d] = %d, not %d)\n", i, j, k, l, space4[i][j][k][l], 0);

		for (i = 0; i < 2; ++i)
			for (j = 0; j < 3; ++j)
				for (k = 0; k < 4; ++k)
					for (l = 0; l < 5; ++l)
						space4[i][j][k][l] = i + j + k + l;

		for (i = 0; i < 2; ++i)
			for (j = 0; j < 3; ++j)
				for (k = 0; k < 4; ++k)
					for (l = 0; l < 5; ++l)
						if (space4[i][j][k][l] != i + j + k + l)
							++error, printf("Test52: mem_space_start(int, 2, 3, 4, 5) failed (space4[%d][%d][%d][%d] = %d, not %d)\n", i, j, k, l, space4[i][j][k][l], i + j + k + l);

		if (error)
			++errors;

		mem_destroy_space(&space4);
		if (space4)
			++errors, printf("Test53: mem_destroy_space(int, 2, 3, 4, 5) failed\n");
	}

	/* Test pool functions */

	start_clock = clock();
	if (!(pool = pool_create(1024 * 1024)))
		++errors, printf("Test54: pool_create(1024 * 1024) failed: %s\n", strerror(errno));
	else
	{
		for (i = 0; i < 1024 * 1024; ++i)
		{
			if (!pool_new(pool, char))
			{
				++errors, printf("Test55: pool_alloc() failed: %s\n", strerror(errno));
				break;
			}
		}

		errno = 0;
		if (pool_alloc(pool, 1) != NULL || errno != ENOSPC)
			++errors, printf("Test56: pool_alloc(pool, 1) failed (errno %d, not %d)\n", errno, ENOSPC);

		errno = 0;
		if (pool_alloc(NULL, 1) != NULL || errno != EINVAL)
			++errors, printf("Test57: pool_alloc(NULL, 1) failed (errno %d, not %d)\n", errno, EINVAL);

		pool_clear(pool);
		if (!pool_alloc(pool, 1))
			++errors, printf("Test58: pool_clear(), pool_alloc() failed: %s\n", strerror(errno));

		pool_destroy(&pool);
		if (pool)
			++errors, printf("Test59: pool_destroy() failed (%p, not NULL)\n", (void *)pool);
	}

	end_clock = clock();
	pool_time = end_clock - start_clock;

	if (ac == 2 && !strcmp(av[1], "pool"))
	{
		start_clock = clock();
		for (i = 0; i < 1024 * 1024; ++i)
			free(malloc(1));
		end_clock = clock();
		malloc_time = end_clock - start_clock;
		printf("malloc %ldus, pool %ldus (pool %g times faster than malloc)\n", malloc_time, pool_time, (double)malloc_time / (double)pool_time);
	}

	/* Test secure mem/pool functions */

	no_secure_mem = 0; /* (getuid() != 0); */

	if (!no_secure_mem)
	{
#ifdef MLOCK_REQUIRES_PAGE_BOUNDARY
		/* Test that page boundary is a power of two */

		long pagesize;

		if ((pagesize = sysconf(_SC_PAGESIZE)) == -1)
			++errors, printf("Test60: Failed to perform test: sysconf(_SC_PAGESIZE) failed\n");
		else
		{
			long size = pagesize;
			int bits;

			for (bits = 0; size; size >>= 1)
				if (size & 1)
					++bits;

			if (bits != 1)
				++errors, printf("Test60: pagesize (%ld) is not a power of 2! Secure memory won't work\n", pagesize);
		}
#endif

		if (!(mem1 = mem_create_secure(1024)))
			++errors, printf("Test61: mem_create_secure(1024) failed: %s (Does mlock() require root privileges on this system? If so, audit this code and rerun it as root)\n", strerror(errno));
		else
		{
			mem_destroy_secure(&mem1);
			if (mem1)
				++errors, printf("Test62: mem_destroy_secure(1024) failed: mem == %p, not NULL\n", (void *)mem1);
		}

		if (!(pool = pool_create_secure(32)))
			++errors, printf("Test63: pool_create_secure(32) failed: %s (Does mlock() require root privileges on this system? If so, audit this code and rerun it as root)\n", strerror(errno));
		else
		{
			void *whitebox = pool->pool;

			if (!(mem1 = pool_alloc(pool, 32)))
				++errors, printf("Test64: pool_alloc(pool, 32) failed: %s\n", strerror(errno));

			memset(mem1, 0xff, 32);
			pool_destroy_secure(&pool);
			/* Note: This test may be invalid because the memory has already been deallocated */
			if (memcmp(whitebox, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", 32))
			{
				if (!memcmp(whitebox, "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff", 32))
					++errors, printf("Test65: pool_destroy_secure(32) failed: memory not cleared\n");
				else
					++errors, printf("Test65: pool_destroy_secure(32) failed: memory not cleared (possibly - or maybe the already-deallocated memory has just been reused by now)\n");
			}
			if (pool)
				++errors, printf("Test66: pool_destroy_secure(32) failed: pool == %p, not NULL\n", (void *)pool);
		}
	}

	/* Test assumption: memory failure results in errno == ENOMEM */

	errno = 0;
	str = NULL;
	if (!mem_resize(&str, UINT_MAX) && errno != ENOMEM)
		++errors, printf("Test67: assumption failed: realloc failed but errno == \"%s\" (not \"%s\")\n", strerror(errno), strerror(ENOMEM));

	if (errors)
		printf("%d/67 tests failed\n", errors);
	else
		printf("All tests passed\n");

	if (no_secure_mem)
	{
		printf("\n");
		printf("    Note: Can't perform secure memory tests.\n");
		printf("    Audit the code and rerun the test as root.\n");
	}

	return (errors == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

#endif

/* vi:set ts=4 sw=4: */
