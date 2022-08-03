/*
# libslack - http://libslack.org/
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
 *  Generic Heapsort.
 *
 *  Synopsis:
 *      hsort(void *base, size_t n, size_t size, int (*fn)(void *, void *))
 *  Description:
 *      Hsort sorts the array of `n' items which starts at address `base'.
 *      The size of each item is as given.  Items are compared by the function
 *      `fn', which is passed pointers to two items as arguments. The function
 *      should return < 0 if item1 < item2, == 0 if item1 == item2, and > 0
 *      if item1 > item2.
 *  Version:
 *      1988 April 28
 *  Author:
 *      Stephen Russell, Department of Computer Science,
 *      University of Sydney, 2006
 *      Australia.
 */

/*

=head1 NAME

I<hsort(3)> - generic heap sort

=head1 SYNOPSIS

    #include <slack/std.h>
    #include <slack/hsort.h>

    typedef int hsort_cmp_t(const void *a, const void *b);
    typedef int hsort_closure_cmp_t(const void *a, const void *b, const void *data);

    void hsort(void *base, size_t n, size_t size, hsort_cmp_t *cmp);
    void hsort_closure(void *base, size_t n, size_t size, hsort_closure_cmp_t *cmp, const void *data);

=head1 DESCRIPTION

I<hsort(3)> is an implementation of the heap sort algorithm. It sorts a
table of data in place. C<base> points to the element at the base of the
table. C<n> is the number of elements in the table. C<size> is the size of
the elements in bytes. C<cmp> is the comparison function, which is called
with two arguments that point to the elements being compared. As the
function must return an integer less than, equal to, or greater than zero,
so must the first argument to be considered be less than, equal to, or
greater than the second. This is a drop-in replacement for I<qsort(3)>.
I<hsort_closure(3)> is the same but it supports passing arbitrary data to
the comparison function.

=head1 NOTES

The comparison function need not compare every byte, so arbitrary data may
be contained in the elements in addition to the values being compared.

The order in the output of two items which compare as equal is
unpredictable.

=head1 MT-Level

I<MT-Safe>

Note that the array being sorted will still have to be write-locked during
I<hsort(3)> if it is accessed by other threads.

=head1 EXAMPLE

This examples sorts and prints an array of strings.

    #include <slack/std.h>
    #include <slack/hsort.h>

    int cmp(const char **p1, const char **p2)
    {
        return strcmp(*p1, *p2);
    }

    int main(int ac, char **av)
    {
        char *string[4] = { "jkl", "ghi", "def", "abc" };
        int i;

        hsort(string, 4, sizeof string[0], (hsort_cmp_t *)cmp);

        for (i = 0; i < 4; ++i)
            printf("%s\n", string[i]);

        return EXIT_SUCCESS;
    }

=head1 SEE ALSO

I<qsort(3)>

=head1 AUTHOR

    Stephen Russell, Department of Computer Science,
    University of Sydney, 2006.
    Australia
    1988 April 28

    Header file, test code and manpage by raf <raf@raf.org>

=cut

*/

#include "std.h"

#include "hsort.h"

#ifndef TEST

#ifdef INLINE

#define swap(p1, p2, n) \
{\
	register char *_p1, *_p2;\
	register size_t _n;\
	register char _tmp;\
\
	for (_p1 = p1, _p2 = p2, _n = n; _n-- > 0; )\
	{\
		_tmp = *_p1; *_p1++ = *_p2; *_p2++ = _tmp;\
	}\
}\

#else

/*
 *   Support routine for swapping elements of the array.
 */

static void swap
(
	register char *p1,
	register char *p2,
	register size_t n
)
{
	register char ctmp;

	/*
	 *  On machines with no alignment restrictions for ints,
	 *  the following loop may improve performance if moving lots
	 *  of data. It has been commented out for portability.

	 register int itmp;

	 for ( ; n > sizeof(int); n -= sizeof(int))
	 {
		itmp = *(int *)p1;
		*(int *)p1 = *(int *)p2;
		p1 += sizeof(int);
		*(int *)p2 = itmp;
		p2 += sizeof(int);
	}

	*/

	while (n-- != 0)
	{
		ctmp = *p1; *p1++ = *p2; *p2++ = ctmp;
	}
}

#endif

/*
 *      To avoid function calls in the inner loops, the code responsible for
 *      constructing a heap from (part of) the array has been expanded inline.
 *      It is possible to convert this common code to a function. The three
 *      parameters base0, cmp and size are invariant - only the size of the
 *      gap and the high bound of the array change. In phase 1, gap decreases
 *      while hi is fixed. In phase 2, gap == size, and hi decreases. The
 *      variables p, q, and g are only used in this common code.
 */

void hsort(void *base, size_t n, size_t size, hsort_cmp_t *cmp)
{
	register char *p, *q, *base0, *hi;
	register unsigned int gap, g;

	if (n < 2)
		return;

	base0 = (char *)base - size;            /* set up address of h[0] */

	/*
	 *  The gap is the distance, in bytes, between h[0] and h[i],
	 *  for some i. It is also the distance between h[i] and h[2*i];
	 *  that is, the distance between a node and its left child.
	 *  The initial node of interest is h[n/2] (the rightmost
	 *  interior node), so gap is set accordingly. The following is
	 *  the only multiplication needed.
	 */

	gap = (n >> 1) * size;          /* initial gap is n/2*size */
	hi = base0 + gap + gap;         /* calculate address of h[n] */
	if (n & 1)
		hi += size;             /* watch out for odd arrays */

	/*
	 *  Phase 1: Construct heap from random data.
	 *
	 *  For i = n/2 downto 2, ensure h[i] is greater than its
	 *  children h[2*i] and h[2*i+1]. By decreasing 'gap' at each
	 *  iteration, we move down the heap towards h[2]. The final step
	 *  of making h[1] the maximum value is done in the next phase.
	 */

	for ( ; gap != size; gap -= size)
	{
		/*  fixheap(base0, size, cmp, gap, hi) */

		for (p = base0 + (g = gap); (q = p + g) <= hi; p = q)
		{
			g += g;         /* double gap for next level */

			/*
			 *  Find greater of left and right children.
			 */

			if (q != hi && (*cmp)(q + size, q) > 0)
			{
				q += size;      /* choose right child */
				g += size;      /* follow right subtree */
			}

			/*
			 *  Compare with parent.
			 */

			if ((*cmp)(p, q) >= 0)
				break;          /* order is correct */

			swap(p, q, size);       /* swap parent and child */
		}
	}

	/*
	 *  Phase 2: Each iteration makes the first item in the
	 *  array the maximum, then swaps it with the last item, which
	 *  is its correct position. The size of the heap is decreased
	 *  each iteration. The gap is always "size", as we are interested
	 *  in the heap starting at h[1].
	 */

	for ( ; hi != base; hi -= size)
	{
		/* fixheap(base0, size, cmp, gap (== size), hi) */

		p = (char *)base;               /* == base0 + size */
		for (g = size; (q = p + g) <= hi; p = q)
		{
			g += g;
			if (q != hi && (*cmp)(q + size, q) > 0)
			{
				q += size;
				g += size;
			}

			if ((*cmp)(p, q) >= 0)
				break;

			swap(p, q, size);
		}

		swap((char *)base, hi, size);           /* move largest item to end */
	}
}

void hsort_closure(void *base, size_t n, size_t size, hsort_closure_cmp_t *cmp, const void *data)
{
	register char *p, *q, *base0, *hi;
	register unsigned int gap, g;

	if (n < 2)
		return;

	base0 = (char *)base - size;            /* set up address of h[0] */

	/*
	 *  The gap is the distance, in bytes, between h[0] and h[i],
	 *  for some i. It is also the distance between h[i] and h[2*i];
	 *  that is, the distance between a node and its left child.
	 *  The initial node of interest is h[n/2] (the rightmost
	 *  interior node), so gap is set accordingly. The following is
	 *  the only multiplication needed.
	 */

	gap = (n >> 1) * size;          /* initial gap is n/2*size */
	hi = base0 + gap + gap;         /* calculate address of h[n] */
	if (n & 1)
		hi += size;             /* watch out for odd arrays */

	/*
	 *  Phase 1: Construct heap from random data.
	 *
	 *  For i = n/2 downto 2, ensure h[i] is greater than its
	 *  children h[2*i] and h[2*i+1]. By decreasing 'gap' at each
	 *  iteration, we move down the heap towards h[2]. The final step
	 *  of making h[1] the maximum value is done in the next phase.
	 */

	for ( ; gap != size; gap -= size)
	{
		/*  fixheap(base0, size, cmp, gap, hi) */

		for (p = base0 + (g = gap); (q = p + g) <= hi; p = q)
		{
			g += g;         /* double gap for next level */

			/*
			 *  Find greater of left and right children.
			 */

			if (q != hi && (*cmp)(q + size, q, data) > 0)
			{
				q += size;      /* choose right child */
				g += size;      /* follow right subtree */
			}

			/*
			 *  Compare with parent.
			 */

			if ((*cmp)(p, q, data) >= 0)
				break;          /* order is correct */

			swap(p, q, size);       /* swap parent and child */
		}
	}

	/*
	 *  Phase 2: Each iteration makes the first item in the
	 *  array the maximum, then swaps it with the last item, which
	 *  is its correct position. The size of the heap is decreased
	 *  each iteration. The gap is always "size", as we are interested
	 *  in the heap starting at h[1].
	 */

	for ( ; hi != base; hi -= size)
	{
		/* fixheap(base0, size, cmp, gap (== size), hi) */

		p = (char *)base;               /* == base0 + size */
		for (g = size; (q = p + g) <= hi; p = q)
		{
			g += g;
			if (q != hi && (*cmp)(q + size, q, data) > 0)
			{
				q += size;
				g += size;
			}

			if ((*cmp)(p, q, data) >= 0)
				break;

			swap(p, q, size);
		}

		swap((char *)base, hi, size);           /* move largest item to end */
	}
}

#endif

#ifdef TEST

int errors = 0;
char *string[4];
char *data = "arbitrary";
int testno = 0;

static int cmp(const char **a, const char **b)
{
	return strcmp(*a, *b);
}

static int cmp_closure(const char **a, const char **b, const char *data)
{
	if (!data || strcmp(data, "arbitrary")) /* may crash here if buggy */
		++errors, printf("Test%d: hsort_closure() failed: data not supplied\n", testno);

	return strcmp(*a, *b);
}

static void verify(int test)
{
	if (strcmp(string[0], "abc"))
		++errors, printf("Test%d: 1st item is '%s' (not 'abc')\n", test, string[0]);
	else if (strcmp(string[1], "def"))
		++errors, printf("Test%d: 2nd item is '%s' (not 'def')\n", test, string[1]);
	else if (strcmp(string[2], "ghi"))
		++errors, printf("Test%d: 3rd item is '%s' (not 'ghi')\n", test, string[2]);
	else if (strcmp(string[3], "jkl"))
		++errors, printf("Test%d: 4th item is '%s' (not 'jkl')\n", test, string[3]);
}

int main(int ac, char **av)
{
	if (ac == 2 && !strcmp(av[1], "help"))
	{
		printf("usage: %s\n", *av);
		return EXIT_SUCCESS;
	}

	printf("Testing: %s\n", "hsort");

	string[0] = "abc";
	string[1] = "def";
	string[2] = "ghi";
	string[3] = "jkl";
	hsort(string, 4, sizeof string[0], (hsort_cmp_t *)cmp);
	verify(1);

	string[0] = "jkl";
	string[1] = "ghi";
	string[2] = "def";
	string[3] = "abc";
	hsort(string, 4, sizeof string[0], (hsort_cmp_t *)cmp);
	verify(2);

	string[0] = "def";
	string[1] = "abc";
	string[2] = "jkl";
	string[3] = "ghi";
	hsort(string, 4, sizeof string[0], (hsort_cmp_t *)cmp);
	verify(3);

	string[0] = "abc";
	string[1] = "def";
	string[2] = "ghi";
	string[3] = "jkl";
	testno = 5;
	hsort_closure(string, 4, sizeof string[0], (hsort_closure_cmp_t *)cmp_closure, data);
	verify(testno);

	string[0] = "jkl";
	string[1] = "ghi";
	string[2] = "def";
	string[3] = "abc";
	testno = 6;
	hsort_closure(string, 4, sizeof string[0], (hsort_closure_cmp_t *)cmp_closure, data);
	verify(testno);

	string[0] = "def";
	string[1] = "abc";
	string[2] = "jkl";
	string[3] = "ghi";
	testno = 7;
	hsort_closure(string, 4, sizeof string[0], (hsort_closure_cmp_t *)cmp_closure, data);
	verify(testno);

	if (errors)
		printf("%d/6 tests failed\n", errors);
	else
		printf("All tests passed\n");

	return (errors == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

#endif

/* vi:set ts=4 sw=4: */
