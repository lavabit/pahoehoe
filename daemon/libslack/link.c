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
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
* or visit http://www.gnu.org/copyleft/gpl.html
*
* 20201111 raf <raf@raf.org>
*/

/*

=head1 NAME

I<libslack(link)> - linked list module

=head1 SYNOPSIS

    #include <slack/std.h>
    #include <slack/link.h>

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

=head1 DESCRIPTION

This module provides functions for manipulating singly and doubly linked
lists. Two abstract types are defined: I<slink_t>, containing a pointer to
the next item, and I<dlink_t>, containing pointers to the next and previous
items. These functions work with any struct whose first element is an
I<slink_t> or a I<dlink_t> struct. There is support for optional growable
free lists so items may be dynamically allocated individually or allocated
from a free list. Free lists can be arrays of structs or dynamically
allocated. When a free list is exhausted, further memory may be attached to
the free list to extend it.

=over 4

=cut

*/

#include "config.h"
#include "std.h"

#include "link.h"
#include "err.h"

#ifndef TEST

/*

=item C<int slink_has_next(void *link)>

Returns C<1> if C<link>'s C<next> pointer is not C<null>. Otherwise, returns
C<0>. On error, returns C<-1> with C<errno> set appropriately.

=cut

*/

int slink_has_next(void *link)
{
	if (!link)
		return set_errno(EINVAL);

	return (((slink_t *)link)->next != NULL);
}

/*

=item C<void *slink_next(void *link)>

Returns C<link>'s C<next> pointer. On error, returns C<null> with C<errno>
set appropriately.

=cut

*/

void *slink_next(void *link)
{
	if (!link)
		return set_errnull(EINVAL);

	return ((slink_t *)link)->next;
}

/*

=item C<int dlink_has_next(void *link)>

Returns C<1> if C<link>'s C<next> pointer is not C<null>. Otherwise, returns
C<0>. On error, returns C<-1> with C<errno> set appropriately.

=cut

*/

int dlink_has_next(void *link)
{
	if (!link)
		return set_errno(EINVAL);

	return (((dlink_t *)link)->next != NULL);
}

/*

=item C<void *dlink_next(void *link)>

Returns C<link>'s C<next> pointer. On error, returns C<null> with C<errno>
set appropriately.

=cut

*/

void *dlink_next(void *link)
{
	if (!link)
		return set_errnull(EINVAL);

	return ((dlink_t *)link)->next;
}

/*

=item C<int dlink_has_prev(void *link)>

Returns C<1> if C<link>'s C<prev> pointer is not C<null>. Otherwise, returns
C<0>. On error, returns C<-1> with C<errno> set appropriately.

=cut

*/

int dlink_has_prev(void *link)
{
	if (!link)
		return set_errno(EINVAL);

	return (((dlink_t *)link)->prev != NULL);
}

/*

=item C<void *dlink_prev(void *link)>

Returns C<link>'s C<prev> pointer. On error, returns C<null> with C<errno>
set appropriately.

=cut

*/

void *dlink_prev(void *link)
{
	if (!link)
		return set_errnull(EINVAL);

	return ((dlink_t *)link)->prev;
}

/*

=item C<void *slink_insert(void *link, void *item)>

Inserts C<item> before C<link>. Returns C<item>. On error, returns C<null>
with C<errno> set appropriately. Items may only be inserted at the beginning
of a singly linked list.

=cut

*/

void *slink_insert(void *link, void *item)
{
	slink_t *insert;

	if (!item)
		return set_errnull(EINVAL);

	insert = item;
	insert->next = link;

	return insert;
}

/*

=item C<void *dlink_insert(void *link, void *item)>

Inserts C<item> before C<link>. Returns C<item>. On error, returns C<null>
with C<errno> set appropriately. Items may be inserted anywhere in a doubly
linked list.

=cut

*/

void *dlink_insert(void *link, void *item)
{
	dlink_t *insert, *next, *prev;

	if (!item)
		return set_errnull(EINVAL);

	insert = item;
	next = link;
	prev = (next) ? next->prev : NULL;

	insert->next = next;
	insert->prev = prev;

	if (next)
		next->prev = insert;

	if (prev)
		prev->next = insert;

	return insert;
}

/*

=item C<void *slink_remove(void *link)>

Removes the first item from the list beginning with C<link>. On success,
returns C<link>'s C<next> pointer. On error, returns C<null> with C<errno>
set appropriately.

=cut

*/

void *slink_remove(void *link)
{
	slink_t *remove;

	if (!link)
		return set_errnull(EINVAL);

	remove = link;

	return remove->next;
}

/*

=item C<void *dlink_remove(void *link)>

Removes C<link> from the list of which it is part. On success, returns
C<link>'s C<next> pointer. On error, returns C<null> with C<errno> set
appropriately.

=cut

*/

void *dlink_remove(void *link)
{
	dlink_t *remove;

	if (!link)
		return set_errnull(EINVAL);

	remove = link;

	if (remove->next)
		((dlink_t *)remove->next)->prev = remove->prev;

	if (remove->prev)
		((dlink_t *)remove->prev)->next = remove->next;

	return remove->next;
}

/*

=item C<void *slink_freelist_init(void *freelist, size_t nelem, size_t size)>

Initialises an array of C<nelem> elements each C<size> bytes for use as a
singly linked free list. On success, returns C<freelist>. On error, returns
C<null> with C<errno> set appropriately.

=cut

*/

void *slink_freelist_init(void *freelist, size_t nelem, size_t size)
{
	char *link;

	if (!freelist || !nelem || !size)
		return set_errnull(EINVAL);

	for (link = freelist; --nelem; link += size)
	{
		((slink_t *)link)->next = link + size;
	}

	((slink_t *)link)->next = NULL;

	return freelist;
}

/*

=item C<void *dlink_freelist_init(void *freelist, size_t nelem, size_t size)>

Initialises an array of C<nelem> elements each C<size> bytes for use as a
doubly linked free list. On success, returns C<freelist>. On error, returns
C<null> with C<errno> set appropriately.

=cut

*/

void *dlink_freelist_init(void *freelist, size_t nelem, size_t size)
{
	char *link, *prev;

	if (!freelist || !nelem || !size)
		return set_errnull(EINVAL);

	for (prev = NULL, link = freelist; --nelem; prev = link, link += size)
	{
		((dlink_t *)link)->next = link + size;
		((dlink_t *)link)->prev = prev;
	}

	((dlink_t *)link)->next = NULL;
	((dlink_t *)link)->prev = prev;

	return freelist;
}

/*

=item C<void *slink_freelist_attach(void *freelist1, void *freelist2)>

Attaches C<freelist2> to the end of C<freelist1>. Both free lists must have
already been initialised with I<slink_freelist_init(3)>. Note that it will
not be possible to separate these free lists. On success, returns a pointer
to the beginning of the combined freelist. On error, returns C<null> with
C<errno> set appropriately.

=cut

*/

void *slink_freelist_attach(void *freelist1, void *freelist2)
{
	char *freelist;

	if (!freelist2)
		return set_errnull(EINVAL);

	if (!(freelist = freelist1))
		return freelist2;

	while (slink_has_next(freelist) == 1)
		freelist = slink_next(freelist);

	((slink_t *)freelist)->next = freelist2;

	return freelist1;
}

/*

=item C<void *dlink_freelist_attach(void *freelist1, void *freelist2)>

Attaches C<freelist2> to the end of C<freelist1>. Both free lists must have
already been initialised with I<dlink_freelist_init(3)>. Note that it will
not be possible to separate these free lists. On success, returns a pointer
to the beginning of the combined freelist. On error, returns C<null> with
C<errno> set appropriately.

=cut

*/

void *dlink_freelist_attach(void *freelist1, void *freelist2)
{
	char *freelist;

	if (!freelist2)
		return set_errnull(EINVAL);

	if (!(freelist = freelist1))
		return freelist2;

	while (dlink_has_next(freelist))
		freelist = dlink_next(freelist);

	((dlink_t *)freelist)->next = freelist2;
	((dlink_t *)freelist2)->prev = freelist;

	return freelist1;
}

/*

=item C<void *slink_alloc(void **freelist)>

Allocates an item from C<*freelist> and updates C<*freelist> to point to the
next free item. C<*freelist> must be a singly linked freelist initialised
with I<slink_freelist_init(3)>. On success, returns the allocated item. On
error, returns C<null> with C<errno> set appropriately.

=cut

*/

void *slink_alloc(void **freelist)
{
	void *alloc;

	if (!freelist)
		return set_errnull(EINVAL);

	if (!*freelist)
		return set_errnull(ENOSPC);

	alloc = *freelist;
	*freelist = slink_remove(*freelist);

	return alloc;
}

/*

=item C<void *dlink_alloc(void **freelist)>

Allocates an item from C<*freelist> and updates C<*freelist> to point to the
next free item. C<*freelist> must be a doubly linked freelist initialised
with I<dlink_freelist_init(3)>. On success, returns the allocated item. On
error, returns C<null> with C<errno> set appropriately.

=cut

*/

void *dlink_alloc(void **freelist)
{
	void *alloc;

	if (!freelist)
		return set_errnull(EINVAL);

	if (!*freelist)
		return set_errnull(ENOSPC);

	alloc = *freelist;
	*freelist = dlink_remove(*freelist);

	return alloc;
}

/*

=item C<void *slink_free(void **freelist, void *item)>

Inserts C<item> into C<*freelist> and updates C<*freelist> to point to
I<item>. C<*freelist> must be a singly linked freelist initialised with
I<slink_freelist_init(3)>. On success, returns the resulting free list. On
error, returns C<null> with C<errno> set appropriately.

=cut

*/

void *slink_free(void **freelist, void *item)
{
	if (!freelist || !item)
		return set_errnull(EINVAL);

	return *freelist = slink_insert(*freelist, item);
}

/*

=item C<void *dlink_free(void **freelist, void *item)>

Inserts C<item> into C<*freelist> and updates C<*freelist> to point to
C<item>. C<*freelist> must be a doubly linked freelist initialised with
I<dlink_freelist_init(3)>. On success, returns the resulting free list. On
error, returns C<null> with C<errno> set appropriately.

=cut

*/

void *dlink_free(void **freelist, void *item)
{
	if (!freelist || !item)
		return set_errnull(EINVAL);

	return *freelist = dlink_insert(*freelist, item);
}

/*

=back

=head1 ERRORS

The following errors are returned by these functions.

=over 4

=item C<EINVAL>

When C<null> pointers are incorrectly passed as arguments to most functions.

=item C<ENOSPC>

When I<slink_alloc(3)> or I<dlink_alloc(3)> is called and the free list is
exhausted.

=back

=head1 MT-Level

Unsafe

This module declares abstract types. They must be used as part of larger
data structures. It is assumed that the surrounding data structure and its
functions will provide any locking that is required.

=head1 EXAMPLES

A singly linked example that reads pairs of numbers from stdin (attaching
more space to the list as necessary), iterates over the items and then
deletes them:

    #include <slack/std.h>
    #include <slack/link.h>

    typedef struct spoint_t spoint_t;

    struct spoint_t
    {
        slink_t link;
        int x;
        int y;
    };

    #define SLIST_SIZE 10
    #define MAX_ADDITIONS 100
    spoint_t sfreespace[SLIST_SIZE];
    spoint_t *sfreelist = sfreespace;
    spoint_t *spoints = NULL;
    spoint_t *additional[MAX_ADDITIONS];
    int added = 0;

    int main(int ac, char **av)
    {
        spoint_t *item, *morespace;
        int x, y, i;

        // Initialize the singly-linked list of points

        if (slink_freelist_init(sfreespace, SLIST_SIZE, sizeof(spoint_t)) != sfreespace)
            return EXIT_FAILURE;

        // Read coordinates from stdin and populate the list

        while (scanf("%d %d", &x, &y) == 2)
        {
            // Add more space to the list when it runs out

            if (!(item = slink_alloc((void **)&sfreelist)))
            {
                if (added == MAX_ADDITIONS)
                    return EXIT_FAILURE; // or extend additional

                if (!(morespace = malloc(SLIST_SIZE * sizeof(spoint_t))))
                    return EXIT_FAILURE;

                additional[added++] = morespace; // remember to free this

                if (slink_freelist_init(morespace, SLIST_SIZE, sizeof(spoint_t)) != morespace)
                    return EXIT_FAILURE;

                if (!(sfreelist = slink_freelist_attach(sfreelist, morespace)))
                    return EXIT_FAILURE;

                if (!(item = slink_alloc((void **)&sfreelist)))
                    return EXIT_FAILURE;
            }

            // Initialize the item

            item->x = x;
            item->y = y;

            // Insert it into the list

            if (!(spoints = slink_insert(spoints, item)))
                return EXIT_FAILURE;
        }

        // Iterate over the list with slink_next()

        for (item = spoints; item; item = slink_next(item))
            printf("%d %d\n", item->x, item->y);

        // Iterate over the list with slink_has_next()

        for (item = spoints; slink_has_next(item) == 1; item = slink_next(item))
        {
            spoint_t *next = slink_next(item);
            printf("%d %d -> %d %d\n", item->x, item->y, next->x, next->y);
        }

        if (item)
            printf("%d %d !\n", item->x, item->y);

        // Remove the items (printing them out)

        while (spoints)
        {
            spoints = slink_remove(item = spoints);

            printf("%d %d\n", item->x, item->y);

            slink_free((void **)&sfreelist, item);
        }

        // Deallocate any attached freelists

        for (i = 0; i < added; ++i)
            free(additional[i]);

        return EXIT_SUCCESS;
    }

A doubly linked example that reads pairs of numbers from stdin (attaching
more space to the list as necessary), iterates over the items and then
deletes them:

    #include <slack/std.h>
    #include <slack/link.h>

    typedef struct dpoint_t dpoint_t;

    struct dpoint_t
    {
        dlink_t link;
        int x;
        int y;
    };

    #define DLIST_SIZE 10
    #define MAX_ADDITIONS 100
    dpoint_t dfreespace[DLIST_SIZE];
    dpoint_t *dfreelist = dfreespace;
    dpoint_t *dpoints = NULL;
    dpoint_t *additional[MAX_ADDITIONS];
    int added = 0;

    int main(int ac, char **av)
    {
        dpoint_t *item, *morespace;
        dpoint_t *last;
        int x, y, i;

        // Initialize the doubly-linked list of points

        if (dlink_freelist_init(dfreespace, DLIST_SIZE, sizeof(dpoint_t)) != dfreespace)
            return EXIT_FAILURE;

        // Read coordinates from stdin and populate the list

        while (scanf("%d %d", &x, &y) == 2)
        {
            // Add more space to the list when it runs out

            if (!(item = dlink_alloc((void **)&dfreelist)))
            {
                if (added == MAX_ADDITIONS)
                    return EXIT_FAILURE; // or extend additional

                if (!(morespace = malloc(DLIST_SIZE * sizeof(dpoint_t))))
                    return EXIT_FAILURE;

                additional[added++] = morespace; // remember to free this

                if (dlink_freelist_init(morespace, DLIST_SIZE, sizeof(dpoint_t)) != morespace)
                    return EXIT_FAILURE;

                if (!(dfreelist = dlink_freelist_attach(dfreelist, morespace)))
                    return EXIT_FAILURE;

                if (!(item = dlink_alloc((void **)&dfreelist)))
                    return EXIT_FAILURE;
            }

            // Initialize the item

            item->x = x;
            item->y = y;

            // Insert it into the list

            if (!(dpoints = dlink_insert(dpoints, item)))
                return EXIT_FAILURE;
        }

        // Iterate over the list with dlink_next()

        for (item = dpoints; item; item = dlink_next(item))
        {
            dpoint_t *prev = dlink_prev(item);
            dpoint_t *next = dlink_next(item);

            if (prev && next)
                printf("%d %d -> %d %d -> %d %d\n", prev->x, prev->y, item->x, item->y, next->x, next->y);
            else if (prev)
                printf("%d %d -> %d %d -> end\n", prev->x, prev->y, item->x, item->y);
            else if (next)
                printf("start -> %d %d -> %d %d\n", item->x, item->y, next->x, next->y);
        }

        // Iterate backwards with dlink_has_next() and dlink_prev()

        for (item = dpoints; dlink_has_next(item) == 1; item = dlink_next(item))
        {}

        for (; item; item = dlink_prev(item))
        {
            dpoint_t *prev = dlink_prev(item);
            dpoint_t *next = dlink_next(item);

            if (prev && next)
                printf("%d %d -> %d %d -> %d %d\n", prev->x, prev->y, item->x, item->y, next->x, next->y);
            else if (prev)
                printf("%d %d -> %d %d -> end\n", prev->x, prev->y, item->x, item->y);
            else if (next)
                printf("start -> %d %d -> %d %d\n", item->x, item->y, next->x, next->y);
        }

        // Remove the items (printing them out)

        while (dpoints)
        {
            dpoints = dlink_remove(item = dpoints);

            printf("%d %d\n", item->x, item->y);

            dlink_free((void **)&dfreelist, item);
        }

        // Deallocate any attached freelists

        for (i = 0; i < added; ++i)
            free(additional[i]);

        return EXIT_SUCCESS;
    }

=head1 BUGS

These functions only work on structs where the C<next> and C<prev> pointers
at the first elements. To fix this would require adding an C<offset>
parameter to each function to tell it where the C<next> and C<prev> pointers
were within the item. It's probably not worth it.

Attached free lists can't be detached. To change this would require more code
and more metadata. Again, it's probably not worth it.

=head1 SEE ALSO

I<libslack(3)>,
I<list(3)>,
I<map(3)>,
I<mem(3)>,
I<locker(3)>

=head1 AUTHOR

20201111 raf <raf@raf.org>

=cut

*/

#endif

#ifdef TEST

typedef struct spoint_t spoint_t;
typedef struct dpoint_t dpoint_t;

struct spoint_t
{
	slink_t link;
	int x;
	int y;
};

struct dpoint_t
{
	dlink_t link;
	int x;
	int y;
};

#define SLIST_SIZE 10
#define DLIST_SIZE 10
spoint_t sfreespace1[SLIST_SIZE];
spoint_t sfreespace2[SLIST_SIZE];
dpoint_t dfreespace1[DLIST_SIZE];
dpoint_t dfreespace2[DLIST_SIZE];
spoint_t *sfreelist = sfreespace1;
dpoint_t *dfreelist = dfreespace1;
spoint_t *spoints = NULL;
dpoint_t *dpoints = NULL;

int main(int ac, char **av)
{
	int errors = 0;

	if (ac == 2 && !strcmp(av[1], "help"))
	{
		printf("usage: %s\n", *av);
		return EXIT_SUCCESS;
	}

	printf("Testing: %s\n", "link");

	/* Test singly linked lists */

	/* Test slink_freelist_init() */

	if (slink_freelist_init(sfreespace1, SLIST_SIZE, sizeof(spoint_t)) != sfreespace1)
		++errors, printf("Test1: slink_freelist_init() failed (%s)\n", strerror(errno));
	else
	{
		spoint_t *item;
		size_t i;

		/* Test slink_alloc(), slink_insert() */

		for (i = 0; i < SLIST_SIZE; ++i)
		{
			if (!(item = slink_alloc((void **)&sfreelist)))
				++errors, printf("Test2: slink_alloc() failed (%s)\n", strerror(errno));
			else if (!(spoints = slink_insert(spoints, item)))
				++errors, printf("Test3: slink_insert() failed (%s)\n", strerror(errno));
			else
			{
				item->x = i;
				item->y = i + 1;
			}
		}

		if ((item = slink_alloc((void **)&sfreelist)))
			++errors, printf("Test4: slink_alloc() with no space failed (item = %p, not null)\n", (void *)item);
		else if (errno != ENOSPC)
			++errors, printf("Test5: slink_alloc() with no space failed (errno = %s, not %s)\n", strerror(errno), strerror(ENOSPC));

		/* Test slink_next() */

		for (i = SLIST_SIZE - 1, item = spoints; item; --i, item = slink_next(item))
		{
			if (item->x != i)
				++errors, printf("Test6: slink_next() failed (item%d->x == %d (not %d)\n", (int)i, item->x, (int)i);
			if (item->y != i + 1)
				++errors, printf("Test7: slink_next() failed (item%d->y == %d (not %d)\n", (int)i, item->y, (int)i + 1);
		}

		if (i != -1)
			++errors, printf("Test8: slink_next() failed (only %d items, not %d)\n", (int)i + SLIST_SIZE + 1, SLIST_SIZE);

		/* Test slink_remove(), slink_free() */

		for (i = 0; i < SLIST_SIZE; ++i)
		{
			spoints = slink_remove(item = spoints);

			if (!spoints && i < SLIST_SIZE - 1)
				++errors, printf("Test9: slink_remove() failed (i = %d)\n", (int)i);

			if (!slink_free((void **)&sfreelist, item))
				++errors, printf("Test10: slink_free() failed (i = %d)\n", (int)i);
		}

		if (spoints)
			++errors, printf("Test11: slink_remove() failed\n");

		/* Test slink_freelist_attach() */

		if (slink_freelist_init(sfreespace2, SLIST_SIZE, sizeof(spoint_t)) != sfreespace2)
			++errors, printf("Test12: slink_freelist_init() failed (%s)\n", strerror(errno));
		else if (!(sfreelist = slink_freelist_attach(sfreelist, sfreespace2)))
			++errors, printf("Test13: slink_freelist_attach() failed (%s)\n", strerror(errno));
		{
			spoint_t *item;
			size_t i;

			/* Test slink_alloc(), slink_insert() */

			for (i = 0; i < SLIST_SIZE * 2; ++i)
			{
				if (!(item = slink_alloc((void **)&sfreelist)))
					++errors, printf("Test14: slink_alloc() failed (%s)\n", strerror(errno));
				else if (!(spoints = slink_insert(spoints, item)))
					++errors, printf("Test15: slink_insert() failed (%s)\n", strerror(errno));
				else
				{
					item->x = i;
					item->y = i + 1;
				}
			}

			if ((item = slink_alloc((void **)&sfreelist)))
				++errors, printf("Test16: slink_alloc() with no space failed (item = %p, not null)\n", (void *)item);
			else if (errno != ENOSPC)
				++errors, printf("Test17: slink_alloc() with no space failed (errno = %s, not %s)\n", strerror(errno), strerror(ENOSPC));

			/* Test slink_next() */

			for (i = SLIST_SIZE * 2 - 1, item = spoints; item; --i, item = slink_next(item))
			{
				if (item->x != i)
					++errors, printf("Test18: slink_next() failed (item%d->x == %d (not %d)\n", (int)i, item->x, (int)i);
				if (item->y != i + 1)
					++errors, printf("Test19: slink_next() failed (item%d->y == %d (not %d)\n", (int)i, item->y, (int)i + 1);
			}

			if (i != -1)
				++errors, printf("Test20: slink_next() failed (only %d items, not %d)\n", (int)i + SLIST_SIZE * 2 + 1, SLIST_SIZE * 2);

			/* Test slink_remove(), slink_free() */

			for (i = SLIST_SIZE * 2 - 1; spoints; --i)
			{
				spoints = slink_remove(item = spoints);

				if (!spoints && i >= -1)
					++errors, printf("Test21: slink_remove() failed (i = %d)\n", (int)i);

				if (!slink_free((void **)&sfreelist, item))
					++errors, printf("Test22: slink_free() failed (i = %d)\n", (int)i);
			}

			if (i != -1)
				++errors, printf("Test23: slink_remove() failed (i = %d, not -1)\n", (int)i);

			if (spoints)
				++errors, printf("Test24: slink_remove() failed (spoints = %p, not null)\n", (void *)spoints);
		}
	}

	/* Test doubly linked lists */

	/* Test dlink_freelist_init() */

	if (dlink_freelist_init(dfreespace1, DLIST_SIZE, sizeof(dpoint_t)) != dfreespace1)
		++errors, printf("Test25: dlink_freelist_init() failed (%s)\n", strerror(errno));
	else
	{
		dpoint_t *item, *last;
		size_t i;

		/* Test dlink_alloc(), dlink_insert() */

		for (i = 0; i < DLIST_SIZE; ++i)
		{
			if (!(item = dlink_alloc((void **)&dfreelist)))
				++errors, printf("Test26: dlink_alloc() failed (%s)\n", strerror(errno));
			else if (!(dpoints = dlink_insert(dpoints, item)))
				++errors, printf("Test27: dlink_insert() failed (%s)\n", strerror(errno));
			else
			{
				item->x = i;
				item->y = i + 1;
			}
		}

		if ((item = dlink_alloc((void **)&dfreelist)))
			++errors, printf("Test28: dlink_alloc() with no space failed (item = %p, not null)\n", (void *)item);
		else if (errno != ENOSPC)
			++errors, printf("Test29: dlink_alloc() with no space failed (errno = %s, not %s)\n", strerror(errno), strerror(ENOSPC));

		/* Test dlink_next */

		for (i = DLIST_SIZE - 1, item = dpoints; item; --i, item = dlink_next(item))
		{
			if (item->x != i)
				++errors, printf("Test30: dlink_next() failed (item%d->x == %d (not %d)\n", (int)i, item->x, (int)i);
			if (item->y != i + 1)
				++errors, printf("Test31: dlink_next() failed (item%d->y == %d (not %d)\n", (int)i, item->y, (int)i + 1);

			last = item;
		}

		if (i != -1)
			++errors, printf("Test32: dlink_next() failed (only %d items, not %d)\n", (int)i + DLIST_SIZE + 1, DLIST_SIZE);

		/* Test dlink_prev() */

		for (item = last, ++i; item; ++i, item = dlink_prev(item))
		{
			if (item->x != i)
				++errors, printf("Test33: dlink_prev() failed (item%d->x == %d (not %d)\n", (int)i, item->x, (int)i);
			if (item->y != i + 1)
				++errors, printf("Test34: dlink_prev() failed (item%d->y == %d (not %d)\n", (int)i, item->y, (int)i + 1);
		}

		if (i != DLIST_SIZE)
			++errors, printf("Test35: dlink_prev() failed (only %d items, not %d)\n", (int)i, DLIST_SIZE);

		/* Test dlink_remove(), dlink_free() */

		for (i = 0; i < DLIST_SIZE; ++i)
		{
			dpoints = dlink_remove(item = dpoints);
			if (!dpoints && i < DLIST_SIZE - 1)
				++errors, printf("Test36: dlink_remove() failed (i = %d)\n", (int)i);

			if (!dlink_free((void **)&dfreelist, item))
				++errors, printf("Test37: dlink_free() failed (i = %d)\n", (int)i);
		}

		if (dpoints)
			++errors, printf("Test38: dlink_remove() failed\n");

		/* Test dlink_freelist_attach() */

		if (dlink_freelist_init(dfreespace2, DLIST_SIZE, sizeof(dpoint_t)) != dfreespace2)
			++errors, printf("Test39: dlink_freelist_init() failed (%s)\n", strerror(errno));
		else if (!(dfreelist = dlink_freelist_attach(dfreelist, dfreespace2)))
			++errors, printf("Test40: dlink_freelist_attach() failed (%s)\n", strerror(errno));
		{
			dpoint_t *item;
			size_t i;

			/* Test dlink_alloc(), dlink_insert() */

			for (i = 0; i < DLIST_SIZE * 2; ++i)
			{
				if (!(item = dlink_alloc((void **)&dfreelist)))
					++errors, printf("Test41: dlink_alloc() failed (%s)\n", strerror(errno));
				else if (!(dpoints = dlink_insert(dpoints, item)))
					++errors, printf("Test42: dlink_insert() failed (%s)\n", strerror(errno));
				else
				{
					item->x = i;
					item->y = i + 1;
				}
			}

			if ((item = dlink_alloc((void **)&dfreelist)))
				++errors, printf("Test43: dlink_alloc() with no space failed (item = %p, not null)\n", (void *)item);
			else if (errno != ENOSPC)
				++errors, printf("Test44: dlink_alloc() with no space failed (errno = %s, not %s)\n", strerror(errno), strerror(ENOSPC));

			/* Test dlink_next() */

			for (i = 0, item = dpoints; item; ++i, item = dlink_next(item))
			{
				if (item->x != DLIST_SIZE * 2 - 1 - i)
					++errors, printf("Test45: dlink_next() failed (item%d->x == %d (not %d)\n", (int)i, item->x, DLIST_SIZE * 2 - (int)i - 1);
				if (item->y != DLIST_SIZE * 2 - 1 - i + 1)
					++errors, printf("Test46: dlink_next() failed (item%d->y == %d (not %d)\n", (int)i, item->y, DLIST_SIZE * 2 - (int)i);
			}

			if (i != DLIST_SIZE * 2)
				++errors, printf("Test47: dlink_next() failed (only %d items, not %d)\n", (int)i, DLIST_SIZE * 2);

			/* Test dlink_prev() */

			for (item = last; item; --i, item = dlink_prev(item))
			{
				if (item->x != DLIST_SIZE * 2 - i)
					++errors, printf("Test48: dlink_prev() failed (item%d->x == %d (not %d)\n", (int)i, item->x, DLIST_SIZE * 2 - (int)i);
				if (item->y != DLIST_SIZE * 2 - i + 1)
					++errors, printf("Test49: dlink_prev() failed (item%d->y == %d (not %d)\n", (int)i, item->y, DLIST_SIZE * 2 - (int)i + 1);

				last = item;
			}

			if (i != 0)
				++errors, printf("Test50: dlink_prev() failed (only %d items, not %d)\n", DLIST_SIZE - (int)i, DLIST_SIZE);

			/* Test dlink_remove(), dlink_free() */

			for (i = 0; i < DLIST_SIZE * 2; ++i)
			{
				dpoints = dlink_remove(item = dpoints);
				if (!dpoints && i < DLIST_SIZE * 2 - 1)
					++errors, printf("Test51: dlink_remove() failed (i = %d)\n", (int)i);

				if (!dlink_free((void **)&dfreelist, item))
					++errors, printf("Test52: dlink_free() failed (i = %d)\n", (int)i);
			}

			if (dpoints)
				++errors, printf("Test53: dlink_remove() failed\n");
		}
	}

	/* Test errors */

	if (slink_has_next(NULL) != -1)
		++errors, printf("Test54: slink_has_next(NULL) failed\n");
	else if (errno != EINVAL)
		++errors, printf("Test55: slink_has_next(NULL) failed (errno = %s, not %s)\n", strerror(errno), strerror(EINVAL));

	if (slink_next(NULL))
		++errors, printf("Test56: slink_next(NULL) failed\n");
	else if (errno != EINVAL)
		++errors, printf("Test57: slink_next(NULL) failed (errno = %s, not %s)\n", strerror(errno), strerror(EINVAL));

	if (dlink_has_next(NULL) != -1)
		++errors, printf("Test58: dlink_has_next(NULL) failed\n");
	else if (errno != EINVAL)
		++errors, printf("Test59: dlink_has_next(NULL) failed (errno = %s, not %s)\n", strerror(errno), strerror(EINVAL));

	if (dlink_next(NULL))
		++errors, printf("Test60: dlink_next(NULL) failed\n");
	else if (errno != EINVAL)
		++errors, printf("Test61: dlink_next(NULL) failed (errno = %s, not %s)\n", strerror(errno), strerror(EINVAL));

	if (dlink_has_prev(NULL) != -1)
		++errors, printf("Test62: dlink_has_prev(NULL) failed\n");
	else if (errno != EINVAL)
		++errors, printf("Test63: dlink_has_prev(NULL) failed (errno = %s, not %s)\n", strerror(errno), strerror(EINVAL));

	if (dlink_prev(NULL))
		++errors, printf("Test64: dlink_prev(NULL) failed\n");
	else if (errno != EINVAL)
		++errors, printf("Test65: dlink_prev(NULL) failed (errno = %s, not %s)\n", strerror(errno), strerror(EINVAL));

	if (slink_insert(NULL, NULL))
		++errors, printf("Test66: slink_insert(NULL, NULL) failed\n");
	else if (errno != EINVAL)
		++errors, printf("Test67: slink_insert(NULL, NULL) failed (errno = %s, not %s)\n", strerror(errno), strerror(EINVAL));

	if (dlink_insert(NULL, NULL))
		++errors, printf("Test68: dlink_insert(NULL, NULL) failed\n");
	else if (errno != EINVAL)
		++errors, printf("Test69: dlink_insert(NULL, NULL) failed (errno = %s, not %s)\n", strerror(errno), strerror(EINVAL));

	if (slink_remove(NULL))
		++errors, printf("Test70: slink_remove(NULL) failed\n");
	else if (errno != EINVAL)
		++errors, printf("Test71: slink_remove(NULL) failed (errno = %s, not %s)\n", strerror(errno), strerror(EINVAL));

	if (dlink_remove(NULL))
		++errors, printf("Test72: dlink_remove(NULL) failed\n");
	else if (errno != EINVAL)
		++errors, printf("Test73: dlink_remove(NULL) failed (errno = %s, not %s)\n", strerror(errno), strerror(EINVAL));

	if (slink_freelist_init(NULL, 1, 1))
		++errors, printf("Test74: slink_freelist_init(NULL, 1, 1) failed\n");
	else if (errno != EINVAL)
		++errors, printf("Test75: slink_freelist_init(NULL, 1, 1) failed (errno = %s, not %s)\n", strerror(errno), strerror(EINVAL));

	if (slink_freelist_init(sfreespace1, 0, 1))
		++errors, printf("Test76: slink_freelist_init(sfreespace1, 0, 1) failed\n");
	else if (errno != EINVAL)
		++errors, printf("Test77: slink_freelist_init(sfreespace1, 0, 1) failed (errno = %s, not %s)\n", strerror(errno), strerror(EINVAL));

	if (slink_freelist_init(sfreespace1, 1, 0))
		++errors, printf("Test78: slink_freelist_init(sfreespace1, 1, 0) failed\n");
	else if (errno != EINVAL)
		++errors, printf("Test79: slink_freelist_init(sfreespace1, 1, 0) failed (errno = %s, not %s)\n", strerror(errno), strerror(EINVAL));

	if (dlink_freelist_init(NULL, 1, 1))
		++errors, printf("Test80: dlink_freelist_init(NULL, 1, 1) failed\n");
	else if (errno != EINVAL)
		++errors, printf("Test81: dlink_freelist_init(NULL, 1, 1) failed (errno = %s, not %s)\n", strerror(errno), strerror(EINVAL));

	if (dlink_freelist_init(dfreespace1, 0, 1))
		++errors, printf("Test82: dlink_freelist_init(dfreespace1, 0, 1) failed\n");
	else if (errno != EINVAL)
		++errors, printf("Test83: dlink_freelist_init(dfreespace1, 0, 1) failed (errno = %s, not %s)\n", strerror(errno), strerror(EINVAL));

	if (dlink_freelist_init(dfreespace1, 1, 0))
		++errors, printf("Test84: dlink_freelist_init(dfreespace1, 1, 0) failed\n");
	else if (errno != EINVAL)
		++errors, printf("Test85: dlink_freelist_init(dfreespace1, 1, 0) failed (errno = %s, not %s)\n", strerror(errno), strerror(EINVAL));

	if (slink_freelist_attach(sfreelist, NULL))
		++errors, printf("Test86: slink_freelist_attach(sfreelist, NULL) failed\n");
	else if (errno != EINVAL)
		++errors, printf("Test87: slink_freelist_attach(sfreelist, NULL) failed (errno = %s, not %s)\n", strerror(errno), strerror(EINVAL));

	if (dlink_freelist_attach(dfreelist, NULL))
		++errors, printf("Test88: dlink_freelist_attach(dfreelist, NULL) failed\n");
	else if (errno != EINVAL)
		++errors, printf("Test89: dlink_freelist_attach(dfreelist, NULL) failed (errno = %s, not %s)\n", strerror(errno), strerror(EINVAL));

	if (slink_alloc(NULL))
		++errors, printf("Test90: slink_alloc(NULL) failed\n");
	else if (errno != EINVAL)
		++errors, printf("Test91: slink_alloc(NULL) failed (errno = %s, not %s)\n", strerror(errno), strerror(EINVAL));

	sfreelist = NULL;
	if (slink_alloc((void **)&sfreelist))
		++errors, printf("Test92: slink_alloc(&NULL) failed\n");
	else if (errno != ENOSPC)
		++errors, printf("Test93: slink_alloc(&NULL) failed (errno = %s, not %s)\n", strerror(errno), strerror(EINVAL));

	if (dlink_alloc(NULL))
		++errors, printf("Test94: dlink_alloc(NULL) failed\n");
	else if (errno != EINVAL)
		++errors, printf("Test95: dlink_alloc(NULL) failed (errno = %s, not %s)\n", strerror(errno), strerror(EINVAL));

	dfreelist = NULL;
	if (dlink_alloc((void **)&dfreelist))
		++errors, printf("Test96: dlink_alloc(&NULL) failed\n");
	else if (errno != ENOSPC)
		++errors, printf("Test97: dlink_alloc(&NULL) failed (errno = %s, not %s)\n", strerror(errno), strerror(EINVAL));

	if (slink_free(NULL, (void *)1))
		++errors, printf("Test98: slink_free(NULL, ...) failed\n");
	else if (errno != EINVAL)
		++errors, printf("Test99: slink_free(NULL, ...) failed (errno = %s, not %s)\n", strerror(errno), strerror(EINVAL));

	if (slink_free((void **)&sfreelist, NULL))
		++errors, printf("Test100: slink_free(&sfreelist, NULL) failed\n");
	else if (errno != EINVAL)
		++errors, printf("Test101: slink_free(&sfreelist, NULL) failed (errno = %s, not %s)\n", strerror(errno), strerror(EINVAL));

	if (dlink_free(NULL, (void *)1))
		++errors, printf("Test102: dlink_free(NULL, ...) failed\n");
	else if (errno != EINVAL)
		++errors, printf("Test103: dlink_free(NULL, ...) failed (errno = %s, not %s)\n", strerror(errno), strerror(EINVAL));

	if (dlink_free((void **)&dfreelist, NULL))
		++errors, printf("Test104: dlink_free(&sfreelist, NULL) failed\n");
	else if (errno != EINVAL)
		++errors, printf("Test105: dlink_free(&sfreelist, NULL) failed (errno = %s, not %s)\n", strerror(errno), strerror(EINVAL));

	if (errors)
		printf("%d/105 tests failed\n", errors);
	else
		printf("All tests passed\n");

	return (errors == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

#endif

/* vi:set ts=4 sw=4: */
