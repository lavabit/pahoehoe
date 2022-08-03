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

/*

=head1 NAME

I<libslack(map)> - map module

=head1 SYNOPSIS

    #include <slack/std.h>
    #include <slack/map.h>

    typedef struct Map Map;
    typedef struct Mapper Mapper;
    typedef struct Mapping Mapping;
    typedef list_release_t map_release_t;
    typedef list_copy_t map_copy_t;
    typedef list_cmp_t map_cmp_t;
    typedef size_t map_hash_t(size_t table_size, const void *key);
    typedef void map_action_t(void *key, void *item, void *data);

    Map *map_create(map_release_t *destroy);
    Map *map_create_sized(size_t size, map_release_t *destroy);
    Map *map_create_with_hash(map_hash_t *hash, map_release_t *destroy);
    Map *map_create_sized_with_hash(size_t size, map_hash_t *hash, map_release_t *destroy);
    Map *map_create_with_locker(Locker *locker, map_release_t *destroy);
    Map *map_create_with_locker_sized(Locker *locker, size_t size, map_release_t *destroy);
    Map *map_create_with_locker_with_hash(Locker *locker, map_hash_t *hash, map_release_t *destroy);
    Map *map_create_with_locker_sized_with_hash(Locker *locker, size_t size, map_hash_t *hash, map_release_t *destroy);
    Map *map_create_generic(map_copy_t *copy, map_cmp_t *cmp, map_hash_t *hash, map_release_t *key_destroy, map_release_t *value_destroy);
    Map *map_create_generic_sized(size_t size, map_copy_t *copy, map_cmp_t *cmp, map_hash_t *hash, map_release_t *key_destroy, map_release_t *value_destroy);
    Map *map_create_generic_with_locker(Locker *locker, map_copy_t *copy, map_cmp_t *cmp, map_hash_t *hash, map_release_t *key_destroy, map_release_t *value_destroy);
    Map *map_create_generic_with_locker_sized(Locker *locker, size_t size, map_copy_t *copy, map_cmp_t *cmp, map_hash_t *hash, map_release_t *key_destroy, map_release_t *value_destroy);
    int map_rdlock(const Map *map);
    int map_wrlock(const Map *map);
    int map_unlock(const Map *map);
    void map_release(Map *map);
    void *map_destroy(Map **map);
    int map_own(Map *map, map_release_t *destroy);
    int map_own_unlocked(Map *map, map_release_t *destroy);
    map_release_t *map_disown(Map *map);
    map_release_t *map_disown_unlocked(Map *map);
    int map_add(Map *map, const void *key, void *value);
    int map_add_unlocked(Map *map, const void *key, void *value);
    int map_put(Map *map, const void *key, void *value);
    int map_put_unlocked(Map *map, const void *key, void *value);
    int map_insert(Map *map, const void *key, void *value, int replace);
    int map_insert_unlocked(Map *map, const void *key, void *value, int replace);
    int map_remove(Map *map, const void *key);
    int map_remove_unlocked(Map *map, const void *key);
    void *map_get(Map *map, const void *key);
    void *map_get_unlocked(const Map *map, const void *key);
    Mapper *mapper_create(Map *map);
    Mapper *mapper_create_rdlocked(Map *map);
    Mapper *mapper_create_wrlocked(Map *map);
    Mapper *mapper_create_unlocked(Map *map);
    void mapper_release(Mapper *mapper);
    void mapper_release_unlocked(Mapper *mapper);
    void *mapper_destroy(Mapper **mapper);
    void *mapper_destroy_unlocked(Mapper **mapper);
    int mapper_has_next(Mapper *mapper);
    void *mapper_next(Mapper *mapper);
    const Mapping *mapper_next_mapping(Mapper *mapper);
    void mapper_remove(Mapper *mapper);
    int map_has_next(Map *map);
    void map_break(Map *map);
    void *map_next(Map *map);
    const Mapping *map_next_mapping(Map *map);
    void map_remove_current(Map *map);
    const void *mapping_key(const Mapping *mapping);
    const void *mapping_value(const Mapping *mapping);
    List *map_keys(Map *map);
    List *map_keys_unlocked(Map *map);
    List *map_keys_with_locker(Locker *locker, Map *map);
    List *map_keys_with_locker_unlocked(Locker *locker, Map *map);
    List *map_values(Map *map);
    List *map_values_unlocked(Map *map);
    List *map_values_with_locker(Locker *locker, Map *map);
    List *map_values_with_locker_unlocked(Locker *locker, Map *map);
    void map_apply(Map *map, map_action_t *action, void *data);
    void map_apply_rdlocked(Map *map, map_action_t *action, void *data);
    void map_apply_wrlocked(Map *map, map_action_t *action, void *data);
    void map_apply_unlocked(Map *map, map_action_t *action, void *data);
    ssize_t map_size(Map *map);
    ssize_t map_size_unlocked(const Map *map);

=head1 DESCRIPTION

This module provides functions for manipulating and iterating over a set of
mappings from one object to another object, also known as hashes or
associative arrays. I<Map>s may own their items. I<Map>s created with a
non-C<null> destroy function use that function to destroy an item when it is
removed from the map and to destroy each item when the map itself it
destroyed. I<Map>s are hash tables with C<11> buckets by default. They grow
when necessary, approximately doubling in size each time up to a maximum
size of C<26,214,401> buckets.

=over 4

=cut

*/

#ifndef _BSD_SOURCE
#define _BSD_SOURCE /* For snprintf() on OpenBSD-4.7 */
#endif

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE /* New name for _BSD_SOURCE */
#endif

#include "config.h"
#include "std.h"

#include "map.h"
#include "mem.h"
#include "err.h"
#include "locker.h"

struct Map
{
	size_t size;                  /* number of buckets */
	size_t items;                 /* number of items */
	List **chain;                 /* array of hash buckets */
	map_hash_t *hash;             /* hash function */
	map_copy_t *copy;             /* key copy function */
	map_cmp_t *cmp;               /* key comparison function */
	map_release_t *key_destroy;   /* destructor function for keys */
	map_release_t *value_destroy; /* destructor function for items */
	Mapper *mapper;               /* built-in iterator */
	Locker *locker;               /* locking strategy for this object */
};

struct Mapping
{
	void *key;                    /* a map key */
	void *value;                  /* a map value */
	map_release_t *key_destroy;   /* destructor function for key */
	map_release_t *value_destroy; /* destructor function for value */
};

struct Mapper
{
	Map *map;                 /* the map being iterated over */
	ssize_t chain_index;      /* the index of the chain of the current item */
	ssize_t item_index;       /* the index of the current item */
	ssize_t next_chain_index; /* the index of the chain of the next item */
	ssize_t next_item_index;  /* the index of the next item */
};

#ifndef TEST

/* Increasing sequence of valid (i.e. prime) table sizes to choose from. */

static const size_t table_sizes[] =
{
	11, 23, 47, 101, 199, 401, 797, 1601, 3203, 6397, 12799, 25601,
	51199, 102397, 204803, 409597, 819187, 1638431, 3276799, 6553621,
	13107197, 26214401
};

static const size_t num_table_sizes = sizeof(table_sizes) / sizeof(table_sizes[0]);

/* Average bucket length threshold that must be reached before a map grows */

static const double table_resize_factor = 2.0;

#if 0
/*

C<size_t hash(size_t size, const void *key)>

JPW hash function. Returns a hash value (in the range 0..size-1) for C<key>.

*/

static size_t hash(size_t size, const void *key)
{
	unsigned char *k = key;
	size_t g, h = 0;

	while (*k)
		if ((g = (h <<= 4, h += *k++) & 0xf0000000))
			h ^= (g >> 24) ^ g;

	return h % size;
}
#endif

/*

C<size_t hash(size_t size, const void *key)>

Hash function from The Practice of Programming by Kernighan and Pike (p57).
Returns a hash value (in the range 0..size-1) for C<key>.

*/

static size_t hash(size_t size, const void *key)
{
	const unsigned char *k = key;
	size_t h = 0;

	while (*k)
		h *= 31, h += *k++;

	return h % size;
}

/*

C<Mapping *mapping_create(const void *key, void *value, map_release_t *destroy)>

Creates a new mapping from C<key> to C<value>. C<destroy> is the destructor
function for C<value>. On success, returns the new mapping. On error,
returns C<null> with C<errno> set appropriately.

*/

static Mapping *mapping_create(void *key, void *value, map_release_t *key_destroy, map_release_t *value_destroy)
{
	Mapping *mapping;

	if (!(mapping = mem_new(Mapping))) /* XXX decouple */
		return NULL;

	mapping->key = key;
	mapping->value = value;
	mapping->key_destroy = key_destroy;
	mapping->value_destroy = value_destroy;

	return mapping;
}

/*

C<void mapping_release(Mapping *mapping)>

Releases (deallocates) C<mapping>, destroying its value if necessary.

*/

static void mapping_release(Mapping *mapping)
{
	if (!mapping)
		return;

	if (mapping->key_destroy)
		mapping->key_destroy(mapping->key);

	if (mapping->value_destroy)
		mapping->value_destroy(mapping->value);

	mem_release(mapping);
}

/*

=item C<Map *map_create(map_release_t *destroy)>

Creates a small I<Map> with string keys and C<destroy> as its item
destructor. It is the caller's responsibility to deallocate the new map with
I<map_release(3)> or I<map_destroy(3)>. It is strongly recommended to use
I<map_destroy(3)>, because it also sets the pointer variable to C<null>. On
success, returns the new map. On error, returns C<null> with C<errno> set
appropriately.

=cut

*/

Map *map_create(map_release_t *destroy)
{
	return map_create_sized_with_hash(table_sizes[0], (map_hash_t *)hash, destroy);
}

/*

=item C<Map *map_create_sized(size_t size, map_release_t *destroy)>

Equivalent to I<map_create(3)> except that the initial number of buckets is
approximately C<size>. The actual size will be the first prime greater than
or equal to C<size> in a prebuilt sequence of primes between C<11> and
C<26,214,401> that double at each step.

=cut

*/

Map *map_create_sized(size_t size, map_release_t *destroy)
{
	return map_create_sized_with_hash(size, (map_hash_t *)hash, destroy);
}

/*

=item C<Map *map_create_with_hash(map_hash_t *hash, map_release_t *destroy)>

Equivalent to I<map_create(3)> except that C<hash> is used as the hash
function. The arguments to C<hash> are a I<size_t> specifying the number of
buckets, and a I<const void *> specifying the key to hash. It must return a
I<size_t> between zero and the table size - 1.

=cut

*/

Map *map_create_with_hash(map_hash_t *hash, map_release_t *destroy)
{
	return map_create_sized_with_hash(table_sizes[0], hash, destroy);
}

/*

=item C<Map *map_create_sized_with_hash(size_t size, map_hash_t *hash, map_release_t *destroy)>

Equivalent to I<map_create_sized(3)> except that C<hash> is used as the hash
function. The arguments to C<hash> are a I<size_t> specifying the number of
buckets, and a I<const void *> specifying the key to hash. It must return a
I<size_t> between zero and the table size - 1.

=cut

*/

Map *map_create_sized_with_hash(size_t size, map_hash_t *hash, map_release_t *destroy)
{
	return map_create_generic_sized(size, (map_copy_t *)mem_strdup, (map_cmp_t *)strcmp, hash, (map_release_t *)free, destroy);
}

/*

=item C<Map *map_create_with_locker(Locker *locker, map_release_t *destroy)>

Equivalent to I<map_create(3)> except that multiple threads accessing the
new map will be synchronised by C<locker>.

=cut

*/

Map *map_create_with_locker(Locker *locker, map_release_t *destroy)
{
	return map_create_with_locker_sized_with_hash(locker, table_sizes[0], (map_hash_t *)hash, destroy);
}

/*

=item C<Map *map_create_with_locker_sized(Locker *locker, size_t size, map_release_t *destroy)>

Equivalent to I<map_create_sized(3)> except that multiple threads accessing
the new map will be synchronised by C<locker>.

=cut

*/

Map *map_create_with_locker_sized(Locker *locker, size_t size, map_release_t *destroy)
{
	return map_create_with_locker_sized_with_hash(locker, size, (map_hash_t *)hash, destroy);
}

/*

=item C<Map *map_create_with_locker_with_hash(Locker *locker, map_hash_t *hash, map_release_t *destroy)>

Equivalent to I<map_create_with_hash(3)> except that multiple threads
accessing the new map will be synchronised by C<locker>.

=cut

*/

Map *map_create_with_locker_with_hash(Locker *locker, map_hash_t *hash, map_release_t *destroy)
{
	return map_create_with_locker_sized_with_hash(locker, table_sizes[0], hash, destroy);
}

/*

=item C<Map *map_create_with_locker_sized_with_hash(Locker *locker, size_t size, map_hash_t *hash, map_release_t *destroy)>

Equivalent to I<map_create_sized_with_hash(3)> except that multiple threads
accessing the new map will be synchronised by C<locker>.

=cut

*/

Map *map_create_with_locker_sized_with_hash(Locker *locker, size_t size, map_hash_t *hash, map_release_t *destroy)
{
	return map_create_generic_with_locker_sized(locker, size, (map_copy_t *)mem_strdup, (map_cmp_t *)strcmp, hash, (map_release_t *)free, destroy);
}

/*

=item C<Map *map_create_generic(map_copy_t *copy, map_cmp_t *cmp, map_hash_t *hash, map_release_t *key_destroy, map_release_t *value_destroy)>

Equivalent to I<map_create(3)> except that the mapping keys can be of any
type. C<copy> is used to copy mapping keys. The argument to C<copy> is the
key to be copied. It must return a copy of its argument. C<cmp> is used to
compare mapping keys. The arguments to C<cmp> are two keys to be compared.
It must return < 0 if the first compares less than the second, 0 if they
compare equal and > 0 if the first compares greater than the second. C<hash>
is the hash function. The arguments to C<hash> are a I<size_t> specifying
the number of buckets, and a I<const void *> specifying the key to hash. It
must return a I<size_t> between zero and the table size - 1. C<key_destroy>
is the destructor for mapping keys. C<value_destroy> is the destructor for
mapping values. On success, returns the new map. On error, returns C<null>
with C<errno> set appropriately.

=cut

*/

Map *map_create_generic(map_copy_t *copy, map_cmp_t *cmp, map_hash_t *hash, map_release_t *key_destroy, map_release_t *value_destroy)
{
	return map_create_generic_with_locker_sized(NULL, table_sizes[0], copy, cmp, hash, key_destroy, value_destroy);
}

/*

=item C<Map *map_create_generic_sized(size_t size, map_copy_t *copy, map_cmp_t *cmp, map_hash_t *hash, map_release_t *key_destroy, map_release_t *value_destroy)>

Equivalent to I<map_create_generic(3)> except that the initial number of
buckets is approximately C<size>. The actual size will be the first prime
greater than or equal to C<size> in a prebuilt sequence of primes between C<11>
and C<26,214,401> that double at each step.

=cut

*/

Map *map_create_generic_sized(size_t size, map_copy_t *copy, map_cmp_t *cmp, map_hash_t *hash, map_release_t *key_destroy, map_release_t *value_destroy)
{
	return map_create_generic_with_locker_sized(NULL, size, copy, cmp, hash, key_destroy, value_destroy);
}

/*

=item C<Map *map_create_generic_with_locker(Locker *locker, map_copy_t *copy, map_cmp_t *cmp, map_hash_t *hash, map_release_t *key_destroy, map_release_t *value_destroy)>

Equivalent to I<map_create_generic(3)> except that multiple threads
accessing the new map will be synchronised by C<locker>.

=cut

*/

Map *map_create_generic_with_locker(Locker *locker, map_copy_t *copy, map_cmp_t *cmp, map_hash_t *hash, map_release_t *key_destroy, map_release_t *value_destroy)
{
	return map_create_generic_with_locker_sized(locker, table_sizes[0], copy, cmp, hash, key_destroy, value_destroy);
}

/*

=item C<Map *map_create_generic_with_locker_sized(Locker *locker, size_t size, map_copy_t *copy, map_cmp_t *cmp, map_hash_t *hash, map_release_t *key_destroy, map_release_t *value_destroy)>

Equivalent to I<map_create_generic_sized(3)> except that multiple threads
accessing the new map will be synchronised by C<locker>.

=cut

*/

Map *map_create_generic_with_locker_sized(Locker *locker, size_t size, map_copy_t *copy, map_cmp_t *cmp, map_hash_t *hash, map_release_t *key_destroy, map_release_t *value_destroy)
{
	Map *map;
	size_t i;

	for (i = 0; i < num_table_sizes; ++i)
	{
		if (table_sizes[i] >= size)
		{
			size = table_sizes[i];
			break;
		}
	}

	if (i == num_table_sizes)
		return set_errnull(EINVAL);

	if (!(map = mem_new(Map))) /* XXX decouple */
		return NULL;

	if (!(map->chain = mem_create(size, List *)))
	{
		mem_release(map);
		return NULL;
	}

	map->size = size;
	map->items = 0;
	memset(map->chain, 0, map->size * sizeof(List *));
	map->hash = hash;
	map->copy = copy;
	map->cmp = cmp;
	map->key_destroy = key_destroy;
	map->value_destroy = value_destroy;
	map->mapper = NULL;
	map->locker = locker;

	return map;
}

/*

=item C<int map_rdlock(const Map *map)>

Claims a read lock on C<map> (if C<map> was created with a I<Locker>). This
is needed when multiple read-only I<map(3)> module functions need to be
called atomically. It is the client's responsibility to call
I<map_unlock(3)> after the atomic operation. The only functions that may be
called on C<map> between calls to I<map_rdlock(3)> and I<map_unlock(3)> are
any read-only I<map(3)> module functions whose name ends with C<_unlocked>.
On success, returns C<0>. On error, returns an error code.

=cut

*/

#define map_rdlock(map) ((map) ? locker_rdlock((map)->locker) : EINVAL)
#define map_wrlock(map) ((map) ? locker_wrlock((map)->locker) : EINVAL)
#define map_unlock(map) ((map) ? locker_unlock((map)->locker) : EINVAL)

int (map_rdlock)(const Map *map)
{
	return map_rdlock(map);
}

/*

=item C<int map_wrlock(const Map *map)>

Claims a write lock on C<map> (if C<map> was created with a I<Locker>). This
is needed when multiple read/write I<map(3)> module functions need to be
called atomically. It is the client's responsibility to subsequently call
I<map_unlock(3)>. The only functions that may be called on C<map> between
calls to I<map_wrlock(3)> and I<map_unlock(3)> are any I<map(3)> module
functions whose name ends with C<_unlocked>. On success, returns C<0>. On
error, returns an error code.

=cut

*/

int (map_wrlock)(const Map *map)
{
	return map_wrlock(map);
}

/*

=item C<int map_unlock(const Map *map)>

Unlocks a read or write lock on C<map> obtained with I<map_rdlock(3)> or
I<map_wrlock(3)> (if C<map> was created with a C<locker>). On success,
returns C<0>. On error, returns an error code.

=cut

*/

int (map_unlock)(const Map *map)
{
	return map_unlock(map);
}

/*

=item C<void map_release(Map *map)>

Releases (deallocates) C<map>, destroying its items if necessary. On error,
sets C<errno> appropriately.

=cut

*/

void map_release(Map *map)
{
	size_t i;

	if (!map)
		return;

	for (i = 0; i < map->size; ++i)
		list_release(map->chain[i]);

	mem_release(map->chain);
	mem_release(map);
}

/*

=item C<void *map_destroy(Map **map)>

Destroys (deallocates and sets to C<null>) C<*map>. Returns C<null>.
B<Note:> maps shared by multiple threads must not be destroyed until after
all threads have finished with it.

=cut

*/

void *map_destroy(Map **map)
{
	if (map && *map)
	{
		map_release(*map);
		*map = NULL;
	}

	return NULL;
}

/*

=item C<int map_own(Map *map, map_release_t *destroy)>

Causes C<map> to take ownership of its items. The items will be destroyed
using C<destroy> when their mappings are removed from C<map> or when C<map>
is destroyed. On success, returns C<0>. On error, returns C<-1> with
C<errno> set appropriately.

=cut

*/

int map_own(Map *map, map_release_t *destroy)
{
	int ret;
	int err;

	if (!map || !destroy)
		return set_errno(EINVAL);

	if ((err = map_wrlock(map)))
		return set_errno(err);

	ret = map_own_unlocked(map, destroy);

	if ((err = map_unlock(map)))
		return set_errno(err);

	return ret;
}

/*

=item C<int map_own_unlocked(Map *map, map_release_t *destroy)>

Equivalent to I<map_own(3)> except that C<map> is not write-locked.

=cut

*/

int map_own_unlocked(Map *map, map_release_t *destroy)
{
	ssize_t length;
	size_t c, i;

	if (!map || !destroy)
		return set_errno(EINVAL);

	if (destroy == map->value_destroy)
		return 0;

	map->value_destroy = destroy;

	for (c = 0; c < map->size; ++c)
	{
		List *chain = map->chain[c];

		if (!chain)
			continue;

		if ((length = list_length_unlocked(chain)) == -1)
			return -1;

		for (i = 0; i < length; ++i)
		{
			Mapping *mapping = (Mapping *)list_item_unlocked(chain, i);
			mapping->value_destroy = destroy;
		}
	}

	return 0;
}

/*

=item C<map_release_t *map_disown(Map *map)>

Causes C<map> to relinquish ownership of its items. The items will not be
destroyed when their mappings are removed from C<map> or when C<map> is
destroyed. On success, returns the previous destroy function, if any. On
error, returns C<null> with C<errno> set appropriately.

=cut

*/

map_release_t *map_disown(Map *map)
{
	map_release_t *ret;
	int err;

	if (!map)
		return (map_release_t *)set_errnullf(EINVAL);

	if ((err = map_wrlock(map)))
		return (map_release_t *)set_errnullf(err);

	ret = map_disown_unlocked(map);

	if ((err = map_unlock(map)))
		return (map_release_t *)set_errnullf(err);

	return ret;
}

/*

=item C<map_release_t *map_disown_unlocked(Map *map)>

Equivalent to I<map_disown(3)> except that C<map> is not write-locked.

=cut

*/

map_release_t *map_disown_unlocked(Map *map)
{
	ssize_t length;
	size_t c, i;
	map_release_t *destroy;

	if (!map)
		return (map_release_t *)set_errnullf(EINVAL);

	if (!map->value_destroy)
		return NULL;

	destroy = map->value_destroy;
	map->value_destroy = NULL;

	for (c = 0; c < map->size; ++c)
	{
		List *chain = map->chain[c];

		if (!chain)
			continue;

		if ((length = list_length_unlocked(chain)) == -1)
			return NULL;

		for (i = 0; i < length; ++i)
		{
			Mapping *mapping = (Mapping *)list_item_unlocked(chain, i);
			mapping->value_destroy = NULL;
		}
	}

	return destroy;
}

/*

C<static int map_resize(Map *map)>

Resizes C<map> to use the next prime in a prebuilt sequence of primes
between C<11> and C<26,214,401> that is greater than the current size. On
success, returns C<0>. On error, returns C<-1> with C<errno> set
appropriately.

*/

static int map_resize(Map *map)
{
	size_t size = 0;
	size_t i;
	Mapper *mapper;
	Map *new_map;

	if (!map)
		return set_errno(EINVAL);

	for (i = 1; i < num_table_sizes; ++i)
	{
		if (table_sizes[i] > map->size)
		{
			size = table_sizes[i];
			break;
		}
	}

	if (i == num_table_sizes || size == 0)
		return set_errno(EINVAL);

	if (!(new_map = map_create_generic_sized(size, map->copy, map->cmp, map->hash, map->key_destroy, map->value_destroy)))
		return -1;

	if (!(mapper = mapper_create_unlocked(map)))
	{
		map_release(new_map);
		return -1;
	}

	while (mapper_has_next(mapper) == 1)
	{
		const Mapping *mapping = mapper_next_mapping(mapper);

		if (map_add_unlocked(new_map, mapping->key, mapping->value) == -1)
		{
			mapper_release_unlocked(mapper);
			map_release(new_map);
			return -1;
		}
	}

	mapper_release_unlocked(mapper);

	errno = 0;
	if (map_disown_unlocked(map) == NULL && errno)
	{
		map_release(new_map);
		return -1;
	}

	for (i = 0; i < map->size; ++i)
		list_release(map->chain[i]);
	mem_release(map->chain);

	map->size = new_map->size;
	map->items = new_map->items;
	map->chain = new_map->chain;
	map->value_destroy = new_map->value_destroy;
	mem_release(new_map);

	return 0;
}

/*

=item C<int map_add(Map *map, const void *key, void *value)>

Adds the C<(key, value)> mapping to C<map>, if C<key> is not already
present. Note that C<key> is copied but C<value> is not. On success, returns
C<0>. On error, returns C<-1> with C<errno> set appropriately.

=cut

*/

int map_add(Map *map, const void *key, void *value)
{
	return map_insert(map, key, value, 0);
}

/*

=item C<int map_add_unlocked(Map *map, const void *key, void *value)>

Equivalent to I<map_add(3)> except that C<map> is not write-locked.

=cut

*/

int map_add_unlocked(Map *map, const void *key, void *value)
{
	return map_insert_unlocked(map, key, value, 0);
}

/*

=item C<int map_put(Map *map, const void *key, void *value)>

Adds the C<(key, value)> mapping to C<map>, replacing any existing C<(key,
value)> mapping. Note that C<key> is copied but C<value> is not. On success,
returns C<0>. On error, returns C<-1> with C<errno> set appropriately.

=cut

*/

int map_put(Map *map, const void *key, void *value)
{
	return map_insert(map, key, value, 1);
}

/*

=item C<int map_put_unlocked(Map *map, const void *key, void *value)>

Equivalent to I<map_put(3)> except that C<map> is not write-locked.

=cut

*/

int map_put_unlocked(Map *map, const void *key, void *value)
{
	return map_insert_unlocked(map, key, value, 1);
}

/*

=item C<int map_insert(Map *map, const void *key, void *value, int replace)>

Adds the C<(key, value)> mapping to C<map>, replacing any existing C<(key,
value)> mapping, if C<replace> is non-zero. Note that C<key> is copied but
C<value> is not. On success, returns C<0>. On error, or if C<key> is already
present and C<replace> is zero, returns C<-1> with C<errno> set
appropriately.

=cut

*/

int map_insert(Map *map, const void *key, void *value, int replace)
{
	int ret;
	int err;

	if (!map || !key)
		return set_errno(EINVAL);

	if ((err = map_wrlock(map)))
		return set_errno(err);

	ret = map_insert_unlocked(map, key, value, replace);

	if ((err = map_unlock(map)))
		return set_errno(err);

	return ret;
}

/*

=item C<int map_insert_unlocked(Map *map, const void *key, void *value, int replace)>

Equivalent to I<map_insert(3)> except that C<map> is not write-locked.

=cut

*/

int map_insert_unlocked(Map *map, const void *key, void *value, int replace)
{
	Mapping *mapping;
	List *chain;
	ssize_t length;
	size_t h, c;

	if (!map || !key)
		return set_errno(EINVAL);

	if ((double)map->items / (double)map->size >= (double)table_resize_factor)
		if (map_resize(map) == -1)
			return -1;

	if ((h = map->hash(map->size, key)) >= map->size)
		return set_errno(EINVAL);

	if (!map->chain[h] && !(map->chain[h] = list_create((map_release_t *)mapping_release)))
		return -1;

	chain = map->chain[h];

	if ((length = list_length_unlocked(chain)) == -1)
		return -1;

	for (c = 0; c < length; ++c)
	{
		mapping = (Mapping *)list_item_unlocked(chain, c);

		if (!map->cmp(mapping->key, key))
		{
			if (replace && list_remove_unlocked(chain, c))
				break;

			return -1;
		}
	}

	if (!(mapping = mapping_create(map->copy(key), value, map->key_destroy, map->value_destroy)))
		return -1;

	if (!list_append_unlocked(chain, mapping))
	{
		mapping_release(mapping);
		return -1;
	}

	++map->items;

	return 0;
}

/*

=item C<int map_remove(Map *map, const void *key)>

Removes C<(key, value)> mapping from C<map> if it is present. If C<map> was
created with a destroy function, then the value will be destroyed. On
success, returns C<0>. On error, returns C<-1> with C<errno> set
appropriately.

=cut

*/

int map_remove(Map *map, const void *key)
{
	int ret;
	int err;

	if (!map || !key)
		return set_errno(EINVAL);

	if ((err = map_wrlock(map)))
		return set_errno(err);

	ret = map_remove_unlocked(map, key);

	if ((err = map_unlock(map)))
		return set_errno(err);

	return ret;
}

/*

=item C<int map_remove_unlocked(Map *map, const void *key)>

Equivalent to I<map_remove(3)> except that C<map> is not write-locked.

=cut

*/

int map_remove_unlocked(Map *map, const void *key)
{
	List *chain;
	ssize_t length;
	size_t h, c;

	if (!map || !key)
		return set_errno(EINVAL);

	if ((h = map->hash(map->size, key)) >= map->size)
		return set_errno(EINVAL);

	if (!(chain = map->chain[h]))
		return set_errno(ENOENT);

	if ((length = list_length_unlocked(chain)) == -1)
		return -1;

	for (c = 0; c < length; ++c)
	{
		Mapping *mapping = (Mapping *)list_item_unlocked(chain, c);

		if (!map->cmp(mapping->key, key))
		{
			if (!list_remove_unlocked(chain, c))
				return -1;

			--map->items;

			return 0;
		}
	}

	return set_errno(ENOENT);
}

/*

=item C<void *map_get(Map *map, const void *key)>

Returns the value associated with C<key> in C<map>, or C<null> if there is
none. On error, returns C<null> with C<errno> set appropriately.

=cut

*/

void *map_get(Map *map, const void *key)
{
	void *ret;
	int err;

	if (!map || !key)
		return set_errnull(EINVAL);

	if ((err = map_rdlock(map)))
		return set_errnull(err);

	ret = map_get_unlocked(map, key);

	if ((err = map_unlock(map)))
		return set_errnull(err);

	return ret;
}

/*

=item C<void *map_get_unlocked(const Map *map, const void *key)>

Equivalent to I<map_get(3)> except that C<map> is not read-locked.

=cut

*/

void *map_get_unlocked(const Map *map, const void *key)
{
	List *chain;
	ssize_t length;
	size_t h, c;

	if (!map || !key)
		return set_errnull(EINVAL);

	if ((h = map->hash(map->size, key)) >= map->size)
		return set_errnull(EINVAL);

	if (!(chain = map->chain[h]))
		return set_errnull(ENOENT);

	if ((length = list_length_unlocked(chain)) == -1)
		return NULL;

	for (c = 0; c < length; ++c)
	{
		Mapping *mapping = (Mapping *)list_item_unlocked(chain, c);

		if (!map->cmp(mapping->key, key))
			return mapping->value;
	}

	return set_errnull(ENOENT);
}

/*

=item C<Mapper *mapper_create(Map *map)>

Creates an iterator for C<map>. The iterator keeps C<map> write-locked until
it is released with I<mapper_release(3)> or I<mapper_destroy(3)>. Note that
the iterator itself is not locked, so it must not be shared between threads.
On success, returns the iterator. On error, returns C<null> with C<errno>
set appropriately.

=cut

*/

Mapper *mapper_create(Map *map)
{
	return mapper_create_wrlocked(map);
}

/*

=item C<Mapper *mapper_create_rdlocked(Map *map)>

Equivalent to I<mapper_create(3)> except that C<map> is read-locked rather
than write-locked. Use this in preference to I<mapper_create(3)> when no
calls to I<mapper_remove(3)> will be made during the iteration.

=cut

*/

Mapper *mapper_create_rdlocked(Map *map)
{
	int err;

	if (!map)
		return set_errnull(EINVAL);

	if ((err = map_rdlock(map)))
		return set_errnull(err);

	return mapper_create_unlocked(map);
}

/*

=item C<Mapper *mapper_create_wrlocked(Map *map)>

Equivalent to I<mapper_create(3)> except that this function name makes the
fact that C<map> is write-locked explicit.

=cut

*/

Mapper *mapper_create_wrlocked(Map *map)
{
	int err;

	if (!map)
		return set_errnull(EINVAL);

	if ((err = map_wrlock(map)))
		return set_errnull(err);

	return mapper_create_unlocked(map);
}

/*

=item C<Mapper *mapper_create_unlocked(Map *map)>

Equivalent to I<mapper_create(3)> except that C<map> is not write-locked.

=cut

*/

Mapper *mapper_create_unlocked(Map *map)
{
	Mapper *mapper;

	if (!map)
		return set_errnull(EINVAL);

	if (!(mapper = mem_new(Mapper))) /* XXX decouple */
		return NULL;

	mapper->map = map;
	mapper->chain_index = -1;
	mapper->item_index = -1;
	mapper->next_chain_index = -1;
	mapper->next_item_index = -1;

	return mapper;
}

/*

=item C<void mapper_release(Mapper *mapper)>

Releases (deallocates) C<mapper> and unlocks the associated map.

=cut

*/

void mapper_release(Mapper *mapper)
{
	int err;

	if (!mapper)
		return;

	if ((err = map_unlock(mapper->map)))
	{
		set_errno(err);
		return;
	}

	mem_release(mapper);
}

/*

=item C<void mapper_release_unlocked(Mapper *mapper)>

Equivalent to I<mapper_release(3)> except that the associated map is not
unlocked.

=cut

*/

void mapper_release_unlocked(Mapper *mapper)
{
	if (!mapper)
		return;

	mem_release(mapper);
}

/*

=item C<void *mapper_destroy(Mapper **mapper)>

Destroys (deallocates and sets to C<null>) C<*mapper> and unlocks the
associated map. Returns C<null>. On error, sets C<errno> appropriately.

=cut

*/

void *mapper_destroy(Mapper **mapper)
{
	if (mapper && *mapper)
	{
		mapper_release(*mapper);
		*mapper = NULL;
	}

	return NULL;
}

/*

=item C<void *mapper_destroy_unlocked(Mapper **mapper)>

Equivalent to I<mapper_destroy(3)> except that the associated map is not
unlocked.

=cut

*/

void *mapper_destroy_unlocked(Mapper **mapper)
{
	if (mapper && *mapper)
	{
		mapper_release_unlocked(*mapper);
		*mapper = NULL;
	}

	return NULL;
}

/*

=item C<int mapper_has_next(Mapper *mapper)>

Returns whether or not there is another item in the map over which C<mapper>
is iterating. On error, returns C<-1> with C<errno> set appropriately.

=cut

*/

int mapper_has_next(Mapper *mapper)
{
	List *chain;
	ssize_t length;

	if (!mapper)
		return set_errno(EINVAL);

	/* Find the current/first chain */

	mapper->next_chain_index = mapper->chain_index;
	mapper->next_item_index = mapper->item_index;

	if (mapper->next_chain_index == -1)
		++mapper->next_chain_index;

	while (mapper->next_chain_index < mapper->map->size && !mapper->map->chain[mapper->next_chain_index])
		++mapper->next_chain_index;

	if (mapper->next_chain_index == mapper->map->size)
		return 0;

	chain = mapper->map->chain[mapper->next_chain_index];

	/* Find the next item */

	if ((length = list_length_unlocked(chain)) == -1)
		return -1;

	if (++mapper->next_item_index < length)
		return 1;

	do
	{
		++mapper->next_chain_index;

		while (mapper->next_chain_index < mapper->map->size && !mapper->map->chain[mapper->next_chain_index])
			++mapper->next_chain_index;

		if (mapper->next_chain_index == mapper->map->size)
			return 0;

		chain = mapper->map->chain[mapper->next_chain_index];

		if ((length = list_length_unlocked(chain)) == -1)
			return -1;
	}
	while (length == 0);

	mapper->next_item_index = 0;

	return 1;
}

/*

=item C<void *mapper_next(Mapper *mapper)>

Returns the next item in the map over which C<mapper> is iterating. On
error, returns C<null> with C<errno> set appropriately.

=cut

*/

void *mapper_next(Mapper *mapper)
{
	if (!mapper)
		return set_errnull(EINVAL);

	return mapper_next_mapping(mapper)->value;
}

/*

=item C<const Mapping *mapper_next_mapping(Mapper *mapper)>

Returns the next mapping (key, value) pair in the map over which C<mapper>
is iterating. On error, returns C<null> with C<errno> set appropriately.

=cut

*/

const Mapping *mapper_next_mapping(Mapper *mapper)
{
	if (!mapper)
		return set_errnull(EINVAL);

	mapper->chain_index = mapper->next_chain_index;
	mapper->item_index = mapper->next_item_index;

	return (Mapping *)list_item_unlocked(mapper->map->chain[mapper->chain_index], mapper->item_index);
}

/*

=item C<void mapper_remove(Mapper *mapper)>

Removes the current item in the iteration C<mapper>. The next item in the
iteration is the item following the removed item, if any. This must be
called after I<mapper_next(3)>. On error, sets C<errno> appropriately.

=cut

*/

void mapper_remove(Mapper *mapper)
{
	if (!mapper)
	{
		set_errno(EINVAL);
		return;
	}

	if (mapper->item_index == -1)
	{
		set_errno(EINVAL);
		return;
	}

	list_remove_unlocked(mapper->map->chain[mapper->chain_index], (size_t)mapper->item_index--);
	--mapper->map->items;
}

/*

=item C<int map_has_next(Map *map)>

Returns whether or not there is another item in C<map> using an internal
iterator. The first time this is called, a new internal I<Mapper> will be
created (Note: There can be only one at any time for a given map). When
there are no more items, returns C<0> and destroys the internal iterator.
When it returns C<1>, use I<map_next(3)> to retrieve the next item. On
error, returns C<-1> with C<errno> set appropriately.

Note: If an iteration using an internal iterator terminates before the end
of the map, it is the caller's responsibility to call I<map_break(3)>.
Failure to do so will cause the internal iterator to leak. It will also
break the next call to I<map_has_next(3)> which will continue where the
current iteration stopped rather than starting at the beginning again.
I<map_release(3)> assumes that there is no internal iterator, so it is the
caller's responsibility to complete the iteration, or call I<map_break(3)>
before releasing C<map> with I<map_release(3)> or I<map_destroy(3)>.

Note: The internal I<Mapper> does not lock C<map> so this function is not
threadsafe. It can only be used with maps created in the current function
(to guarantee that no other thread can access it). This practice should be
observed even in single-threaded applications to avoid breaking iterator
semantics (possible with nested function calls). If C<map> is a parameter or
a variable declared outside the function, it is best to create an explicit
I<Mapper> instead. If this function is used on such maps instead, it is the
caller's responsibility to explicitly lock C<map> first with
I<map_wrlock(3)> and explicitly unlock it with I<map_unlock(3)>. Do this
even if you are writing single-threaded code, in case your function may one
day be used in a multi-threaded application.

=cut

*/

int map_has_next(Map *map)
{
	int has;

	if (!map)
		return set_errno(EINVAL);

	if (!map->mapper && !(map->mapper = mapper_create_unlocked(map)))
		return -1;

	if ((has = mapper_has_next(map->mapper)) != 1)
		map_break(map);

	return has;
}

/*

=item C<void map_break(Map *map)>

Unlocks C<map> and destroys its internal iterator. Must be used only when an
iteration using an internal iterator has terminated before reaching the end
of C<map>. On error, returns C<null> with C<errno> set appropriately.

=cut

*/

void map_break(Map *map)
{
	if (!map)
	{
		set_errno(EINVAL);
		return;
	}

	mapper_destroy_unlocked(&map->mapper);
}

/*

=item C<void *map_next(Map *map)>

Returns the next item in C<map> using its internal iterator. On error,
returns C<null> with C<errno> set appropriately.

=cut

*/

void *map_next(Map *map)
{
	if (!map || !map->mapper)
		return set_errnull(EINVAL);

	return mapper_next(map->mapper);
}

/*

=item C<const Mapping *map_next_mapping(Map *map)>

Returns the next mapping (key, value) pair in C<map> using its internal
iterator. On error, returns C<-1> with C<errno> set appropriately.

=cut

*/

const Mapping *map_next_mapping(Map *map)
{
	if (!map || !map->mapper)
		return set_errnull(EINVAL);

	return mapper_next_mapping(map->mapper);
}

/*

=item C<void map_remove_current(Map *map)>

Removes the current item in C<map> using its internal iterator. The next
item in the iteration is the item following the removed item, if any. This
must be called after I<map_next(3)>.

=cut

*/

void map_remove_current(Map *map)
{
	if (!map || !map->mapper)
	{
		set_errno(EINVAL);
		return;
	}

	mapper_remove(map->mapper);
}

/*

=item C<const void *mapping_key(const Mapping *mapping)>

Returns the key in C<mapping>. On error, returns C<null> with C<errno> set
appropriately.

=cut

*/

const void *mapping_key(const Mapping *mapping)
{
	if (!mapping)
		return set_errnull(EINVAL);

	return mapping->key;
}

/*

=item C<const void *mapping_value(const Mapping *mapping)>

Returns the value in C<mapping>. On error, returns C<null> with C<errno> set
appropriately.

=cut

*/

const void *mapping_value(const Mapping *mapping)
{
	if (!mapping)
		return set_errnull(EINVAL);

	return mapping->value;
}

/*

=item C<List *map_keys(Map *map)>

Creates and returns a list of all of the keys contained in C<map>. It is the
caller's responsibility to deallocate the new list with I<list_release(3)>
or I<list_destroy(3)>. It is strongly recommended to use I<list_destroy(3)>,
because it also sets the pointer variable to C<null>. The keys in the new
list are owned by C<map>, so the list returned must not outlive C<map>. On
error, returns C<null> with C<errno> set appropriately.

=cut

*/

List *map_keys(Map *map)
{
	return map_keys_with_locker(NULL, map);
}

/*

=item C<List *map_keys_unlocked(Map *map)>

Equivalent to I<map_keys(3)> except that C<map> is not read-locked.

=cut

*/

List *map_keys_unlocked(Map *map)
{
	return map_keys_with_locker_unlocked(NULL, map);
}

/*

=item C<List *map_keys_with_locker(Locker *locker, Map *map)>

Equivalent to I<map_keys(3)> except that multiple threads accessing the list
returned will be synchronised by C<locker>.

=cut

*/

List *map_keys_with_locker(Locker *locker, Map *map)
{
	List *keys;
	int err;

	if (!map)
		return set_errnull(EINVAL);

	if ((err = map_rdlock(map)))
		return set_errnull(err);

	keys = map_keys_with_locker_unlocked(locker, map);

	if ((err = map_unlock(map)))
		return set_errnull(err);

	return keys;
}

/*

=item C<List *map_keys_with_locker_unlocked(Locker *locker, Map *map)>

Equivalent to I<map_keys_with_locker(3)> except that C<map> is not
read-locked.

=cut

*/

List *map_keys_with_locker_unlocked(Locker *locker, Map *map)
{
	Mapper *mapper;
	List *keys;

	if (!map)
		return set_errnull(EINVAL);

	if (!(keys = list_create_with_locker(locker, NULL)))
		return NULL;

	if (!(mapper = mapper_create_unlocked(map)))
	{
		list_release(keys);
		return NULL;
	}

	while (mapper_has_next(mapper) == 1)
	{
		const Mapping *mapping = mapper_next_mapping(mapper);

		if (!list_append(keys, mapping->key))
		{
			list_destroy(&keys);
			break;
		}
	}

	mapper_release_unlocked(mapper);

	return keys;
}

/*

=item C<List *map_values(Map *map)>

Creates and returns a list of all of the values contained in C<map>. It is
the caller's responsibility to deallocate the new list with
I<list_release(3)> or I<list_destroy(3)>. It is strongly recommended to use
I<list_destroy(3)>, because it also sets the pointer variable to C<null>.
The values in the list are not owned by the list, so it must not outlive the
owner of the items in C<map>. On error, returns C<null> with C<errno> set
appropriately.

=cut

*/

List *map_values(Map *map)
{
	return map_values_with_locker(NULL, map);
}

/*

=item C<List *map_values_unlocked(Map *map)>

Equivalent to I<map_values(3)> except that C<map> is not read-locked.

=cut

*/

List *map_values_unlocked(Map *map)
{
	return map_values_with_locker_unlocked(NULL, map);
}

/*

=item C<List *map_values_with_locker(Locker *locker, Map *map)>

Equivalent to I<map_values(3)> except that multiple threads accessing the
list returned will be synchronised by C<locker>.

=cut

*/

List *map_values_with_locker(Locker *locker, Map *map)
{
	List *values;
	int err;

	if (!map)
		return set_errnull(EINVAL);

	if ((err = map_rdlock(map)))
		return set_errnull(err);

	values = map_values_with_locker_unlocked(locker, map);

	if ((err = map_unlock(map)))
		return set_errnull(err);

	return values;
}

/*

=item C<List *map_values_with_locker_unlocked(Locker *locker, Map *map)>

Equivalent to I<map_values_with_locker(3)> except that C<map> is not
read-locked.

=cut

*/

List *map_values_with_locker_unlocked(Locker *locker, Map *map)
{
	Mapper *mapper;
	List *values;

	if (!map)
		return set_errnull(EINVAL);

	if (!(values = list_create_with_locker(locker, NULL)))
		return NULL;

	if (!(mapper = mapper_create_unlocked(map)))
	{
		list_release(values);
		return NULL;
	}

	while (mapper_has_next(mapper) == 1)
	{
		const Mapping *mapping = mapper_next_mapping(mapper);

		if (!list_append(values, mapping->value))
		{
			list_destroy(&values);
			break;
		}
	}

	mapper_release_unlocked(mapper);

	return values;
}

/*

=item C<void map_apply(Map *map, map_action_t *action, void *data)>

Invokes C<action> for each of C<map>'s items. The arguments passed to
C<action> are the key, the item, and C<data>. On error, sets C<errno>
appropriately.

=cut

*/

void map_apply(Map *map, map_action_t *action, void *data)
{
	map_apply_wrlocked(map, action, data);
}

/*

=item C<void map_apply_rdlocked(Map *map, map_action_t *action, void *data)>

Equivalent to I<map_apply(3)> except that C<map> is read-locked rather than
write-locked. Use this in preference to I<map_apply(3)> when C<map> and its
items will not be modified during the iteration.

=cut

*/

void map_apply_rdlocked(Map *map, map_action_t *action, void *data)
{
	int err;

	if (!map || !action)
	{
		set_errno(EINVAL);
		return;
	}

	if ((err = map_rdlock(map)))
	{
		set_errno(err);
		return;
	}

	map_apply_unlocked(map, action, data);

	if ((err = map_unlock(map)))
	{
		set_errno(err);
		return;
	}
}

/*

=item C<void map_apply_wrlocked(Map *map, map_action_t *action, void *data)>

Equivalent to I<map_apply(3)> except that this function name makes the fact
that C<map> is write-locked explicit.

=cut

*/

void map_apply_wrlocked(Map *map, map_action_t *action, void *data)
{
	int err;

	if (!map || !action)
	{
		set_errno(EINVAL);
		return;
	}

	if ((err = map_wrlock(map)))
	{
		set_errno(err);
		return;
	}

	map_apply_unlocked(map, action, data);

	if ((err = map_unlock(map)))
	{
		set_errno(err);
		return;
	}
}

/*

=item C<void map_apply_unlocked(Map *map, map_action_t *action, void *data)>

Equivalent to I<map_apply(3)> except that C<map> is not write-locked.

=cut

*/

void map_apply_unlocked(Map *map, map_action_t *action, void *data)
{
	Mapper *mapper;

	if (!map || !action)
	{
		set_errno(EINVAL);
		return;
	}

	if (!(mapper = mapper_create_unlocked(map)))
		return;

	while (mapper_has_next(mapper) == 1)
	{
		const Mapping *mapping = mapper_next_mapping(mapper);
		action(mapping->key, mapping->value, data);
	}

	mapper_release_unlocked(mapper);
}

/*

=item C<ssize_t map_size(Map *map)>

Returns the number of mappings in C<map>. On error, returns C<-1> with
C<errno> set appropriately.

=cut

*/

ssize_t map_size(Map *map)
{
	size_t size;
	int err;

	if (!map)
		return set_errno(EINVAL);

	if ((err = map_rdlock(map)))
		return set_errno(err);

	size = map->items;

	if ((err = map_unlock(map)))
		return set_errno(err);

	return size;
}

/*

=item C<ssize_t map_size_unlocked(const Map *map)>

Equivalent to I<map_size(3)> except that C<map> is not read-locked.

=cut

*/

ssize_t map_size_unlocked(const Map *map)
{
	if (!map)
		return set_errno(EINVAL);

	return map->items;
}

/*

=back

=head1 ERRORS

On error, C<errno> is set either by an underlying function, or as follows:

=over 4

=item C<EINVAL>

When arguments are C<null> or out of range.

=item C<ENOENT>

When I<map_get(3)> tries to get, or I<map_remove(3)> tries to remove, a
non-existent mapping.

=back

=head1 MT-Level

I<MT-Disciplined>

By default, I<Map>s are not I<MT-Safe> because most programs are
single-threaded, and synchronisation doesn't come for free. Even in
multi-threaded programs, not all I<Map>s are necessarily shared between
multiple threads.

When a I<Map> is shared between multiple threads which need to be
synchronised, the method of synchronisation must be carefully selected by
the client code. There are tradeoffs between concurrency and overhead. The
greater the concurrency, the greater the overhead. More locks give greater
concurrency, but have greater overhead. Readers/Writer locks can give
greater concurrency than Mutex, locks but have greater overhead. One lock
for each I<Map> may be required, or one lock for all (or a set of) I<Map>s
may be more appropriate.

Generally, the best synchronisation strategy for a given application can
only be determined by testing/benchmarking the written application. It is
important to be able to experiment with the synchronisation strategy at this
stage of development without pain.

To facilitate this, I<Map>s can be created with
I<map_create_with_locker(3)>, which takes a I<Locker> argument. The
I<Locker> specifies a lock and a set of functions for manipulating the lock.
Each I<Map> can have its own lock by creating a separate I<Locker> for each
I<Map>. Multiple I<Map>s can share the same lock by sharing the same
I<Locker>. Only the application developer can determine what is appropriate
for each application on a case by case basis.

I<MT-Disciplined> means that the application developer has a mechanism for
specifying the synchronisation requirements to be applied to library code.

=head1 EXAMPLES

Create a map that doesn't own its items, populate it, and then iterate over
its values with the internal iterator to print the values:

    #include <slack/std.h>
    #include <slack/map.h>

    int main()
    {
        Map *map;

        if (!(map = map_create(NULL)))
            return EXIT_FAILURE;

        map_add(map, "abc", "123");
        map_add(map, "def", "456");
        map_add(map, "ghi", "789");

        while (map_has_next(map) == 1)
            printf("%s\n", (char *)map_next(map));

        map_destroy(&map);

        return EXIT_SUCCESS;
    }

Create a map that does own its items, populate it, and then iterate over its
items with an external iterator to print its keys and values:

    #include <slack/std.h>
    #include <slack/map.h>

    int main()
    {
        Map *map;
        Mapper *mapper;

        if (!(map = map_create(free)))
            return EXIT_FAILURE;

        map_add(map, "abc", strdup("123"));
        map_add(map, "def", strdup("456"));
        map_add(map, "ghi", strdup("789"));

        if (!(mapper = mapper_create(map)))
        {
            map_destroy(&map);
            return EXIT_FAILURE;
        }

        while (mapper_has_next(mapper) == 1)
        {
            const Mapping *mapping = mapper_next_mapping(mapper);

            printf("%s -> %s\n", (char *)mapping_key(mapping), (char *)mapping_value(mapping));
        }

        mapper_destroy(&mapper);
        map_destroy(&map);

        return EXIT_SUCCESS;
    }

The same, but with an apply function:

    #include <slack/std.h>
    #include <slack/map.h>

    void action(void *key, void *item, void *data)
    {
        printf("%s -> %s\n", (char *)key, (char *)item);
    }

    int main()
    {
        Map *map;

        if (!(map = map_create(free)))
            return EXIT_FAILURE;

        map_add(map, "abc", strdup("123"));
        map_add(map, "def", strdup("456"));
        map_add(map, "ghi", strdup("789"));
        map_apply(map, action, NULL);
        map_destroy(&map);

        return EXIT_SUCCESS;
    }

The same, but with integer keys:

    #include <slack/std.h>
    #include <slack/map.h>

    static void *int_copy(const void *key)
    {
        return (void *)key;
    }

    static int int_cmp(const void *a, const void *b)
    {
        return (int)a - (int)b;
    }

    static size_t int_hash(size_t size, const void *key)
    {
        return (int)key % size;
    }

    void action(void *key, void *item, void *data)
    {
        printf("%d -> %s\n", (int)key, (char *)item);
    }

    int main(int ac, char **av)
    {
        Map *map;

        if (!(map = map_create_generic(int_copy, int_cmp, int_hash, NULL, free)))
            return EXIT_FAILURE;

        map_add(map, (void *)123, strdup("abc"));
        map_add(map, (void *)456, strdup("def"));
        map_add(map, (void *)789, strdup("ghi"));

        map_apply(map, action, NULL);
        map_destroy(&map);

        return EXIT_SUCCESS;
    }

=head1 CAVEAT

A C<null> pointer can't be a key. Neither can zero when using integers as
keys.

If you use an internal iterator in a loop that terminates before the end of
the map, and fail to call I<map_break(3)>, the internal iterator will leak.

=head1 BUGS

Uses I<malloc(3)> directly. The type of memory used and the allocation
strategy should be decoupled from this code.

=head1 SEE ALSO

I<libslack(3)>,
I<list(3)>,
I<mem(3)>,
I<locker(3)>

=head1 AUTHOR

20210220 raf <raf@raf.org>

=cut

*/

#endif

#ifdef TEST

#ifndef HAVE_SNPRINTF
#include <slack/snprintf.h>
#endif

#if 0
static void map_print(const char *name, Map *map)
{
	size_t i, j;

	if (!map)
	{
		printf("%s = nil\n", name);
		return;
	}

	printf("%s =\n{\n", name);

	for (i = 0; i < map->size; ++i)
	{
		if (map->chain[i])
		{
			List *chain = map->chain[i];
			ssize_t length = list_length(chain);

			if (length == -1)
				printf("    !!! length == -1 !!!\n");

			for (j = 0; j < length; ++j)
			{
				Mapping *mapping = (Mapping *)list_item(chain, j);

				printf("    [%d/%d] \"%s\" -> \"%s\"\n", i, j, (char *)mapping->key, (char *)mapping->value);
			}
		}
	}

	printf("}\n");
}
#endif

static void map_histogram(const char *name, Map *map)
{
	size_t i;
	int *histogram;

	if (!map)
	{
		printf("%s = nil\n", name);
		return;
	}

	if (!(histogram = mem_create(map->items, int)))
	{
		printf("Failed to allocate histogram for map %s\n", name);
		return;
	}

	memset(histogram, 0, map->items * sizeof(int));

	for (i = 0; i < map->size; ++i)
	{
		size_t length = list_length(map->chain[i]);
		if (length == -1)
		{
			printf("  length[%d] = -1\n", (int)i);
			continue;
		}
		++histogram[length];
	}

	printf("\nhistogram %s =\n{\n", name);

	for (i = 0; i < map->items; ++i)
		if (histogram[i])
			printf("    %d chain%s of length %d\n", histogram[i], (histogram[i] == 1) ? "" : "s", (int)i);

	printf("}\n");
}

static void test_hash(void)
{
	FILE *words = fopen("/usr/dict/words", "r");
	char word[BUFSIZ];
	Map *map;
	size_t c;
	size_t min = 0xffffffff;
	size_t max = 0x00000000;
	int sum = 0;

	if (!words)
	{
		printf("Failed to open /usr/dict/words\n");
		exit(EXIT_FAILURE);
	}

	if (!(map = map_create(free)))
	{
		printf("Failed to create map\n");
		exit(EXIT_FAILURE);
	}

	while (fgets(word, BUFSIZ, words))
	{
		char *eow = strchr(word, '\n');
		if (eow)
			*eow = nul;

		map_add(map, word, mem_strdup(word));
	}

	fclose(words);

	printf("%d entries into %d buckets:\n\n", (int)map->items, (int)map->size);

	for (c = 0; c < map->size; ++c)
	{
		size_t length;

		if (!map->chain[c])
			continue;

		if ((length = list_length(map->chain[c])) == -1)
		{
			printf(" length[%d] == -1\n", (int)c);
			continue;
		}

		if (length > max)
			max = length;
		if (length < min)
			min = length;
		sum += length;
	}

	printf("avg = %g\n", (double)sum / (double)map->size);
	printf("min = %d\n", (int)min);
	printf("max = %d\n", (int)max);
	map_histogram("dict", map);
	map_release(map);

	exit(EXIT_SUCCESS);
}

static int sort_cmp(const char **a, const char **b)
{
	return strcmp(*a, *b);
}

static void test_action(char *key, char *value, char *cat)
{
	size_t len = strlen(cat);
	snprintf(cat + len, BUFSIZ, "%s%s=%s", (len) ? ", " : "", key, value);
}

typedef struct Point Point;
struct Point
{
	int x;
	int y;
};

static Point *point_create(int x, int y)
{
	Point *point = mem_create(1, Point);

	if (!point)
		return NULL;

	point->x = x;
	point->y = y;

	return point;
}

static Point *point_copy(Point *point)
{
	return point_create(point->x, point->y);
}

static int point_cmp(Point *a, Point *b)
{
	if (a->x > b->x)
		return 1;
	if (a->y > b->y)
		return 1;
	if (a->x == b->x && a->y == b->y)
		return 0;
	return -1;
}

static size_t point_hash(size_t size, Point *point)
{
	return (point->x * 31 + point->y * 37) % size;
}

static void point_release(Point *point)
{
	mem_release(point);
}

static int direct_copy(int key)
{
	return key;
}

static int direct_cmp(int a, int b)
{
	return a - b;
}

static size_t direct_hash(size_t size, int key)
{
	return key % size;
}

#define RD 0
#define WR 1
Map *mtmap = NULL;
Locker *locker = NULL;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
#ifdef PTHREAD_RWLOCK_INITIALIZER
pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;
#else
pthread_rwlock_t rwlock;
#endif
int barrier[2];
int size[2];
const int lim = 1000;
int debug = 0;
int errors = 0;

void *produce(void *arg)
{
	int i;
	int test = *(int *)arg;

	for (i = 1; i <= lim; ++i)
	{
		if (debug)
			printf("p: add %d\n", i);

		if (map_add(mtmap, (void *)(long)i, (void *)(long)i) == -1)
			++errors, printf("Test%d: map_add(mtmap, %d), failed (%s)\n", test, i, strerror(errno));

		write(size[WR], "", 1);
	}

	write(barrier[WR], "", 1);
	return NULL;
}

void *consume(void *arg)
{
	int i;
	int test = *(int *)arg;
	char ack;

	for (i = 1; i <= lim; ++i)
	{
		if (debug)
			printf("c: pop\n");

		while (read(size[RD], &ack, 1) == -1 && errno == EINTR)
		{}

		if (map_remove(mtmap, (void *)(long)i) == -1)
		{
			++errors, printf("Test%d: map_remove(mtmap, %d), failed\n", test, i);
			break;
		}

		if (debug)
			printf("c: remove %d\n", i);
	}

	if (i != lim + 1)
		++errors, printf("Test%d: consumer read %d items, not %d\n", test, i - 1, lim);

	write(barrier[WR], "", 1);
	return NULL;
}

void *iterate_builtin(void *arg)
{
	int i;
	int t = *(int *)arg;
	int broken = 0;

	if (debug)
		printf("i%d: loop\n", t);

	for (i = 0; i < lim / 10; ++i)
	{
		map_wrlock(mtmap);

		while (map_has_next(mtmap) == 1)
		{
			int val = (int)(long)map_next(mtmap);

			if (debug)
				printf("i%d: loop %d/%d val %d\n", t, i, lim / 10, val);

			if (!broken)
			{
				map_break(mtmap);
				broken = 1;
				break;
			}
		}

		map_unlock(mtmap);
	}

	write(barrier[WR], "", 1);
	return NULL;
}

void *iterate_rdlocked(void *arg)
{
	int i;
	int t = *(int *)arg;

	if (debug)
		printf("j%d: loop\n", t);

	for (i = 0; i < lim / 10; ++i)
	{
		Mapper *mapper = mapper_create_rdlocked(mtmap);

		while (mapper_has_next(mapper) == 1)
		{
			int val = (int)(long)mapper_next(mapper);

			if (debug)
				printf("j%d: loop %d/%d val %d\n", t, i, lim / 10, val);
		}

		mapper_release(mapper);
	}

	write(barrier[WR], "", 1);
	return NULL;
}

void *iterate_wrlocked(void *arg)
{
	int i;
	int t = *(int *)arg;

	if (debug)
		printf("k%d: loop\n", t);

	for (i = 0; i < lim / 10; ++i)
	{
		Mapper *mapper = mapper_create_wrlocked(mtmap);

		while (mapper_has_next(mapper) == 1)
		{
			int val = (int)(long)mapper_next(mapper);

			if (debug)
				printf("k%d: loop %d/%d val %d\n", t, i, lim / 10, val);
		}

		mapper_release(mapper);
	}

	write(barrier[WR], "", 1);
	return NULL;
}

pthread_mutex_t rand_mutex[1] = { PTHREAD_MUTEX_INITIALIZER };

void *reader(void *arg)
{
	int i;
	int t = *(int *)arg;

	if (debug)
		printf("r%d: loop\n", t);

	for (i = 0; i < lim / 10; ++i)
	{
		int key, value, r;
		List *keys, *values;

		pthread_mutex_lock(rand_mutex);
		r = rand();
		pthread_mutex_unlock(rand_mutex);

		map_rdlock(mtmap);
		key = 1 + (int)((double)(map_size_unlocked(mtmap) - 1) * r / (RAND_MAX + 1.0));
		value = (int)(long)map_get_unlocked(mtmap, (void *)(long)key);
		map_unlock(mtmap);

		keys = map_keys(mtmap);
		values = map_values(mtmap);

		if (debug)
			printf("r%d: loop %d/%d key/val %d/%d, #keys %d, #values %d\n", t, i, lim / 10, key, value, (int)list_length(keys), (int)list_length(values));

		list_destroy(&keys);
		list_destroy(&values);
	}

	write(barrier[WR], "", 1);
	return NULL;
}

void mt_test(int test, Locker *locker)
{
	mtmap = map_create_generic_with_locker(locker, (map_copy_t *)direct_copy, (map_cmp_t *)direct_cmp, (map_hash_t *)direct_hash, NULL, NULL);
	if (!mtmap)
		++errors, printf("Test%d: map_create_generic_with_locker(NULL) failed\n", test);
	else
	{
		static int iid[13] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
		pthread_attr_t attr;
		pthread_t id;
		int i;
		char ack;

		if (pipe(size) == -1 || pipe(barrier) == -1)
		{
			++errors, printf("Test%d: failed to perform test: pipe() failed\n", test);
			return;
		}

		srand(getpid() ^ time(NULL));
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		pthread_create(&id, &attr, produce, &test);
		pthread_create(&id, &attr, consume, &test);
		pthread_create(&id, &attr, iterate_builtin, iid + 0);
		pthread_create(&id, &attr, iterate_builtin, iid + 1);
		pthread_create(&id, &attr, iterate_builtin, iid + 2);
		pthread_create(&id, &attr, iterate_builtin, iid + 3);
		pthread_create(&id, &attr, iterate_rdlocked, iid + 4);
		pthread_create(&id, &attr, iterate_rdlocked, iid + 5);
		pthread_create(&id, &attr, iterate_rdlocked, iid + 6);
		pthread_create(&id, &attr, iterate_wrlocked, iid + 7);
		pthread_create(&id, &attr, iterate_wrlocked, iid + 8);
		pthread_create(&id, &attr, iterate_wrlocked, iid + 9);
		pthread_create(&id, &attr, reader, iid + 10);
		pthread_create(&id, &attr, reader, iid + 11);
		pthread_create(&id, &attr, reader, iid + 12);
		pthread_attr_destroy(&attr);

		for (i = 0; i < 15; ++i)
			while (read(barrier[RD], &ack, 1) == -1 && errno == EINTR)
			{}

		map_destroy(&mtmap);
		if (mtmap)
			++errors, printf("Test%d: map_destroy(&mtmap) failed\n", test);

		close(size[RD]);
		close(size[WR]);
		close(barrier[RD]);
		close(barrier[WR]);
	}
}

#define TEST_ACT(i, action) \
	if (!(action)) \
		++errors, printf("Test%d: %s failed\n", (i), (#action));

#define TEST_INT_ACT(i, action) \
	if ((action) == -1) \
		++errors, printf("Test%d: %s failed\n", (i), (#action));

#define TEST_EQ(i, action, value) \
	if ((val = (action)) != (value)) \
		++errors, printf("Test%d: %s failed (returned %d, not %d)\n", (i), (#action), val, (value));

#define TEST_NEQ(i, action, value) \
	if ((val = (action)) == (value)) \
		++errors, printf("Test%d: %s failed (returned %d)\n", (i), (#action), val);

#define CHECK_ITEM(i, action, key, result) \
	if (!(value = (char *)(action)) || strcmp(value, (result))) \
		++errors, printf("Test%d: %s failed (mapping \"%s\" is \"%s\", not \"%s\")\n", (i), (#action), (key), value, (result));

#define CHECK_LIST_ITEM(i, action, list, item, value) \
	if (!list_item((list), (item)) || strcmp(list_item((list), (item)), (value))) \
		++errors, printf("Test%d: %s failed (item %d is \"%s\", not \"%s\")\n", (i), (#action), (item), (char *)list_item(list, (item)), (value));

#define CHECK_LIST(i, desc, list) \
	if (list_length(list) != 4) \
		++errors, printf("Test%d: %s failed (%d items, not 4)\n", (i), (desc), (int)list_length(list)); \
	else \
	{ \
		TEST_ACT((i), list_sort((list), (list_cmp_t *)sort_cmp)); \
		CHECK_LIST_ITEM((i), list_sort((list), sort_cmp), (list), 0, "abc") \
		CHECK_LIST_ITEM((i), list_sort((list), sort_cmp), (list), 1, "def") \
		CHECK_LIST_ITEM((i), list_sort((list), sort_cmp), (list), 2, "ghi") \
		CHECK_LIST_ITEM((i), list_sort((list), sort_cmp), (list), 3, "jkl") \
	}

int main(int ac, char **av)
{
	Map *map;
	Mapper *mapper;
	List *keys, *values;
	const void *ckey;
	const char *cvalue;
	char *value;
	char cat[BUFSIZ];
	void *ptr;
	int val;

	if (ac == 2 && !strcmp(av[1], "help"))
	{
		printf("usage: %s [debug|hash]\n", *av);
		return EXIT_SUCCESS;
	}

	if (ac == 2 && !strcmp(av[1], "hash"))
		test_hash();

	printf("Testing: %s\n", "map");

	/* Test map_create, map_add, map_get */

	TEST_ACT(1, map = map_create(NULL))
	else
	{
		TEST_INT_ACT(2, map_add(map, "abc", "abc"))
		TEST_INT_ACT(3, map_add(map, "def", "def"))
		TEST_INT_ACT(4, map_add(map, "ghi", "ghi"))
		TEST_INT_ACT(5, map_add(map, "jkl", "jkl"))

		CHECK_ITEM(6, map_get(map, "abc"), "abc", "abc")
		CHECK_ITEM(7, map_get(map, "def"), "def", "def")
		CHECK_ITEM(8, map_get(map, "ghi"), "ghi", "ghi")
		CHECK_ITEM(9, map_get(map, "jkl"), "jkl", "jkl")
		if ((value = (char *)map_get(map, "zzz")))
			++errors, printf("Test10: map_get(\"zzz\") failed\n");

		/* Test mapper_create, mapper_has_next, mapper_next, map_destroy */

		TEST_ACT(11, mapper = mapper_create(map))
		else
		{
			TEST_ACT(12, keys = list_create(NULL))

			while (mapper_has_next(mapper) == 1)
			{
				void *item = mapper_next(mapper);
				TEST_ACT(13, item)
				else TEST_ACT(13, list_append(keys, item))
			}

			mapper_destroy(&mapper);
			if (mapper)
				++errors, printf("Test14: mapper_destroy(&mapper) failed (%p, not NULL)", (void *)mapper);

			CHECK_LIST(15, "mapper_has_next/mapper_next", keys)
			list_destroy(&keys);
		}

		/* Test mapper_next_mapping, mapping_key, mapping_value */

		TEST_ACT(16, mapper = mapper_create(map))
		else
		{
			TEST_ACT(16, keys = list_create(NULL))
			TEST_ACT(16, values = list_create(NULL))

			while (mapper_has_next(mapper) == 1)
			{
				const Mapping *mapping = mapper_next_mapping(mapper);

				if (!mapping)
					++errors, printf("Test17: mapper_next_mapping() failed\n");
				else
				{
					TEST_ACT(17, ckey = mapping_key((void *)mapping))
					else TEST_ACT(17, list_append(keys, (void *)ckey))
					TEST_ACT(17, cvalue = mapping_value(mapping))
					else TEST_ACT(17, list_append(values, (void *)cvalue))
				}
			}

			mapper_destroy(&mapper);
			if (mapper)
				++errors, printf("Test18: mapper_destroy(&mapper) failed (%p, not NULL)", (void *)mapper);

			CHECK_LIST(19, "mapper_has_next/mapper_next_mapping", keys)
			list_destroy(&keys);
			CHECK_LIST(20, "mapper_has_next/mapper_next_mapping", values)
			list_destroy(&values);
		}

		/* Test map_has_next, map_next */

		TEST_ACT(21, keys = list_create(NULL))

		while (map_has_next(map) == 1)
		{
			void *item;

			TEST_ACT(22, item = map_next(map))
			else TEST_ACT(22, list_append(keys, item))
		}

		CHECK_LIST(23, "map_has_next/map_next", keys)
		list_destroy(&keys);

		/* Test map_next_mapping */

		TEST_ACT(24, keys = list_create(NULL))
		TEST_ACT(24, values = list_create(NULL))

		while (map_has_next(map) == 1)
		{
			const Mapping *mapping;

			TEST_ACT(25, mapping = map_next_mapping(map))
			else
			{
				TEST_ACT(25, ckey = mapping_key(mapping))
				else TEST_ACT(25, list_append(keys, (void *)ckey))
				TEST_ACT(25, cvalue = mapping_value(mapping))
				else TEST_ACT(25, list_append(values, (void *)cvalue))
			}
		}

		CHECK_LIST(26, "map_has_next/map_next_mapping", keys)
		list_destroy(&keys);
		CHECK_LIST(27, "map_has_next/map_next_mapping", values)
		list_destroy(&values);

		/* Test map_keys */

		TEST_ACT(28, keys = map_keys(map))
		else
		{
			CHECK_LIST(29, "map_keys", keys)
			list_destroy(&keys);
		}

		/* Test map_values */

		TEST_ACT(30, values = map_values(map))
		else
		{
			CHECK_LIST(31, "map_values", values)
			list_destroy(&values);
		}

		/* Test map_remove */

		TEST_ACT(32, map_remove(map, "zzz") == -1)
		TEST_ACT(33, map_remove(map, "abc") != -1)
		TEST_ACT(34, map_remove(map, "def") != -1)
		TEST_ACT(35, map_remove(map, "ghi") != -1)
		TEST_ACT(36, map_remove(map, "jkl") != -1)

		map_destroy(&map);
		if (map)
			++errors, printf("Test37: map_destroy(&map) failed (%p, not NULL)\n", (void *)map);
	}

	/* Test map_apply */

	TEST_ACT(38, map = map_create(NULL))
	else
	{
		TEST_ACT(39, map_add(map, "1", "7") == 0)
		TEST_ACT(40, map_add(map, "2", "6") == 0)
		TEST_ACT(41, map_add(map, "3", "5") == 0)
		TEST_ACT(42, map_add(map, "4", "4") == 0)
		TEST_ACT(43, map_add(map, "5", "3") == 0)
		TEST_ACT(44, map_add(map, "6", "2") == 0)
		TEST_ACT(45, map_add(map, "7", "1") == 0)

		CHECK_ITEM(46, map_get(map, "1"), "1", "7")
		CHECK_ITEM(47, map_get(map, "2"), "2", "6")
		CHECK_ITEM(48, map_get(map, "3"), "3", "5")
		CHECK_ITEM(49, map_get(map, "4"), "4", "4")
		CHECK_ITEM(50, map_get(map, "5"), "5", "3")
		CHECK_ITEM(51, map_get(map, "6"), "6", "2")
		CHECK_ITEM(52, map_get(map, "7"), "7", "1")

		cat[0] = nul;
		map_apply(map, (map_action_t *)test_action, cat);
		if (strcmp(cat, "7=1, 1=7, 2=6, 3=5, 4=4, 5=3, 6=2"))
			++errors, printf("Test53: map_apply(cat) failed (cat = \"%s\", not \"%s\")\n", cat, "7=1, 1=7, 2=6, 3=5, 4=4, 5=3, 6=2");

		map_destroy(&map);
		if (map)
			++errors, printf("Test54: map_destroy(&map) failed (%p, not NULL)\n", (void *)map);
	}

	/* Test mapper_remove, map_size */

	TEST_ACT(55, map = map_create(NULL))
	else
	{
		TEST_ACT(56, map_add(map, "1", "7") == 0)
		TEST_ACT(57, map_add(map, "2", "6") == 0)
		TEST_ACT(58, map_add(map, "3", "5") == 0)
		TEST_ACT(59, map_add(map, "4", "4") == 0)
		TEST_ACT(60, map_add(map, "5", "3") == 0)
		TEST_ACT(61, map_add(map, "6", "2") == 0)
		TEST_ACT(62, map_add(map, "7", "1") == 0)

		TEST_ACT(63, mapper = mapper_create(map))
		else
		{
			while (mapper_has_next(mapper) == 1)
			{
				void *item;

				TEST_ACT(64, item = mapper_next(mapper))
				mapper_remove(mapper);
			}

			mapper_destroy(&mapper);
			if (mapper)
				++errors, printf("Test65: mapper_destroy(&mapper) failed (%p, not NULL)\n", (void *)mapper);

			TEST_EQ(66, map_size(map), 0)
		}

		map_destroy(&map);
		if (map)
			++errors, printf("Test67: map_destroy(&map) failed (%p, not NULL)\n", (void *)map);
	}

	/* Test map_remove_current */

	TEST_ACT(68, map = map_create(NULL))
	else
	{
		TEST_ACT(69, map_add(map, "1", "1") == 0)
		TEST_ACT(70, map_add(map, "2", "2") == 0)
		TEST_ACT(71, map_add(map, "3", "3") == 0)
		TEST_ACT(72, map_add(map, "4", "4") == 0)

		while (map_has_next(map) == 1)
		{
			void *item;

			TEST_ACT(73, item = map_next(map))
			map_remove_current(map);
		}

		TEST_EQ(74, map_size(map), 0)
		mapper_destroy(&mapper);
		map_destroy(&map);
	}

	/* Test map_create_generic (Point -> char *) */

#define ADD_POINT(i, xpos, ypos, value, rc) \
	point->x = (xpos); \
	point->y = (ypos); \
	TEST_ACT((i), map_add(map, point, (value)) == (rc))

#define GET_POINT(i, xpos, ypos, str) \
	point->x = (xpos); \
	point->y = (ypos); \
	if (!(value = map_get(map, point))) \
		++errors, printf("Test%d: map_get(generic) failed\n", (i)); \
	else if (strcmp(value, (str))) \
		++errors, printf("Test%d: map_get(generic) failed (\"%s\", not \"%s\")\n", (i), value, (str));

	TEST_ACT(75, map = map_create_generic((map_copy_t *)point_copy, (map_cmp_t *)point_cmp, (map_hash_t *)point_hash, (map_release_t *)point_release, NULL))
	else
	{
		Point *point = point_create(0, 0);

		ADD_POINT(76, 0, 0, "(0, 0)", 0)
		ADD_POINT(77, 1, 0, "(1, 0)", 0)
		ADD_POINT(78, 0, 1, "(0, 1)", 0)
		ADD_POINT(79, 1, 1, "(1, 1)", 0)
		ADD_POINT(80, -1, 0, "(-1, 0)", 0)
		ADD_POINT(81, 0, -1, "(0, -1)", 0)
		ADD_POINT(82, -1, -1, "(-1, -1)", 0)
		ADD_POINT(83, 2, 0, "(2, 0)", 0)
		ADD_POINT(84, 0, 2, "(0, 2)", 0)
		ADD_POINT(85, 2, 2, "(2, 2)", 0)
		ADD_POINT(86, -2, 0, "(-2, 0)", 0)
		ADD_POINT(87, 0, -2, "(0, -2)", 0)
		ADD_POINT(88, -2, -2, "(-2, -2)", 0)
		ADD_POINT(89, 0, 0, "(0, 0)", -1)
		TEST_EQ(90, map_size(map), 13)

		GET_POINT(91, 0, 0, "(0, 0)")
		GET_POINT(92, 1, 0, "(1, 0)")
		GET_POINT(93, 0, 1, "(0, 1)")
		GET_POINT(94, 1, 1, "(1, 1)")
		GET_POINT(95, -1, 0, "(-1, 0)")
		GET_POINT(96, 0, -1, "(0, -1)")
		GET_POINT(97, -1, -1, "(-1, -1)")
		GET_POINT(98, 2, 0, "(2, 0)")
		GET_POINT(99, 0, 2, "(0, 2)")
		GET_POINT(100, 2, 2, "(2, 2)")
		GET_POINT(101, -2, 0, "(-2, 0)")
		GET_POINT(102, 0, -2, "(0, -2)")
		GET_POINT(103, -2, -2, "(-2, -2)")
		point_release(point);
		map_destroy(&map);
	}

	/* Test map_create_generic (int -> int) and map growth */

	TEST_ACT(104, map = map_create_generic((map_copy_t *)direct_copy, (map_cmp_t *)direct_cmp, (map_hash_t *)direct_hash, NULL, NULL))
	else
	{
		TEST_ACT(105, map_add(map, (void *)1, (void *)1) == 0)
		TEST_ACT(106, map_add(map, (void *)2, (void *)2) == 0)
		TEST_ACT(107, map_add(map, (void *)3, (void *)3) == 0)
		TEST_ACT(108, map_add(map, (void *)4, (void *)4) == 0)
		TEST_ACT(109, map_add(map, (void *)5, (void *)5) == 0)
		TEST_ACT(110, map_add(map, (void *)6, (void *)6) == 0)
		TEST_ACT(111, map_add(map, (void *)7, (void *)7) == 0)
		TEST_ACT(112, map_add(map, (void *)8, (void *)8) == 0)
		TEST_ACT(113, map_add(map, (void *)9, (void *)9) == 0)
		TEST_ACT(114, map_add(map, (void *)10, (void *)10) == 0)
		TEST_ACT(115, map_add(map, (void *)11, (void *)11) == 0)
		TEST_ACT(116, map_add(map, (void *)12, (void *)12) == 0)
		TEST_ACT(117, map_add(map, (void *)13, (void *)13) == 0)
		TEST_ACT(118, map_add(map, (void *)14, (void *)14) == 0)
		TEST_ACT(119, map_add(map, (void *)15, (void *)15) == 0)
		TEST_ACT(120, map_add(map, (void *)16, (void *)16) == 0)
		TEST_ACT(121, map_add(map, (void *)17, (void *)17) == 0)
		TEST_ACT(122, map_add(map, (void *)18, (void *)18) == 0)
		TEST_ACT(123, map_add(map, (void *)19, (void *)19) == 0)
		TEST_ACT(124, map_add(map, (void *)20, (void *)20) == 0)
		TEST_ACT(125, map_add(map, (void *)21, (void *)21) == 0)
		TEST_ACT(126, map_add(map, (void *)22, (void *)22) == 0)
		TEST_ACT(127, map_add(map, (void *)23, (void *)23) == 0)
		TEST_ACT(128, map_add(map, (void *)24, (void *)24) == 0)
		TEST_ACT(129, map_add(map, (void *)25, (void *)25) == 0)
		TEST_ACT(130, map_add(map, (void *)25, (void *)25) != 0)
		TEST_EQ(130, map_size(map), 25)

		TEST_EQ(131, (int)(long)map_get(map, (void *)1), 1)
		TEST_EQ(132, (int)(long)map_get(map, (void *)2), 2)
		TEST_EQ(133, (int)(long)map_get(map, (void *)3), 3)
		TEST_EQ(134, (int)(long)map_get(map, (void *)4), 4)
		TEST_EQ(135, (int)(long)map_get(map, (void *)5), 5)
		TEST_EQ(136, (int)(long)map_get(map, (void *)6), 6)
		TEST_EQ(137, (int)(long)map_get(map, (void *)7), 7)
		TEST_EQ(138, (int)(long)map_get(map, (void *)8), 8)
		TEST_EQ(139, (int)(long)map_get(map, (void *)9), 9)
		TEST_EQ(140, (int)(long)map_get(map, (void *)10), 10)
		TEST_EQ(141, (int)(long)map_get(map, (void *)11), 11)
		TEST_EQ(142, (int)(long)map_get(map, (void *)12), 12)
		TEST_EQ(143, (int)(long)map_get(map, (void *)13), 13)
		TEST_EQ(144, (int)(long)map_get(map, (void *)14), 14)
		TEST_EQ(145, (int)(long)map_get(map, (void *)15), 15)
		TEST_EQ(146, (int)(long)map_get(map, (void *)16), 16)
		TEST_EQ(147, (int)(long)map_get(map, (void *)17), 17)
		TEST_EQ(148, (int)(long)map_get(map, (void *)18), 18)
		TEST_EQ(149, (int)(long)map_get(map, (void *)19), 19)
		TEST_EQ(150, (int)(long)map_get(map, (void *)20), 20)
		TEST_EQ(151, (int)(long)map_get(map, (void *)21), 21)
		TEST_EQ(152, (int)(long)map_get(map, (void *)22), 22)
		TEST_EQ(153, (int)(long)map_get(map, (void *)23), 23)
		TEST_EQ(154, (int)(long)map_get(map, (void *)24), 24)
		TEST_EQ(155, (int)(long)map_get(map, (void *)25), 25)
		map_destroy(&map);
	}

	/* Test MT Safety */

	debug = ac == 2 && !strcmp(av[1], "debug");

	if (debug)
		setbuf(stdout, NULL);

#ifndef PTHREAD_RWLOCK_INITIALIZER
	pthread_rwlock_init(&rwlock, NULL);
#endif

	if (debug)
		locker = locker_create_debug_rwlock(&rwlock);
	else
		locker = locker_create_rwlock(&rwlock);

	if (!locker)
		++errors, printf("Test220: locker_create_rwlock() failed\n");
	else
	{
		mt_test(220, locker);
		locker_destroy(&locker);
	}

	if (debug)
		locker = locker_create_debug_mutex(&mutex);
	else
		locker = locker_create_mutex(&mutex);

	if (!locker)
		++errors, printf("Test221: locker_create_mutex() failed\n");
	else
	{
		mt_test(221, locker);
		locker_destroy(&locker);
	}

	/* Test assumption: sizeof(int) <= sizeof(void *) */

	if (sizeof(int) > sizeof(void *))
		++errors, printf("Test 222: assumption failed: sizeof(int) > sizeof(void *): int maps are limited to %d bytes\n", (int)sizeof(void *));

	/* Test assumption: memset(&ptr, 0, sizeof(void *)) same as NULL */

	memset(&ptr, 0, sizeof(void *));
	if (ptr != NULL)
		++errors, printf("Test223: assumption failed: memset(&ptr, 0, sizeof(void *)) not same as NULL\n");

	if (errors)
		printf("%d/223 tests failed\n", errors);
	else
		printf("All tests passed\n");

	return (errors == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

#endif

/* vi:set ts=4 sw=4: */
