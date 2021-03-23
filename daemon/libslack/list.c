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

I<libslack(list)> - list module

=head1 SYNOPSIS

    #include <slack/std.h>
    #include <slack/list.h>

    typedef struct List List;
    typedef struct Lister Lister;
    typedef void list_release_t(void *item);
    typedef void *list_copy_t(const void *item);
    typedef int list_cmp_t(const void *a, const void *b);
    typedef void list_action_t(void *item, size_t *index, void *data);
    typedef void *list_map_t(void *item, size_t *index, void *data);
    typedef int list_query_t(void *item, size_t *index, void *data);

    List *list_create(list_release_t *destroy);
    List *list_make(list_release_t *destroy, ...);
    List *list_vmake(list_release_t *destroy, va_list args);
    List *list_copy(const List *src, list_copy_t *copy);
    List *list_create_with_locker(Locker *locker, list_release_t *destroy);
    List *list_make_with_locker(Locker *locker, list_release_t *destroy, ...);
    List *list_vmake_with_locker(Locker *locker, list_release_t *destroy, va_list args);
    List *list_copy_with_locker(Locker *locker, const List *src, list_copy_t *copy);
    int list_rdlock(const List *list);
    int list_wrlock(const List *list);
    int list_unlock(const List *list);
    void list_release(List *list);
    void *list_destroy(List **list);
    int list_own(List *list, list_release_t *destroy);
    int list_own_unlocked(List *list, list_release_t *destroy);
    list_release_t *list_disown(List *list);
    list_release_t *list_disown_unlocked(List *list);
    void *list_item(const List *list, ssize_t index);
    void *list_item_unlocked(const List *list, ssize_t index);
    int list_item_int(const List *list, ssize_t index);
    int list_item_int_unlocked(const List *list, ssize_t index);
    int list_empty(const List *list);
    int list_empty_unlocked(const List *list);
    ssize_t list_length(const List *list);
    ssize_t list_length_unlocked(const List *list);
    ssize_t list_last(const List *list);
    ssize_t list_last_unlocked(const List *list);
    List *list_remove(List *list, ssize_t index);
    List *list_remove_unlocked(List *list, ssize_t index);
    List *list_remove_range(List *list, ssize_t index, ssize_t range);
    List *list_remove_range_unlocked(List *list, ssize_t index, ssize_t range);
    List *list_insert(List *list, ssize_t index, void *item);
    List *list_insert_unlocked(List *list, ssize_t index, void *item);
    List *list_insert_int(List *list, ssize_t index, int item);
    List *list_insert_int_unlocked(List *list, ssize_t index, int item);
    List *list_insert_list(List *list, ssize_t index, const List *src, list_copy_t *copy);
    List *list_insert_list_unlocked(List *list, ssize_t index, const List *src, list_copy_t *copy);
    List *list_append(List *list, void *item);
    List *list_append_unlocked(List *list, void *item);
    List *list_append_int(List *list, int item);
    List *list_append_int_unlocked(List *list, int item);
    List *list_append_list(List *list, const List *src, list_copy_t *copy);
    List *list_append_list_unlocked(List *list, const List *src, list_copy_t *copy);
    List *list_prepend(List *list, void *item);
    List *list_prepend_unlocked(List *list, void *item);
    List *list_prepend_int(List *list, int item);
    List *list_prepend_int_unlocked(List *list, int item);
    List *list_prepend_list(List *list, const List *src, list_copy_t *copy);
    List *list_prepend_list_unlocked(List *list, const List *src, list_copy_t *copy);
    List *list_replace(List *list, ssize_t index, ssize_t range, void *item);
    List *list_replace_unlocked(List *list, ssize_t index, ssize_t range, void *item);
    List *list_replace_int(List *list, ssize_t index, ssize_t range, int item);
    List *list_replace_int_unlocked(List *list, ssize_t index, ssize_t range, int item);
    List *list_replace_list(List *list, ssize_t index, ssize_t range, const List *src, list_copy_t *copy);
    List *list_replace_list_unlocked(List *list, ssize_t index, ssize_t range, const List *src, list_copy_t *copy);
    List *list_extract(const List *list, ssize_t index, ssize_t range, list_copy_t *copy);
    List *list_extract_unlocked(const List *list, ssize_t index, ssize_t range, list_copy_t *copy);
    List *list_extract_with_locker(Locker *locker, const List *list, ssize_t index, ssize_t range, list_copy_t *copy);
    List *list_extract_with_locker_unlocked(Locker *locker, const List *list, ssize_t index, ssize_t range, list_copy_t *copy);
    List *list_push(List *list, void *item);
    List *list_push_unlocked(List *list, void *item);
    List *list_push_int(List *list, int item);
    List *list_push_int_unlocked(List *list, int item);
    void *list_pop(List *list);
    void *list_pop_unlocked(List *list);
    int list_pop_int(List *list);
    int list_pop_int_unlocked(List *list);
    void *list_shift(List *list);
    void *list_shift_unlocked(List *list);
    int list_shift_int(List *list);
    int list_shift_int_unlocked(List *list);
    List *list_unshift(List *list, void *item);
    List *list_unshift_unlocked(List *list, void *item);
    List *list_unshift_int(List *list, int item);
    List *list_unshift_int_unlocked(List *list, int item);
    List *list_splice(List *list, ssize_t index, ssize_t range, list_copy_t *copy);
    List *list_splice_unlocked(List *list, ssize_t index, ssize_t range, list_copy_t *copy);
    List *list_splice_with_locker(Locker *locker, List *list, ssize_t index, ssize_t range, list_copy_t *copy);
    List *list_splice_with_locker_unlocked(Locker *locker, List *list, ssize_t index, ssize_t range, list_copy_t *copy);
    List *list_sort(List *list, list_cmp_t *cmp);
    List *list_sort_unlocked(List *list, list_cmp_t *cmp);
    void list_apply(List *list, list_action_t *action, void *data);
    void list_apply_rdlocked(List *list, list_action_t *action, void *data);
    void list_apply_wrlocked(List *list, list_action_t *action, void *data);
    void list_apply_unlocked(List *list, list_action_t *action, void *data);
    List *list_map(List *list, list_release_t *destroy, list_map_t *map, void *data);
    List *list_map_unlocked(List *list, list_release_t *destroy, list_map_t *map, void *data);
    List *list_map_with_locker(Locker *locker, List *list, list_release_t *destroy, list_map_t *map, void *data);
    List *list_map_with_locker_unlocked(Locker *locker, List *list, list_release_t *destroy, list_map_t *map, void *data);
    List *list_grep(List *list, list_query_t *grep, void *data);
    List *list_grep_unlocked(List *list, list_query_t *grep, void *data);
    List *list_grep_with_locker(Locker *locker, List *list, list_query_t *grep, void *data);
    List *list_grep_with_locker_unlocked(Locker *locker, List *list, list_query_t *grep, void *data);
    ssize_t list_query(List *list, ssize_t *index, list_query_t *query, void *data);
    ssize_t list_query_unlocked(List *list, ssize_t *index, list_query_t *query, void *data);
    Lister *lister_create(List *list);
    Lister *lister_create_rdlocked(List *list);
    Lister *lister_create_wrlocked(List *list);
    Lister *lister_create_unlocked(const List *list);
    void lister_release(Lister *lister);
    void lister_release_unlocked(Lister *lister);
    void *lister_destroy(Lister **lister);
    void *lister_destroy_unlocked(Lister **lister);
    int lister_has_next(Lister *lister);
    void *lister_next(Lister *lister);
    int lister_next_int(Lister *lister);
    void lister_remove(Lister *lister);
    int list_has_next(List *list);
    void list_break(List *list);
    void *list_next(List *list);
    int list_next_int(List *list);
    void list_remove_current(List *list);

=head1 DESCRIPTION

This module provides functions for manipulating and iterating over lists of
homogeneous data (or heterogeneous data if it's polymorphic). I<List>s may
own their items. I<List>s created with a non-C<null> destroy function use
that function to destroy an item when it is removed from the list and to
destroy each item when the list itself is destroyed. Be careful not to
insert items owned by one list into a list that doesn't own its own items
unless you know that the source list (and all of the shared items) will
outlive the destination list.

=over 4

=cut

*/

#include "config.h"
#include "std.h"

#include "list.h"
#include "mem.h"
#include "err.h"
#include "hsort.h"
#include "locker.h"

#define xor(a, b) (!(a) ^ !(b))
#define iff(a, b) !xor(a, b)
#define implies(a, b) (!(a) || (b))

struct List
{
	size_t size;             /* number of item slots allocated */
	size_t length;           /* number of items used */
	void **list;             /* vector of items (void *) */
	list_release_t *destroy; /* item destructor, if any */
	Lister *lister;          /* built-in iterator */
	Locker *locker;          /* locking strategy for this object */
};

struct Lister
{
	List *list;              /* the list being iterated over */
	ssize_t index;           /* the index of the current item */
};

#ifndef TEST

/* Minimum list length: must be a power of 2 */

static const size_t MIN_LIST_SIZE = 4;

/*

C<int grow(List *list, size_t items)>

Allocates enough memory to add C<item> extra items to C<list> if necessary.
On success, returns C<0>. On error, returns C<-1>.

*/

static int grow(List *list, size_t items)
{
	int grown = 0;

	while (list->length + items > list->size)
	{
		if (list->size)
			list->size <<= 1;
		else
			list->size = MIN_LIST_SIZE;

		grown = 1;
	}

	if (grown)
		return mem_resize(&list->list, list->size) ? 0 : -1;

	return 0;
}

/*

C<int shrink(List *list, size_t items)>

Allocates less memory for removing C<items> items from C<list> if necessary.
On success, returns C<0>. On error, returns C<-1>.

*/

static int shrink(List *list, size_t items)
{
	int shrunk = 0;

	while (list->length - items < list->size >> 1)
	{
		if (list->size == MIN_LIST_SIZE)
			break;

		list->size >>= 1;
		shrunk = 1;
	}

	if (shrunk)
		return mem_resize(&list->list, list->size) ? 0 : -1;

	return 0;
}

/*

C<int expand(List *list, ssize_t index, size_t range)>

Slides C<list>'s items, starting at C<index>, C<range> slots to the right to
make room for more. On success, returns C<0>. On error, returns C<-1>.

*/

static int expand(List *list, ssize_t index, size_t range)
{
	if (grow(list, range) == -1)
		return -1;

	memmove(list->list + index + range, list->list + index, (list->length - index) * sizeof(*list->list));
	list->length += range;

	return 0;
}

/*

C<int contract(List *list, ssize_t index, size_t range)>

Slides C<list>'s items, starting at C<index> + C<range>, C<range> positions
to the left to close a gap starting at C<index>. On success, returns C<0>.
On error, returns C<-1>.

*/

static int contract(List *list, ssize_t index, size_t range)
{
	memmove(list->list + index, list->list + index + range, (list->length - index - range) * sizeof(*list->list));

	if (shrink(list, range) == -1)
		return -1;

	list->length -= range;

	return 0;
}

/*

C<int adjust(List *list, ssize_t index, size_t range, size_t length)>

Expands or contracts C<list> as required so that C<list[index + range ..]>
occupies C<list[index + length ..]>. On success, returns C<0>. On error,
returns C<-1>.

*/

static int adjust(List *list, ssize_t index, size_t range, size_t length)
{
	if (range < length)
		return expand(list, index + range, length - range);

	if (range > length)
		return contract(list, index + length, range - length);

	return 0;
}

/*

C<void killitems(List *list, size_t index, size_t range)>

Destroys the items in C<list> ranging from C<index> to C<range>.

*/

static void killitems(List *list, size_t index, size_t range)
{
	while (range--)
	{
		if (list->destroy)
			list->destroy(list->list[index]);
		list->list[index++] = NULL;
	}
}

/*

=item C<List *list_create(list_release_t *destroy)>

Creates a I<List> with C<destroy> as its item destructor. It is the caller's
responsibility to deallocate the new list with I<list_release(3)> or
I<list_destroy(3)>. On success, returns the new list. On error, returns
C<null> with C<errno> set appropriately.

=cut

*/

List *list_create(list_release_t *destroy)
{
	return list_create_with_locker(NULL, destroy);
}

/*

=item C<List *list_make(list_release_t *destroy, ...)>

Creates a I<List> with C<destroy> as its item destructor and the remaining
arguments as its initial items. It is the caller's responsibility to
deallocate the new list with I<list_release(3)> or I<list_destroy(3)>. On
success, returns the new list. On error, return C<null> with C<errno> set
appropriately.

=cut

*/

List *list_make(list_release_t *destroy, ...)
{
	List *list;
	va_list args;
	va_start(args, destroy);
	list = list_vmake_with_locker(NULL, destroy, args);
	va_end(args);
	return list;
}

/*

=item C<List *list_vmake(list_release_t *destroy, va_list args)>

Equivalent to I<list_make(3)> with the variable argument list specified
directly as for I<vprintf(3)>.

=cut

*/

List *list_vmake(list_release_t *destroy, va_list args)
{
	return list_vmake_with_locker(NULL, destroy, args);
}

/*

=item C<List *list_copy(const List *src, list_copy_t *copy)>

Creates a copy of C<src> using C<copy> as the copy constructor (if not
C<null>). It is the caller's responsibility to deallocate the new list with
I<list_release(3)> or I<list_destroy(3)>. On success, returns the new copy.
On error, returns C<null> with C<errno> set appropriately.

=cut

*/

List *list_copy(const List *src, list_copy_t *copy)
{
	return list_copy_with_locker(NULL, src, copy);
}

/*

=item C<List *list_create_with_locker(Locker *locker, list_release_t *destroy)>

Equivalent to I<list_create(3)> except that multiple threads accessing the
new list will be synchronised by C<locker>.

=cut

*/

List *list_create_with_locker(Locker *locker, list_release_t *destroy)
{
	List *list;

	if (!(list = mem_new(List))) /* XXX decouple */
		return NULL;

	list->size = list->length = 0;
	list->list = NULL;
	list->destroy = destroy;
	list->lister = NULL;
	list->locker = locker;

	return list;
}

/*

=item C<List *list_make_with_locker(Locker *locker, list_release_t *destroy, ...)>

Equivalent to I<list_make(3)> except that multiple threads accessing the new
list will be synchronised by C<locker>.

=cut

*/

List *list_make_with_locker(Locker *locker, list_release_t *destroy, ...)
{
	List *list;
	va_list args;
	va_start(args, destroy);
	list = list_vmake_with_locker(locker, destroy, args);
	va_end(args);
	return list;
}

/*

=item C<List *list_vmake_with_locker(Locker *locker, list_release_t *destroy, va_list args)>

Equivalent to I<list_vmake(3)> except that multiple threads accessing the
new list will be synchronised by C<locker>.

=cut

*/

List *list_vmake_with_locker(Locker *locker, list_release_t *destroy, va_list args)
{
	List *list;
	void *item;

	if (!(list = list_create_with_locker(locker, destroy)))
		return NULL;

	while ((item = va_arg(args, void *)) != NULL)
	{
		if (!list_append(list, item))
		{
			list_release(list);
			return NULL;
		}
	}

	return list;
}

/*

=item C<List *list_copy_with_locker(Locker *locker, const List *src, list_copy_t *copy)>

Equivalent to I<list_copy(3)> except that multiple threads accessing the new
list will be synchronised by C<locker>.

=cut

*/

List *list_copy_with_locker(Locker *locker, const List *src, list_copy_t *copy)
{
	List *list;

	if (!src)
		return set_errnull(EINVAL);

	if (!(list = list_extract(src, 0, src->length, copy)))
		return NULL;

	list->locker = locker;

	return list;
}

/*

=item C<int list_rdlock(const List *list)>

Claims a read lock on C<list> (if C<list> was created with a I<Locker>).
This is needed when multiple read only I<list(3)> module functions need to
be called atomically. It is the client's responsibility to call
I<list_unlock(3)> after the atomic operation. The only functions that may be
called on C<list> between calls to I<list_rdlock(3)> and I<list_unlock(3)>
are any read only I<list(3)> module functions whose name ends with
C<_unlocked>. On success, returns C<0>. On error, returns an error code.

=cut

*/

#define list_rdlock(list) ((list) ? locker_rdlock((list)->locker) : EINVAL)
#define list_wrlock(list) ((list) ? locker_wrlock((list)->locker) : EINVAL)
#define list_unlock(list) ((list) ? locker_unlock((list)->locker) : EINVAL)

int (list_rdlock)(const List *list)
{
	return list_rdlock(list);
}

/*

=item C<int list_wrlock(const List *list)>

Claims a write lock on C<list> (if C<list> was created with a I<Locker>).
This is needed when multiple read/write I<list(3)> module functions need to
be called atomically. It is the client's responsibility to call
I<list_unlock(3)> after the atomic operation. The only functions that may be
called on C<list> between calls to I<list_wrlock(3)> and I<list_unlock(3)>
are any I<list(3)> module functions whose name ends with C<_unlocked>. On
success, returns C<0>. On error, returns an error code.

=cut

*/

int (list_wrlock)(const List *list)
{
	return list_wrlock(list);
}

/*

=item C<int list_unlock(const List *list)>

Unlocks a read or write lock on C<list> obtained with I<list_rdlock(3)> or
I<list_wrlock(3)> (if C<list> was created with a C<locker>). On success,
returns C<0>. On error, returns an error code.

=cut

*/

int (list_unlock)(const List *list)
{
	return list_unlock(list);
}

/*

=item C<void list_release(List *list)>

Releases (deallocates) C<list>, destroying its items if necessary. On error,
sets C<errno> appropriately.

=cut

*/

void list_release(List *list)
{
	if (!list)
		return;

	if (list->list)
	{
		killitems(list, 0, list->length);
		mem_release(list->list);
	}

	mem_release(list);
}

/*

=item C<void *list_destroy(List **list)>

Destroys (deallocates and sets to C<null>) C<*list>. Returns C<null>.
B<Note:> lists shared by multiple threads must not be destroyed until after
all threads have finished with it.

=cut

*/

void *list_destroy(List **list)
{
	if (list && *list)
	{
		list_release(*list);
		*list = NULL;
	}

	return NULL;
}

/*

=item C<int list_own(List *list, list_release_t *destroy)>

Causes C<list> to take ownership of its items. The items will be destroyed
using C<destroy> when they are removed or when C<list> is destroyed. On
success, returns C<0>. On error, returns C<-1> with C<errno> set
appropriately.

=cut

*/

int list_own(List *list, list_release_t *destroy)
{
	int err;

	if (!list || !destroy)
		return set_errno(EINVAL);

	if ((err = list_wrlock(list)))
		return set_errno(err);

	list->destroy = destroy;

	if ((err = list_unlock(list)))
		return set_errno(err);

	return 0;
}

/*

=item C<int list_own_unlocked(List *list, list_release_t *destroy)>

Equivalent to I<list_own(3)> except that C<list> is not write locked.

=cut

*/

int list_own_unlocked(List *list, list_release_t *destroy)
{
	if (!list || !destroy)
		return set_errno(EINVAL);

	list->destroy = destroy;

	return 0;
}

/*

=item C<list_release_t *list_disown(List *list)>

Causes C<list> to relinquish ownership of its items. The items will not be
destroyed when they are removed from C<list> or when C<list> is destroyed.
On success, returns the previous destroy function, if any. On error, returns
C<null> with C<errno> set appropriately.

=cut

*/

list_release_t *list_disown(List *list)
{
	list_release_t *destroy;
	int err;

	if (!list)
		return (list_release_t *)set_errnullf(EINVAL);

	if ((err = list_wrlock(list)))
		return (list_release_t *)set_errnullf(err);

	destroy = list_disown_unlocked(list);

	if ((err = list_unlock(list)))
		return (list_release_t *)set_errnullf(err);

	return destroy;
}

/*

=item C<list_release_t *list_disown_unlocked(List *list)>

Equivalent to I<list_disown(3)> except that C<list> is not write locked.

=cut

*/

list_release_t *list_disown_unlocked(List *list)
{
	list_release_t *destroy;

	if (!list)
		return (list_release_t *)set_errnullf(EINVAL);

	destroy = list->destroy;
	list->destroy = NULL;

	return destroy;
}

/*

=item C<void *list_item(const List *list, ssize_t index)>

Returns the C<index>'th item in C<list>. If C<index> is negative, it refers
to an item position relative to the end of the list (C<-1> is the position
after the last item, C<-2> is the position of the last item and so on). On
error, returns C<null> with C<errno> set appropriately.

=cut

*/

void *list_item(const List *list, ssize_t index)
{
	void *item;
	int err;

	if (!list)
		return set_errnull(EINVAL);

	if ((err = list_rdlock(list)))
		return set_errnull(err);

	item = list_item_unlocked(list, index);

	if ((err = list_unlock(list)))
		return set_errnull(err);

	return item;
}

/*

=item C<void *list_item_unlocked(const List *list, ssize_t index)>

Equivalent to I<list_item(3)> except that C<list> is not read locked.

=cut

*/

void *list_item_unlocked(const List *list, ssize_t index)
{
	if (!list)
		return set_errnull(EINVAL);

	if (index < 0)
		index = list->length + 1 + index;

	if (index < 0)
		return set_errnull(EINVAL);

	if (index >= list->length)
		return set_errnull(EINVAL);

	return list->list[index];
}

/*

=item C<int list_item_int(const List *list, ssize_t index)>

Equivalent to I<list_item(3)> except that the item is cast to an integer
type.

=cut

*/

int list_item_int(const List *list, ssize_t index)
{
	return (int)(long)list_item(list, index);
}

/*

=item C<int list_item_int_unlocked(const List *list, ssize_t index)>

Equivalent to I<list_item_int(3)> except that C<list> is not read locked.

=cut

*/

int list_item_int_unlocked(const List *list, ssize_t index)
{
	return (int)(long)list_item_unlocked(list, index);
}

/*

=item C<int list_empty(const List *list)>

Returns whether or not C<list> is empty. On error, returns C<-1> with
C<errno> set appropriately.

=cut

*/

int list_empty(const List *list)
{
	int empty;
	int err;

	if (!list)
		return set_errno(EINVAL);

	if ((err = list_rdlock(list)))
		return set_errno(err);

	empty = (list->length == 0);

	if ((err = list_unlock(list)))
		return set_errno(err);

	return empty;
}

/*

=item C<int list_empty_unlocked(const List *list)>

Equivalent to I<list_empty(3)> except that C<list> is not read locked.

=cut

*/

int list_empty_unlocked(const List *list)
{
	if (!list)
		return set_errno(EINVAL);

	return (list->length == 0);
}

/*

=item C<ssize_t list_length(const List *list)>

Returns the length of C<list>. On error, returns C<-1> with C<errno> set
appropriately.

=cut

*/

ssize_t list_length(const List *list)
{
	size_t length;
	int err;

	if (!list)
		return set_errno(EINVAL);

	if ((err = list_rdlock(list)))
		return set_errno(err);

	length = list->length;

	if ((err = list_unlock(list)))
		return set_errno(err);

	return length;
}

/*

=item C<ssize_t list_length_unlocked(const List *list)>

Equivalent to I<list_length(3)> except that C<list> is not read locked.

=cut

*/

ssize_t list_length_unlocked(const List *list)
{
	if (!list)
		return set_errno(EINVAL);

	return list->length;
}

/*

=item C<ssize_t list_last(const List *list)>

Returns the index of the last item in C<list>, or C<-1> if there are no
items. On error, returns C<-1> with C<errno> set appropriately.

=cut

*/

ssize_t list_last(const List *list)
{
	ssize_t last;
	int err;

	if (!list)
		return set_errno(EINVAL);

	if ((err = list_rdlock(list)))
		return set_errno(err);

	last = list->length - 1;

	if ((err = list_unlock(list)))
		return set_errno(err);

	return last;
}

/*

=item C<ssize_t list_last_unlocked(const List *list)>

Equivalent to I<list_last(3)> except that C<list> is not read locked.

=cut

*/

ssize_t list_last_unlocked(const List *list)
{
	if (!list)
		return set_errno(EINVAL);

	return list->length - 1;
}

/*

=item C<List *list_remove(List *list, ssize_t index)>

Removes the C<index>'th item from C<list>. If C<index> is negative, it
refers to an item position relative to the end of the list (C<-1> is the
position after the last item, C<-2> is the position of the last item and so
on). On success, returns C<list>. On error, returns C<null> with C<errno>
set appropriately.

=cut

*/

List *list_remove(List *list, ssize_t index)
{
	return list_remove_range(list, index, 1);
}

/*

=item C<List *list_remove_unlocked(List *list, ssize_t index)>

Equivalent to I<list_remove(3)> except that C<list> is not write locked.

=cut

*/

List *list_remove_unlocked(List *list, ssize_t index)
{
	return list_remove_range_unlocked(list, index, 1);
}

/*

=item C<List *list_remove_range(List *list, ssize_t index, ssize_t range)>

Removes C<range> items from C<list> starting at C<index>. If C<index> or
C<range> are negative, they refer to item positions relative to the end of
the list (C<-1> is the position after the last item, C<-2> is the position
of the last item and so on). On success, returns C<list>. On error, returns
C<null> with C<errno> set appropriately.

=cut

*/

List *list_remove_range(List *list, ssize_t index, ssize_t range)
{
	List *ret;
	int err;

	if (!list)
		return set_errnull(EINVAL);

	if ((err = list_wrlock(list)))
		return set_errnull(err);

	ret = list_remove_range_unlocked(list, index, range);

	if ((err = list_unlock(list)))
		return set_errnull(err);

	return ret;
}

/*

=item C<List *list_remove_range_unlocked(List *list, ssize_t index, ssize_t range)>

Equivalent to I<list_remove_range(3)> except that C<list> is not write
locked.

=cut

*/

List *list_remove_range_unlocked(List *list, ssize_t index, ssize_t range)
{
	if (!list)
		return set_errnull(EINVAL);

	if (index < 0)
		index = list->length + 1 + index;

	if (index < 0)
		return set_errnull(EINVAL);

	if (range < 0)
		range = list->length + 1 + range - index;

	if (range < 0)
		return set_errnull(EINVAL);

	if (list->length < index + range)
		return set_errnull(EINVAL);

	killitems(list, index, range);

	if (contract(list, index, range) == -1)
		return NULL;

	return list;
}

/*

=item C<List *list_insert(List *list, ssize_t index, void *item)>

Adds C<item> to C<list> at position C<index>. If C<index> is negative, it
refers to an item position relative to the end of the list (C<-1> is the
position after the last item, C<-2> is the position of the last item and so
on). On success, returns C<list>. On error, returns C<null> with C<errno>
set appropriately.

=cut

*/

List *list_insert(List *list, ssize_t index, void *item)
{
	List *ret;
	int err;

	if (!list)
		return set_errnull(EINVAL);

	if ((err = list_wrlock(list)))
		return set_errnull(err);

	ret = list_insert_unlocked(list, index, item);

	if ((err = list_unlock(list)))
		return set_errnull(err);

	return ret;
}

/*

=item C<List *list_insert_unlocked(List *list, ssize_t index, void *item)>

Equivalent to I<list_insert(3)> except that C<list> is not write locked.

=cut

*/

List *list_insert_unlocked(List *list, ssize_t index, void *item)
{
	if (!list)
		return set_errnull(EINVAL);

	if (index < 0)
		index = list->length + 1 + index;

	if (index < 0)
		return set_errnull(EINVAL);

	if (list->length < index)
		return set_errnull(EINVAL);

	if (expand(list, index, 1) == -1)
		return NULL;

	list->list[index] = item;

	return list;
}

/*

=item C<List *list_insert_int(List *list, ssize_t index, int item)>

Equivalent to I<list_insert(3)> except that C<item> is an integer type.

=cut

*/

List *list_insert_int(List *list, ssize_t index, int item)
{
	return list_insert(list, index, (void *)(long)item);
}

/*

=item C<List *list_insert_int_unlocked(List *list, ssize_t index, int item)>

Equivalent to I<list_insert_int(3)> except that C<list> is not write locked.

=cut

*/

List *list_insert_int_unlocked(List *list, ssize_t index, int item)
{
	return list_insert_unlocked(list, index, (void *)(long)item);
}

/*

=item C<List *list_insert_list(List *list, ssize_t index, const List *src, list_copy_t *copy)>

Inserts the items from C<src> into C<list>, starting at position C<index>
using C<copy> as the copy constructor (if not C<null>). If C<index> is
negative, it refers to an item position relative to the end of the list
(C<-1> is the position after the last item, C<-2> is the position of the
last item and so on). On success, returns C<list>. On error, returns C<null>
with C<errno> set appropriately.

=cut

*/

List *list_insert_list(List *list, ssize_t index, const List *src, list_copy_t *copy)
{
	List *ret;
	int err;

	if (!src || !list)
		return set_errnull(EINVAL);

	if ((err = list_wrlock(list)))
		return set_errnull(err);

	if ((err = list_rdlock(src)))
	{
		list_unlock(list);
		return set_errnull(err);
	}

	ret = list_insert_list_unlocked(list, index, src, copy);

	if ((err = list_unlock(src)))
	{
		list_unlock(list);
		return set_errnull(err);
	}

	if ((err = list_unlock(list)))
		return set_errnull(err);

	return ret;
}

/*

=item C<List *list_insert_list_unlocked(List *list, ssize_t index, const List *src, list_copy_t *copy)>

Equivalent to I<list_insert_list(3)> except that C<list> is not write locked
and C<src> is not read locked. Note: If C<src> needs to be read locked, it
is the caller's responsibility to lock and unlock it explicitly with
I<list_rdlock(3)> and I<list_unlock(3)>.

=cut

*/

#define enlist(item, copy) ((copy) ? (copy)(item) : (item))

List *list_insert_list_unlocked(List *list, ssize_t index, const List *src, list_copy_t *copy)
{
	size_t i;

	if (!src || !list)
		return set_errnull(EINVAL);

	if (index < 0)
		index = list->length + 1 + index;

	if (index < 0)
		return set_errnull(EINVAL);

	if (list->length < index || xor(list->destroy, copy))
		return set_errnull(EINVAL);

	if (expand(list, index, src->length) == -1)
		return NULL;

	for (i = 0; i < src->length; ++i)
		list->list[index + i] = enlist(src->list[i], copy);

	return list;
}

/*

=item C<List *list_append(List *list, void *item)>

Appends C<item> to C<list>. On success, returns C<list>. On error, returns
C<null> with C<errno> set appropriately.

=cut

*/

List *list_append(List *list, void *item)
{
	return list_insert(list, -1, item);
}

/*

=item C<List *list_append_unlocked(List *list, void *item)>

Equivalent to I<list_append(3)> except that C<list> is not write locked.

=cut

*/

List *list_append_unlocked(List *list, void *item)
{
	return list_insert_unlocked(list, -1, item);
}

/*

=item C<List *list_append_int(List *list, int item)>

Equivalent to I<list_append(3)> except that C<item> is an integer.

=cut

*/

List *list_append_int(List *list, int item)
{
	return list_insert_int(list, -1, item);
}

/*

=item C<List *list_append_int_unlocked(List *list, int item)>

Equivalent to I<list_append_int(3)> except that C<list> is not write locked.

=cut

*/

List *list_append_int_unlocked(List *list, int item)
{
	return list_insert_int_unlocked(list, -1, item);
}

/*

=item C<List *list_append_list(List *list, const List *src, list_copy_t *copy)>

Appends the items in C<src> to C<list> using C<copy> as the copy constructor
(if not C<null>). On success, returns C<list>. On error, returns C<null>
with C<errno> set appropriately.

=cut

*/

List *list_append_list(List *list, const List *src, list_copy_t *copy)
{
	return list_insert_list(list, -1, src, copy);
}

/*

=item C<List *list_append_list_unlocked(List *list, const List *src, list_copy_t *copy)>

Equivalent to I<list_append_list(3)> except that C<list> is not write
locked.

=cut

*/

List *list_append_list_unlocked(List *list, const List *src, list_copy_t *copy)
{
	return list_insert_list_unlocked(list, -1, src, copy);
}

/*

=item C<List *list_prepend(List *list, void *item)>

Prepends C<item> to C<list>. On success, returns C<list>. On error, returns
C<null> with C<errno> set appropriately.

=cut

*/

List *list_prepend(List *list, void *item)
{
	return list_insert(list, 0, item);
}

/*

=item C<List *list_prepend_unlocked(List *list, void *item)>

Equivalent to I<list_prepend(3)> except that C<list> is not write locked.

=cut

*/

List *list_prepend_unlocked(List *list, void *item)
{
	return list_insert_unlocked(list, 0, item);
}

/*

=item C<List *list_prepend_int(List *list, int item)>

Equivalent to I<list_prepend(3)> except that C<item> is an integer.

=cut

*/

List *list_prepend_int(List *list, int item)
{
	return list_insert_int(list, 0, item);
}

/*

=item C<List *list_prepend_int_unlocked(List *list, int item)>

Equivalent to I<list_prepend_int(3)> except that C<list> is not write
locked.

=cut

*/

List *list_prepend_int_unlocked(List *list, int item)
{
	return list_insert_int_unlocked(list, 0, item);
}

/*

=item C<List *list_prepend_list(List *list, const List *src, list_copy_t *copy)>

Prepends the items in C<src> to C<list> using C<copy> as the copy
constructor (if not C<null>). On success, returns C<list>. On error, returns
C<null> with C<errno> set appropriately.

=cut

*/

List *list_prepend_list(List *list, const List *src, list_copy_t *copy)
{
	return list_insert_list(list, 0, src, copy);
}

/*

=item C<List *list_prepend_list_unlocked(List *list, const List *src, list_copy_t *copy)>

Equivalent to I<list_prepend_list(3)> except that C<list> is not write
locked.

=cut

*/

List *list_prepend_list_unlocked(List *list, const List *src, list_copy_t *copy)
{
	return list_insert_list_unlocked(list, 0, src, copy);
}

/*

=item C<List *list_replace(List *list, ssize_t index, ssize_t range, void *item)>

Replaces C<range> items in C<list>, starting at C<index>, with C<item>. If
C<index> or C<range> are negative, they refer to item positions relative to
the end of the list (C<-1> is the position after the last item, C<-2> is the
position of the last item and so on). On success, returns C<list>. On error,
returns C<null> with C<errno> set appropriately.

=cut

*/

List *list_replace(List *list, ssize_t index, ssize_t range, void *item)
{
	List *ret;
	int err;

	if (!list)
		return set_errnull(EINVAL);

	if ((err = list_wrlock(list)))
		return set_errnull(err);

	ret = list_replace_unlocked(list, index, range, item);

	if ((err = list_unlock(list)))
		return set_errnull(err);

	return ret;
}

/*

=item C<List *list_replace_unlocked(List *list, ssize_t index, ssize_t range, void *item)>

Equivalent to I<list_replace(3)> except that C<list> is not write locked.

=cut

*/

List *list_replace_unlocked(List *list, ssize_t index, ssize_t range, void *item)
{
	if (!list)
		return set_errnull(EINVAL);

	if (index < 0)
		index = list->length + 1 + index;

	if (index < 0)
		return set_errnull(EINVAL);

	if (range < 0)
		range = list->length + 1 + range - index;

	if (range < 0)
		return set_errnull(EINVAL);

	if (list->length < index + range)
		return set_errnull(EINVAL);

	killitems(list, index, range);

	if (adjust(list, index, range, 1) == -1)
		return NULL;

	list->list[index] = item;

	return list;
}

/*

=item C<List *list_replace_int(List *list, ssize_t index, ssize_t range, int item)>

Equivalent to I<list_replace(3)> except that C<item> is an integer type.

=cut

*/

List *list_replace_int(List *list, ssize_t index, ssize_t range, int item)
{
	return list_replace(list, index, range, (void *)(long)item);
}

/*

=item C<List *list_replace_int_unlocked(List *list, ssize_t index, ssize_t range, int item)>

Equivalent to I<list_replace_int(3)> except that C<list> is not write locked.

=cut

*/

List *list_replace_int_unlocked(List *list, ssize_t index, ssize_t range, int item)
{
	return list_replace_unlocked(list, index, range, (void *)(long)item);
}

/*

=item C<List *list_replace_list(List *list, ssize_t index, ssize_t range, const List *src, list_copy_t *copy)>

Replaces C<range> items in C<list>, starting at C<index>, with the items in
C<src> using C<copy> as the copy constructor (if not C<null>). If C<index>
or C<range> are negative, they refer to item positions relative to the end
of the list (C<-1> is the position after the last item, C<-2> is the
position of the last item and so on). On success, return C<list>. On error,
returns C<null> with C<errno> set appropriately.

=cut

*/

List *list_replace_list(List *list, ssize_t index, ssize_t range, const List *src, list_copy_t *copy)
{
	List *ret;
	int err;

	if (!src || !list)
		return set_errnull(EINVAL);

	if ((err = list_wrlock(list)))
		return set_errnull(err);

	if ((err = list_rdlock(src)))
	{
		list_unlock(list);
		return set_errnull(err);
	}

	ret = list_replace_list_unlocked(list, index, range, src, copy);

	if ((err = list_unlock(src)))
	{
		list_unlock(list);
		return set_errnull(err);
	}

	if ((err = list_unlock(list)))
		return set_errnull(err);

	return ret;
}

/*

=item C<List *list_replace_list_unlocked(List *list, ssize_t index, ssize_t range, const List *src, list_copy_t *copy)>

Equivalent to I<list_replace_list(3)> except that C<list> is not write
locked and C<src> is not read locked. Note: If C<src> needs to be read
locked, it is the caller's responsibility to lock and unlock it explicitly
with I<list_rdlock(3)> and I<list_unlock(3)>.

=cut

*/

List *list_replace_list_unlocked(List *list, ssize_t index, ssize_t range, const List *src, list_copy_t *copy)
{
	ssize_t length;

	if (!src || !list)
		return set_errnull(EINVAL);

	if (index < 0)
		index = list->length + 1 + index;

	if (index < 0)
		return set_errnull(EINVAL);

	if (range < 0)
		range = list->length + 1 + range - index;

	if (range < 0)
		return set_errnull(EINVAL);

	if (list->length < index + range || xor(list->destroy, copy))
		return set_errnull(EINVAL);

	killitems(list, index, range);

	length = src->length;

	if (adjust(list, index, range, length) == -1)
		return NULL;

	while (length--)
		list->list[index + length] = enlist(src->list[length], copy);

	return list;
}

/*

=item C<List *list_extract(const List *list, ssize_t index, ssize_t range, list_copy_t *copy)>

Creates a new list consisting of C<range> items from C<list>, starting at
C<index>, using C<copy> as the copy constructor (if not C<null>). If
C<index> or C<range> are negative, they refer to item positions relative to
the end of the list (C<-1> is the position after the last item, C<-2> is the
position of the last item and so on). It is the caller's responsibility to
deallocate the new list with I<list_release(3)> or I<list_destroy(3)>. On
success, returns the new list. On error, returns C<null> with C<errno> set
appropriately.

=cut

*/

List *list_extract(const List *list, ssize_t index, ssize_t range, list_copy_t *copy)
{
	return list_extract_with_locker(NULL, list, index, range, copy);
}

/*

=item C<List *list_extract_unlocked(const List *list, ssize_t index, ssize_t range, list_copy_t *copy)>

Equivalent to I<list_extract(3)> except that C<list> is not read locked.

=cut

*/

List *list_extract_unlocked(const List *list, ssize_t index, ssize_t range, list_copy_t *copy)
{
	return list_extract_with_locker_unlocked(NULL, list, index, range, copy);
}

/*

=item C<List *list_extract_with_locker(Locker *locker, const List *list, ssize_t index, ssize_t range, list_copy_t *copy)>

Equivalent to I<list_extract(3)> except that multiple threads accessing the
new list will be synchronised by C<locker>.

=cut

*/

List *list_extract_with_locker(Locker *locker, const List *list, ssize_t index, ssize_t range, list_copy_t *copy)
{
	List *ret;
	int err;

	if (!list)
		return set_errnull(EINVAL);

	if ((err = list_rdlock(list)))
		return set_errnull(err);

	ret = list_extract_with_locker_unlocked(locker, list, index, range, copy);

	if ((err = list_unlock(list)))
		return set_errnull(err);

	return ret;
}

/*

=item C<List *list_extract_with_locker_unlocked(Locker *locker, const List *list, ssize_t index, ssize_t range, list_copy_t *copy)>

Equivalent to I<list_extract_with_locker(3)> except that C<list> is not read
locked.

=cut

*/

List *list_extract_with_locker_unlocked(Locker *locker, const List *list, ssize_t index, ssize_t range, list_copy_t *copy)
{
	List *ret;

	if (!list)
		return set_errnull(EINVAL);

	if (index < 0)
		index = list->length + 1 + index;

	if (index < 0)
		return set_errnull(EINVAL);

	if (range < 0)
		range = list->length + 1 + range - index;

	if (range < 0)
		return set_errnull(EINVAL);

	if (list->length < index + range || xor(list->destroy, copy))
		return set_errnull(EINVAL);

	if (!(ret = list_create_with_locker(locker, copy ? list->destroy : NULL)))
		return NULL;

	while (range--)
	{
		if (!list_append(ret, enlist(list->list[index++], copy)))
		{
			list_release(ret);
			return NULL;
		}
	}

	return ret;
}

#undef enlist

/*

=item C<List *list_push(List *list, void *item)>

Pushes C<item> onto the end of C<list>. On success, returns C<list>. On
error, returns C<null> with C<errno> set appropriately.

=cut

*/

List *list_push(List *list, void *item)
{
	return list_append(list, item);
}

/*

=item C<List *list_push_unlocked(List *list, void *item)>

Equivalent to I<list_push(3)> except that C<list> is not write locked.

=cut

*/

List *list_push_unlocked(List *list, void *item)
{
	return list_append_unlocked(list, item);
}

/*

=item C<List *list_push_int(List *list, int item)>

Equivalent to I<list_push(3)> except that C<item> is an integer.

=cut

*/

List *list_push_int(List *list, int item)
{
	return list_append_int(list, item);
}

/*

=item C<List *list_push_int_unlocked(List *list, int item)>

Equivalent to I<list_push_int(3)> except that C<list> is not write locked.

=cut

*/

List *list_push_int_unlocked(List *list, int item)
{
	return list_append_int_unlocked(list, item);
}

/*

=item C<void *list_pop(List *list)>

Pops the last item off C<list>. On success, returns the item popped. On
error, returns C<null> with C<errno> set appropriately.

=cut

*/

void *list_pop(List *list)
{
	void *item;
	int err;

	if (!list)
		return set_errnull(EINVAL);

	if ((err = list_wrlock(list)))
		return set_errnull(err);

	item = list_pop_unlocked(list);

	if ((err = list_unlock(list)))
		return set_errnull(err);

	return item;
}

/*

=item C<void *list_pop_unlocked(List *list)>

Equivalent to I<list_pop(3)> except that C<list> is not write locked.

=cut

*/

void *list_pop_unlocked(List *list)
{
	void *item;

	if (!list)
		return set_errnull(EINVAL);

	if (!list->length)
		return set_errnull(EINVAL);

	item = list->list[list->length - 1];
	list->list[list->length - 1] = NULL;

	if (!list_remove_unlocked(list, list->length - 1))
	{
		list->list[list->length - 1] = item;
		return NULL;
	}

	return item;
}

/*

=item C<int list_pop_int(List *list)>

Equivalent to I<list_pop(3)> except that the item popped has an integer
type.

=cut

*/

int list_pop_int(List *list)
{
	return (int)(long)list_pop(list);
}

/*

=item C<int list_pop_int_unlocked(List *list)>

Equivalent to I<list_pop_int(3)> except that C<list> is not write locked.

=cut

*/

int list_pop_int_unlocked(List *list)
{
	return (int)(long)list_pop_unlocked(list);
}

/*

=item C<void *list_shift(List *list)>

Removes and returns the first item in C<list>. On success, returns the item
shifted. On error, returns C<null> with C<errno> set appropriately.

=cut

*/

void *list_shift(List *list)
{
	void *item;
	int err;

	if (!list)
		return set_errnull(EINVAL);

	if ((err = list_wrlock(list)))
		return set_errnull(err);

	item = list_shift_unlocked(list);

	if ((err = list_unlock(list)))
		return set_errnull(err);

	return item;
}

/*

=item C<void *list_shift_unlocked(List *list)>

Equivalent to I<list_shift(3)> except that C<list> is not write locked.

=cut

*/

void *list_shift_unlocked(List *list)
{
	void *item;

	if (!list)
		return set_errnull(EINVAL);

	if (!list->length)
		return set_errnull(EINVAL);

	item = list->list[0];
	list->list[0] = NULL;

	if (!list_remove_unlocked(list, 0))
	{
		list->list[0] = item;
		return NULL;
	}

	return item;
}

/*

=item C<int list_shift_int(List *list)>

Equivalent to I<list_shift(3)> except that the item shifted has an integer
type.

=cut

*/

int list_shift_int(List *list)
{
	return (int)(long)list_shift(list);
}

/*

=item C<int list_shift_int_unlocked(List *list)>

Equivalent to I<list_shift_int(3)> except that C<list> is not write locked.

=cut

*/

int list_shift_int_unlocked(List *list)
{
	return (int)(long)list_shift_unlocked(list);
}

/*

=item C<List *list_unshift(List *list, void *item)>

Inserts C<item> at the start of C<list>. On success, returns C<list>. On
error, returns C<null> with C<errno> set appropriately.

=cut

*/

List *list_unshift(List *list, void *item)
{
	return list_prepend(list, item);
}

/*

=item C<List *list_unshift_unlocked(List *list, void *item)>

Equivalent to I<list_unshift(3)> except that C<list> is not write locked.

=cut

*/

List *list_unshift_unlocked(List *list, void *item)
{
	return list_prepend_unlocked(list, item);
}

/*

=item C<List *list_unshift_int(List *list, int item)>

Equivalent to I<list_unshift(3)> except that C<item> is an integer.

=cut

*/

List *list_unshift_int(List *list, int item)
{
	return list_prepend_int(list, item);
}

/*

=item C<List *list_unshift_int_unlocked(List *list, int item)>

Equivalent to I<list_unshift_int(3)> except that C<list> is not write
locked.

=cut

*/

List *list_unshift_int_unlocked(List *list, int item)
{
	return list_prepend_int_unlocked(list, item);
}

/*

=item C<List *list_splice(List *list, ssize_t index, ssize_t range, list_copy_t *copy)>

Removes a sublist from C<list> starting at C<index> with length C<range>
after copying the items with C<copy> (if not C<null>). If C<index> or
C<range> are negative, they refer to item positions relative to the end of
the list (C<-1> is the position after the last item, C<-2> is the position
of the last item and so on). On success, returns the sublist. It is the
caller's responsibility to deallocate the new list with I<list_release(3)>
or I<list_destroy(3)>. On error, returns C<null> with C<errno> set
appropriately.

=cut

*/

List *list_splice(List *list, ssize_t index, ssize_t range, list_copy_t *copy)
{
	return list_splice_with_locker(NULL, list, index, range, copy);
}

/*

=item C<List *list_splice_unlocked(List *list, ssize_t index, ssize_t range, list_copy_t *copy)>

Equivalent to I<list_splice(3)> except that C<list> is not write locked.

=cut

*/

List *list_splice_unlocked(List *list, ssize_t index, ssize_t range, list_copy_t *copy)
{
	return list_splice_with_locker_unlocked(NULL, list, index, range, copy);
}

/*

=item C<List *list_splice_with_locker(Locker *locker, List *list, ssize_t index, ssize_t range, list_copy_t *copy)>

Equivalent to I<list_splice(3)> except that multiple threads accessing the
new list will be synchronised by C<locker>.

=cut

*/

List *list_splice_with_locker(Locker *locker, List *list, ssize_t index, ssize_t range, list_copy_t *copy)
{
	List *ret;
	int err;

	if (!list)
		return set_errnull(EINVAL);

	if ((err = list_wrlock(list)))
		return set_errnull(err);

	ret = list_splice_with_locker_unlocked(locker, list, index, range, copy);

	if ((err = list_unlock(list)))
	{
		list_release(ret);
		return set_errnull(err);
	}

	return ret;
}

/*

=item C<List *list_splice_with_locker_unlocked(Locker *locker, List *list, ssize_t index, ssize_t range, list_copy_t *copy)>

Equivalent to I<list_splice_with_locker(3)> except that C<list> is not write
locked.

=cut

*/

List *list_splice_with_locker_unlocked(Locker *locker, List *list, ssize_t index, ssize_t range, list_copy_t *copy)
{
	List *ret;

	if (!list)
		return set_errnull(EINVAL);

	if (!(ret = list_extract_with_locker_unlocked(locker, list, index, range, copy)))
		return NULL;

	if (!list_remove_range_unlocked(list, index, range))
	{
		list_release(ret);
		return NULL;
	}

	return ret;
}

/*

=item C<List *list_sort(List *list, list_cmp_t *cmp)>

Sorts the items in C<list> using the item comparison function C<cmp> and
I<qsort(3)> for lists of fewer than 10000 items and I<hsort(3)> for larger
lists. On success, returns C<list>. On error, returns C<null> with C<errno>
set appropriately.

=cut

*/

List *list_sort(List *list, list_cmp_t *cmp)
{
	List *ret;
	int err;

	if (!list)
		return set_errnull(EINVAL);

	if ((err = list_wrlock(list)))
		return set_errnull(err);

	ret = list_sort_unlocked(list, cmp);

	if ((err = list_unlock(list)))
		return set_errnull(err);

	return ret;
}

/*

=item C<List *list_sort_unlocked(List *list, list_cmp_t *cmp)>

Equivalent to I<list_sort(3)> except that C<list> is not write locked.

=cut

*/

List *list_sort_unlocked(List *list, list_cmp_t *cmp)
{
	if (!list)
		return set_errnull(EINVAL);

	if (!list->list || !list->length)
		return set_errnull(EINVAL);

	((list->length >= 10000) ? hsort : qsort)(list->list, list->length, sizeof list->list[0], cmp);

	return list;
}

/*

=item C<void list_apply(List *list, list_action_t *action, void *data)>

Invokes C<action> for each of C<list>'s items. The arguments passed to
C<action> are the item, a pointer to the loop variable containing the item's
position within C<list> and C<data>. On error, sets C<errno> appropriately.

=cut

*/

void list_apply(List *list, list_action_t *action, void *data)
{
	list_apply_wrlocked(list, action, data);
}

/*

=item C<void list_apply_rdlocked(List *list, list_action_t *action, void *data)>

Equivalent to I<list_apply(3)> except that C<list> is read locked rather
than write locked. Use this in preference to I<list_apply(3)> when C<list>
and its items will not be modified during the iteration.

=cut

*/

void list_apply_rdlocked(List *list, list_action_t *action, void *data)
{
	int err;

	if (!list || !action)
	{
		set_errno(EINVAL);
		return;
	}

	if ((err = list_rdlock(list)))
	{
		set_errno(err);
		return;
	}

	list_apply_unlocked(list, action, data);

	if ((err = list_unlock(list)))
		set_errno(err);
}

/*

=item C<void list_apply_wrlocked(List *list, list_action_t *action, void *data)>

Equivalent to I<list_apply(3)> except that this function name makes the fact
that C<list> is write locked explicit.

=cut

*/

void list_apply_wrlocked(List *list, list_action_t *action, void *data)
{
	int err;

	if (!list || !action)
	{
		set_errno(EINVAL);
		return;
	}

	if ((err = list_wrlock(list)))
	{
		set_errno(err);
		return;
	}

	list_apply_unlocked(list, action, data);

	if ((err = list_unlock(list)))
		set_errno(err);
}

/*

=item C<void list_apply_unlocked(List *list, list_action_t *action, void *data)>

Equivalent to I<list_apply(3)> except that C<list> is not write locked.

=cut

*/

void list_apply_unlocked(List *list, list_action_t *action, void *data)
{
	size_t i;

	if (!list || !action)
	{
		set_errno(EINVAL);
		return;
	}

	for (i = 0; i < list->length; ++i)
		action(list->list[i], &i, data);
}

/*

=item C<List *list_map(List *list, list_release_t *destroy, list_map_t *map, void *data)>

Creates and returns a new list containing the return values of C<map>,
invoked once for each item in C<list>. The arguments passed to C<map> are
the item, a pointer to the loop variable containing the item's position
within C<list> and C<data>. C<destroy> will be the item destructor for the
new list. It is the caller's responsibility to deallocate the new list with
I<list_release(3)> or I<list_destroy(3)>. On success, returns the new list.
On error, returns C<null> with C<errno> set appropriately.

=cut

*/

List *list_map(List *list, list_release_t *destroy, list_map_t *map, void *data)
{
	return list_map_with_locker(NULL, list, destroy, map, data);
}

/*

=item C<List *list_map_unlocked(List *list, list_release_t *destroy, list_map_t *map, void *data)>

Equivalent to I<list_map(3)> except that C<list> is not read locked.

=cut

*/

List *list_map_unlocked(List *list, list_release_t *destroy, list_map_t *map, void *data)
{
	return list_map_with_locker_unlocked(NULL, list, destroy, map, data);
}

/*

=item C<List *list_map_with_locker(Locker *locker, List *list, list_release_t *destroy, list_map_t *map, void *data)>

Equivalent to I<list_map(3)> except that multiple threads accessing the new
list will be synchronised by C<locker>.

=cut

*/

List *list_map_with_locker(Locker *locker, List *list, list_release_t *destroy, list_map_t *map, void *data)
{
	List *mapping;
	int err;

	if (!list || !map)
		return set_errnull(EINVAL);

	if ((err = list_rdlock(list)))
		return set_errnull(err);

	mapping = list_map_with_locker_unlocked(locker, list, destroy, map, data);

	if ((err = list_unlock(list)))
	{
		list_release(mapping);
		return set_errnull(err);
	}

	return mapping;
}

/*

=item C<List *list_map_with_locker_unlocked(Locker *locker, List *list, list_release_t *destroy, list_map_t *map, void *data)>

Equivalent to I<list_map_with_locker(3)> except that C<list> is not read
locked.

=cut

*/

List *list_map_with_locker_unlocked(Locker *locker, List *list, list_release_t *destroy, list_map_t *map, void *data)
{
	List *mapping;
	size_t i;

	if (!list || !map)
		return set_errnull(EINVAL);

	if (!(mapping = list_create_with_locker(locker, destroy)))
		return NULL;

	for (i = 0; i < list->length; ++i)
	{
		if (!list_append(mapping, map(list->list[i], &i, data)))
		{
			list_release(mapping);
			return NULL;
		}
	}

	return mapping;
}

/*

=item C<List *list_grep(List *list, list_query_t *grep, void *data)>

Creates and returns a new list by invoking C<grep> for each item in C<list>,
and appending the items for which C<grep> returned a non-zero value. The
arguments passed to C<grep> are the item, a pointer to the loop variable
containing the item's position within C<list> and C<data>. It is the
caller's responsibility to deallocate the new list with I<list_release(3)>
or I<list_destroy(3)>. Note that the new list does not own its items since
it does not copy them. On success, returns the new list. On error, returns
C<null> with C<errno> set appropriately.

=cut

*/

List *list_grep(List *list, list_query_t *grep, void *data)
{
	return list_grep_with_locker(NULL, list, grep, data);
}

/*

=item C<List *list_grep_unlocked(List *list, list_query_t *grep, void *data)>

Equivalent to I<list_grep(3)> except that C<list> is not read locked.

=cut

*/

List *list_grep_unlocked(List *list, list_query_t *grep, void *data)
{
	return list_grep_with_locker_unlocked(NULL, list, grep, data);
}

/*

=item C<List *list_grep_with_locker(Locker *locker, List *list, list_query_t *grep, void *data)>

Equivalent to I<list_grep(3)> except that multiple threads accessing the new
list will be synchronised by C<locker>.

=cut

*/

List *list_grep_with_locker(Locker *locker, List *list, list_query_t *grep, void *data)
{
	List *grepping;
	int err;

	if (!list || !list)
		return set_errnull(EINVAL);

	if ((err = list_rdlock(list)))
		return set_errnull(err);

	grepping = list_grep_with_locker_unlocked(locker, list, grep, data);

	if ((err = list_unlock(list)))
	{
		list_release(grepping);
		return set_errnull(err);
	}

	return grepping;
}

/*

=item C<List *list_grep_with_locker_unlocked(Locker *locker, List *list, list_query_t *grep, void *data)>

Equivalent to I<list_grep_with_locker(3)> except that C<list> is not read
locked.

=cut

*/

List *list_grep_with_locker_unlocked(Locker *locker, List *list, list_query_t *grep, void *data)
{
	List *grepping;
	size_t i;

	if (!list || !list)
		return set_errnull(EINVAL);

	if (!(grepping = list_create(NULL)))
		return NULL;

	for (i = 0; i < list->length; ++i)
	{
		if (grep(list->list[i], &i, data))
		{
			if (!list_append(grepping, list->list[i]))
			{
				list_release(grepping);
				return NULL;
			}
		}
	}

	return grepping;
}

/*

=item C<ssize_t list_query(List *list, ssize_t *index, list_query_t *query, void *data)>

Invokes C<query> on each item in C<list>, starting at C<*index> until
C<query> returns a non-zero value. The arguments passed to C<query> are the
item, C<index> and C<data>. Returns the index of the item that satisfied
C<query>, or C<-1> when C<query> is not satisfied by any remaining items.
The value pointed to by C<index> is set to the return value. On error, sets
C<errno> appropriately.

=cut

*/

ssize_t list_query(List *list, ssize_t *index, list_query_t *query, void *data)
{
	ssize_t ret;
	int err;

	if (!list || !index || !query)
		return set_errno(EINVAL);

	if ((err = list_rdlock(list)))
		return set_errno(err);

	ret = list_query_unlocked(list, index, query, data);

	if ((err = list_unlock(list)))
		return set_errno(err);

	return ret;
}

/*

=item C<ssize_t list_query_unlocked(List *list, ssize_t *index, list_query_t *query, void *data)>

Equivalent to I<list_query(3)> except that C<list> is not read locked.

=cut

*/

ssize_t list_query_unlocked(List *list, ssize_t *index, list_query_t *query, void *data)
{
	size_t i;

	if (!list || !index || !query)
		return set_errno(EINVAL);

	if (*index >= list->length)
		return set_errno(EINVAL);

	for (i = *index; i < list->length; ++i)
	{
		if (query(list->list[i], (size_t *)index, data))
			return *index = i;
	}

	return *index = -1;
}

/*

=item C<Lister *lister_create(List *list)>

Creates an iterator for C<list>. It is the caller's responsibility to
deallocate the iterator with I<lister_release(3)> or I<lister_destroy(3)>.
The iterator keeps C<list> write locked until it is released with
I<lister_release(3)> or I<lister_destroy(3)>. Note that the iterator itself
is not locked so it must not be shared between threads. On success, returns
the iterator. On error, returns C<null> with C<errno> set appropriately.

=cut

*/

Lister *lister_create(List *list)
{
	return lister_create_wrlocked(list);
}

/*

=item C<Lister *lister_create_rdlocked(List *list)>

Equivalent to I<lister_create(3)> except that C<list> is read locked rather
than write locked. Use this in preference to I<lister_create(3)> when no
calls to I<lister_remove(3)> will be made during the iteration.

=cut

*/

Lister *lister_create_rdlocked(List *list)
{
	int err;

	if (!list)
		return set_errnull(EINVAL);

	if ((err = list_rdlock(list)))
		return set_errnull(err);

	return lister_create_unlocked(list);
}

/*

=item C<Lister *lister_create_wrlocked(List *list)>

Equivalent to I<lister_create(3)> except that this function name makes the
fact that C<list> is write locked explicit.

=cut

*/

Lister *lister_create_wrlocked(List *list)
{
	int err;

	if (!list)
		return set_errnull(EINVAL);

	if ((err = list_wrlock(list)))
		return set_errnull(err);

	return lister_create_unlocked(list);
}

/*

=item C<Lister *lister_create_unlocked(const List *list)>

Equivalent to I<lister_create(3)> except that C<list> is not write locked.

=cut

*/

Lister *lister_create_unlocked(const List *list)
{
	Lister *lister;

	if (!list)
		return set_errnull(EINVAL);

	if (!(lister = mem_new(Lister))) /* XXX decouple */
		return NULL;

	lister->list = (List *)list;
	lister->index = -1;

	return lister;
}

/*

=item C<void lister_release(Lister *lister)>

Releases (deallocates) C<lister> and unlocks the associated list.

=cut

*/

void lister_release(Lister *lister)
{
	int err;

	if (!lister)
		return;

	if ((err = list_unlock(lister->list)))
	{
		set_errno(err);
		return;
	}

	mem_release(lister);
}

/*

=item C<void lister_release_unlocked(Lister *lister)>

Equivalent to I<lister_release(3)> except that the associated list is not
unlocked.

=cut

*/

void lister_release_unlocked(Lister *lister)
{
	if (!lister)
		return;

	mem_release(lister);
}

/*

=item C<void *lister_destroy(Lister **lister)>

Destroys (deallocates and sets to C<null>) C<*lister> and unlocks the
associated list. Returns C<null>. On error, sets C<errno> appropriately.

=cut

*/

void *lister_destroy(Lister **lister)
{
	if (lister && *lister)
	{
		lister_release(*lister);
		*lister = NULL;
	}

	return NULL;
}

/*

=item C<void *lister_destroy_unlocked(Lister **lister)>

Equivalent to I<lister_destroy(3)> except that the associated list is not
unlocked.

=cut

*/

void *lister_destroy_unlocked(Lister **lister)
{
	if (lister && *lister)
	{
		lister_release_unlocked(*lister);
		*lister = NULL;
	}

	return NULL;
}

/*

=item C<int lister_has_next(Lister *lister)>

Returns whether or not there is another item in the list over which
C<lister> is iterating. On error, returns C<-1> with C<errno> set
appropriately.

=cut

*/

int lister_has_next(Lister *lister)
{
	if (!lister)
		return set_errno(EINVAL);

	return lister->index + 1 < lister->list->length;
}

/*

=item C<void *lister_next(Lister *lister)>

Returns the next item in the iteration, C<lister>. On error, returns C<null>
with C<errno> set appropriately.

=cut

*/

void *lister_next(Lister *lister)
{
	if (!lister)
		return set_errnull(EINVAL);

	return list_item_unlocked(lister->list, (size_t)++lister->index);
}

/*

=item C<int lister_next_int(Lister *lister)>

Equivalent to I<lister_next(3)> except that the item returned is an integer.

=cut

*/

int lister_next_int(Lister *lister)
{
	if (!lister)
		return set_errno(EINVAL);

	return list_item_int_unlocked(lister->list, (size_t)++lister->index);
}

/*

=item C<void lister_remove(Lister *lister)>

Removes the current item in the iteration, C<lister>. The next item in the
iteration is the item following the removed item, if any. This must be
called after I<lister_next(3)> or I<lister_next_int(3)>. On error, sets
C<errno> appropriately.

=cut

*/

void lister_remove(Lister *lister)
{

	if (!lister)
	{
		set_errno(EINVAL);
		return;
	}

	if (lister->index == -1)
	{
		set_errno(EINVAL);
		return;
	}

	list_remove_unlocked(lister->list, (size_t)lister->index--);
}

/*

=item C<int list_has_next(List *list)>

Returns whether or not there is another item in C<list> using an internal
iterator. The first time this is called, a new internal I<Lister> will be
created (Note: There can be only one). When there are no more items, returns
C<0> and destroys the internal iterator. When it returns C<1>, use
I<list_next(3)> or I<list_next_int(3)> to retrieve the next item. On error,
returns C<-1> with C<errno> set appropriately.

Note: If an iteration using an internal iterator terminates before the end
of the list, it is the caller's responsibility to call I<list_break(3)>.
Failure to do so will cause the internal iterator to leak. It will also
break the next call to I<list_has_next(3)> which will continue where the
current iteration stopped rather than starting at the beginning again.
I<list_release(3)> assumes that there is no internal iterator so it is the
caller's responsibility to complete the iteration or call I<list_break(3)>
before releasing C<list> with I<list_release(3)> or I<list_destroy(3)>.

Note: The internal I<Lister> does not lock C<list> so this function is not
threadsafe. It can only be used with lists created in the current function
(to guarantee that no other thread can access it). This practice should be
observed even in single threaded applications to avoid breaking iterator
semantics (possible with nested function calls). If C<list> is a parameter
or a variable declared outside the function, it is best to create an
explicit I<Lister> instead. If this function is used on such lists instead,
it is the caller's responsibility to explicitly lock C<list> first with
I<list_wrlock(3)> and explicitly unlock it with I<list_unlock(3)>. Do this
even if you are writing single threaded code in case your function may one
day be used in a multi threaded application.

=cut

*/

int list_has_next(List *list)
{
	int has;

	if (!list)
		return set_errno(EINVAL);

	if (!list->lister && !(list->lister = lister_create_unlocked(list)))
			return -1;

	if ((has = lister_has_next(list->lister)) != 1)
	  list_break(list);

	return has;
}

/*

=item C<void list_break(List *list)>

Unlocks C<list> and destroys its internal iterator. Must be used only when
an iteration using an internal iterator has terminated before reaching the
end of C<list>. On error, returns C<null> with C<errno> set appropriately.

=cut

*/

void list_break(List *list)
{
	if (!list)
	{
		set_errno(EINVAL);
		return;
	}

	lister_destroy_unlocked(&list->lister);
}

/*

=item C<void *list_next(List *list)>

Returns the next item in C<list> using it's internal iterator. On error,
returns C<null> with C<errno> set appropriately.

=cut

*/

void *list_next(List *list)
{
	if (!list || !list->lister)
		return set_errnull(EINVAL);

	return lister_next(list->lister);
}

/*

=item C<int list_next_int(List *list)>

Equivalent to I<list_next(3)> except that the item returned is an integer.

=cut

*/

int list_next_int(List *list)
{
	if (!list || !list->lister)
		return set_errno(EINVAL);

	return lister_next_int(list->lister);
}

/*

=item C<void list_remove_current(List *list)>

Removes the current item in C<list> using it's internal iterator. The next
item in the iteration is the item following the removed item, if any. This
must be called after I<list_next(3)>. On error, sets C<errno> appropriately.

=cut

*/

void list_remove_current(List *list)
{
	if (!list || !list->lister)
	{
		set_errno(EINVAL);
		return;
	}

	lister_remove(list->lister);
}

/*

=back

=head1 ERRORS

On error, C<errno> is set either by an underlying function, or as follows:

=over 4

=item C<EINVAL>

When arguments are C<null> or out of range.

=back

=head1 MT-Level

MT-Disciplined

By default, I<List>s are not MT-Safe because most programs are single
threaded and synchronisation doesn't come for free. Even in multi threaded
programs, not all I<List>s are necessarily shared between multiple threads.

When a I<List> is shared between multiple threads which need to be
synchronised, the method of synchronisation must be carefully selected by
the client code. There are tradeoffs between concurrency and overhead. The
greater the concurrency, the greater the overhead. More locks give greater
concurrency but have greater overhead. Readers/Writer locks can give greater
concurrency than mutex locks but have greater overhead. One lock for each
I<List> may be required, or one lock for all (or a set of) I<List>s may be
more appropriate.

Generally, the best synchronisation strategy for a given application can
only be determined by testing/benchmarking the written application. It is
important to be able to experiment with the synchronisation strategy at this
stage of development without pain.

To facilitate this, I<List>s can be created with
I<list_create_with_locker(3)> which takes a I<Locker> argument. The
I<Locker> specifies a lock and a set of functions for manipulating the lock.
Each I<List> can have it's own lock by creating a separate I<Locker> for
each I<List>. Multiple I<List>s can share the same lock by sharing the same
I<Locker>. Only the application developer can determine what is appropriate
for each application on a case by case basis.

I<MT-Disciplined> means that the application developer has a mechanism for
specifying the synchronisation requirements to be applied to library code.

=head1 EXAMPLES

Create a list that doesn't own its items, populate it and then iterate over
its values with the internal iterator to print the values:

    #include <slack/std.h>
    #include <slack/list.h>

    int main()
    {
        List *list;

        if (!(list = list_create(NULL)))
            return EXIT_FAILURE;

        list_append(list, "123");
        list_append(list, "456");
        list_append(list, "789");

        while (list_has_next(list) == 1)
            printf("%s\n", (char *)list_next(list));

        list_destroy(&list);

        return EXIT_SUCCESS;
    }

The same but create the list and populate it at the same time:

    #include <slack/std.h>
    #include <slack/list.h>

    int main()
    {
        List *list;

        if (!(list = list_make(NULL, "123", "456", "789", NULL)))
            return EXIT_FAILURE;

        while (list_has_next(list) == 1)
            printf("%s\n", (char *)list_next(list));

        list_destroy(&list);

        return EXIT_SUCCESS;
    }

Create a map that does own its items, populate it and then iterator over it
with an external iterator to print its items.

    #include <slack/std.h>
    #include <slack/list.h>

    int main()
    {
        List *list;
        Lister *lister;

        if (!(list = list_create(NULL)))
            return EXIT_FAILURE;

        list_append(list, "123");
        list_append(list, "456");
        list_append(list, "789");

        if (!(lister = lister_create(list)))
        {
            list_destroy(&list);
            return EXIT_FAILURE;
        }

        while (lister_has_next(lister) == 1)
            printf("%s\n", (char *)lister_next(lister));

        lister_destroy(&lister);
        list_destroy(&list);

        return EXIT_SUCCESS;
    }

The same but with an apply function:

    #include <slack/std.h>
    #include <slack/list.h>

    void action(void *item, size_t *index, void *data)
    {
        printf("%s\n", (char *)item);
    }

    int main()
    {
        List *list;

        if (!(list = list_create(free)))
            return EXIT_FAILURE;

        list_append(list, strdup("123"));
        list_append(list, strdup("456"));
        list_append(list, strdup("789"));

        list_apply(list, action, NULL);
        list_destroy(&list);

        return EXIT_SUCCESS;
    }

The same but with a list of integers:

    #include <slack/std.h>
    #include <slack/list.h>

    int main()
    {
        List *list;

        if (!(list = list_create(NULL)))
            return EXIT_FAILURE;

        list_append(list, (void *)123);
        list_append(list, (void *)456);
        list_append(list, (void *)789);

        while (list_has_next(list) == 1)
            printf("%d\n", list_next_int(list));

        list_destroy(&list);

        return EXIT_SUCCESS;
    }

Create a copy of a list:

    #include <slack/std.h>
    #include <slack/list.h>

    int main()
    {
        List *orig;
        List *copy;

        if (!(orig = list_make(free, strdup("123"), strdup("456"), strdup("789"), NULL)))
            return EXIT_FAILURE;

        if (!(copy = list_copy(orig, (list_copy_t *)strdup)))
        {
            list_destroy(&orig);
            return EXIT_FAILURE;
        }

        list_destroy(&orig);

        while (list_has_next(copy) == 1)
            printf("%s\n", (char *)list_next(copy));

        list_destroy(&copy);

        return EXIT_SUCCESS;
    }

Transfer ownership from one list to another:

    #include <slack/std.h>
    #include <slack/list.h>

    int main()
    {
        List *donor;
        List *recipient;

        if (!(donor = list_make(free, strdup("123"), strdup("456"), strdup("789"), NULL)))
            return EXIT_FAILURE;

        if (!(recipient = list_create(NULL)))
        {
            list_destroy(&donor);
            return EXIT_FAILURE;
        }

        while (list_has_next(donor) == 1)
            list_append(recipient, list_next(donor));

        list_own(recipient, list_disown(donor));
        list_destroy(&donor);

        while (list_has_next(recipient) == 1)
            printf("%s\n", (char *)list_next(recipient));

        list_destroy(&recipient);

        return EXIT_SUCCESS;
    }

Manipulate a list, examine it, use apply, map, grep and query,
remove items while iterating:

    #include <slack/std.h>
    #include <slack/list.h>

    int cmp(const void *a, const void *b)
    {
        return strcmp(*(char **)a, *(char **)b);
    }

    void action(void *item, size_t *index, void *data)
    {
        printf("%s\n", (char *)item);
    }

    void *upper(void *item, size_t *index, void *data)
    {
        char *uc = strdup(item);
        if (uc)
            *uc = toupper(*uc);

        return uc;
    }

    int even(void *item, size_t *index, void *data)
    {
        return item && (*(char *)item & 1) == 0;
    }

    int main()
    {
        Lister *lister;
        List *list;
        void *item;
        List *res;
        ssize_t i;

        if (!(list = list_create(NULL)))
            return EXIT_FAILURE;

        // Manipulate a list

        printf("length %d empty %d\n", list_length(list), list_empty(list));

        list_append(list, "a");
        list_append(list, "b");
        list_append(list, "c");
        list_remove(list, 0);
        list_insert(list, 1, "d");
        list_prepend(list, "e");
        list_replace(list, 1, 2, "f");
        list_push(list, "g");
        list_push(list, "h");
        list_push(list, "i");
        item = list_pop(list);
        list_unshift(list, list_shift(list));
        list_release(list_splice(list, 0, 1, NULL));
        list_sort(list, cmp);
        printf("last %s\n", (char *)list_item(list, list_last(list)));

        // Apply an action to a list

        list_apply(list, action, NULL);

        // Map a list into another list

        res = list_map(list, free, upper, NULL);
        list_apply(res, action, NULL);
        list_destroy(&res);

        // Grep a list for items that match some criteria

        res = list_grep(list, even, NULL);
        list_apply(res, action, NULL);
        list_destroy(&res);

        // Locate a list's items that match some criteria

        for (i = 0; list_query(list, &i, even, NULL) != -1; ++i)
            printf("%d %s even\n", i, (char *)list_item(list, i));

        // Remove elements via the internal iterator and break out of loop

        while (list_has_next(list) == 1)
        {
            item = list_next(list);
            list_remove_current(list);

            if (!strcmp(item, "f"))
            {
                list_break(list);
                break;
            }
        }

        // Remove elements via an external iterator

        for (lister = lister_create(list); lister_has_next(lister) == 1; )
        {
            item = lister_next(lister);
            lister_remove(lister);
        }

        lister_destroy(&lister);
        list_destroy(&list);

        return EXIT_SUCCESS;
    }

Manipulate a list of integers:

    #include <slack/std.h>
    #include <slack/list.h>

    int main()
    {
        Lister *lister;
        List *list;
        int item;
        int i;

        if (!(list = list_create(NULL)))
            return EXIT_FAILURE;

        // Manipulate a list

        list_append_int(list, 1);
        list_append_int(list, 2);
        list_append_int(list, 3);
        list_remove(list, 0);
        list_insert_int(list, 1, 4);
        list_prepend_int(list, 5);
        list_replace_int(list, 1, 2, 6);
        list_push_int(list, 7);
        list_push_int(list, 8);
        list_push_int(list, 9);
        item = list_pop_int(list);
        list_unshift_int(list, list_shift_int(list));
        list_release(list_splice(list, 0, 1, NULL));

        // Get items as integers

        for (i = 0; i < list_length(list); ++i)
            printf("%d\n", list_item_int(list, i));

        // Remove elements via the internal iterator

        while (list_has_next(list) == 1)
        {
            item = list_next_int(list);
            list_remove_current(list);
        }

        // Remove elements via an external iterator

        for (lister = lister_create(list); lister_has_next(lister) == 1; )
        {
            item = lister_next_int(lister);
            lister_remove(lister);
        }

        list_destroy(&list);

        return EXIT_SUCCESS;
    }

Append, insert, prepend and replace using another list, not just one item:

    #include <slack/std.h>
    #include <slack/list.h>

    int main()
    {
        List *list;
        List *src;
        int i;

        if (!(list = list_create(NULL)))
            return EXIT_FAILURE;

        if (!(src = list_make(NULL, "a", "b", "c", NULL)))
        {
            list_destroy(&list);
            return EXIT_FAILURE;
        }

        list_append_list(list, src, NULL);
        list_insert_list(list, 1, src, NULL);
        list_prepend_list(list, src, NULL);
        list_replace_list(list, 1, 2, src, NULL);

        for (i = 0; i < list_length(list); ++i)
            printf("%s\n", (char *)list_item(list, i));

        list_destroy(&list);

        return EXIT_SUCCESS;
    }

The same as the previous example but with a list that owns its items:

    #include <slack/std.h>
    #include <slack/list.h>

    int main()
    {
        List *list;
        List *src;
        int i;

        if (!(list = list_create(free)))
            return EXIT_FAILURE;

        if (!(src = list_make(NULL, "a", "b", "c", NULL)))
        {
            list_destroy(&list);
            return EXIT_FAILURE;
        }

        list_append_list(list, src, (list_copy_t *)strdup);
        list_insert_list(list, 1, src, (list_copy_t *)strdup);
        list_prepend_list(list, src, (list_copy_t *)strdup);
        list_replace_list(list, 1, 2, src, (list_copy_t *)strdup);

        for (i = 0; i < list_length(list); ++i)
            printf("%s\n", (char *)list_item(list, i));

        list_destroy(&list);

        return EXIT_SUCCESS;
    }

=head1 CAVEAT

Little attempt is made to protect the client from sharing items between
lists with differing ownership policies and getting it wrong. When copying
items from any list to an owning list, a copy function must be supplied.
When adding a single item to an owning list, it is assumed that the list may
take over ownership of the item. When an owning list is destroyed, all of
its items are destroyed. If any of these items had been shared with a
non-owning list that outlived the owning list, then the non-owning list will
contain items that point to deallocated memory.

If you use an internal iterator in a loop that terminates before the end of
the list, and fail to call I<list_break(3)>, the internal iterator will
leak.

=head1 BUGS

Uses I<malloc(3)>. The type of memory used and the allocation strategy need
to be decoupled from this code.

=head1 SEE ALSO

I<libslack(3)>,
I<map(3)>,
I<mem(3)>,
I<hsort(3)>,
I<qsort(3)>,
I<locker(3)>

=head1 AUTHOR

20201111 raf <raf@raf.org>

=cut

*/

#endif

#ifdef TEST

#include <slack/str.h>

char action_data[1024];

void action(void *item, size_t *index)
{
	strlcat(action_data, item, 1024);
}

int query_data[] = { 2, 6, 8 , -1 };

int query(void *item, size_t *index)
{
	return !strcmp((const char *)item, "def");
}

int sort_cmp(const char **a, const char **b)
{
	return strcmp(*a, *b);
}

int mapf(int item, size_t *index, int *sum)
{
	return (sum) ? *sum += item : item;
}

int grepf(int item, size_t *index, int *data)
{
	return !(item & 1);
}

#define RD 0
#define WR 1
List *mtlist = NULL;
Locker *locker = NULL;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
#ifdef PTHREAD_RWLOCK_INITIALIZER
pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;
#else
pthread_rwlock_t rwlock;
#endif
int barrier[2];
int length[2];
const int lim = 1000;
int debug = 0;
int errors = 0;

void *produce(void *arg)
{
	int i;
	int test = *(int *)arg;

	for (i = 0; i <= lim; ++i)
	{
		if (debug)
			printf("p: prepend %d\n", i);

		if (!list_prepend_int(mtlist, i))
			++errors, printf("Test%d: list_prepend_int(mtlist, %d), failed\n", test, i);

		write(length[WR], "", 1);
	}

	write(barrier[WR], "", 1);
	return NULL;
}

void *consume(void *arg)
{
	int i, v;
	int test = *(int *)arg;
	char ack;

	for (i = 0; i < lim * 2; ++i)
	{
		if (debug)
			printf("c: pop\n");

		while (read(length[RD], &ack, 1) == -1 && errno == EINTR)
		{}

		v = list_pop_int(mtlist);

		if (debug)
			printf("c: pop %d\n", v);

		if (v == lim)
			break;
	}

	if (i != lim)
		++errors, printf("Test%d: consumer read %d items, not %d\n", test, i, lim);

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
		list_wrlock(mtlist);

		while (list_has_next(mtlist) == 1)
		{
			int val = list_next_int(mtlist);

			if (debug)
				printf("i%d: loop %d/%d val %d\n", t, i, lim / 10, val);

			if (!broken)
			{
				list_break(mtlist);
				broken = 1;
				break;
			}
		}

		list_unlock(mtlist);
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
		Lister *lister = lister_create_rdlocked(mtlist);

		while (lister_has_next(lister) == 1)
		{
			int val = (int)(long)lister_next(lister);

			if (debug)
				printf("j%d: loop %d/%d val %d\n", t, i, lim / 10, val);
		}

		lister_release(lister);
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
		Lister *lister = lister_create_wrlocked(mtlist);

		while (lister_has_next(lister) == 1)
		{
			int val = (int)(long)lister_next(lister);

			if (debug)
				printf("k%d: loop %d/%d val %d\n", t, i, lim / 10, val);
		}

		lister_release(lister);
	}

	write(barrier[WR], "", 1);
	return NULL;
}

void mt_test(int test, Locker *locker)
{
	if (!(mtlist = list_create_with_locker(locker, NULL)))
		++errors, printf("Test%d: list_create_with_locker(NULL) failed\n", test);
	else
	{
		static int iid[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
		pthread_attr_t attr;
		pthread_t id;
		int i;
		char ack;

		if (pipe(length) == -1 || pipe(barrier) == -1)
		{
			++errors, printf("Test%d: failed to perform test: pipe() failed\n", test);
			return;
		}

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
		pthread_attr_destroy(&attr);

		for (i = 0; i < 12; ++i)
			while (read(barrier[RD], &ack, 1) == -1 && errno == EINTR)
			{}

		list_destroy(&mtlist);
		if (mtlist)
			++errors, printf("Test%d: list_destroy(&mtlist) failed\n", test);

		close(length[RD]);
		close(length[WR]);
		close(barrier[RD]);
		close(barrier[WR]);
	}
}

#define TEST_ACT(i, action) \
	if (!(action)) \
		++errors, printf("Test%d: %s failed\n", (i), (#action));

#define TEST_EQ(i, action, value) \
	if ((val = (action)) != (value)) \
		++errors, printf("Test%d: %s failed (returned %d, not %d)\n", (i), (#action), val, (value));

#define CHECK_LENGTH(i, action, list, length) \
	if (list_length(list) != (length)) \
		++errors, printf("Test%d: %s failed: list_length(%s) = %d, not %d\n", (i), (#action), (#list), (int)list_length(list), (length));

#define CHECK_ITEM(i, action, list, item, value) \
	if (!list_item((list), (item)) || strcmp(list_item((list), (item)), (value))) \
		++errors, printf("Test%d: %s failed (item %d is \"%s\", not \"%s\")\n", (i), (#action), (item), (char *)list_item(list, (item)), (value));

#define CHECK_INT_ITEM(i, action, list, item, value) \
	if (list_item_int((list), (item)) != (value)) \
		++errors, printf("Test%d: %s failed (item %d is %d, not %d)\n", (i), (#action), (item), list_item_int(list, (item)), (value));

int main(int ac, char **av)
{
	int errors = 0;
	List *a, *b, *c, *d;
	Lister *lister;
	ssize_t index = 0;
	int i, val;

	if (ac == 2 && !strcmp(av[1], "help"))
	{
		printf("usage: %s [debug]\n", *av);
		return EXIT_SUCCESS;
	}

	printf("Testing: %s\n", "list");

	/* Test list_make, list_length, list_item */

	TEST_ACT(1, a = list_make(NULL, "abc", "def", "ghi", "jkl", NULL))
	else CHECK_LENGTH(1, list_make(), a, 4)
	else CHECK_ITEM(1, list_make(), a, 0, "abc")
	else CHECK_ITEM(1, list_make(), a, 1, "def")
	else CHECK_ITEM(1, list_make(), a, 2, "ghi")
	else CHECK_ITEM(1, list_make(), a, 3, "jkl")

	/* Test list_create(NULL), list_empty, list_append, list_prepend, list_insert, list_last */

	TEST_ACT(2, b = list_create(NULL))
	TEST_ACT(3, list_empty(b))

	TEST_ACT(4, list_append(b, "abc"))
	else CHECK_LENGTH(4, list_append(b, "abc"), b, 1)
	else CHECK_ITEM(4, list_append(b, "abc"), b, 0, "abc")

	TEST_ACT(5, !list_empty(b))

	TEST_ACT(6, list_prepend(b, "def"))
	else CHECK_LENGTH(6, list_prepend(b, "def"), b, 2)
	else CHECK_ITEM(6, list_prepend(b, "def"), b, 0, "def")
	else CHECK_ITEM(6, list_prepend(b, "def"), b, 1, "abc")

	TEST_ACT(7, list_insert(b, 1, "ghi"))
	else CHECK_LENGTH(7, list_insert(b, "ghi"), b, 3)
	else CHECK_ITEM(7, list_insert(b, "ghi"), b, 0, "def")
	else CHECK_ITEM(7, list_insert(b, "ghi"), b, 1, "ghi")
	else CHECK_ITEM(7, list_insert(b, "ghi"), b, 2, "abc")

	TEST_EQ(8, list_last(b), 2)

	TEST_EQ(9, list_length(NULL), -1)
	else if (errno != EINVAL)
		++errors, printf("Test9: list_length(NULL) failed (errno = %d, not EINVAL)\n", errno);

	TEST_EQ(10, list_empty(NULL), -1)
	else if (errno != EINVAL)
		++errors, printf("Test10: list_empty(NULL) failed (errno = %d, not EINVAL)\n", errno);

	/* Test list_copy, list_destroy */

	if ((c = list_copy(a, (list_copy_t *)free)))
		++errors, printf("Test11: list_copy() with copy() but no destroy() failed\n");

	if (!(c = list_copy(a, NULL)))
		++errors, printf("Test12: list_copy() without copy() or destroy() failed\n");
	else CHECK_LENGTH(12, list_copy(a, NULL), c, 4)
	else CHECK_ITEM(12, list_copy(a, NULL), c, 0, "abc")
	else CHECK_ITEM(12, list_copy(a, NULL), c, 1, "def")
	else CHECK_ITEM(12, list_copy(a, NULL), c, 2, "ghi")
	else CHECK_ITEM(12, list_copy(a, NULL), c, 3, "jkl")

	list_destroy(&c);
	if (c)
		++errors, printf("Test13: list_destroy(&c) failed\n");

	/* Test list_create(free), list_append, list_copy */

	if (!(c = list_create((list_release_t *)free)))
		++errors, printf("Test14: list_create(free) failed\n");
	else
	{
		TEST_ACT(15, list_append(c, mem_strdup("abc")))
		TEST_ACT(16, list_append(c, mem_strdup("def")))
		TEST_ACT(17, list_append(c, mem_strdup("ghi")))
		TEST_ACT(18, list_append(c, mem_strdup("jkl")))

		if ((d = list_copy(c, NULL)))
			++errors, printf("Test19: list_copy() with destroy() but no copy() failed\n");

		if (!(d = list_copy(c, (list_copy_t *)mem_strdup)))
			++errors, printf("Test20: list_copy() with copy() and destroy() failed\n");
	}

	/* Test list_remove, list_replace */

	TEST_ACT(21, list_remove(a, 3))
	else CHECK_LENGTH(21, list_remove(a, 3), a, 3)
	else CHECK_ITEM(21, list_remove(a, 3), a, 0, "abc")
	else CHECK_ITEM(21, list_remove(a, 3), a, 1, "def")
	else CHECK_ITEM(21, list_remove(a, 3), a, 2, "ghi")

	TEST_ACT(22, list_remove(a, 0))
	else CHECK_LENGTH(22, list_remove(a, 0), a, 2)
	else CHECK_ITEM(22, list_remove(a, 0), a, 0, "def")
	else CHECK_ITEM(22, list_remove(a, 0), a, 1, "ghi")

	TEST_ACT(23, list_replace(a, 1, 1, "123"))
	else CHECK_LENGTH(23, list_replace(a, 1, 1, "123"), a, 2)
	else CHECK_ITEM(23, list_replace(a, 1, 1, "123"), a, 0, "def")
	else CHECK_ITEM(23, list_replace(a, 1, 1, "123"), a, 1, "123")

	/* Test list_append_list, list_prepend_list, list_insert_list */

	TEST_ACT(24, list_append_list(a, b, NULL))
	else CHECK_LENGTH(24, list_append_list(a, b, NULL), a, 5)
	else CHECK_ITEM(24, list_append_list(a, b, NULL), a, 0, "def")
	else CHECK_ITEM(24, list_append_list(a, b, NULL), a, 1, "123")
	else CHECK_ITEM(24, list_append_list(a, b, NULL), a, 2, "def")
	else CHECK_ITEM(24, list_append_list(a, b, NULL), a, 3, "ghi")
	else CHECK_ITEM(24, list_append_list(a, b, NULL), a, 4, "abc")

	TEST_ACT(25, list_prepend_list(a, c, NULL))
	else CHECK_LENGTH(25, list_prepend_list(a, c, NULL), a, 9)
	else CHECK_ITEM(25, list_prepend_list(a, c, NULL), a, 0, "abc")
	else CHECK_ITEM(25, list_prepend_list(a, c, NULL), a, 1, "def")
	else CHECK_ITEM(25, list_prepend_list(a, c, NULL), a, 2, "ghi")
	else CHECK_ITEM(25, list_prepend_list(a, c, NULL), a, 3, "jkl")
	else CHECK_ITEM(25, list_prepend_list(a, c, NULL), a, 4, "def")
	else CHECK_ITEM(25, list_prepend_list(a, c, NULL), a, 5, "123")
	else CHECK_ITEM(25, list_prepend_list(a, c, NULL), a, 6, "def")
	else CHECK_ITEM(25, list_prepend_list(a, c, NULL), a, 7, "ghi")
	else CHECK_ITEM(25, list_prepend_list(a, c, NULL), a, 8, "abc")

	TEST_ACT(26, list_insert_list(b, 1, c, NULL))
	else CHECK_LENGTH(26, list_insert_list(b, 1, c, NULL), b, 7)
	else CHECK_ITEM(26, list_insert_list(b, 1, c, NULL), b, 0, "def")
	else CHECK_ITEM(26, list_insert_list(b, 1, c, NULL), b, 1, "abc")
	else CHECK_ITEM(26, list_insert_list(b, 1, c, NULL), b, 2, "def")
	else CHECK_ITEM(26, list_insert_list(b, 1, c, NULL), b, 3, "ghi")
	else CHECK_ITEM(26, list_insert_list(b, 1, c, NULL), b, 4, "jkl")
	else CHECK_ITEM(26, list_insert_list(b, 1, c, NULL), b, 5, "ghi")
	else CHECK_ITEM(26, list_insert_list(b, 1, c, NULL), b, 6, "abc")

	/* Test list_replace_list, list_remove_range */

	if (list_replace_list(c, 1, 2, d, NULL))
		++errors, printf("Test27: list_replace_list() with destroy() but not copy() failed\n");

	if (list_replace_list(a, 1, 2, d, (list_copy_t *)mem_strdup))
		++errors, printf("Test28: list_replace_list() with copy() but not destroy() failed\n");

	TEST_ACT(29, list_replace_list(a, 1, 2, d, NULL))
	else CHECK_LENGTH(29, list_replace_list(a, 1, 2, d, NULL), a, 11)
	else CHECK_ITEM(29, list_replace_list(a, 1, 2, d, NULL), a, 0, "abc")
	else CHECK_ITEM(29, list_replace_list(a, 1, 2, d, NULL), a, 1, "abc")
	else CHECK_ITEM(29, list_replace_list(a, 1, 2, d, NULL), a, 2, "def")
	else CHECK_ITEM(29, list_replace_list(a, 1, 2, d, NULL), a, 3, "ghi")
	else CHECK_ITEM(29, list_replace_list(a, 1, 2, d, NULL), a, 4, "jkl")
	else CHECK_ITEM(29, list_replace_list(a, 1, 2, d, NULL), a, 5, "jkl")
	else CHECK_ITEM(29, list_replace_list(a, 1, 2, d, NULL), a, 6, "def")
	else CHECK_ITEM(29, list_replace_list(a, 1, 2, d, NULL), a, 7, "123")
	else CHECK_ITEM(29, list_replace_list(a, 1, 2, d, NULL), a, 8, "def")
	else CHECK_ITEM(29, list_replace_list(a, 1, 2, d, NULL), a, 9, "ghi")
	else CHECK_ITEM(29, list_replace_list(a, 1, 2, d, NULL), a, 10, "abc")

	TEST_ACT(30, list_remove_range(b, 1, 3))
	else CHECK_LENGTH(30, list_remove_range(b, 1, 3), b, 4)
	else CHECK_ITEM(30, list_remove_range(b, 1, 3), b, 0, "def")
	else CHECK_ITEM(30, list_remove_range(b, 1, 3), b, 1, "jkl")
	else CHECK_ITEM(30, list_remove_range(b, 1, 3), b, 2, "ghi")
	else CHECK_ITEM(30, list_remove_range(b, 1, 3), b, 3, "abc")

	/* Test list_apply, list_query, list_sort */

	list_apply(a, (list_action_t *)action, NULL);
	if (strcmp(action_data, "abcabcdefghijkljkldef123defghiabc"))
		++errors, printf("Test31: list_apply() failed\n");

	for (i = 0; list_query(a, &index, (list_query_t *)query, NULL) != -1; ++i, ++index)
	{
		if (index != query_data[i])
		{
			++errors, printf("Test32: list_query returned %d (not %d)\n", (int)index, query_data[i]);
			break;
		}
	}

	TEST_ACT(33, list_sort(a, (list_cmp_t *)sort_cmp))
	else CHECK_LENGTH(33, list_sort(a, sort_cmp), a, 11)
	else CHECK_ITEM(33, list_sort(a, sort_cmp), a, 0, "123")
	else CHECK_ITEM(33, list_sort(a, sort_cmp), a, 1, "abc")
	else CHECK_ITEM(33, list_sort(a, sort_cmp), a, 2, "abc")
	else CHECK_ITEM(33, list_sort(a, sort_cmp), a, 3, "abc")
	else CHECK_ITEM(33, list_sort(a, sort_cmp), a, 4, "def")
	else CHECK_ITEM(33, list_sort(a, sort_cmp), a, 5, "def")
	else CHECK_ITEM(33, list_sort(a, sort_cmp), a, 6, "def")
	else CHECK_ITEM(33, list_sort(a, sort_cmp), a, 7, "ghi")
	else CHECK_ITEM(33, list_sort(a, sort_cmp), a, 8, "ghi")
	else CHECK_ITEM(33, list_sort(a, sort_cmp), a, 9, "jkl")
	else CHECK_ITEM(33, list_sort(a, sort_cmp), a, 10, "jkl")

	/* Test lister_create, lister_has_next, lister_next, lister_destroy */

	TEST_ACT(34, lister = lister_create(a))

	for (i = 0; lister_has_next(lister) == 1; ++i)
	{
		void *item = lister_next(lister);

		if (item != a->list[i]) /* white box */
		{
			++errors, printf("Test35: iteration %d is '%s' not '%s'\n", i, (char *)item, (char *)list_item(a, i));
			break;
		}
	}

	lister_destroy(&lister);
	if (lister)
		++errors, printf("Test36: lister_destroy(&lister) failed, lister is %p not NULL\n", (void *)lister);

	/* Test list_has_next, list_next, list_break */

	for (i = 0; list_has_next(a) == 1; ++i)
	{
		void *item = list_next(a);

		if (item != a->list[i]) /* white box */
		{
			++errors, printf("Test37: internal iteration %d is '%s' not '%s'\n", i, (char *)item, (char *)list_item(a, i));
			list_break(a);
			break;
		}
	}

	for (i = 0; list_has_next(a) == 1; ++i)
	{
		void *item = list_next(a);

		if (item != a->list[i]) /* white box */
		{
			++errors, printf("Test38: internal iteration %d is '%s' not '%s'\n", i, (char *)item, (char *)list_item(a, i));
			list_break(a);
			break;
		}

		if (i == 2)
		{
			list_break(a);
			break;
		}
	}

	if (a->lister)
		++errors, printf("Test38: list_break() failed\n");

	/* Test lister_remove */

	if (!(lister = lister_create(a)))
		++errors, printf("Test39: lister_create() failed\n");

	for (i = 0; lister_has_next(lister) == 1; ++i)
	{
		lister_next(lister);

		if (i == 4)
			lister_remove(lister);
	}

	lister_destroy(&lister);
	if (lister)
		++errors, printf("Test40: lister_destroy(&lister) failed, lister is %p not NULL\n", (void *)lister);

	CHECK_LENGTH(41, list_remove(lister), a, 10)
	else CHECK_ITEM(41, lister_remove(lister), a, 0, "123")
	else CHECK_ITEM(41, lister_remove(lister), a, 1, "abc")
	else CHECK_ITEM(41, lister_remove(lister), a, 2, "abc")
	else CHECK_ITEM(41, lister_remove(lister), a, 3, "abc")
	else CHECK_ITEM(41, lister_remove(lister), a, 4, "def")
	else CHECK_ITEM(41, lister_remove(lister), a, 5, "def")
	else CHECK_ITEM(41, lister_remove(lister), a, 6, "ghi")
	else CHECK_ITEM(41, lister_remove(lister), a, 7, "ghi")
	else CHECK_ITEM(41, lister_remove(lister), a, 8, "jkl")
	else CHECK_ITEM(41, lister_remove(lister), a, 9, "jkl")

	list_destroy(&a);
	if (a)
		++errors, printf("Test42: list_destroy(&a) failed, a is %p not NULL\n", (void *)a);

	list_destroy(&b);
	if (b)
		++errors, printf("Test43: list_destroy(&b) failed, b is %p not NULL\n", (void *)b);

	list_destroy(&c);
	if (c)
		++errors, printf("Test44: list_destroy(&c) failed, c is %p not NULL\n", (void *)c);

	list_destroy(&d);
	if (d)
		++errors, printf("Test45: list_destroy(&d) failed, d is %p not NULL\n", (void *)d);

	/* Test relative index/range */

	TEST_ACT(46, a = list_make(NULL, "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", NULL))
	else
	{
		void *item;

		TEST_ACT(47, list_item(a, -1) == NULL)
		TEST_ACT(47, item = list_item(a, -2))
		TEST_ACT(47, !strcmp(item, "9"))
		TEST_ACT(47, item = list_item(a, -3))
		TEST_ACT(47, !strcmp(item, "8"))
		TEST_ACT(47, item = list_item(a, -4))
		TEST_ACT(47, !strcmp(item, "7"))
		TEST_ACT(47, item = list_item(a, -5))
		TEST_ACT(47, !strcmp(item, "6"))
		TEST_ACT(47, item = list_item(a, -6))
		TEST_ACT(47, !strcmp(item, "5"))
		TEST_ACT(47, item = list_item(a, -7))
		TEST_ACT(47, !strcmp(item, "4"))
		TEST_ACT(47, item = list_item(a, -8))
		TEST_ACT(47, !strcmp(item, "3"))
		TEST_ACT(47, item = list_item(a, -9))
		TEST_ACT(47, !strcmp(item, "2"))
		TEST_ACT(47, item = list_item(a, -10))
		TEST_ACT(47, !strcmp(item, "1"))
		TEST_ACT(47, item = list_item(a, -11))
		TEST_ACT(47, !strcmp(item, "0"))
		TEST_ACT(47, list_item(a, -12) == NULL)
		TEST_ACT(47, list_item(a, -1000) == NULL)

		TEST_ACT(48, list_remove_range(a, -5, -1))
		else CHECK_LENGTH(48, list_remove_range(a, -5, -1), a, 6)
		else CHECK_ITEM(48, list_remove_range(a, -5, -1), a, 0, "0")
		else CHECK_ITEM(48, list_remove_range(a, -5, -1), a, 1, "1")
		else CHECK_ITEM(48, list_remove_range(a, -5, -1), a, 2, "2")
		else CHECK_ITEM(48, list_remove_range(a, -5, -1), a, 3, "3")
		else CHECK_ITEM(48, list_remove_range(a, -5, -1), a, 4, "4")
		else CHECK_ITEM(48, list_remove_range(a, -5, -1), a, 5, "5")

		TEST_ACT(49, list_remove_range(a, -1, -1))
		else CHECK_LENGTH(49, list_remove_range(a, -1, -1), a, 6)
		else CHECK_ITEM(49, list_remove_range(a, -1, -1), a, 0, "0")
		else CHECK_ITEM(49, list_remove_range(a, -1, -1), a, 1, "1")
		else CHECK_ITEM(49, list_remove_range(a, -1, -1), a, 2, "2")
		else CHECK_ITEM(49, list_remove_range(a, -1, -1), a, 3, "3")
		else CHECK_ITEM(49, list_remove_range(a, -1, -1), a, 4, "4")
		else CHECK_ITEM(49, list_remove_range(a, -1, -1), a, 5, "5")

		TEST_ACT(50, list_remove_range(a, -3, -2))
		else CHECK_LENGTH(50, list_remove_range(a, -3, -2), a, 5)
		else CHECK_ITEM(50, list_remove_range(a, -3, -2), a, 0, "0")
		else CHECK_ITEM(50, list_remove_range(a, -3, -2), a, 1, "1")
		else CHECK_ITEM(50, list_remove_range(a, -3, -2), a, 2, "2")
		else CHECK_ITEM(50, list_remove_range(a, -3, -2), a, 3, "3")
		else CHECK_ITEM(50, list_remove_range(a, -3, -2), a, 4, "5")

		TEST_ACT(51, list_insert(a, -1, "X"))
		else CHECK_LENGTH(51, list_insert(a, -1, "X"), a, 6)
		else CHECK_ITEM(51, list_insert(a, -1, "X"), a, 0, "0")
		else CHECK_ITEM(51, list_insert(a, -1, "X"), a, 1, "1")
		else CHECK_ITEM(51, list_insert(a, -1, "X"), a, 2, "2")
		else CHECK_ITEM(51, list_insert(a, -1, "X"), a, 3, "3")
		else CHECK_ITEM(51, list_insert(a, -1, "X"), a, 4, "5")
		else CHECK_ITEM(51, list_insert(a, -1, "X"), a, 5, "X")

		TEST_ACT(52, b = list_make(NULL, "a", "b", "c", NULL))
		else TEST_ACT(52, list_insert_list(a, -1, b, NULL))
		else CHECK_LENGTH(52, list_insert_list(a, -1, b, NULL), a, 9)
		else CHECK_ITEM(52, list_insert_list(a, -1, b, NULL), a, 0, "0")
		else CHECK_ITEM(52, list_insert_list(a, -1, b, NULL), a, 1, "1")
		else CHECK_ITEM(52, list_insert_list(a, -1, b, NULL), a, 2, "2")
		else CHECK_ITEM(52, list_insert_list(a, -1, b, NULL), a, 3, "3")
		else CHECK_ITEM(52, list_insert_list(a, -1, b, NULL), a, 4, "5")
		else CHECK_ITEM(52, list_insert_list(a, -1, b, NULL), a, 5, "X")
		else CHECK_ITEM(52, list_insert_list(a, -1, b, NULL), a, 6, "a")
		else CHECK_ITEM(52, list_insert_list(a, -1, b, NULL), a, 7, "b")
		else CHECK_ITEM(52, list_insert_list(a, -1, b, NULL), a, 8, "c")
		list_destroy(&b);

		TEST_ACT(53, b = list_make(NULL, "x", "y", "z", NULL))
		else TEST_ACT(53, list_insert_list(a, -10, b, NULL))
		else CHECK_LENGTH(53, list_insert_list(a, -10, b, NULL), a, 12)
		else CHECK_ITEM(53, list_insert_list(a, -10, b, NULL), a, 0, "x")
		else CHECK_ITEM(53, list_insert_list(a, -10, b, NULL), a, 1, "y")
		else CHECK_ITEM(53, list_insert_list(a, -10, b, NULL), a, 2, "z")
		else CHECK_ITEM(53, list_insert_list(a, -10, b, NULL), a, 3, "0")
		else CHECK_ITEM(53, list_insert_list(a, -10, b, NULL), a, 4, "1")
		else CHECK_ITEM(53, list_insert_list(a, -10, b, NULL), a, 5, "2")
		else CHECK_ITEM(53, list_insert_list(a, -10, b, NULL), a, 6, "3")
		else CHECK_ITEM(53, list_insert_list(a, -10, b, NULL), a, 7, "5")
		else CHECK_ITEM(53, list_insert_list(a, -10, b, NULL), a, 8, "X")
		else CHECK_ITEM(53, list_insert_list(a, -10, b, NULL), a, 9, "a")
		else CHECK_ITEM(53, list_insert_list(a, -10, b, NULL), a, 10, "b")
		else CHECK_ITEM(53, list_insert_list(a, -10, b, NULL), a, 11, "c")
		list_destroy(&b);

		TEST_ACT(54, b = list_make(NULL, "0", NULL))
		else TEST_ACT(54, list_replace_list(a, -13, -9, b, NULL))
		else CHECK_LENGTH(54, list_replace_list(a, -13, -9, b, NULL), a, 9)
		else CHECK_ITEM(54, list_replace_list(a, -13, -9, b, NULL), a, 0, "0")
		else CHECK_ITEM(54, list_replace_list(a, -13, -9, b, NULL), a, 1, "1")
		else CHECK_ITEM(54, list_replace_list(a, -13, -9, b, NULL), a, 2, "2")
		else CHECK_ITEM(54, list_replace_list(a, -13, -9, b, NULL), a, 3, "3")
		else CHECK_ITEM(54, list_replace_list(a, -13, -9, b, NULL), a, 4, "5")
		else CHECK_ITEM(54, list_replace_list(a, -13, -9, b, NULL), a, 5, "X")
		else CHECK_ITEM(54, list_replace_list(a, -13, -9, b, NULL), a, 6, "a")
		else CHECK_ITEM(54, list_replace_list(a, -13, -9, b, NULL), a, 7, "b")
		else CHECK_ITEM(54, list_replace_list(a, -13, -9, b, NULL), a, 8, "c")
		list_destroy(&b);

		TEST_ACT(55, b = list_make(NULL, "X", NULL))
		else TEST_ACT(55, list_replace_list(a, -5, -1, b, NULL))
		else CHECK_LENGTH(55, list_replace_list(a, -5, -1, b, NULL), a, 6)
		else CHECK_ITEM(55, list_replace_list(a, -5, -1, b, NULL), a, 0, "0")
		else CHECK_ITEM(55, list_replace_list(a, -5, -1, b, NULL), a, 1, "1")
		else CHECK_ITEM(55, list_replace_list(a, -5, -1, b, NULL), a, 2, "2")
		else CHECK_ITEM(55, list_replace_list(a, -5, -1, b, NULL), a, 3, "3")
		else CHECK_ITEM(55, list_replace_list(a, -5, -1, b, NULL), a, 4, "5")
		else CHECK_ITEM(55, list_replace_list(a, -5, -1, b, NULL), a, 5, "X")
		list_destroy(&b);

		TEST_ACT(56, list_replace(a, -5, -2, "Y"))
		else CHECK_LENGTH(56, list_replace(a, -5, -2, "Y"), a, 4)
		else CHECK_ITEM(56, list_replace(a, -5, -2, "Y"), a, 0, "0")
		else CHECK_ITEM(56, list_replace(a, -5, -2, "Y"), a, 1, "1")
		else CHECK_ITEM(56, list_replace(a, -5, -2, "Y"), a, 2, "Y")
		else CHECK_ITEM(56, list_replace(a, -5, -2, "Y"), a, 3, "X")

		TEST_ACT(57, b = list_extract(a, -4, -2, NULL))
		else CHECK_LENGTH(57, b = list_extract(a, -4, -2, NULL), b, 2)
		else CHECK_ITEM(57, b = list_extract(a, -4, -2, NULL), b, 0, "1")
		else CHECK_ITEM(57, b = list_extract(a, -4, -2, NULL), b, 1, "Y")
		list_destroy(&b);

		/* Test relative error checking */

		TEST_ACT(58, list_remove_range(a, -2, -1000) == NULL)
		TEST_ACT(59, list_remove_range(a, -1000, -2) == NULL)
		TEST_ACT(60, list_remove_range(a, -1000, -1000) == NULL)

		TEST_ACT(61, list_insert(a, -1000, "?") == NULL)

		TEST_ACT(62, b = list_make(NULL, "?", NULL))
		else
		{
			TEST_ACT(63, list_insert_list(a, -1000, b, NULL) == NULL)
			TEST_ACT(64, list_replace_list(a, -2, -1000, b, NULL) == NULL)
			TEST_ACT(65, list_replace_list(a, -1000, -2, b, NULL) == NULL)
			TEST_ACT(66, list_replace_list(a, -1000, -1000, b, NULL) == NULL)
			list_destroy(&b);
		}

		TEST_ACT(67, list_replace(a, -2, -1000, "?") == NULL)
		TEST_ACT(68, list_replace(a, -1000, -2, "?") == NULL)
		TEST_ACT(69, list_replace(a, -1000, -1000, "?") == NULL)

		TEST_ACT(70, list_extract(a, -2, -1000, NULL) == NULL)
		TEST_ACT(71, list_extract(a, -1000, -2, NULL) == NULL)
		TEST_ACT(72, list_extract(a, -1000, -1000, NULL) == NULL)

		list_destroy(&a);
	}

	/* Test lists with int items, list_map, list_grep */

	TEST_ACT(73, a = list_create(NULL))
	else
	{
		TEST_ACT(74, list_append_int(a, 2))
		else CHECK_LENGTH(74, list_append_int(a, 2), a, 1)
		else CHECK_INT_ITEM(74, list_append_int(a, 2), a, 0, 2)

		TEST_ACT(75, list_prepend_int(a, 0))
		else CHECK_LENGTH(75, list_prepend_int(a, 0), a, 2)
		else CHECK_INT_ITEM(75, list_prepend_int(a, 0), a, 0, 0)
		else CHECK_INT_ITEM(75, list_prepend_int(a, 0), a, 1, 2)

		TEST_ACT(76, list_insert_int(a, 1, 1))
		else CHECK_LENGTH(76, list_insert_int(a, 1, 1), a, 3)
		else CHECK_INT_ITEM(76, list_insert_int(a, 1, 1), a, 0, 0)
		else CHECK_INT_ITEM(76, list_insert_int(a, 1, 1), a, 1, 1)
		else CHECK_INT_ITEM(76, list_insert_int(a, 1, 1), a, 2, 2)

		for (i = 0; list_has_next(a) == 1; ++i)
		{
			int item = list_next_int(a);

			if (item != (int)(long)a->list[i]) /* white box */
				++errors, printf("Test77: int list test failed (item %d = %d, not %d)\n", i, item, list_item_int(a, i));
		}

		if (i != 3)
			++errors, printf("Test78: list_has_next() failed (only %d items, not %d)\n", i, 3);

		if (!(lister = lister_create(a)))
			++errors, printf("Test79: lister_create(a) failed\n");
		else
		{
			for (i = 0; lister_has_next(lister) == 1; ++i)
			{
				int item = lister_next_int(lister);

				if (item != (int)(long)a->list[i]) /* white box */
					++errors, printf("Test80: int list test failed (item %d = %d, not %d)\n", i, item, i);
			}

			if (i != 3)
				++errors, printf("Test81: lister_has_next() failed (only %d items, not %d)\n", i, 3);

			lister_destroy(&lister);
		}

		TEST_ACT(82, list_replace_int(a, 2, 1, 4))
		else CHECK_LENGTH(82, list_replace_int(a, 2, 1, 4), a, 3)
		else CHECK_INT_ITEM(82, list_replace_int(a, 2, 1, 4), a, 0, 0)
		else CHECK_INT_ITEM(82, list_replace_int(a, 2, 1, 4), a, 1, 1)
		else CHECK_INT_ITEM(82, list_replace_int(a, 2, 1, 4), a, 2, 4)

		i = 0;
		TEST_ACT(83, b = list_map(a, NULL, (list_map_t *)mapf, &i))
		else
		{
			CHECK_LENGTH(83, b = list_map(), b, 3)
			CHECK_INT_ITEM(83, b = list_map(), b, 0, 0)
			CHECK_INT_ITEM(83, b = list_map(), b, 1, 1)
			CHECK_INT_ITEM(83, b = list_map(), b, 2, 5)
			list_destroy(&b);
		}

		TEST_ACT(84, b = list_grep(a, (list_query_t *)grepf, NULL))
		else
		{
			CHECK_LENGTH(84, b = list_grep(), b, 2)
			CHECK_INT_ITEM(84, b = list_grep(), b, 0, 0)
			CHECK_INT_ITEM(84, b = list_grep(), b, 1, 4)
			list_destroy(&b);
		}

		list_destroy(&a);
	}

	/* Test list_push_int, list_pop_int, list_unshift_int, list_shift_int */

	TEST_ACT(85, a = list_create(NULL))
	else
	{
		TEST_ACT(86, list_push_int(a, 1))
		TEST_ACT(87, list_push_int(a, 2))
		TEST_ACT(88, list_push_int(a, 3))
		TEST_ACT(89, list_push_int(a, 0))
		TEST_ACT(90, list_push_int(a, 5))
		TEST_ACT(91, list_push_int(a, 6))
		TEST_ACT(92, list_push_int(a, 7))

		TEST_EQ(93, list_pop_int(a), 7)
		TEST_EQ(94, list_pop_int(a), 6)
		TEST_EQ(95, list_pop_int(a), 5)
		TEST_EQ(96, list_pop_int(a), 0)
		TEST_EQ(97, list_pop_int(a), 3)
		TEST_EQ(98, list_pop_int(a), 2)
		TEST_EQ(99, list_pop_int(a), 1)
		TEST_EQ(100, list_pop_int(a), 0)

		TEST_ACT(101, list_unshift_int(a, 1))
		TEST_ACT(102, list_unshift_int(a, 2))
		TEST_ACT(103, list_unshift_int(a, 3))
		TEST_ACT(104, list_unshift_int(a, 0))
		TEST_ACT(105, list_unshift_int(a, 5))
		TEST_ACT(106, list_unshift_int(a, 6))
		TEST_ACT(107, list_unshift_int(a, 7))

		TEST_EQ(108, list_shift_int(a), 7)
		TEST_EQ(109, list_shift_int(a), 6)
		TEST_EQ(110, list_shift_int(a), 5)
		TEST_EQ(111, list_shift_int(a), 0)
		TEST_EQ(112, list_shift_int(a), 3)
		TEST_EQ(113, list_shift_int(a), 2)
		TEST_EQ(114, list_shift_int(a), 1)
		TEST_EQ(115, list_shift_int(a), 0)

		list_destroy(&a);
	}

	/* Test list_push, list_pop, list_unshift, list_shift */

	TEST_ACT(116, a = list_create(free))
	else
	{
		char *item;

		TEST_ACT(117, list_push(a, mem_strdup("1")))
		TEST_ACT(118, list_push(a, mem_strdup("2")))
		TEST_ACT(119, list_push(a, mem_strdup("3")))
		TEST_ACT(120, list_push(a, mem_strdup("4")))
		TEST_ACT(121, list_push(a, mem_strdup("5")))
		TEST_ACT(122, list_push(a, mem_strdup("6")))
		TEST_ACT(123, list_push(a, mem_strdup("7")))

#define CHECK_POP(i, action, value) \
	if (!(item = (action)) || strcmp(item, (value))) \
		++errors, printf("Test%d: %s failed (\"%s\", not \"%s\")\n", (i), (#action), (item) ? item : "NULL", (value) ? (value) : NULL); \
	free(item);

		CHECK_POP(124, list_pop(a), "7")
		CHECK_POP(125, list_pop(a), "6")
		CHECK_POP(126, list_pop(a), "5")
		CHECK_POP(127, list_pop(a), "4")
		CHECK_POP(128, list_pop(a), "3")
		CHECK_POP(129, list_pop(a), "2")
		CHECK_POP(130, list_pop(a), "1")
		TEST_ACT(131, !list_pop(a))

		TEST_ACT(132, list_unshift(a, mem_strdup("1")))
		TEST_ACT(133, list_unshift(a, mem_strdup("2")))
		TEST_ACT(134, list_unshift(a, mem_strdup("3")))
		TEST_ACT(135, list_unshift(a, mem_strdup("4")))
		TEST_ACT(136, list_unshift(a, mem_strdup("5")))
		TEST_ACT(137, list_unshift(a, mem_strdup("6")))
		TEST_ACT(138, list_unshift(a, mem_strdup("7")))

		CHECK_POP(139, list_shift(a), "7")
		CHECK_POP(140, list_shift(a), "6")
		CHECK_POP(141, list_shift(a), "5")
		CHECK_POP(142, list_shift(a), "4")
		CHECK_POP(143, list_shift(a), "3")
		CHECK_POP(144, list_shift(a), "2")
		CHECK_POP(145, list_shift(a), "1")
		TEST_ACT(146, !list_shift(a))

		list_destroy(&a);
	}

	/* Test list_make, list_splice */

	TEST_ACT(147, a = list_make(NULL, "a", "b", "c", "d", "e", "f", NULL))
	else
	{
		List *splice;

		TEST_ACT(148, splice = list_splice(a, 0, 1, NULL))
		else
		{
			CHECK_LENGTH(149, splice = list_splice(a, 0, 1, NULL), splice, 1)
			CHECK_ITEM(150, splice = list_splice(a, 0, 1, NULL), splice, 0, "a")
			CHECK_LENGTH(151, splice = list_splice(a, 0, 1, NULL), a, 5)
			CHECK_ITEM(152, splice = list_splice(a, 0, 1, NULL), a, 0, "b")
			CHECK_ITEM(153, splice = list_splice(a, 0, 1, NULL), a, 1, "c")
			CHECK_ITEM(154, splice = list_splice(a, 0, 1, NULL), a, 2, "d")
			CHECK_ITEM(155, splice = list_splice(a, 0, 1, NULL), a, 3, "e")
			CHECK_ITEM(156, splice = list_splice(a, 0, 1, NULL), a, 4, "f")
			list_destroy(&splice);
		}

		TEST_ACT(157, splice = list_splice(a, 4, 1, NULL))
		else
		{
			CHECK_LENGTH(158, splice = list_splice(a, 4, 1, NULL), splice, 1)
			CHECK_ITEM(159, splice = list_splice(a, 4, 1, NULL), splice, 0, "f")
			CHECK_LENGTH(160, splice = list_splice(a, 4, 1, NULL), a, 4)
			CHECK_ITEM(161, splice = list_splice(a, 4, 1, NULL), a, 0, "b")
			CHECK_ITEM(162, splice = list_splice(a, 4, 1, NULL), a, 1, "c")
			CHECK_ITEM(163, splice = list_splice(a, 4, 1, NULL), a, 2, "d")
			CHECK_ITEM(164, splice = list_splice(a, 4, 1, NULL), a, 3, "e")
			list_destroy(&splice);
		}

		TEST_ACT(165, splice = list_splice(a, 1, 2, NULL))
		else
		{
			CHECK_LENGTH(166, splice = list_splice(a, 1, 2, NULL), splice, 2)
			CHECK_ITEM(167, splice = list_splice(a, 1, 2, NULL), splice, 0, "c")
			CHECK_ITEM(168, splice = list_splice(a, 1, 2, NULL), splice, 1, "d")
			CHECK_LENGTH(169, splice = list_splice(a, 1, 2, NULL), a, 2)
			CHECK_ITEM(170, splice = list_splice(a, 1, 2, NULL), a, 0, "b")
			CHECK_ITEM(171, splice = list_splice(a, 1, 2, NULL), a, 1, "e")
			list_destroy(&splice);
		}

		list_destroy(&a);
	}

	/* Test MT Safety */

	debug = av[1] && !strcmp(av[1], "debug");

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
		++errors, printf("Test172: locker_create_rwlock() failed\n");
	else
	{
		mt_test(173, locker);
		locker_destroy(&locker);
	}

	if (debug)
		locker = locker_create_debug_mutex(&mutex);
	else
		locker = locker_create_mutex(&mutex);

	if (!locker)
		++errors, printf("Test174: locker_create_mutex() failed\n");
	else
	{
		mt_test(175, locker);
		locker_destroy(&locker);
	}

	/* Test assumption: sizeof(int) <= sizeof(void *) */

	if (sizeof(int) > sizeof(void *))
		++errors, printf("Test176: assumption failed: sizeof(int) > sizeof(void *): int lists are limited to %d bytes\n", (int)sizeof(void *));

	if (errors)
		printf("%d/176 tests failed\n", errors);
	else
		printf("All tests passed\n");

	return (errors == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

#endif

/* vi:set ts=4 sw=4: */
