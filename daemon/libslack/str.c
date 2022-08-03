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
* $OpenBSD: strlcpy.c,v 1.4 1999/05/01 18:56:41 millert Exp $
* $OpenBSD: strlcat.c,v 1.5 2001/01/13 16:17:24 millert Exp $
* Modified by raf <raf@raf.org>
*
* Copyright (c) 1998 Todd C. Miller <Todd.Miller@courtesan.com>
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
* 3. The name of the author may not be used to endorse or promote products
*    derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
* AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
* THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*

=head1 NAME

I<libslack(str)> - string module

=head1 SYNOPSIS

    #include <slack/std.h>
    #include <slack/str.h>

    typedef struct String String;
    typedef struct StringTR StringTR;

    enum StringAlignment
    {
        ALIGN_LEFT       = '<',
        ALIGN_RIGHT      = '>',
        ALIGN_CENTRE     = '|',
        ALIGN_CENTER     = '|',
        ALIGN_FULL       = '='
    };

    enum StringTROption
    {
        TR_COMPLEMENT = 1,
        TR_DELETE     = 2,
        TR_SQUASH     = 4
    };

    typedef enum StringAlignment StringAlignment;
    typedef enum StringTROption StringTROption;

    String *str_create(const char *format, ...);
    String *str_create_with_locker(Locker *locker, const char *format, ...);
    String *str_vcreate(const char *format, va_list args);
    String *str_vcreate_with_locker(Locker *locker, const char *format, va_list args);
    String *str_create_sized(size_t size, const char *format, ...);
    String *str_create_with_locker_sized(Locker *locker, size_t size, const char *format, ...);
    String *str_vcreate_sized(size_t size, const char *format, va_list args);
    String *str_vcreate_with_locker_sized(Locker *locker, size_t size, const char *format, va_list args);
    String *str_copy(const String *str);
    String *str_copy_unlocked(const String *str);
    String *str_copy_with_locker(Locker *locker, const String *str);
    String *str_copy_with_locker_unlocked(Locker *locker, const String *str);
    String *str_fgetline(FILE *stream);
    String *str_fgetline_with_locker(Locker *locker, FILE *stream);
    void str_release(String *str);
    void *str_destroy(String **str);
    int str_rdlock(const String *str);
    int str_wrlock(const String *str);
    int str_unlock(const String *str);
    int str_empty(const String *str);
    int str_empty_unlocked(const String *str);
    ssize_t str_length(const String *str);
    ssize_t str_length_unlocked(const String *str);
    char *cstr(const String *str);
    ssize_t str_set_length(String *str, size_t length);
    ssize_t str_set_length_unlocked(String *str, size_t length);
    ssize_t str_recalc_length(String *str);
    ssize_t str_recalc_length_unlocked(String *str);
    String *str_clear(String *str);
    String *str_clear_unlocked(String *str);
    String *str_remove(String *str, ssize_t index);
    String *str_remove_unlocked(String *str, ssize_t index);
    String *str_remove_range(String *str, ssize_t index, ssize_t range);
    String *str_remove_range_unlocked(String *str, ssize_t index, ssize_t range);
    String *str_insert(String *str, ssize_t index, const char *format, ...);
    String *str_insert_unlocked(String *str, ssize_t index, const char *format, ...);
    String *str_vinsert(String *str, ssize_t index, const char *format, va_list args);
    String *str_vinsert_unlocked(String *str, ssize_t index, const char *format, va_list args);
    String *str_insert_str(String *str, ssize_t index, const String *src);
    String *str_insert_str_unlocked(String *str, ssize_t index, const String *src);
    String *str_append(String *str, const char *format, ...);
    String *str_append_unlocked(String *str, const char *format, ...);
    String *str_vappend(String *str, const char *format, va_list args);
    String *str_vappend_unlocked(String *str, const char *format, va_list args);
    String *str_append_str(String *str, const String *src);
    String *str_append_str_unlocked(String *str, const String *src);
    String *str_prepend(String *str, const char *format, ...);
    String *str_prepend_unlocked(String *str, const char *format, ...);
    String *str_vprepend(String *str, const char *format, va_list args);
    String *str_vprepend_unlocked(String *str, const char *format, va_list args);
    String *str_prepend_str(String *str, const String *src);
    String *str_prepend_str_unlocked(String *str, const String *src);
    String *str_replace(String *str, ssize_t index, ssize_t range, const char *format, ...);
    String *str_replace_unlocked(String *str, ssize_t index, ssize_t range, const char *format, ...);
    String *str_vreplace(String *str, ssize_t index, ssize_t range, const char *format, va_list args);
    String *str_vreplace_unlocked(String *str, ssize_t index, ssize_t range, const char *format, va_list args);
    String *str_replace_str(String *str, ssize_t index, ssize_t range, const String *src);
    String *str_replace_str_unlocked(String *str, ssize_t index, ssize_t range, const String *src);
    String *str_substr(const String *str, ssize_t index, ssize_t range);
    String *str_substr_unlocked(const String *str, ssize_t index, ssize_t range);
    String *str_substr_with_locker(Locker *locker, const String *str, ssize_t index, ssize_t range);
    String *str_substr_with_locker_unlocked(Locker *locker, const String *str, ssize_t index, ssize_t range);
    String *substr(const char *str, ssize_t index, ssize_t range);
    String *substr_with_locker(Locker *locker, const char *str, ssize_t index, ssize_t range);
    String *str_splice(String *str, ssize_t index, ssize_t range);
    String *str_splice_unlocked(String *str, ssize_t index, ssize_t range);
    String *str_splice_with_locker(Locker *locker, String *str, ssize_t index, ssize_t range);
    String *str_splice_with_locker_unlocked(Locker *locker, String *str, ssize_t index, ssize_t range);
    String *str_repeat(size_t count, const char *format, ...);
    String *str_repeat_with_locker(Locker *locker, size_t count, const char *format, ...);
    String *str_vrepeat(size_t count, const char *format, va_list args);
    String *str_vrepeat_with_locker(Locker *locker, size_t count, const char *format, va_list args);
    int str_tr(String *str, const char *from, const char *to, int option);
    int str_tr_unlocked(String *str, const char *from, const char *to, int option);
    int str_tr_str(String *str, const String *from, const String *to, int option);
    int str_tr_str_unlocked(String *str, const String *from, const String *to, int option);
    int tr(char *str, const char *from, const char *to, int option);
    StringTR *tr_compile(const char *from, const char *to, int option);
    StringTR *tr_compile_with_locker(Locker *locker, const char *from, const char *to, int option);
    StringTR *str_tr_compile(const String *from, const String *to, int option);
    StringTR *str_tr_compile_unlocked(const String *from, const String *to, int option);
    StringTR *str_tr_compile_with_locker(Locker *locker, const String *from, const String *to, int option);
    StringTR *str_tr_compile_with_locker_unlocked(Locker *locker, const String *from, const String *to, int option);
    void tr_release(StringTR *table);
    void *tr_destroy(StringTR **table);
    int str_tr_compiled(String *str, StringTR *table);
    int str_tr_compiled_unlocked(String *str, StringTR *table);
    int tr_compiled(char *str, StringTR *table);
    List *str_regexpr(const char *pattern, const String *text, int cflags, int eflags);
    List *str_regexpr_unlocked(const char *pattern, const String *text, int cflags, int eflags);
    List *str_regexpr_with_locker(Locker *locker, const char *pattern, const String *text, int cflags, int eflags);
    List *str_regexpr_with_locker_unlocked(Locker *locker, const char *pattern, const String *text, int cflags, int eflags);
    List *regexpr(const char *pattern, const char *text, int cflags, int eflags);
    List *regexpr_with_locker(Locker *locker, const char *pattern, const char *text, int cflags, int eflags);
    int regexpr_compile(regex_t *compiled, const char *pattern, int cflags);
    void regexpr_release(regex_t *compiled);
    List *str_regexpr_compiled(const regex_t *compiled, const String *text, int eflags);
    List *str_regexpr_compiled_unlocked(const regex_t *compiled, const String *text, int eflags);
    List *str_regexpr_compiled_with_locker(Locker *locker, const regex_t *compiled, const String *text, int eflags);
    List *str_regexpr_compiled_with_locker_unlocked(Locker *locker, const regex_t *compiled, const String *text, int eflags);
    List *regexpr_compiled(const regex_t *compiled, const char *text, int eflags);
    List *regexpr_compiled_with_locker(Locker *locker, const regex_t *compiled, const char *text, int eflags);
    String *str_regsub(const char *pattern, const char *replacement, String *text, int cflags, int eflags, int all);
    String *str_regsub_unlocked(const char *pattern, const char *replacement, String *text, int cflags, int eflags, int all);
    String *str_regsub_compiled(const regex_t *compiled, const char *replacement, String *text, int eflags, int all);
    String *str_regsub_compiled_unlocked(const regex_t *compiled, const char *replacement, String *text, int eflags, int all);
    List *str_fmt(const String *str, size_t line_width, StringAlignment alignment);
    List *str_fmt_unlocked(const String *str, size_t line_width, StringAlignment alignment);
    List *str_fmt_with_locker(Locker *locker, const String *str, size_t line_width, StringAlignment alignment);
    List *str_fmt_with_locker_unlocked(Locker *locker, const String *str, size_t line_width, StringAlignment alignment);
    List *fmt(const char *str, size_t line_width, StringAlignment alignment);
    List *fmt_with_locker(Locker *locker, const char *str, size_t line_width, StringAlignment alignment);
    List *str_split(const String *str, const char *delim);
    List *str_split_unlocked(const String *str, const char *delim);
    List *str_split_with_locker(Locker *locker, const String *str, const char *delim);
    List *str_split_with_locker_unlocked(Locker *locker, const String *str, const char *delim);
    List *split(const char *str, const char *delim);
    List *split_with_locker(Locker *locker, const char *str, const char *delim);
    List *str_regexpr_split(const String *str, const char *delim, int cflags, int eflags);
    List *str_regexpr_split_unlocked(const String *str, const char *delim, int cflags, int eflags);
    List *str_regexpr_split_with_locker(Locker *locker, const String *str, const char *delim, int cflags, int eflags);
    List *str_regexpr_split_with_locker_unlocked(Locker *locker, const String *str, const char *delim, int cflags, int eflags);
    List *regexpr_split(const char *str, const char *delim, int cflags, int eflags);
    List *regexpr_split_with_locker(Locker *locker, const char *str, const char *delim, int cflags, int eflags);
    String *str_join(const List *list, const char *delim);
    String *str_join_unlocked(const List *list, const char *delim);
    String *str_join_with_locker(Locker *locker, const List *list, const char *delim);
    String *str_join_with_locker_unlocked(Locker *locker, const List *list, const char *delim);
    String *join(const List *list, const char *delim);
    String *join_with_locker(Locker *locker, const List *list, const char *delim);
    int str_soundex(const String *str);
    int str_soundex_unlocked(const String *str);
    int soundex(const char *str);
    String *str_trim(String *str);
    String *str_trim_unlocked(String *str);
    char *trim(char *str);
    String *str_trim_left(String *str);
    String *str_trim_left_unlocked(String *str);
    char *trim_left(char *str);
    String *str_trim_right(String *str);
    String *str_trim_right_unlocked(String *str);
    char *trim_right(char *str);
    String *str_squeeze(String *str);
    String *str_squeeze_unlocked(String *str);
    char *squeeze(char *str);
    String *str_quote(const String *str, const char *quotable, char quote_char);
    String *str_quote_unlocked(const String *str, const char *quotable, char quote_char);
    String *str_quote_with_locker(Locker *locker, const String *str, const char *quotable, char quote_char);
    String *str_quote_with_locker_unlocked(Locker *locker, const String *str, const char *quotable, char quote_char);
    String *quote(const char *str, const char *quotable, char quote_char);
    String *quote_with_locker(Locker *locker, const char *str, const char *quotable, char quote_char);
    String *str_unquote(const String *str, const char *quotable, char quote_char);
    String *str_unquote_unlocked(const String *str, const char *quotable, char quote_char);
    String *str_unquote_with_locker(Locker *locker, const String *str, const char *quotable, char quote_char);
    String *str_unquote_with_locker_unlocked(Locker *locker, const String *str, const char *quotable, char quote_char);
    String *unquote(const char *str, const char *quotable, char quote_char);
    String *unquote_with_locker(Locker *locker, const char *str, const char *quotable, char quote_char);
    String *str_encode(const String *str, const char *uncoded, const char *coded, char quote_char, int printable);
    String *str_encode_unlocked(const String *str, const char *uncoded, const char *coded, char quote_char, int printable);
    String *str_encode_with_locker(Locker *locker, const String *str, const char *uncoded, const char *coded, char quote_char, int printable);
    String *str_encode_with_locker_unlocked(Locker *locker, const String *str, const char *uncoded, const char *coded, char quote_char, int printable);
    String *str_decode(const String *str, const char *uncoded, const char *coded, char quote_char, int printable);
    String *str_decode_unlocked(const String *str, const char *uncoded, const char *coded, char quote_char, int printable);
    String *str_decode_with_locker(Locker *locker, const String *str, const char *uncoded, const char *coded, char quote_char, int printable);
    String *str_decode_with_locker_unlocked(Locker *locker, const String *str, const char *uncoded, const char *coded, char quote_char, int printable);
    String *encode(const char *str, const char *uncoded, const char *coded, char quote_char, int printable);
    String *encode_with_locker(Locker *locker, const char *str, const char *uncoded, const char *coded, char quote_char, int printable);
    String *decode(const char *str, const char *uncoded, const char *coded, char quote_char, int printable);
    String *decode_with_locker(Locker *locker, const char *str, const char *uncoded, const char *coded, char quote_char, int printable);
    String *str_lc(String *str);
    String *str_lc_unlocked(String *str);
    char *lc(char *str);
    String *str_lcfirst(String *str);
    String *str_lcfirst_unlocked(String *str);
    char *lcfirst(char *str);
    String *str_uc(String *str);
    String *str_uc_unlocked(String *str);
    char *uc(char *str);
    String *str_ucfirst(String *str);
    String *str_ucfirst_unlocked(String *str);
    char *ucfirst(char *str);
    int str_chop(String *str);
    int str_chop_unlocked(String *str);
    int chop(char *str);
    int str_chomp(String *str);
    int str_chomp_unlocked(String *str);
    int chomp(char *str);
    int str_bin(const String *str);
    int str_bin_unlocked(const String *str);
    int bin(const char *str);
    int str_hex(const String *str);
    int str_hex_unlocked(const String *str);
    int hex(const char *str);
    int str_oct(const String *str);
    int str_oct_unlocked(const String *str);
    int oct(const char *str);
    int strcasecmp(const char *s1, const char *s2);
    int strncasecmp(const char *s1, const char *s2, size_t n);
    size_t strlcpy(char *dst, const char *src, size_t size);
    size_t strlcat(char *dst, const char *src, size_t size);
    char *cstrcpy(char *dst, const char *src);
    char *cstrcat(char *dst, const char *src);
    char *cstrchr(const char *str, int c);
    char *cstrpbrk(const char *str, const char *brk);
    char *cstrrchr(const char *str, int c);
    char *cstrstr(const char *str, const char *srch);
    int asprintf(char **str, const char *format, ...);
    int vasprintf(char **str, const char *format, va_list args);

=head1 DESCRIPTION

This module provides text strings that grow and shrink automatically, and
functions for manipulating them. Some of the functions were modelled on the
I<list(3)> module. Others were modelled on the string functions and
operators in I<perlfunc(1)> and I<perlop(1)>. Others came from I<OpenBSD>.

=over 4

=cut

*/

#ifndef _BSD_SOURCE
#define _BSD_SOURCE /* For snprintf() on OpenBSD-4.7 */
#endif

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE /* New name for _BSD_SOURCE */
#endif

#ifndef __BSD_VISIBLE
#define __BSD_VISIBLE 1 /* For ntohl() on FreeBSD-8.0 */
#endif

#ifndef _NETBSD_SOURCE
#define _NETBSD_SOURCE /* For ntohl() on NetBSD-5.0.2 */
#endif

#include "config.h"
#include "std.h"

#include <netinet/in.h>

#include "err.h"
#include "str.h"
#include "mem.h"
#include "fio.h"

#ifndef HAVE_SNPRINTF
#include "snprintf.h"
#endif

struct String
{
	size_t size;    /* number of bytes allocated */
	size_t length;  /* number of bytes used (including nul) */
	char *str;      /* vector of characters */
	Locker *locker; /* locking strategy for this string */
};

#define CHARSET 256

struct StringTR
{
	int squash;           /* whether or not to squash duplicate characters */
	short table[CHARSET]; /* the translation table */
	Locker *locker;       /* locking strategy for this structure */
};

typedef enum
{
	TRCODE_NOMAP = -1,
	TRCODE_DELETE = -2
}
TRCode;

#define is_alpha(c)  isalpha((int)(unsigned char)(c))
#define is_alnum(c)  isalnum((int)(unsigned char)(c))
#define is_print(c)  isprint((int)(unsigned char)(c))
#define is_space(c)  isspace((int)(unsigned char)(c))
#define is_digit(c)  isdigit((int)(unsigned char)(c))
#define is_xdigit(c) isxdigit((int)(unsigned char)(c))
#define to_lower(c)  tolower((int)(unsigned char)(c))
#define to_upper(c)  toupper((int)(unsigned char)(c))

#ifndef TEST

/* Minimum string length: must be a power of 2 */

static const size_t MIN_STRING_SIZE = 32;

/* Maximum bytes for an empty string: must be a power of 2 greater than MIN_STRING_SIZE */

static const size_t MIN_EMPTY_STRING_SIZE = 1024;

void (flockfile)(FILE *stream); /* Missing from old glibc headers */
void (funlockfile)(FILE *stream);

#ifndef HAVE_FLOCKFILE
#define flockfile(stream)
#define funlockfile(stream)
#define getc_unlocked(stream) getc(stream)
#endif

/*

C<int grow(String *str, size_t bytes)>

Allocates enough memory to add C<bytes> extra bytes to C<str> if necessary.
On success, returns C<0>. On error, returns C<-1>.

*/

static int grow(String *str, size_t bytes)
{
	int grown = 0;

	while (str->length + bytes > str->size)
	{
		if (str->size)
			str->size <<= 1;
		else
			str->size = MIN_STRING_SIZE;

		grown = 1;
	}

	if (grown)
		return mem_resize(&str->str, str->size) ? 0 : -1;

	return 0;
}

/*

C<int shrink(String *str, size_t bytes)>

Allocates less memory for removing C<bytes> bytes from C<str> if necessary.
On success, returns C<0>. On error, returns C<-1>.

*/

static int shrink(String *str, size_t bytes)
{
	int shrunk = 0;

	while (str->length - bytes < str->size >> 1)
	{
		if (str->size <= MIN_EMPTY_STRING_SIZE)
			break;

		str->size >>= 1;
		shrunk = 1;
	}

	if (shrunk)
		return mem_resize(&str->str, str->size) ? 0 : -1;

	return 0;
}

/*

C<int expand(String *str, ssize_t index, size_t range)>

Slides C<str>'s bytes, starting at C<index>, C<range> positions to the right
to make room for more. On success, returns C<0>. On error, returns C<-1>.

*/

static int expand(String *str, ssize_t index, size_t range)
{
	if (grow(str, range) == -1)
		return -1;

	memmove(str->str + index + range, str->str + index, (str->length - index) * sizeof(*str->str));
	str->length += range;

	return 0;
}

/*

C<int contract(String *str, ssize_t index, size_t range)>

Slides C<str>'s bytes, starting at C<index> + C<range>, C<range> positions
to the left to close a gap starting at C<index>. On success, returns C<0>.
On error, returns C<-1>.

*/

static int contract(String *str, ssize_t index, size_t range)
{
	memmove(str->str + index, str->str + index + range, (str->length - index - range) * sizeof(*str->str));

	if (shrink(str, range) == -1)
		return -1;

	str->length -= range;

	return 0;
}

/*

C<int adjust(String *str, ssize_t index, size_t range, size_t length)>

Expands or contracts C<str> as required so that C<str[index + range ..]>
occupies C<str[index + length ..]>. On success, returns C<0>. On error,
returns C<-1>.

*/

static int adjust(String *str, ssize_t index, size_t range, size_t length)
{
	if (range < length)
		return expand(str, index + range, length - range);

	if (range > length)
		return contract(str, index + length, range - length);

	return 0;
}

/*

=item C<String *str_create(const char *format, ...)>

Creates a I<String> specified by C<format> and the following arguments as in
I<sprintf(3)>. On success, returns the new string. It is the caller's
responsibility to deallocate the new string with I<str_release(3)> or
I<str_destroy(3)>. It is strongly recommended to use I<str_destroy(3)>,
because it also sets the pointer variable to C<null>. On error, returns
C<null> with C<errno> set appropriately.

B<Warning: Do not under any circumstances ever pass a non-literal string as
the format argument unless you know exactly how many conversions will take
place. Being careless with this is a very good way to build potential
security vulnerabilities into your programs. The same is true for all
functions that take a printf()-like format string as an argument.>

    String *str = str_create(buf);       // EVIL
    String *str = str_create("%s", buf); // GOOD

=cut

*/

String *str_create(const char *format, ...)
{
	String *str;
	va_list args;
	va_start(args, format);
	str = str_vcreate_with_locker_sized(NULL, MIN_STRING_SIZE, format, args);
	va_end(args);
	return str;
}

/*

=item C<String *str_create_with_locker(Locker *locker, const char *format, ...)>

Equivalent to I<str_create(3)> except that multiple threads accessing the
new string will be synchronized by C<locker>.

=cut

*/

String *str_create_with_locker(Locker *locker, const char *format, ...)
{
	String *str;
	va_list args;
	va_start(args, format);
	str = str_vcreate_with_locker_sized(locker, MIN_STRING_SIZE, format, args);
	va_end(args);
	return str;
}

/*

=item C<String *str_vcreate(const char *format, va_list args)>

Equivalent to I<str_create(3)> with the variable argument list specified
directly as for I<vprintf(3)>.

=cut

*/

String *str_vcreate(const char *format, va_list args)
{
	return str_vcreate_with_locker_sized(NULL, MIN_STRING_SIZE, format, args);
}

/*

=item C<String *str_vcreate_with_locker(Locker *locker, const char *format, va_list args)>

Equivalent to I<str_vcreate(3)> except that multiple threads accessing the
new string will be synchronized by C<locker>.

=cut

*/

String *str_vcreate_with_locker(Locker *locker, const char *format, va_list args)
{
	return str_vcreate_with_locker_sized(locker, MIN_STRING_SIZE, format, args);
}

/*

=item C<String *str_create_sized(size_t size, const char *format, ...)>

Creates a I<String> specified by C<format> and the following arguments as in
I<sprintf(3)>. The initial allocation for the string data is at least
C<size> bytes. On success, returns the new string. It is the caller's
responsibility to deallocate the new string with I<str_release(3)> or
I<str_destroy(3)>. It is strongly recommended to use I<str_destroy(3)>,
because it also sets the pointer variable to C<null>. On error, returns
C<null> with C<errno> set appropriately.

=cut

*/

String *str_create_sized(size_t size, const char *format, ...)
{
	String *str;
	va_list args;
	va_start(args, format);
	str = str_vcreate_with_locker_sized(NULL, size, format, args);
	va_end(args);
	return str;
}

/*

=item C<String *str_create_with_locker_sized(Locker *locker, size_t size, const char *format, ...)>

Equivalent to I<str_create_sized(3)> except that multiple threads accessing
the new string will be synchronised by C<locker>.

=cut

*/

String *str_create_with_locker_sized(Locker *locker, size_t size, const char *format, ...)
{
	String *str;
	va_list args;
	va_start(args, format);
	str = str_vcreate_with_locker_sized(locker, size, format, args);
	va_end(args);
	return str;
}

/*

=item C<String *str_vcreate_sized(size_t size, const char *format, va_list args)>

Equivalent to I<str_create_sized(3)> with the variable argument list
specified directly as for I<vprintf(3)>.

=cut

*/

String *str_vcreate_sized(size_t size, const char *format, va_list args)
{
	return str_vcreate_with_locker_sized(NULL, size, format, args);
}

/*

=item C<String *str_vcreate_with_locker_sized(Locker *locker, size_t size, const char *format, va_list args)>

Equivalent to I<str_vcreate_sized(3)> except that multiple threads accessing
the new string will be synchronised by C<locker>.

=cut

*/

#ifndef va_copy
#define va_copy(dst, src) __va_copy((dst), (src))
#endif

String *str_vcreate_with_locker_sized(Locker *locker, size_t size, const char *format, va_list args)
{
	String *str;
	char *buf = NULL;
	ssize_t length;
	unsigned int bit;
	va_list args_copy;

	for (bit = 1; bit; bit <<= 1)
	{
		if (bit >= size)
		{
			size = bit;
			break;
		}
	}

	if (!bit)
		return set_errnull(EINVAL);

	if (!format)
		format = "";

	for (;; size <<= 1)
	{
		if (!mem_resize(&buf, size))
		{
			mem_release(buf);
			return NULL;
		}

#ifdef va_copy
		va_copy(args_copy, args);
		length = vsnprintf(buf, size, format, args_copy);
		va_end(args_copy);
#else
		length = vsnprintf(buf, size, format, args);
#endif
		if (length != -1 && length < size)
			break;
	}

	if (!(str = mem_new(String))) /* XXX decouple */
	{
		mem_release(buf);
		return NULL;
	}

	str->size = size;
	str->length = length + 1;
	str->str = buf;
	str->locker = locker;

	return str;
}

/*

=item C<String *str_copy(const String *str)>

Creates a copy of C<str>. On success, returns the copy. It is the caller's
responsibility to deallocate the new string with I<str_release(3)> or
I<str_destroy(3)>. It is strongly recommended to use I<str_destroy(3)>,
because it also sets the pointer variable to C<null>. On error, returns
C<null> with C<errno> set appropriately.

=cut

*/

String *str_copy(const String *str)
{
	return str_copy_with_locker(NULL, str);
}

/*

=item C<String *str_copy_unlocked(const String *str)>

Equivalent to I<str_copy(3)> except that C<str> is not read-locked.

=cut

*/

String *str_copy_unlocked(const String *str)
{
	return str_copy_with_locker_unlocked(NULL, str);
}

/*

=item C<String *str_copy_with_locker(Locker *locker, const String *str)>

Equivalent to I<str_copy(3)> except that multiple threads accessing the new
string will be synchronised by C<locker>.

=cut

*/

String *str_copy_with_locker(Locker *locker, const String *str)
{
	return str_substr_with_locker(locker, str, 0, -1);
}

/*

=item C<String *str_copy_with_locker_unlocked(Locker *locker, const String *str)>

Equivalent to I<str_copy_with_locker(3)> except that C<str> is not
read-locked.

=cut

*/

String *str_copy_with_locker_unlocked(Locker *locker, const String *str)
{
	return str_substr_with_locker_unlocked(locker, str, 0, -1);
}

/*

=item C<String *str_fgetline(FILE *stream)>

Similar to I<fgets(3)> except that it recognises UNIX (C<"\n">), DOS/Windows
(C<"\r\n">) and old Macintosh (C<"\r">) line endings (even different line
endings in the same file), and it can read a line of any size into the
I<String> that it returns. Reading stops after the C<EOF>, or after the end
of the line is reached. Line endings are always stored as a single C<"\n">
character. A C<nul> is placed after the last character in the buffer. On
success, returns a new I<String>. It is the caller's responsibility to
deallocate the new string with I<str_release(3)> or I<str_destroy(3)>. It is
strongly recommended to use I<str_destroy(3)>, because it also sets the
pointer variable to C<null>. On error, or when the end of file occurs while
no characters have been read, returns C<null>. Calls to this function can be
mixed with calls to other input functions from the I<stdio> library on the
same input stream.

=cut

*/

String *str_fgetline(FILE *stream)
{
	return str_fgetline_with_locker(NULL, stream);
}

/*

=item C<String *str_fgetline_with_locker(Locker *locker, FILE *stream)>

Equivalent to I<str_fgetline(3)> except that multiple threads accessing the
new string will be synchronised by C<locker>.

=cut

*/

String *str_fgetline_with_locker(Locker *locker, FILE *stream)
{
	String *ret = NULL;
	char buf[BUFSIZ];

	flockfile(stream);

	while (fgetline_unlocked(buf, BUFSIZ, stream))
	{
		if (!ret)
		{
			if (!(ret = str_create_with_locker(locker, "%s", buf)))
				break;
		}
		else if (!str_append(ret, "%s", buf))
		{
			str_destroy(&ret);
			break;
		}

		if (cstr(ret)[ret->length - 2] == '\n')
			break;
	}

	funlockfile(stream);
	return ret;
}

/*

=item C<void str_release(String *str)>

Releases (deallocates) C<str>.

=cut

*/

void str_release(String *str)
{
	Locker *locker;

	if (!str)
		return;

	if (str_wrlock(str))
		return;

	locker = str->locker;
	mem_release(str->str);
	mem_release(str);
	locker_unlock(locker);
}

/*

=item C<void *str_destroy(String **str)>

Destroys (deallocates and sets to C<null>) C<*str>. Returns C<null>.
B<Note:> strings shared by multiple threads must not be destroyed until
after all threads have finished with it.

=cut

*/

void *str_destroy(String **str)
{
	if (str && *str)
	{
		str_release(*str);
		*str = NULL;
	}

	return NULL;
}

/*

=item C<int str_rdlock(const String *str)>

Claims a read lock on C<str> (if C<str> was created with a I<Locker>).
Clients must call this before calling I<cstr(3)> (for the purpose of reading
the raw string data) on a string that was created with a I<Locker>. It is
the client's responsibility to call I<str_unlock(3)> when finished with the
raw string data. It is also needed when multiple read-only I<str(3)> module
functions need to be called atomically. It is the caller's responsibility to
call I<str_unlock(3)> after the atomic operation. The only functions that
may be called on C<str> between calls to I<str_rdlock(3)> and
I<str_unlock(3)> are I<cstr(3)> and any read-only I<str(3)> module functions
whose name ends with C<_unlocked>. On success, returns C<0>. On error,
returns an error code.

=cut

*/

#define str_rdlock(str) ((str) ? locker_rdlock((str)->locker) : EINVAL)
#define str_wrlock(str) ((str) ? locker_wrlock((str)->locker) : EINVAL)
#define str_unlock(str) ((str) ? locker_unlock((str)->locker) : EINVAL)

int (str_rdlock)(const String *str)
{
	return str_rdlock(str);
}

/*

=item C<int str_wrlock(const String *str)>

Claims a write lock on C<str> (if C<str> was created with a I<Locker>).
Clients need to call this before calling I<cstr(3)> (for the purpose of
modifying the raw string data) on a string that was created with a
I<Locker>. It is the client's responsibility to call I<str_unlock(3)> when
finished with the raw string data. It is also needed when multiple
read/write I<str(3)> module functions need to be called atomically. It is
the caller's responsibility to call I<str_unlock(3)> after the atomic
operation. The only functions that may be called on C<str> between calls to
I<str_wrlock(3)> and I<str_unlock(3)> are I<cstr(3)> and any I<str(3)>
module functions whose name ends with C<_unlocked>. On success, returns
C<0>. On error, returns an error code.

=cut

*/

int (str_wrlock)(const String *str)
{
	return str_wrlock(str);
}

/*

=item C<int str_unlock(const String *str)>

Unlocks a read lock or a write lock on C<str> obtained with I<str_rdlock(3)>
or I<str_wrlock(3)> (if C<str> was created with a I<Locker>). On success,
returns C<0>. On error, returns an error code.

=cut

*/

int (str_unlock)(const String *str)
{
	return str_unlock(str);
}

/*

=item C<int str_empty(const String *str)>

Returns whether or not C<str> is the empty string. On error, returns C<-1>
with C<errno> set appropriately.

=cut

*/

int str_empty(const String *str)
{
	int empty;
	int err;

	if (!str)
		return set_errno(EINVAL);

	if ((err = str_rdlock(str)))
		return set_errno(err);

	empty = str_empty_unlocked(str);

	if ((err = str_unlock(str)))
		return set_errno(err);

	return empty;
}

/*

=item C<int str_empty_unlocked(const String *str)>

Equivalent to I<str_empty(3)> except that C<str> is not read-locked.

=cut

*/

int str_empty_unlocked(const String *str)
{
	if (!str)
		return set_errno(EINVAL);

	return (str->length == 1);
}

/*

=item C<ssize_t str_length(const String *str)>

Returns the length of C<str>. On error, returns C<-1> with C<errno> set
appropriately.

=cut

*/

ssize_t str_length(const String *str)
{
	size_t length;
	int err;

	if (!str)
		return set_errno(EINVAL);

	if ((err = str_rdlock(str)))
		return set_errno(err);

	length = str_length_unlocked(str);

	if ((err = str_unlock(str)))
		return set_errno(err);

	return length;
}
/*

=item C<ssize_t str_length_unlocked(const String *str)>

Equivalent to I<str_length(3)> except that C<str> is not read-locked.

=cut

*/

ssize_t str_length_unlocked(const String *str)
{
	if (!str)
		return set_errno(EINVAL);

	return str->length - 1;
}

/*

=item C<char *cstr(const String *str)>

Returns the raw I<C> string in C<str>. Do not use this pointer to extend the
length of the string. It's OK to use it to reduce the length of the string,
provided that you call I<str_set_length_unlocked(3)> or
I<str_recalc_length_unlocked(3)> immediately afterwards. When used on a
string that is shared by multiple threads, I<cstr(3)> must appear between
calls to I<str_rdlock(3)> or I<str_wrlock(3)> and I<str_unlock(3)>.

=cut

*/

char *cstr(const String *str)
{
	if (!str)
		return set_errnull(EINVAL);

	return str->str;
}

/*

=item C<ssize_t str_set_length(String *str, size_t length)>

Sets the length of C<str> to C<length>. Only needed after the raw I<C>
string returned by I<cstr(3)> has been used to shorten a string. On success,
returns the length of C<str>. On error, returns C<-1> with C<errno> set
appropriately.

=cut

*/

ssize_t str_set_length(String *str, size_t length)
{
	ssize_t len;
	int err;

	if (!str)
		return set_errno(EINVAL);

	if ((err = str_wrlock(str)))
		return set_errno(err);

	len = str_set_length_unlocked(str, length);

	if ((err = str_unlock(str)))
		return set_errno(err);

	return len;
}

/*

=item C<ssize_t str_set_length_unlocked(String *str, size_t length)>

Equivalent to I<str_set_length(3)> except that C<str> is not write-locked.

=cut

*/

ssize_t str_set_length_unlocked(String *str, size_t length)
{
	if (!str || length >= str->length)
		return set_errno(EINVAL);

	str->length = length + 1;
	str->str[str->length - 1] = '\0';

	return str->length - 1;
}

/*

=item C<ssize_t str_recalc_length(String *str)>

Calculates and stores the length of C<str>. Only needed after the raw I<C>
string returned by I<cstr(3)> has been used to shorten a string. Note:
Treats C<str> as a C<nul>-terminated string and should be avoided. Use
I<str_set_length(3)> instead. On success, returns the length of C<str>. On
error, returns C<-1> with C<errno> set appropriately.

=cut

*/

ssize_t str_recalc_length(String *str)
{
	ssize_t len;
	int err;

	if (!str)
		return set_errno(EINVAL);

	if ((err = str_wrlock(str)))
		return set_errno(err);

	len = str_recalc_length_unlocked(str);

	if ((err = str_unlock(str)))
		return set_errno(err);

	return len;
}

/*

=item C<ssize_t str_recalc_length_unlocked(String *str)>

Equivalent to I<str_recalc_length(3)> except that C<str> is not
write-locked.

=cut

*/

ssize_t str_recalc_length_unlocked(String *str)
{
	if (!str)
		return set_errno(EINVAL);

	str->length = strlen(str->str) + 1;

	return str->length - 1;
}

/*

=item C<String *str_clear(String *str)>

Makes C<str> the empty string. On success, returns C<str>. On error, returns
C<null> with C<errno> set appropriately.

=cut

*/

String *str_clear(String *str)
{
	return str_remove_range(str, 0, -1);
}

/*

=item C<String *str_clear_unlocked(String *str)>

Equivalent to I<str_clear(3)> except that C<str> is not write-locked.

=cut

*/

String *str_clear_unlocked(String *str)
{
	return str_remove_range_unlocked(str, 0, -1);
}

/*

=item C<String *str_remove(String *str, ssize_t index)>

Removes the C<index>'th character from C<str>. If C<index> is negative, it
refers to a character position relative to the end of the string (C<-1> is
the position after the last character, C<-2> is the position of the last
character, and so on). On success, returns C<str>. On error, returns C<null>
with C<errno> set appropriately.

=cut

*/

String *str_remove(String *str, ssize_t index)
{
	return str_remove_range(str, index, 1);
}

/*

=item C<String *str_remove_unlocked(String *str, ssize_t index)>

Equivalent to I<str_remove(3)> except that C<str> is not write-locked.

=cut

*/

String *str_remove_unlocked(String *str, ssize_t index)
{
	return str_remove_range_unlocked(str, index, 1);
}

/*

=item C<String *str_remove_range(String *str, ssize_t index, ssize_t range)>

Removes C<range> characters from C<str> starting at C<index>. If C<index> or
C<range> are negative, they refer to character positions relative to the end
of the string (C<-1> is the position after the last character, C<-2> is the
position of the last character, and so on). On success, returns C<str>. On
error, returns C<null> with C<errno> set appropriately.

=cut

*/

String *str_remove_range(String *str, ssize_t index, ssize_t range)
{
	String *ret;
	int err;

	if (!str)
		return set_errnull(EINVAL);

	if ((err = str_wrlock(str)))
		return set_errnull(err);

	ret = str_remove_range_unlocked(str, index, range);

	if ((err = str_unlock(str)))
		return set_errnull(err);

	return ret;
}

/*

=item C<String *str_remove_range_unlocked(String *str, ssize_t index, ssize_t range)>

Equivalent to I<str_remove_range(3)> except that C<str> is not write-locked.

=cut

*/

String *str_remove_range_unlocked(String *str, ssize_t index, ssize_t range)
{
	if (!str)
		return set_errnull(EINVAL);

	if (index < 0)
		index = str->length + index;

	if (index < 0)
		return set_errnull(EINVAL);

	if (range < 0)
		range = str->length + range - index;

	if (range < 0)
		return set_errnull(EINVAL);

	if (str->length - 1 < index + range)
		return set_errnull(EINVAL);

	contract(str, index, range);

	return str;
}

/*

=item C<String *str_insert(String *str, ssize_t index, const char *format, ...)>

Adds the string specified by C<format> to C<str> at position C<index>. If
C<index> is negative, it refers to a character position relative to the end
of the string (C<-1> is the position after the last character, C<-2> is the
position of the last character, and so on). On success, returns C<str>. On
error, returns C<null> with C<errno> set appropriately.

=cut

*/

String *str_insert(String *str, ssize_t index, const char *format, ...)
{
	String *ret;
	va_list args;
	va_start(args, format);
	ret = str_vinsert(str, index, format, args);
	va_end(args);
	return ret;
}

/*

=item C<String *str_insert_unlocked(String *str, ssize_t index, const char *format, ...)>

Equivalent to I<str_insert(3)> except that C<str> is not write-locked.

=cut

*/

String *str_insert_unlocked(String *str, ssize_t index, const char *format, ...)
{
	String *ret;
	va_list args;
	va_start(args, format);
	ret = str_vinsert_unlocked(str, index, format, args);
	va_end(args);
	return ret;
}

/*

=item C<String *str_vinsert(String *str, ssize_t index, const char *format, va_list args)>

Equivalent to I<str_insert(3)> with the variable argument list specified
directly as for I<vprintf(3)>.

=cut

*/

String *str_vinsert(String *str, ssize_t index, const char *format, va_list args)
{
	String *ret;
	int err;

	if (!str)
		return set_errnull(EINVAL);

	if ((err = str_wrlock(str)))
		return set_errnull(err);

	ret = str_vinsert_unlocked(str, index, format, args);

	if ((err = str_unlock(str)))
		return set_errnull(err);

	return ret;
}

/*

=item C<String *str_vinsert_unlocked(String *str, ssize_t index, const char *format, va_list args)>

Equivalent to I<str_vinsert(3)> except that C<str> is not write-locked.

=cut

*/

String *str_vinsert_unlocked(String *str, ssize_t index, const char *format, va_list args)
{
	String *tmp, *ret;

	if (!str)
		return set_errnull(EINVAL);

	if (index < 0)
		index = str->length + index;

	if (index < 0)
		return set_errnull(EINVAL);

	if (str->length - 1 < index)
		return set_errnull(EINVAL);

	if (!(tmp = str_vcreate(format, args)))
		return NULL;

	ret = str_insert_str_unlocked(str, index, tmp);
	str_release(tmp);

	return ret;
}

/*

=item C<String *str_insert_str(String *str, ssize_t index, const String *src)>

Inserts C<src> into C<str>, starting at position C<index>. If C<index> is
negative, it refers to a character position relative to the end of the
string (C<-1> is the position after the last character, C<-2> is the
position of the last character, and so on). On success, returns C<str>. On
error, returns C<null> with C<errno> set appropriately.

=cut

*/

String *str_insert_str(String *str, ssize_t index, const String *src)
{
	String *ret;
	int err;

	if (!str || !src)
		return set_errnull(EINVAL);

	if ((err = str_rdlock(src)))
		return set_errnull(err);

	if ((err = str_wrlock(str)))
	{
		str_unlock(src);
		return set_errnull(err);
	}

	ret = str_insert_str_unlocked(str, index, src);

	if ((err = str_unlock(str)))
	{
		str_unlock(src);
		return set_errnull(err);
	}

	if ((err = str_unlock(src)))
		return set_errnull(err);

	return ret;
}

/*

=item C<String *str_insert_str_unlocked(String *str, ssize_t index, const String *src)>

Equivalent to I<str_insert_str(3)> except that C<str> is not write-locked
and C<src> is not read-locked. Note: If C<src> needs to be read-locked, it
is the caller's responsibility to lock and unlock it explicitly with
I<str_rdlock(3)> and I<str_unlock(3)>.

=cut

*/

String *str_insert_str_unlocked(String *str, ssize_t index, const String *src)
{
	size_t length;

	if (!str || !src)
		return set_errnull(EINVAL);

	if (index < 0)
		index = str->length + index;

	if (index < 0)
		return set_errnull(EINVAL);

	if (str->length - 1 < index)
		return set_errnull(EINVAL);

	length = src->length - 1;

	if (expand(str, index, length) == -1)
		return NULL;

	memcpy(str->str + index, src->str, length);

	return str;
}

/*

=item C<String *str_append(String *str, const char *format, ...)>

Appends the string specified by C<format> to C<str>. On success, returns
C<str>. On error, returns C<null> with C<errno> set appropriately.

=cut

*/

String *str_append(String *str, const char *format, ...)
{
	String *ret;
	va_list args;
	va_start(args, format);
	ret = str_vinsert(str, -1, format, args);
	va_end(args);
	return ret;
}

/*

=item C<String *str_append_unlocked(String *str, const char *format, ...)>

Equivalent to I<str_append(3)> except that C<str> is not write-locked.

=cut

*/

String *str_append_unlocked(String *str, const char *format, ...)
{
	String *ret;
	va_list args;
	va_start(args, format);
	ret = str_vinsert_unlocked(str, -1, format, args);
	va_end(args);
	return ret;
}

/*

=item C<String *str_vappend(String *str, const char *format, va_list args)>

Equivalent to I<str_append(3)> with the variable argument list specified
directly as for I<vprintf(3)>.

=cut

*/

String *str_vappend(String *str, const char *format, va_list args)
{
	return str_vinsert(str, -1, format, args);
}

/*

=item C<String *str_vappend_unlocked(String *str, const char *format, va_list args)>

Equivalent to I<str_vappend(3)> except that C<str> is not write-locked.

=cut

*/

String *str_vappend_unlocked(String *str, const char *format, va_list args)
{
	return str_vinsert_unlocked(str, -1, format, args);
}

/*

=item C<String *str_append_str(String *str, const String *src)>

Appends C<src> to C<str>. On success, returns C<str>. On error, returns
C<null> with C<errno> set appropriately.

=cut

*/

String *str_append_str(String *str, const String *src)
{
	return str_insert_str(str, -1, src);
}

/*

=item C<String *str_append_str_unlocked(String *str, const String *src)>

Equivalent to I<str_append_str(3)> except that C<str> is not write-locked
and C<src> is not read-locked. Note: If C<src> needs to be read-locked, it
is the caller's responsibility to lock and unlock it explicitly with
I<str_rdlock(3)> and I<str_unlock(3)>.

=cut

*/

String *str_append_str_unlocked(String *str, const String *src)
{
	return str_insert_str_unlocked(str, -1, src);
}

/*

=item C<String *str_prepend(String *str, const char *format, ...)>

Prepends the string specified by C<format> to C<str>. On success, returns
C<str>. On error, returns C<null> with C<errno> set appropriately.

=cut

*/

String *str_prepend(String *str, const char *format, ...)
{
	String *ret;
	va_list args;
	va_start(args, format);
	ret = str_vinsert(str, 0, format, args);
	va_end(args);
	return ret;
}

/*

=item C<String *str_prepend_unlocked(String *str, const char *format, ...)>

Equivalent to I<str_prepend(3)> except that C<str> is not write-locked.

=cut

*/

String *str_prepend_unlocked(String *str, const char *format, ...)
{
	String *ret;
	va_list args;
	va_start(args, format);
	ret = str_vinsert_unlocked(str, 0, format, args);
	va_end(args);
	return ret;
}

/*

=item C<String *str_vprepend(String *str, const char *format, va_list args)>

Equivalent to I<str_prepend(3)> with the variable argument list specified
directly as for I<vprintf(3)>.

=cut

*/

String *str_vprepend(String *str, const char *format, va_list args)
{
	return str_vinsert(str, 0, format, args);
}

/*

=item C<String *str_vprepend_unlocked(String *str, const char *format, va_list args)>

Equivalent to I<str_vprepend(3)> except that C<str> is not write-locked.

=cut

*/

String *str_vprepend_unlocked(String *str, const char *format, va_list args)
{
	return str_vinsert_unlocked(str, 0, format, args);
}

/*

=item C<String *str_prepend_str(String *str, const String *src)>

Prepends C<src> to C<str>. On success, returns C<str>. On error, returns
C<null> with C<errno> set appropriately.

=cut

*/

String *str_prepend_str(String *str, const String *src)
{
	return str_insert_str(str, 0, src);
}

/*

=item C<String *str_prepend_str_unlocked(String *str, const String *src)>

Equivalent to I<str_prepend_str(3)> except that C<str> is not write-locked
and C<src> is not read-locked. Note: If C<src> needs to be read-locked, it
is the caller's responsibility to lock and unlock it explicitly with
I<str_rdlock(3)> and I<str_unlock(3)>.

=cut

*/

String *str_prepend_str_unlocked(String *str, const String *src)
{
	return str_insert_str_unlocked(str, 0, src);
}

/*

=item C<String *str_replace(String *str, ssize_t index, ssize_t range, const char *format, ...)>

Replaces C<range> characters in C<str>, starting at C<index>, with the
string specified by C<format>. If C<index> or C<range> are negative, they
refer to character positions relative to the end of the string (C<-1> is the
position after the last character, C<-2> is the position of the last
character, and so on). On success, returns C<str>. On error, returns C<null>
with C<errno> set appropriately.

=cut

*/

String *str_replace(String *str, ssize_t index, ssize_t range, const char *format, ...)
{
	String *ret;
	va_list args;
	va_start(args, format);
	ret = str_vreplace(str, index, range, format, args);
	va_end(args);
	return ret;
}

/*

=item C<String *str_replace_unlocked(String *str, ssize_t index, ssize_t range, const char *format, ...)>

Equivalent to I<str_replace(3)> except that C<str> is not write-locked.

=cut

*/

String *str_replace_unlocked(String *str, ssize_t index, ssize_t range, const char *format, ...)
{
	String *ret;
	va_list args;
	va_start(args, format);
	ret = str_vreplace_unlocked(str, index, range, format, args);
	va_end(args);
	return ret;
}

/*

=item C<String *str_vreplace(String *str, ssize_t index, ssize_t range, const char *format, va_list args)>

Equivalent to I<str_replace(3)> with the variable argument list specified
directly as for I<vprintf(3)>.

=cut

*/

String *str_vreplace(String *str, ssize_t index, ssize_t range, const char *format, va_list args)
{
	String *tmp, *ret;

	if (!str)
		return set_errnull(EINVAL);

	if (!(tmp = str_vcreate(format, args)))
		return NULL;

	ret = str_replace_str(str, index, range, tmp);
	str_release(tmp);

	return ret;
}

/*

=item C<String *str_vreplace_unlocked(String *str, ssize_t index, ssize_t range, const char *format, va_list args)>

Equivalent to I<str_vreplace(3)> except that C<str> is not write-locked.

=cut

*/

String *str_vreplace_unlocked(String *str, ssize_t index, ssize_t range, const char *format, va_list args)
{
	String *tmp, *ret;

	if (!str)
		return set_errnull(EINVAL);

	if (!(tmp = str_vcreate(format, args)))
		return NULL;

	ret = str_replace_str_unlocked(str, index, range, tmp);
	str_release(tmp);

	return ret;
}

/*

=item C<String *str_replace_str(String *str, ssize_t index, ssize_t range, const String *src)>

Replaces C<range> characters in C<str>, starting at C<index>, with C<src>.
If C<index> or C<range> are negative, they refer to character positions
relative to the end of the string (C<-1> is the position after the last
character, C<-2> is the position of the last character, and so on). On
success, return C<str>. On error, returns C<null> with C<errno> set
appropriately.

=cut

*/

String *str_replace_str(String *str, ssize_t index, ssize_t range, const String *src)
{
	String *ret;
	int err;

	if (!src || !str)
		return set_errnull(EINVAL);

	if ((err = str_rdlock(src)))
		return set_errnull(err);

	if ((err = str_wrlock(str)))
	{
		str_unlock(src);
		return set_errnull(err);
	}

	ret = str_replace_str_unlocked(str, index, range, src);

	if ((err = str_unlock(str)))
	{
		str_unlock(src);
		return set_errnull(err);
	}

	if ((err = str_unlock(src)))
		return set_errnull(err);

	return ret;
}

/*

=item C<String *str_replace_str_unlocked(String *str, ssize_t index, ssize_t range, const String *src)>

Equivalent to I<str_replace_str(3)> except that C<str> is not write-locked
and C<src> is not read-locked. Note: If C<src> needs to be read-locked, it
is the caller's responsibility to lock and unlock it explicitly with
I<str_rdlock(3)> and I<str_unlock(3)>.

=cut

*/

String *str_replace_str_unlocked(String *str, ssize_t index, ssize_t range, const String *src)
{
	size_t length;

	if (!src || !str)
		return set_errnull(EINVAL);

	if (index < 0)
		index = str->length + index;

	if (index < 0)
		return set_errnull(EINVAL);

	if (range < 0)
		range = str->length + range - index;

	if (range < 0)
		return set_errnull(EINVAL);

	if (str->length - 1 < index + range)
		return set_errnull(EINVAL);

	length = src->length - 1;

	if (adjust(str, index, range, length) == -1)
		return NULL;

	memcpy(str->str + index, src->str, length);

	return str;
}

/*

=item C<String *str_substr(const String *str, ssize_t index, ssize_t range)>

Creates a new I<String> object consisting of C<range> characters from
C<str>, starting at C<index>. If C<index> or C<range> are negative, they
refer to character positions relative to the end of the string (C<-1> is the
position after the last character, C<-2> is the position of the last
character, and so on). On success, returns the new string. It is the
caller's responsibility to deallocate the new string with I<str_release(3)>
or I<str_destroy(3)>. On error, returns C<null> with C<errno> set
appropriately.

=cut

*/

String *str_substr(const String *str, ssize_t index, ssize_t range)
{
	return str_substr_with_locker(NULL, str, index, range);
}

/*

=item C<String *str_substr_unlocked(const String *str, ssize_t index, ssize_t range)>

Equivalent to I<str_substr(3)> except that C<str> is not read-locked.

=cut

*/

String *str_substr_unlocked(const String *str, ssize_t index, ssize_t range)
{
	return str_substr_with_locker_unlocked(NULL, str, index, range);
}

/*

=item C<String *str_substr_with_locker(Locker *locker, const String *str, ssize_t index, ssize_t range)>

Equivalent to I<str_substr(3)> except that multiple threads accessing the
new substring will be synchronised by C<locker>.

=cut

*/

String *str_substr_with_locker(Locker *locker, const String *str, ssize_t index, ssize_t range)
{
	String *ret;
	int err;

	if (!str)
		return set_errnull(EINVAL);

	if ((err = str_rdlock(str)))
		return set_errnull(err);

	ret = str_substr_with_locker_unlocked(locker, str, index, range);

	if ((err = str_unlock(str)))
		return set_errnull(err);

	return ret;
}

/*

=item C<String *str_substr_with_locker_unlocked(Locker *locker, const String *str, ssize_t index, ssize_t range)>

Equivalent to I<str_substr_with_locker(3)> except that C<str> is not
read-locked.

=cut

*/

String *str_substr_with_locker_unlocked(Locker *locker, const String *str, ssize_t index, ssize_t range)
{
	String *ret;

	if (!str)
		return set_errnull(EINVAL);

	if (index < 0)
		index = str->length + index;

	if (index < 0)
		return set_errnull(EINVAL);

	if (range < 0)
		range = str->length + range - index;

	if (range < 0)
		return set_errnull(EINVAL);

	if (str->length - 1 < index + range)
		return set_errnull(EINVAL);

	if (!(ret = str_create_with_locker_sized(locker, range + 1, NULL)))
		return NULL;

	memcpy(ret->str, str->str + index, range);
	ret->length = range + 1;
	ret->str[ret->length - 1] = '\0';

	return ret;
}

/*

=item C<String *substr(const char *str, ssize_t index, ssize_t range)>

Equivalent to I<str_substr(3)> but works on an ordinary I<C> string.

=cut

*/

String *substr(const char *str, ssize_t index, ssize_t range)
{
	return substr_with_locker(NULL, str, index, range);
}

/*

=item C<String *substr_with_locker(Locker *locker, const char *str, ssize_t index, ssize_t range)>

Equivalent to I<substr(3)> except that multiple threads accessing the new
substring will be synchronised by C<locker>. Note that no locking is
performed on C<str> as it is a raw I<C> string.

=cut

*/

String *substr_with_locker(Locker *locker, const char *str, ssize_t index, ssize_t range)
{
	String *ret;
	size_t len = 0;

	if (!str)
		return set_errnull(EINVAL);

	if (index < 0 || range < 0)
		len = strlen(str) + 1;

	if (index < 0)
		index = len + index;

	if (index < 0)
		return set_errnull(EINVAL);

	if (range < 0)
		range = len + range - index;

	if (range < 0)
		return set_errnull(EINVAL);

	if (!(ret = str_create_with_locker_sized(locker, range + 1, NULL)))
		return NULL;

	memcpy(ret->str, str + index, range);
	ret->length = range + 1;
	ret->str[ret->length - 1] = '\0';

	return ret;
}

/*

=item C<String *str_splice(String *str, ssize_t index, ssize_t range)>

Removes a substring from C<str> starting at C<index> of length C<range>
characters. If C<index> or C<range> are negative, they refer to character
positions relative to the end of the string (C<-1> is the position after the
last character, C<-2> is the position of the last character, and so on). On
success, returns the substring. It is the caller's responsibility to
deallocate the new substring with I<str_release(3)> or I<str_destroy(3)>. It
is strongly recommended to use I<str_destroy(3)>, because it also sets the
pointer variable to C<null>. On error, returns C<null> with C<errno> set
appropriately.

=cut

*/

String *str_splice(String *str, ssize_t index, ssize_t range)
{
	return str_splice_with_locker(NULL, str, index, range);
}

/*

=item C<String *str_splice_unlocked(String *str, ssize_t index, ssize_t range)>

Equivalent to I<str_splice(3)> except that C<str> is not write-locked.

=cut

*/

String *str_splice_unlocked(String *str, ssize_t index, ssize_t range)
{
	return str_splice_with_locker_unlocked(NULL, str, index, range);
}

/*

=item C<String *str_splice_with_locker(Locker *locker, String *str, ssize_t index, ssize_t range)>

Equivalent to I<str_splice(3)> except that multiple threads accessing the
new string will be synchronised by C<locker>.

=cut

*/

String *str_splice_with_locker(Locker *locker, String *str, ssize_t index, ssize_t range)
{
	String *ret;
	int err;

	if (!str)
		return set_errnull(EINVAL);

	if ((err = str_wrlock(str)))
		return set_errnull(err);

	ret = str_splice_with_locker_unlocked(locker, str, index, range);

	if ((err = str_unlock(str)))
	{
		str_release(ret);
		return set_errnull(err);
	}

	return ret;
}

/*

=item C<String *str_splice_with_locker_unlocked(Locker *locker, String *str, ssize_t index, ssize_t range)>

Equivalent to I<str_splice_with_locker(3)> except that C<str> is not
write-locked.

=cut

*/

String *str_splice_with_locker_unlocked(Locker *locker, String *str, ssize_t index, ssize_t range)
{
	String *ret;

	if (!str)
		return set_errnull(EINVAL);

	if (!(ret = str_substr_with_locker_unlocked(locker, str, index, range)))
		return NULL;

	if (!str_remove_range_unlocked(str, index, range))
	{
		str_release(ret);
		return NULL;
	}

	return ret;
}

/*

=item C<String *str_repeat(size_t count, const char *format, ...)>

Creates a new I<String> containing the string determined by C<format>
repeated C<count> times. On success, return the new string. It is the
caller's responsibility to deallocate the new string with I<str_release(3)>
or I<str_destroy(3)>. It is strongly recommended to use I<str_destroy(3)>,
because it also sets the pointer variable to C<null>. On error, returns
C<null> with C<errno> set appropriately.

=cut

*/

String *str_repeat(size_t count, const char *format, ...)
{
	String *ret;
	va_list args;
	va_start(args, format);
	ret = str_vrepeat_with_locker(NULL, count, format, args);
	va_end(args);
	return ret;
}

/*

=item C<String *str_repeat_with_locker(Locker *locker, size_t count, const char *format, ...)>

Equivalent to I<str_repeat(3)> except that multiple threads accessing the
new string will be synchronised by C<locker>.

=cut

*/

String *str_repeat_with_locker(Locker *locker, size_t count, const char *format, ...)
{
	String *ret;
	va_list args;
	va_start(args, format);
	ret = str_vrepeat_with_locker(locker, count, format, args);
	va_end(args);
	return ret;
}

/*

=item C<String *str_vrepeat(size_t count, const char *format, va_list args)>

Equivalent to I<str_repeat(3)> with the variable argument list specified
directly as for I<vprintf(3)>.

=cut

*/

String *str_vrepeat(size_t count, const char *format, va_list args)
{
	return str_vrepeat_with_locker(NULL, count, format, args);
}

/*

=item C<String *str_vrepeat_with_locker(Locker *locker, size_t count, const char *format, va_list args)>

Equivalent to I<str_vrepeat(3)> except that multiple threads accessing the
new string will be synchronised by C<locker>.

=cut

*/

String *str_vrepeat_with_locker(Locker *locker, size_t count, const char *format, va_list args)
{
	String *tmp, *ret;
	ssize_t length;
	size_t i;

	if (!(tmp = str_vcreate(format, args)))
		return NULL;

	if ((length = str_length(tmp)) == -1)
		return NULL;

	if (!(ret = str_create_with_locker_sized(locker, length * count + 1, NULL)))
	{
		str_release(tmp);
		return NULL;
	}

	for (i = 0; i < count; ++i)
	{
		if (!str_append_str(ret, tmp))
		{
			str_release(tmp);
			str_release(ret);
			return NULL;
		}
	}

	str_release(tmp);

	return ret;
}

/*

=item C<int str_tr(String *str, const char *from, const char *to, int option)>

This is just like the I<perl(1) tr> operator. The following documentation
was taken from I<perlop(1)>.

Transliterates all occurrences of the characters in C<from> with the
corresponding character in C<to>. On success, returns the number of
characters replaced or deleted. On error, returns C<-1> with C<errno> set
appropriately.

A character range can be specified with a hyphen, so C<str_tr(str, "A-J",
"0-9")> does the same replacement as C<str_tr(str, "ACEGIBDFHJ",
"0246813579")>.

Note also that the whole range idea is rather unportable between character
sets - and even within character sets they might cause results you probably
didn't expect. A sound principle is to use only ranges that begin from and
end at either alphabets of equal case (a-e, A-E), or digits (0-4). Anything
else is unsafe. If in doubt, spell out the character sets in full.

Options:

    TR_COMPLEMENT Complement from.
    TR_DELETE     Delete found but unreplaced characters.
    TR_SQUASH     Squash duplicate replaced characters.

If C<TR_COMPLEMENT> is specified, C<from> is complemented. If C<TR_DELETE>
is specified, any characters specified by C<from> not found in C<to> are
deleted. (Note that this is slightly more flexible than the behavior of some
tr programs, which delete anything they find in C<from>.) If C<TR_SQUASH> is
specified, sequences of characters that were transliterated to the same
character are squashed down to a single instance of the character.

If C<TR_DELETE> is used, C<to> is always interpreted exactly as specified.
Otherwise, if C<to> is shorter than C<from>, the final character is
replicated till it is long enough. If C<to> is empty or C<null>, C<from> is
replicated. This latter is useful for counting characters in a class or for
squashing character sequences in a class.

Examples:

    str_tr(s, "A-Z", "a-z", 0);             // canonicalize to lower case
    str_tr(s, "a-z", "A-Z", 0);             // canonicalize to upper case
    str_tr(s, "a-zA-Z", "A-Za-z", 0);       // swap upper and lower case
    str_tr(s, "*", "*", 0);                 // count the stars in str
    str_tr(s, "0-9", "", 0);                // count the digits in $_
    str_tr(s, "a-zA-Z", "", TR_SQUASH);     // bookkeeper -> bokeper
    str_tr(s, "a-zA-Z", " ", TR_COMPLEMENT | TR_SQUASH); // change non-alphas to single space
    str_tr(c, "a-zA-Z", "n-za-mN-ZA-M", 0); // Rot13

    from = str_create("\200-\377");
    to = str_create("%c-\177", '\000');
    str_tr_str(s, from, to, 0);             // clear 8th bit

If multiple transliterations are given for a character, only the first one
is used:

    str_tr(str, "AAA", "XYZ", 0);

will transliterate any A to X.

=cut

*/

static StringTR *tr_compile_table(StringTR *table, const char *from, const char *to, int option);

int str_tr(String *str, const char *from, const char *to, int option)
{
	StringTR table[1];

	if (!str || !from)
		return set_errno(EINVAL);

	table->locker = NULL;

	if (!tr_compile_table(table, from, to, option))
		return -1;

	return str_tr_compiled(str, table);
}

/*

=item C<int str_tr_unlocked(String *str, const char *from, const char *to, int option)>

Equivalent to I<str_tr(3)> except that C<str> is not write-locked.

=cut

*/

int str_tr_unlocked(String *str, const char *from, const char *to, int option)
{
	StringTR table[1];

	if (!str || !from)
		return set_errno(EINVAL);

	table->locker = NULL;

	if (!tr_compile_table(table, from, to, option))
		return -1;

	return str_tr_compiled_unlocked(str, table);
}

/*

=item C<int str_tr_str(String *str, const String *from, const String *to, int option)>

Equivalent to I<str_tr(3)> except that C<from> and C<to> are I<String>
objects. This is needed when C<from> or C<to> need to contain C<nul>
characters.

=cut

*/

static StringTR *str_tr_compile_table(StringTR *table, const String *from, const String *to, int option);

int str_tr_str(String *str, const String *from, const String *to, int option)
{
	StringTR table[1];

	if (!str || !from)
		return set_errno(EINVAL);

	table->locker = NULL;

	if (!str_tr_compile_table(table, from, to, option))
		return -1;

	return str_tr_compiled(str, table);
}

/*

=item C<int str_tr_str_unlocked(String *str, const String *from, const String *to, int option)>

Equivalent to I<str_tr_str(3)> except that C<str> is not write-locked and
C<from> and C<to> are not read-locked. Note: If C<to> and C<from> need to be
read-locked, it is the caller's responsibility to lock and unlock them
explicitly with I<str_rdlock(3)> and I<str_unlock(3)>.

=cut

*/

static StringTR *str_tr_compile_table_unlocked(StringTR *table, const String *from, const String *to, int option);

int str_tr_str_unlocked(String *str, const String *from, const String *to, int option)
{
	StringTR table[1];

	if (!str || !from)
		return set_errno(EINVAL);

	table->locker = NULL;

	if (!str_tr_compile_table_unlocked(table, from, to, option))
		return -1;

	return str_tr_compiled_unlocked(str, table);
}

/*

=item C<int tr(char *str, const char *from, const char *to, int option)>

Equivalent to I<str_tr(3)> but works on an ordinary I<C> string.

=cut

*/

int tr(char *str, const char *from, const char *to, int option)
{
	StringTR table[1];

	if (!str || !from)
		return set_errno(EINVAL);

	table->locker = NULL;

	if (!tr_compile_table(table, from, to, option))
		return -1;

	return tr_compiled(str, table);
}

/*

=item C<StringTR *tr_compile(const char *from, const char *to, int option)>

Compiles C<from>, C<to> and C<option> into a translation table to be passed
to I<str_tr_compiled(3)> or I<tr_compiled(3)>. On success, returns the new
translation table. It is the caller's responsibility to deallocate the
translation table with I<tr_release(3)> or I<tr_destroy(3)>. It is strongly
recommended to use I<tr_destroy(3)>, because it also sets the pointer
variable to C<null>. On error, returns C<null> with C<errno> set
appropriately.

=cut

*/

StringTR *tr_compile(const char *from, const char *to, int option)
{
	return tr_compile_with_locker(NULL, from, to, option);
}

/*

=item C<StringTR *tr_compile_with_locker(Locker *locker, const char *from, const char *to, int option)>

Equivalent to I<tr_compile(3)> except that multiple threads accessing the new
translation table will be synchronised by C<locker>.

=cut

*/

StringTR *tr_compile_with_locker(Locker *locker, const char *from, const char *to, int option)
{
	StringTR *ret;

	if (!(ret = mem_new(StringTR))) /* XXX decouple */
		return NULL;

	ret->locker = locker;

	return tr_compile_table(ret, from, to, option);
}

/*

=item C<StringTR *str_tr_compile(const String *from, const String *to, int option)>

Equivalent to I<tr_compile(3)> except that C<from> and C<to> are I<String>
objects. This is needed when C<from> or C<to> need to contain C<nul>
characters.

=cut

*/

StringTR *str_tr_compile(const String *from, const String *to, int option)
{
	return str_tr_compile_with_locker(NULL, from, to, option);
}

/*

=item C<StringTR *str_tr_compile_unlocked(const String *from, const String *to, int option)>

Equivalent to I<str_tr_compile(3)> except that C<from> and C<to> are not
read-locked.

=cut

*/

StringTR *str_tr_compile_unlocked(const String *from, const String *to, int option)
{
	return str_tr_compile_with_locker_unlocked(NULL, from, to, option);
}

/*

=item C<StringTR *str_tr_compile_with_locker(Locker *locker, const String *from, const String *to, int option)>

Equivalent to I<str_tr_compile(3)> except that multiple threads accessing
the new translation table will be synchronised by C<locker>.

=cut

*/

StringTR *str_tr_compile_with_locker(Locker *locker, const String *from, const String *to, int option)
{
	StringTR *ret;

	if (!(ret = mem_new(StringTR))) /* XXX decouple */
		return NULL;

	ret->locker = locker;

	return str_tr_compile_table(ret, from, to, option);
}

/*

=item C<StringTR *str_tr_compile_with_locker_unlocked(Locker *locker, const String *from, const String *to, int option)>

Equivalent to I<str_tr_compile_with_locker(3)> except that C<from> and C<to>
are not read-locked. Note: If C<to> and C<from> need to be read-locked, it
is the caller's responsibility to lock and unlock them explicitly with
I<str_rdlock(3)> and I<str_unlock(3)>.

=cut

*/

StringTR *str_tr_compile_with_locker_unlocked(Locker *locker, const String *from, const String *to, int option)
{
	StringTR *ret;

	if (!(ret = mem_new(StringTR))) /* XXX decouple */
		return NULL;

	ret->locker = locker;

	return str_tr_compile_table_unlocked(ret, from, to, option);
}

/*

=item C<void tr_release(StringTR *table)>

Releases (deallocates) C<table>.

=cut

*/

void tr_release(StringTR *table)
{
	Locker *locker;

	if (!table)
		return;

	locker = table->locker;
	if (locker_wrlock(locker))
		return;

	mem_release(table);
	locker_unlock(locker);
}

/*

=item C<void *tr_destroy(StringTR **table)>

Destroys (deallocates and sets to C<null>) C<*table>. Returns C<null>.
B<Note:> translation tables shared by multiple threads must not be destroyed
until after all threads have finished with it.

=cut

*/

void *tr_destroy(StringTR **table)
{
	if (table && *table)
	{
		tr_release(*table);
		*table = NULL;
	}

	return NULL;
}

/*

C<static StringTR *do_tr_compile_table(StringTR *table, const char *from, ssize_t fromlen, const char *to, ssize_t tolen, int option)>

Compiles C<from>, C<to> and C<option> into the translation table, C<table>,
to be passed to I<str_tr_compiled(3)> or I<tr_compiled(3)>. If C<fromlen> is
C<-1>, then C<from> is interpreted as a C<nul>-terminated I<C> string.
Otherwise, C<from> is an arbitrary string of length C<fromlen>. If C<tolen>
is C<-1>, then C<to> is interpreted as a C<nul>-terminated I<C> string.
Otherwise, C<to> is an arbitrary string of length C<tolen>. On success,
returns C<table>. On error, returns C<null> with C<errno> set appropriately.

*/

static StringTR *do_tr_compile_table(StringTR *table, const char *from, ssize_t fromlen, const char *to, ssize_t tolen, int option)
{
	const char *f, *t;
	char *xf, *xt;
	char xfrom[CHARSET], xto[CHARSET];
	short tbl[CHARSET];
	int i, j, k;
	int err;

	if (!table || !from)
		return set_errnull(EINVAL);

	for (i = 0; i < CHARSET; ++i)
		tbl[i] = TRCODE_NOMAP;

	/* Parse the from string */

	for (xf = xfrom, f = from; ((fromlen == -1) ? *f : (f - from < fromlen)) && xf - xfrom < CHARSET; ++f)
	{
		i = j = *f;

		if (f[1] == '-' && f[2])
			j = f[2], f += 2;

		if ((unsigned char)j < (unsigned char)i)
			return set_errnull(EINVAL);

		for (k = (unsigned char)i; k <= (unsigned char)j; ++k)
			*xf++ = tbl[k] = k;
	}

	if (xf - xfrom == CHARSET)
		return set_errnull(EINVAL);

	if (option & TR_COMPLEMENT)
	{
		char tmp[CHARSET];

		for (xf = tmp, k = 0; k < CHARSET; ++k)
			if (tbl[k] == TRCODE_NOMAP)
				*xf++ = k;

		memcpy(xfrom, tmp, xf - tmp);
		xf = xfrom + (xf - tmp);
	}

	/* Parse the to string */

	if (!to || ((tolen == -1) ? *to == '\0' : tolen == 0))
		to = (option & TR_DELETE) ? "" : from;

	for (xt = xto, t = to; ((tolen == -1) ? *t : (t - to < tolen)) && xt - xto < CHARSET; ++t)
	{
		i = j = *t;

		if (t[1] == '-' && t[2])
			j = t[2], t += 2;

		if ((unsigned char)j < (unsigned char)i)
			return set_errnull(EINVAL);

		for (k = (unsigned char)i; k <= (unsigned char)j; ++k)
			*xt++ = k;
	}

	if (xt - xto == CHARSET)
		return set_errnull(EINVAL);

	if (!(option & TR_DELETE))
	{
		size_t flen = xf - xfrom;
		size_t tlen = xt - xto;

		if (tlen < flen)
		{
			memset(xt, xt[-1], flen - tlen);
			xt += flen - tlen;
		}
	}

	/* Build the translation table */

	if ((err = locker_wrlock(table->locker)))
		return set_errnull(err);

	table->squash = option & TR_SQUASH;
	for (i = 0; i < CHARSET; ++i)
		table->table[i] = TRCODE_NOMAP;

	for (i = j = 0; xfrom + i < xf; ++i, ++j)
	{
		unsigned char fc = xfrom[i];
		unsigned char tc = xto[j];
		if (table->table[fc] == TRCODE_NOMAP)
			table->table[fc] = (xto + j < xt) ? tc : TRCODE_DELETE;
	}

	if ((err = locker_unlock(table->locker)))
		return set_errnull(err);

	return table;
}

/*

C<StringTR *tr_compile_table(StringTR *table, const char *from, const char *to, int option)>

Equivalent to I<tr_compile(3)> except that C<from>, C<to> and C<option> are
compiled into the translation table pointed to by C<table>. On success,
returns C<table>. On error, returns C<null> with C<errno> set appropriately.

*/

static StringTR *tr_compile_table(StringTR *table, const char *from, const char *to, int option)
{
	return do_tr_compile_table(table, from, -1, to, -1, option);
}

/*

C<StringTR *str_tr_compile_table(StringTR *table, const String *from, const String *to, int option)>

Equivalent to I<tr_compile_table(3)> except that C<from> and C<to> are
I<String> objects. This is needed when C<from> or C<to> need to contain
C<nul> characters.

*/

static StringTR *str_tr_compile_table(StringTR *table, const String *from, const String *to, int option)
{
	StringTR *ret;
	int err;

	if ((err = str_rdlock(from)))
		return set_errnull(err);

	if ((err = str_rdlock(to)))
	{
		str_unlock(from);
		return set_errnull(err);
	}

	ret = str_tr_compile_table_unlocked(table, from, to, option);

	if ((err = str_unlock(from)))
	{
		str_unlock(to);
		return set_errnull(err);
	}

	if ((err = str_unlock(to)))
		return set_errnull(err);

	return ret;
}

/*

C<StringTR *str_tr_compile_table_unlocked(StringTR *table, const String *from, const String *to, int option)>

Equivalent to I<str_tr_compile_table(3)> except that C<from> and C<to> are
not read-locked.

*/

static StringTR *str_tr_compile_table_unlocked(StringTR *table, const String *from, const String *to, int option)
{
	return do_tr_compile_table(table, from->str, str_length_unlocked(from), to->str, str_length_unlocked(to), option);
}

/*

C<static int do_tr_compiled(unsigned char *str, size_t *length, StringTR *table)>

Performs the character translation specified by C<table> (as created by
I<tr_compile(3)> or equivalent) on C<str>. If C<length> is C<null>, C<str> is
interpreted as a C<nul>-terminated I<C> string. Otherwise, C<str> is
interpreted as an arbitrary string of length C<*length>. The integer that
C<length> points to is decremented by the number of bytes deleted by the
translation. On success, returns the number of characters replaced or
deleted. On error, returns C<-1> with C<errno> set appropriately.

*/

static int do_tr_compiled(unsigned char *str, size_t *length, StringTR *table)
{
	int ret = 0;
	int deleted = 0;
	unsigned char *r, *s;
	short t;
	int err;

	if (!str || !table)
		return set_errno(EINVAL);

	if ((err = locker_rdlock(table->locker)))
		return set_errno(err);

	for (r = s = str; (length) ? s - str < *length - 1 : *s; ++s)
	{
		switch (t = table->table[(int)*s])
		{
			case TRCODE_DELETE:
				++deleted;
				++ret;
				break;

			case TRCODE_NOMAP:
				if (!table->squash || r == str || r[-1] != *s)
					*r++ = *s;
				else
					++deleted;
				break;

			default:
				if (!table->squash || r == str || r[-1] != t)
					*r++ = t;
				else
					++deleted;
				++ret;
				break;
		}
	}

	if ((err = locker_unlock(table->locker)))
		return set_errno(err);

	*r = '\0';
	if (length && deleted)
		*length -= deleted;

	return ret;
}

/*

=item C<int str_tr_compiled(String *str, StringTR *table)>

Performs the character translation specified by C<table> (as created by
I<tr_compile(3)> or equivalent) on C<str>. Use this whenever the same
translation will be performed multiple times. On success, returns the number
of characters replaced or deleted. On error, returns C<-1> with C<errno> set
appropriately.

=cut

*/

int str_tr_compiled(String *str, StringTR *table)
{
	int ret;
	int err;

	if (!str || !table)
		return set_errno(EINVAL);

	if ((err = locker_rdlock(table->locker)))
		return set_errno(err);

	if ((err = str_wrlock(str)))
	{
		locker_unlock(table->locker);
		return set_errno(err);
	}

	ret = str_tr_compiled_unlocked(str, table);

	if ((err = str_unlock(str)))
	{
		locker_unlock(table->locker);
		return set_errno(err);
	}

	if ((err = locker_unlock(table->locker)))
		return set_errno(err);

	return ret;
}

/*

=item C<int str_tr_compiled_unlocked(String *str, StringTR *table)>

Equivalent to I<str_tr_compiled(3)> except that C<str> is not write-locked.

=cut

*/

int str_tr_compiled_unlocked(String *str, StringTR *table)
{
	if (!str || !table)
		return set_errno(EINVAL);

	return do_tr_compiled((unsigned char *)str->str, &str->length, table);
}

/*

=item C<int tr_compiled(char *str, StringTR *table)>

Equivalent to I<str_tr_compiled(3)> but works on an ordinary I<C> string.

=cut

*/

int tr_compiled(char *str, StringTR *table)
{
	int ret;
	int err;

	if (!str || !table)
		return set_errno(EINVAL);

	if ((err = locker_rdlock(table->locker)))
		return set_errno(err);

	ret = do_tr_compiled((unsigned char *)str, NULL, table);

	if ((err = locker_unlock(table->locker)))
		return set_errno(err);

	return ret;
}

#ifdef HAVE_REGEX_H

/*

=item C<List *str_regexpr(const char *pattern, const String *text, int cflags, int eflags)>

I<str_regexpr(3)> is an interface to I<POSIX 1003.2>-compliant regular
expression matching. C<pattern> is a regular expression. C<text> is the
string to be searched for matches. C<cflags> is passed to I<regcomp(3)>
along with C<REG_EXTENDED>. C<eflags> is passed to I<regexec(3)>. On
success, returns a I<List> of (at most 33) I<String>s containing the
matching substring followed by the matching substrings of any parenthesised
subexpressions. It is the caller's responsibility to deallocate the list
with I<list_release(3)> or I<list_destroy(3)>. On error (including no
match), returns C<null> with C<errno> set appropriately. Only use this
function when the regular expression will be used only once. Otherwise, use
I<regexpr_compile(3)> or I<regcomp(3)> and I<str_regexpr_compiled(3)> or
I<regexpr_compiled(3)> or I<regexec(3)>.

Note: If you require perl pattern matching, you could use Philip Hazel's
I<PCRE> package, C<ftp://ftp.cus.cam.ac.uk/pub/software/programs/pcre/> or
link against the perl library itself.

=cut

*/

List *str_regexpr(const char *pattern, const String *text, int cflags, int eflags)
{
	return str_regexpr_with_locker(NULL, pattern, text, cflags, eflags);
}

/*

=item C<List *str_regexpr_unlocked(const char *pattern, const String *text, int cflags, int eflags)>

Equivalent to I<str_regexpr(3)> except that C<text> is not read-locked.

=cut

*/

List *str_regexpr_unlocked(const char *pattern, const String *text, int cflags, int eflags)
{
	return str_regexpr_with_locker_unlocked(NULL, pattern, text, cflags, eflags);
}

/*

=item C<List *str_regexpr_with_locker(Locker *locker, const char *pattern, const String *text, int cflags, int eflags)>

Equivalent to I<str_regexpr(3)> except that multiple threads accessing the
new list will be synchronised by C<locker>.

=cut

*/

List *str_regexpr_with_locker(Locker *locker, const char *pattern, const String *text, int cflags, int eflags)
{
	List *ret;
	int err;

	if (!pattern || !text)
		return set_errnull(EINVAL);

	if ((err = str_rdlock(text)))
		return set_errnull(err);

	ret = str_regexpr_with_locker_unlocked(locker, pattern, text, cflags, eflags);

	if ((err = str_unlock(text)))
	{
		list_release(ret);
		return set_errnull(err);
	}

	return ret;
}

/*

=item C<List *str_regexpr_with_locker_unlocked(Locker *locker, const char *pattern, const String *text, int cflags, int eflags)>

Equivalent to I<str_regexpr_with_locker(3)> except that C<text> is not
read-locked.

=cut

*/

List *str_regexpr_with_locker_unlocked(Locker *locker, const char *pattern, const String *text, int cflags, int eflags)
{
	if (!pattern || !text)
		return set_errnull(EINVAL);

	return regexpr_with_locker(locker, pattern, text->str, cflags, eflags);
}

/*

=item C<List *regexpr(const char *pattern, const char *text, int cflags, int eflags)>

Equivalent to I<str_regexpr(3)> but works on an ordinary I<C> string.

=cut

*/

List *regexpr(const char *pattern, const char *text, int cflags, int eflags)
{
	return regexpr_with_locker(NULL, pattern, text, cflags, eflags);
}

/*

=item C<List *regexpr_with_locker(Locker *locker, const char *pattern, const char *text, int cflags, int eflags)>

Equivalent to I<regexpr(3)> except that multiple threads accessing the new
list will be synchronised by C<locker>.

=cut

*/

List *regexpr_with_locker(Locker *locker, const char *pattern, const char *text, int cflags, int eflags)
{
	regex_t compiled[1];
	List *ret;
	int err;

	if (!pattern || !text)
		return set_errnull(EINVAL);

	if ((err = regexpr_compile(compiled, pattern, cflags)))
		return set_errnull(err);

	ret = regexpr_compiled_with_locker(locker, compiled, text, eflags);
	regfree(compiled);

	return ret;
}

/*

=item C<int regexpr_compile(regex_t *compiled, const char *pattern, int cflags)>

Compiles a I<POSIX 1003.2>-compliant regular expression. C<compiled> is the
location in which to compile the expression. C<pattern> is the regular
expression. C<cflags> is passed to I<regcomp(3)> along with C<REG_EXTENDED>.
Call this, followed by I<re_compiled(3)> when the regular expression will be
used multiple times. On success, returns C<0>. On error, returns an error
code.

=cut

*/

int regexpr_compile(regex_t *compiled, const char *pattern, int cflags)
{
	if (!compiled || !pattern)
		return REG_BADPAT;

	return regcomp(compiled, pattern, cflags | REG_EXTENDED);
}

/*

=item C<void regexpr_release(regex_t *compiled)>

Just another name for I<regfree(3)>.

=cut

*/

void regexpr_release(regex_t *compiled)
{
	if (compiled)
		regfree(compiled);
}

/*

=item C<List *str_regexpr_compiled(const regex_t *compiled, const String *text, int eflags)>

I<regexpr_compiled(3)> is an interface to the I<POSIX 1003.2> regular
expression function, I<regexec(3)>. C<compiled> is the compiled regular
expression prepared by I<regexpr_compile(3)> or I<regcomp(3)>. C<text> is
the string to be searched for a match. C<eflags> is passed to I<regexec(3)>.
On success, returns a I<List> of (at most 33) I<String>s containing the
matching substring followed by the matching substrings of any parenthesised
subexpressions. It is the caller's responsibility to deallocate the list
with I<list_release(3)> or I<list_destroy(3)>. On error (including no
match), returns C<null> with C<errno> set appropriately.

=cut

*/

List *str_regexpr_compiled(const regex_t *compiled, const String *text, int eflags)
{
	return str_regexpr_compiled_with_locker(NULL, compiled, text, eflags);
}

/*

=item C<List *str_regexpr_compiled_unlocked(const regex_t *compiled, const String *text, int eflags)>

Equivalent to I<str_regexpr_compiled(3)> except that C<text> is not
write-locked.

=cut

*/

List *str_regexpr_compiled_unlocked(const regex_t *compiled, const String *text, int eflags)
{
	return str_regexpr_compiled_with_locker_unlocked(NULL, compiled, text, eflags);
}

/*

=item C<List *str_regexpr_compiled_with_locker(Locker *locker, const regex_t *compiled, const String *text, int eflags)>

Equivalent to I<str_regexpr_compiled(3)> except that multiple threads
accessing the new list will be synchronised by C<locker>.

=cut

*/

List *str_regexpr_compiled_with_locker(Locker *locker, const regex_t *compiled, const String *text, int eflags)
{
	List *ret;
	int err;

	if (!compiled || !text)
		return set_errnull(EINVAL);

	if ((err = str_rdlock(text)))
		return set_errnull(err);

	ret = str_regexpr_compiled_with_locker_unlocked(locker, compiled, text, eflags);

	if ((err = str_unlock(text)))
	{
		list_release(ret);
		return set_errnull(err);
	}

	return ret;
}

/*

=item C<List *str_regexpr_compiled_with_locker_unlocked(Locker *locker, const regex_t *compiled, const String *text, int eflags)>

Equivalent to I<str_regexpr_compiled_with_locker(3)> except that C<text> is
not read-locked.

=cut

*/

List *str_regexpr_compiled_with_locker_unlocked(Locker *locker, const regex_t *compiled, const String *text, int eflags)
{
	if (!compiled || !text)
		return set_errnull(EINVAL);

	return regexpr_compiled_with_locker(locker, compiled, text->str, eflags);
}

/*

=item C<List *regexpr_compiled(const regex_t *compiled, const char *text, int eflags)>

Equivalent to I<str_regexpr_compiled(3)> but works on an ordinary I<C> string.

=cut

*/

List *regexpr_compiled(const regex_t *compiled, const char *text, int eflags)
{
	return regexpr_compiled_with_locker(NULL, compiled, text, eflags);
}

/*

=item C<List *regexpr_compiled_with_locker(Locker *locker, const regex_t *compiled, const char *text, int eflags)>

Equivalent to I<regexpr_compiled(3)> except that multiple threads accessing
the new list will be synchronised by C<locker>.

=cut

*/

List *regexpr_compiled_with_locker(Locker *locker, const regex_t *compiled, const char *text, int eflags)
{
	regmatch_t match[33];
	List *ret;
	int i;
	int err;

	if (!compiled || !text)
		return set_errnull(EINVAL);

	if ((err = regexec(compiled, text, 33, match, eflags)))
		return set_errnull(err);

	if (!(ret = list_create_with_locker(locker, (list_release_t *)str_release)))
		return NULL;

	for (i = 0; i < 33 && match[i].rm_so != -1; ++i)
	{
		String *m = substr(text, (ssize_t)match[i].rm_so, (ssize_t)(match[i].rm_eo - match[i].rm_so));

		if (!m)
		{
			list_release(ret);
			return NULL;
		}

		if (!list_append(ret, m))
		{
			str_release(m);
			list_release(ret);
			return NULL;
		}
	}

	return ret;
}

/*

=item C<String *str_regsub(const char *pattern, const char *replacement, String *text, int cflags, int eflags, int all)>

I<str_regsub(3)> is an interface to I<POSIX 1003.2>-compliant regular
expression matching and substitution. C<pattern> is a regular expression.
C<text> is the string to be searched for matches. C<cflags> is passed to
I<regcomp(3)> along with C<REG_EXTENDED>. C<eflags> is passed to
I<regexec(3)>. C<all> specifies whether to substitute the first match (if
zero) or all matches (if non-zero). C<replacement> specifies the string that
replaces each match. If C<replacement> contains C<"$#"> or C<"${##}"> (where
C<"#"> is a decimal digit), the substring that matches the corresponding
subexpression is interpolated in its place. Up to 32 subexpressions are
supported. If C<replacement> contains C<"$$">, then C<"$"> is interpolated
in its place. The following I<perl(1)> quote escape sequences are also
understood:

    \l  lowercase next character
    \u  uppercase next character
    \L  lowercase until matching \E
    \U  uppercase until matching \E
    \Q  backslash non-alphanumeric characters until matching \E
    \E  end case/quotemeta modification

Note that these sequences don't behave exactly like in I<perl(1)>. Namely,
an C<\l> appearing between a C<\U> and an C<\E> does lowercase the next
character and a C<\E> sequence without a matching C<\L>, C<\U> or C<\Q> is
an error. Also note that only 32 levels of nesting are supported.

On success, returns C<text>. On error (including no match), returns C<null>
with C<errno> set appropriately. Only use this function when the regular
expression will be used only once. Otherwise, use I<regexpr_compile(3)> or
I<regcomp(3)> and I<str_regsub_compiled(3)>.

=cut

*/

String *str_regsub(const char *pattern, const char *replacement, String *text, int cflags, int eflags, int all)
{
	regex_t compiled[1];
	String *ret;
	int err;

	if (!pattern || !replacement || !text)
		return set_errnull(EINVAL);

	if ((err = regexpr_compile(compiled, pattern, cflags)))
		return set_errnull(err);

	ret = str_regsub_compiled(compiled, replacement, text, eflags, all);
	regfree(compiled);

	return ret;
}

/*

=item C<String *str_regsub_unlocked(const char *pattern, const char *replacement, String *text, int cflags, int eflags, int all)>

Equivalent to I<str_regsub(3)> except that C<text> is not write-locked.

=cut

*/

String *str_regsub_unlocked(const char *pattern, const char *replacement, String *text, int cflags, int eflags, int all)
{
	regex_t compiled[1];
	String *ret;
	int err;

	if (!pattern || !replacement || !text)
		return set_errnull(EINVAL);

	if ((err = regexpr_compile(compiled, pattern, cflags)))
		return set_errnull(err);

	ret = str_regsub_compiled_unlocked(compiled, replacement, text, eflags, all);
	regfree(compiled);

	return ret;
}

/*

=item C<String *str_regsub_compiled(const regex_t *compiled, const char *replacement, String *text, int eflags, int all)>

Equivalent to I<str_regsub(3)> but works on an already compiled I<regex_t>,
C<compiled>.

=cut

*/

String *str_regsub_compiled(const regex_t *compiled, const char *replacement, String *text, int eflags, int all)
{
	String *ret;
	int err;

	if (!compiled || !replacement || !text)
		return set_errnull(EINVAL);

	if ((err = str_wrlock(text)))
		return set_errnull(err);

	ret = str_regsub_compiled_unlocked(compiled, replacement, text, eflags, all);

	if ((err = str_unlock(text)))
		return set_errnull(err);

	return ret;
}

/*

=item C<String *str_regsub_compiled_unlocked(const regex_t *compiled, const char *replacement, String *text, int eflags, int all)>

Equivalent to I<str_regsub_compiled(3)> except that C<text> is not
write-locked.

=cut

*/

String *str_regsub_compiled_unlocked(const regex_t *compiled, const char *replacement, String *text, int eflags, int all)
{
#define MAX_MATCHES 33
#define MAX_STATES 33
	enum
	{
		RS_LC = 1,
		RS_UC = 2,
		RS_QM = 4,
		RS_FIRST = 8,
		RS_LCFIRST = RS_LC | RS_FIRST,
		RS_UCFIRST = RS_UC | RS_FIRST
	};

	regmatch_t match[MAX_MATCHES];
	String *rep;
	int matches;
	size_t start;
	int states[MAX_STATES];
	int i, s;

	if (!compiled || !replacement || !text)
		return set_errnull(EINVAL);

	for (start = 0, matches = 0; start <= text->length - 1; ++matches)
	{
		if (regexec(compiled, text->str + start, MAX_MATCHES, match, eflags))
			return (matches) ? text : NULL;

		/*
		** Interpolate any $$, $# and ${##} in replacement
		** with subexpression matching substrings
		*/

		if (!(rep = str_create("%s", replacement)))
			return NULL;

		for (i = 0; i < rep->length - 1; ++i)
		{
			if (rep->str[i] == '$')
			{
				if (rep->str[i + 1] == '$')
				{
					if (!str_remove(rep, i))
					{
						str_release(rep);
						return NULL;
					}
				}
				else
				{
					int ref;
					int j = i + 1;

					if (rep->str[j] == '{')
					{
						for (++j, ref = 0; is_digit(rep->str[j]); ++j)
							ref *= 10, ref += rep->str[j] - '0';

						if (rep->str[j] != '}')
						{
							str_release(rep);
							return set_errnull(EINVAL);
						}
					}
					else if (is_digit(rep->str[i + 1]))
					{
						ref = rep->str[j] - '0';
					}
					else
					{
						str_release(rep);
						return set_errnull(EINVAL);
					}

					if (ref < 0 || ref >= MAX_MATCHES || match[ref].rm_so == -1)
					{
						str_release(rep);
						return set_errnull(EINVAL);
					}

					if (!str_replace(rep, i, j + 1 - i, "%.*s", (int)(match[ref].rm_eo - match[ref].rm_so), text->str + match[ref].rm_so))
					{
						str_release(rep);
						return NULL;
					}

					i += match[ref].rm_eo - match[ref].rm_so - 1;
				}
			}
		}

		/* Perform \l \L \u \U \Q \E transformations on replacement */

#define FAIL { str_release(rep); return set_errnull(EINVAL); }
#define PUSH_STATE(state) { if (s >= MAX_STATES - 1) FAIL states[s + 1] = states[s] | (state); ++s; }
#define POP_STATE { if (s == 0) FAIL --s; }
#define REMOVE_CODE { if (!str_remove_range(rep, i, 2)) FAIL --i; }
#define NEG(t) states[s] &= ~(RS_##t##C);

		for (states[s = 0] = 0, i = 0; i < rep->length - 1; ++i)
		{
			if (rep->str[i] == '\\')
			{
				switch (rep->str[i + 1])
				{
					case 'l': { PUSH_STATE(RS_LCFIRST) NEG(U) REMOVE_CODE break; }
					case 'L': { PUSH_STATE(RS_LC) NEG(U) REMOVE_CODE break; }
					case 'u': { PUSH_STATE(RS_UCFIRST) NEG(L) REMOVE_CODE break; }
					case 'U': { PUSH_STATE(RS_UC) NEG(L) REMOVE_CODE break; }
					case 'Q': { PUSH_STATE(RS_QM) REMOVE_CODE break; }
					case 'E': { POP_STATE REMOVE_CODE break; }
					case '\\': { if (!str_remove(rep, i)) FAIL break; }
				}
			}
			else
			{
				if (states[s] & RS_LC)
					rep->str[i] = to_lower(rep->str[i]);

				if (states[s] & RS_UC)
					rep->str[i] = to_upper(rep->str[i]);

				if (states[s] & RS_QM && !is_alnum(rep->str[i]))
					if (!str_insert(rep, i++, "\\"))
						FAIL

				if (states[s] & RS_FIRST)
					POP_STATE
			}
		}

		/* Replace matching substring in text with rep */

		if (!str_replace_str_unlocked(text, start + match[0].rm_so, (ssize_t)(match[0].rm_eo - match[0].rm_so), rep))
		{
			str_release(rep);
			return NULL;
		}

		/* Zero length match (at every position), move on or get stuck */

		if (match[0].rm_so == 0 && match[0].rm_eo == 0)
		{
			++match[0].rm_so;
			++match[0].rm_eo;
		}

		start += match[0].rm_so + rep->length - 1;
		str_release(rep);

		if (!all)
			break;
	}

	return text;
}

#endif

/*

=item C<List *str_fmt(const String *str, size_t line_width, StringAlignment alignment)>

Formats C<str> into a I<List> of I<String> objects with length no greater
than C<line_width> (unless there are individual words longer than
C<line_width>) with the alignment specified by C<alignment>:

=over 4

=item C<ALIGN_LEFT> (C<'<'>)

The lines will be left justified (with one space between words).

=item C<ALIGN_RIGHT> ('>')

The lines will be right justified (with one space between words).

=item C<ALIGN_CENTRE> or C<ALIGN_CENTER> (C<'|'>)

C<str> will be split into lines at each newline character (C<'\n'>). The
lines will then be centred (with one space between words) padded with spaces
to the left.

=item C<ALIGN_FULL> (C<'='>)

The lines will be fully justified (possibly with multiple spaces between
words).

=back

On success, returns a new I<List> of I<String> objects. It is the caller's
responsibility to deallocate the list with I<list_release(3)> or
I<list_destroy(3)>. On error, returns C<null> with C<errno> set
appropriately. Note that C<str> is interpreted as a C<nul>-terminated
string.

B<Note:> I<str_fmt(3)> provides straightforward formatting completely
lacking in any aesthetic sensibilities. If you need awesome paragraph
formatting, pipe text through I<par(1)> instead (available from
C<http://www.cs.berkeley.edu/~amc/Par/>).

=cut

*/

List *str_fmt(const String *str, size_t line_width, StringAlignment alignment)
{
	return str_fmt_with_locker(NULL, str, line_width, alignment);
}

/*

=item C<List *str_fmt_unlocked(const String *str, size_t line_width, StringAlignment alignment)>

Equivalent to I<str_fmt(3)> except that C<str> is not read-locked.

=cut

*/

List *str_fmt_unlocked(const String *str, size_t line_width, StringAlignment alignment)
{
	return str_fmt_with_locker_unlocked(NULL, str, line_width, alignment);
}

/*

=item C<List *str_fmt_with_locker(Locker *locker, const String *str, size_t line_width, StringAlignment alignment)>

Equivalent to I<str_fmt(3)> except that multiple threads accessing the new
list will be synchronised by C<locker>.

=cut

*/

List *str_fmt_with_locker(Locker *locker, const String *str, size_t line_width, StringAlignment alignment)
{
	List *ret;
	int err;

	if (!str)
		return set_errnull(EINVAL);

	if ((err = str_rdlock(str)))
		return set_errnull(err);

	ret = str_fmt_with_locker_unlocked(locker, str, line_width, alignment);

	if ((err = str_unlock(str)))
	{
		list_release(ret);
		return set_errnull(err);
	}

	return ret;
}

/*

=item C<List *str_fmt_with_locker_unlocked(Locker *locker, const String *str, size_t line_width, StringAlignment alignment)>

Equivalent to I<str_fmt_with_locker(3)> except that C<str> is not
read-locked.

=cut

*/

List *str_fmt_with_locker_unlocked(Locker *locker, const String *str, size_t line_width, StringAlignment alignment)
{
	if (!str)
		return set_errnull(EINVAL);

	return fmt_with_locker(locker, str->str, line_width, alignment);
}

/*

=item C<List *fmt(const char *str, size_t line_width, StringAlignment alignment)>

Equivalent to I<str_fmt(3)> but works on an ordinary I<C> string.

=cut

*/

List *fmt(const char *str, size_t line_width, StringAlignment alignment)
{
	return fmt_with_locker(NULL, str, line_width, alignment);
}

/*

=item C<List *fmt_with_locker(Locker *locker, const char *str, size_t line_width, StringAlignment alignment)>

Equivalent to I<fmt(3)> except that multiple threads accessing the new list
will be synchronised by C<locker>.

=cut

*/

List *fmt_with_locker(Locker *locker, const char *str, size_t line_width, StringAlignment alignment)
{
	List *para;
	String *line = NULL;
	const char *s, *r;
	ssize_t len;

	if (!str)
		return set_errnull(EINVAL);

	switch (alignment)
	{
		case ALIGN_LEFT:
		case ALIGN_RIGHT:
		case ALIGN_FULL:
		{
			if (!(para = list_create_with_locker(locker, (list_release_t *)str_release)))
				return NULL;

			for (s = str; *s; ++s)
			{
				while (is_space(*s))
					++s;

				for (r = s; *r && !is_space(*r); ++r)
				{}

				if (r > s)
				{
					if ((len = str_length(line)) == -1)
						++len;

					if (len + (len != 0) + (r - s) > line_width)
					{
						if (len && !list_append(para, line))
						{
							str_release(line);
							list_release(para);
							return NULL;
						}

						line = NULL;
					}

					if (!line)
					{
						if (!(line = str_create_sized(line_width, "%.*s", r - s, s)))
						{
							list_release(para);
							return NULL;
						}
					}
					else if (!str_append(line, " %.*s", r - s, s))
					{
						str_release(line);
						list_release(para);
						return NULL;
					}

					s = r;

					if (!*s)
						--s;
				}
			}

			if (str_length(line) > 0 && !list_append(para, line))
			{
				str_release(line);
				list_release(para);
				return NULL;
			}

			if (alignment == ALIGN_RIGHT)
			{
				while (list_has_next(para) == 1)
				{
					line = (String *)list_next(para);
					len = str_length(line);

					if (len >= line_width)
						continue;

					if (!str_prepend(line, "%*s", line_width - len, ""))
					{
						list_release(para);
						return NULL;
					}
				}
			}
			else if (alignment == ALIGN_FULL)
			{
				ssize_t lines;
				int i;

				if ((lines = list_length(para)) == -1)
				{
					list_release(para);
					return NULL;
				}

				for (i = 0; i < lines - 1; ++i)
				{
					size_t extra;
					size_t gaps;

					line = (String *)list_item(para, i);
					len = str_length(line);

					if (len >= line_width)
						continue;

					extra = line_width - len;
					gaps = 0;

					for (s = line->str; *s; ++s)
						if (*s == ' ')
							++gaps;

					for (s = line->str; gaps && *s; ++s)
					{
						if (*s == ' ')
						{
							int gap = extra / gaps;

							if (!str_insert(line, s - line->str, "%*s", gap, ""))
							{
								list_release(para);
								return NULL;
							}

							extra -= gap;
							--gaps;
							s += gap;
						}
					}
				}
			}

			break;
		}

		case ALIGN_CENTRE:
		{
			if (!(para = split_with_locker(locker, str, "\n")))
				return NULL;

			while (list_has_next(para) == 1)
			{
				size_t extra;
				line = (String *)list_next(para);
				str_squeeze(line);
				len = str_length(line);

				if (len >= line_width)
					continue;

				extra = (line_width - len) / 2;

				if (extra && !str_prepend(line, "%*s", extra, ""))
				{
					list_release(para);
					return NULL;
				}
			}

			break;
		}

		default:
		{
			return set_errnull(EINVAL);
		}
	}

	return para;
}

/*

C<List *do_split_with_locker(Locker *locker, const char *str, ssize_t length, const char *delim)>

Splits C<str> into tokens separated by sequences of characters occurring in
C<delim>. If C<length> is C<-1>, C<str> is interpreted as a
C<nul>-terminated I<C> string. Otherwise, C<str> is interpreted as an
arbitrary string of length C<length>. On success, returns a new I<List> of
I<String> objects. It is the caller's responsibility to deallocate the list
with I<list_release(3)> or I<list_destroy(3)>. If C<locker> is non-C<null>,
multiple threads accessing the new list will be synchronised by C<locker>.
On error, returns C<null> with C<errno> set appropriately.

*/

static List *do_split_with_locker(Locker *locker, const char *str, ssize_t length, const char *delim)
{
	List *ret;
	const char *s, *r;

	if (!str || !delim)
		return set_errnull(EINVAL);

	if (!(ret = list_create_with_locker(locker, (list_release_t *)str_release)))
		return NULL;

	for (s = str; (length == -1) ? *s : s - str < length; ++s)
	{
		while ((length == -1) ? (*s && strchr(delim, *s)) : (s - str < length && (*s && strchr(delim, *s))))
			++s;

		if (!*delim)
			r = s + 1;
		else
			for (r = s; (length == -1) ? (*r && !strchr(delim, *r)) : (r - str < length && (!*r || !strchr(delim, *r))); ++r)
			{}

		if (r > s)
		{
			String *token = substr(s, 0, r - s);
			if (!token)
			{
				list_release(ret);
				return NULL;
			}

			if (!list_append(ret, token))
			{
				str_release(token);
				list_release(ret);
				return NULL;
			}

			s = r;
			if (!*delim)
				--s;
		}

		if ((length == -1) ? !*s : (s - str == length))
			break;
	}

	return ret;
}

/*

=item C<List *str_split(const String *str, const char *delim)>

Splits C<str> into tokens separated by sequences of characters occurring in
C<delim>. On success, returns a new I<List> of I<String> objects. It is the
caller's responsibility to deallocate the list with I<list_release(3)> or
I<list_destroy(3)>. On error, returns C<null> with C<errno> set
appropriately.

=cut

*/

List *str_split(const String *str, const char *delim)
{
	return str_split_with_locker(NULL, str, delim);
}

/*

=item C<List *str_split_unlocked(const String *str, const char *delim)>

Equivalent to I<str_split(3)> except that C<str> is not read-locked.

=cut

*/

List *str_split_unlocked(const String *str, const char *delim)
{
	return str_split_with_locker_unlocked(NULL, str, delim);
}

/*

=item C<List *str_split_with_locker(Locker *locker, const String *str, const char *delim)>

Equivalent to I<str_split(3)> except that multiple threads accessing the new
list will be synchronised by C<locker>.

=cut

*/

List *str_split_with_locker(Locker *locker, const String *str, const char *delim)
{
	List *ret;
	int err;

	if (!str || !delim)
		return set_errnull(EINVAL);

	if ((err = str_rdlock(str)))
		return set_errnull(err);

	ret = str_split_with_locker_unlocked(locker, str, delim);

	if ((err = str_unlock(str)))
	{
		list_release(ret);
		return set_errnull(err);
	}

	return ret;
}

/*

=item C<List *str_split_with_locker_unlocked(Locker *locker, const String *str, const char *delim)>

Equivalent to I<str_split_with_locker(3)> except that C<str> is not
read-locked.

=cut

*/

List *str_split_with_locker_unlocked(Locker *locker, const String *str, const char *delim)
{
	if (!str || !delim)
		return set_errnull(EINVAL);

	return do_split_with_locker(locker, str->str, str->length - 1, delim);
}

/*

=item C<List *split(const char *str, const char *delim)>

Equivalent to I<str_split(3)> but works on an ordinary I<C> string.

=cut

*/

List *split(const char *str, const char *delim)
{
	return split_with_locker(NULL, str, delim);
}

/*

=item C<List *split_with_locker(Locker *locker, const char *str, const char *delim)>

Equivalent to I<split(3)> except that multiple threads accessing the new
list will be synchronised by C<locker>.

=cut

*/

List *split_with_locker(Locker *locker, const char *str, const char *delim)
{
	if (!str || !delim)
		return set_errnull(EINVAL);

	return do_split_with_locker(locker, str, -1, delim);
}

#ifdef HAVE_REGEX_H

/*

=item C<List *str_regexpr_split(const String *str, const char *delim, int cflags, int eflags)>

Splits C<str> into tokens separated by occurrences of the regular
expression, C<delim>. C<str> is interpreted as a C<nul>-terminated I<C>
string. C<cflags> is passed to I<regcomp(3)> along with C<REG_EXTENDED> and
C<eflags> is passed to I<regexec(3)>. On success, returns a new I<List> of
I<String> objects. It is the caller's responsibility to deallocate the list
with I<list_release(3)> or I<list_destroy(3)>. On error, returns C<null>
with C<errno> set appropriately.

=cut

*/

List *str_regexpr_split(const String *str, const char *delim, int cflags, int eflags)
{
	return str_regexpr_split_with_locker(NULL, str, delim, cflags, eflags);
}

/*

=item C<List *str_regexpr_split_unlocked(const String *str, const char *delim, int cflags, int eflags)>

Equivalent to I<str_regexpr_split(3)> except that C<str> is not read-locked.

=cut

*/

List *str_regexpr_split_unlocked(const String *str, const char *delim, int cflags, int eflags)
{
	return str_regexpr_split_with_locker_unlocked(NULL, str, delim, cflags, eflags);
}

/*

=item C<List *str_regexpr_split_with_locker(Locker *locker, const String *str, const char *delim, int cflags, int eflags)>

Equivalent to I<str_regexpr_split(3)> except that multiple threads accessing
the new list will be synchronised by C<locker>.

=cut

*/

List *str_regexpr_split_with_locker(Locker *locker, const String *str, const char *delim, int cflags, int eflags)
{
	List *ret;
	int err;

	if (!str || !delim)
		return set_errnull(EINVAL);

	if ((err = str_rdlock(str)))
		return set_errnull(err);

	ret = str_regexpr_split_with_locker_unlocked(locker, str, delim, cflags, eflags);

	if ((err = str_unlock(str)))
	{
		list_release(ret);
		return set_errnull(err);
	}

	return ret;
}

/*

=item C<List *str_regexpr_split_with_locker_unlocked(Locker *locker, const String *str, const char *delim, int cflags, int eflags)>

Equivalent to I<str_regexpr_split_with_locker(3)> except that C<str> is not
read-locked.

=cut

*/

List *str_regexpr_split_with_locker_unlocked(Locker *locker, const String *str, const char *delim, int cflags, int eflags)
{
	if (!str || !delim)
		return set_errnull(EINVAL);

	return regexpr_split_with_locker(locker, str->str, delim, cflags, eflags);
}

/*

=item C<List *regexpr_split(const char *str, const char *delim, int cflags, int eflags)>

Equivalent to I<str_regexpr_split(3)> but works on an ordinary I<C> string.

=cut

*/

List *regexpr_split(const char *str, const char *delim, int cflags, int eflags)
{
	return regexpr_split_with_locker(NULL, str, delim, cflags, eflags);
}

/*

=item C<List *regexpr_split_with_locker(Locker *locker, const char *str, const char *delim, int cflags, int eflags)>

Equivalent to I<regexpr_split(3)> except that multiple threads accessing the
new list will be synchronised by C<locker>.

=cut

*/

List *regexpr_split_with_locker(Locker *locker, const char *str, const char *delim, int cflags, int eflags)
{
	List *ret;
	String *token;
	regex_t compiled[1];
	regmatch_t match[1];
	int start, matches;
	int err;

	if (!str || !delim)
		return set_errnull(EINVAL);

	if ((err = regexpr_compile(compiled, delim, cflags)))
		return set_errnull(err);

	if (!(ret = list_create_with_locker(locker, (list_release_t *)str_release)))
		return NULL;

	for (start = 0, matches = 0; str[start]; ++matches)
	{
		if (regexec(compiled, str + start, 1, match, eflags))
			break;

		/* Zero length match (at every position), make a token of each character */

		if (match[0].rm_so == 0 && match[0].rm_eo == 0)
		{
			++match[0].rm_so;
			++match[0].rm_eo;
		}

		/* Make a token of any text before the match */

		if (match[0].rm_so)
		{
			if (!(token = substr(str, start, (ssize_t)match[0].rm_so)))
			{
				list_release(ret);
				return NULL;
			}

			if (!list_append(ret, token))
			{
				str_release(token);
				list_release(ret);
				return NULL;
			}
		}

		start += match[0].rm_eo;
	}

	/* Make a token of any text after the last match */

	if (str[start])
	{
		if (!(token = str_create("%s", str + start)))
		{
			list_release(ret);
			return NULL;
		}

		if (!list_append(ret, token))
		{
			str_release(token);
			list_release(ret);
			return NULL;
		}
	}

	return ret;
}

#endif

/*

=item C<String *str_join(const List *list, const char *delim)>

Joins the I<String> objects in C<list> with C<delim> inserted between each
one. On success, returns the resulting I<String>. It is the caller's
responsibility to deallocate the string with I<str_release(3)> or
I<str_destroy(3)>. On error, returns C<null> with C<errno> set
appropriately.

=cut

*/

String *str_join(const List *list, const char *delim)
{
	return str_join_with_locker(NULL, list, delim);
}

/*

=item C<String *str_join_unlocked(const List *list, const char *delim)>

Equivalent to I<str_join(3)> except that C<list> is not read-locked.

=cut

*/

String *str_join_unlocked(const List *list, const char *delim)
{
	return str_join_with_locker_unlocked(NULL, list, delim);
}

/*

=item C<String *str_join_with_locker(Locker *locker, const List *list, const char *delim)>

Equivalent to I<str_join(3)> except that multiple threads accessing the new
string will be synchronised by C<locker>.

=cut

*/

String *str_join_with_locker(Locker *locker, const List *list, const char *delim)
{
	String *ret;
	int err;

	if (!list)
		return set_errnull(EINVAL);

	if ((err = list_rdlock(list)))
		return set_errnull(err);

	ret = str_join_with_locker_unlocked(locker, list, delim);

	if ((err = list_unlock(list)))
		return set_errnull(err);

	return ret;
}

/*

=item C<String *str_join_with_locker_unlocked(Locker *locker, const List *list, const char *delim)>

Equivalent to I<str_join_with_locker(3)> except that C<list> is not
read-locked.

=cut

*/

String *str_join_with_locker_unlocked(Locker *locker, const List *list, const char *delim)
{
	String *ret;
	String *del;
	Lister *lister;
	int i;

	if (!list)
		return set_errnull(EINVAL);

	if (!(ret = str_create_with_locker(locker, NULL)))
		return NULL;

	if (!(del = str_create(delim ? "%s" : NULL, delim)))
	{
		str_release(ret);
		return NULL;
	}

	if (!(lister = lister_create_unlocked(list)))
	{
		str_release(ret);
		str_release(del);
		return NULL;
	}

	for (i = 0; lister_has_next(lister) == 1; ++i)
	{
		String *s = (String *)lister_next(lister);

		if (i && !str_append_str(ret, del))
		{
			str_release(ret);
			str_release(del);
			lister_release_unlocked(lister);
			return NULL;
		}

		if (s && !str_append_str(ret, s))
		{
			str_release(ret);
			str_release(del);
			lister_release_unlocked(lister);
			return NULL;
		}
	}

	str_release(del);
	lister_release_unlocked(lister);

	return ret;
}

/*

=item C<String *join(const List *list, const char *delim)>

Equivalent to I<str_join(3)> but works on a list of ordinary I<C> strings.

=cut

*/

String *join(const List *list, const char *delim)
{
	return join_with_locker(NULL, list, delim);
}

/*

=item C<String *join_with_locker(Locker *locker, const List *list, const char *delim)>

Equivalent to I<join(3)> except that multiple threads accessing the new
string will be synchronised by C<locker>.

=cut

*/

String *join_with_locker(Locker *locker, const List *list, const char *delim)
{
	String *ret;
	String *del;
	Lister *lister;
	int i;

	if (!list)
		return set_errnull(EINVAL);

	if (!(ret = str_create_with_locker(locker, NULL)))
		return NULL;

	if (!(del = str_create(delim ? "%s" : NULL, delim)))
	{
		str_release(ret);
		return NULL;
	}

	if (!(lister = lister_create((List *)list)))
	{
		str_release(ret);
		str_release(del);
		return NULL;
	}

	for (i = 0; lister_has_next(lister) == 1; ++i)
	{
		char *s = (char *)lister_next(lister);

		if (i && !str_append_str(ret, del))
		{
			str_release(ret);
			str_release(del);
			lister_release(lister);
			return NULL;
		}

		if (s && !str_append(ret, "%s", s))
		{
			str_release(ret);
			str_release(del);
			lister_release(lister);
			return NULL;
		}
	}

	str_release(del);
	lister_release(lister);

	return ret;
}

/*

=item C<int str_soundex(const String *str)>

Returns the soundex code of C<str> as an integer. On error, returns C<-1>
with C<errno> set appropriately.

=cut

*/

int str_soundex(const String *str)
{
	int ret;
	int err;

	if (!str)
		return set_errno(EINVAL);

	if ((err = str_rdlock(str)))
		return set_errno(err);

	ret = str_soundex_unlocked(str);

	if ((err = str_unlock(str)))
		return set_errno(err);

	return ret;
}

/*

=item C<int str_soundex_unlocked(const String *str)>

Equivalent to I<str_soundex(3)> except that C<str> is not read-locked.

=cut

*/

int str_soundex_unlocked(const String *str)
{
	if (!str)
		return set_errno(EINVAL);

	return soundex(str->str);
}

/*

=item C<int soundex(const char *str)>

Equivalent to I<str_soundex(3)> but works on an ordinary I<C> string.

=cut

*/

int soundex(const char *str)
{
	const char * const soundex_table = "\000123\00012\000\00022455\00012623\0001\0002\0002";
	union { char c[4]; int i; } soundex;
	int last, small;

	if (!str)
		return set_errno(EINVAL);

	soundex.i = 0;

	for (last = -1, small = 0; *str && small < 4; ++str)
	{
		if (is_alpha(*str))
		{
			int code = to_upper(*str);

			if (small == 0)
			{
				soundex.c[small++] = code;
				last = soundex_table[code - 'A'];
			}
			else
			{
				if ((code = soundex_table[code - 'A']) != last)
				{
					if (code != 0)
						soundex.c[small++] = code;

					last = code;
				}
			}
		}
	}

	while (small < 4)
		soundex.c[small++] = '0';

	return ntohl(soundex.i);
}

/*

=item C<String *str_trim(String *str)>

Trims leading and trailing whitespace from C<str>. On success, returns
C<str>. On error, returns C<null> with C<errno> set appropriately.

=cut

*/

String *str_trim(String *str)
{
	String *ret;
	int err;

	if (!str)
		return set_errnull(EINVAL);

	if ((err = str_wrlock(str)))
		return set_errnull(err);

	ret = str_trim_unlocked(str);

	if ((err = str_unlock(str)))
		return set_errnull(err);

	return ret;
}

/*

=item C<String *str_trim_unlocked(String *str)>

Equivalent to I<str_trim(3)> except that C<str> is not write-locked.

=cut

*/

String *str_trim_unlocked(String *str)
{
	char *s;

	if (!str)
		return set_errnull(EINVAL);

	for (s = str->str; is_space(*s); ++s)
	{}

	if (s > str->str)
	{
		if (!str_remove_range_unlocked(str, 0, s - str->str))
			return NULL;
	}

	for (s = str->str + str->length - 1; s > str->str && is_space(s[-1]); --s)
	{}

	if (is_space(*s))
	{
		if (!str_remove_range_unlocked(str, s - str->str, str->length - 1 - (s - str->str)))
			return NULL;
	}

	return str;
}

/*

=item C<char *trim(char *str)>

Equivalent to I<str_trim(3)> but works on an ordinary I<C> string.

=cut

*/

char *trim(char *str)
{
	char *s;
	size_t len;

	if (!str)
		return set_errnull(EINVAL);

	for (s = str; is_space(*s); ++s)
	{}

	len = strlen(s);

	if (s > str)
		memmove(str, s, len + 1);

	for (s = str + len; s > str && is_space(*--s); )
		*s = '\0';

	return str;
}

/*

=item C<String *str_trim_left(String *str)>

Trims leading whitespace from C<str>. On success, returns C<str>. On error,
returns C<null> with C<errno> set appropriately.

=cut

*/

String *str_trim_left(String *str)
{
	String *ret;
	int err;

	if (!str)
		return set_errnull(EINVAL);

	if ((err = str_wrlock(str)))
		return set_errnull(err);

	ret = str_trim_left_unlocked(str);

	if ((err = str_unlock(str)))
		return set_errnull(err);

	return ret;
}

/*

=item C<String *str_trim_left_unlocked(String *str)>

Equivalent to I<str_trim_left(3)> except that C<str> is not write-locked.

=cut

*/

String *str_trim_left_unlocked(String *str)
{
	char *s;

	if (!str)
		return set_errnull(EINVAL);

	for (s = str->str; is_space(*s); ++s)
	{}

	if (s > str->str)
		if (!str_remove_range_unlocked(str, 0, s - str->str))
			return NULL;

	return str;
}

/*

=item C<char *trim_left(char *str)>

Equivalent to I<str_trim_left(3)> but works on an ordinary I<C> string.

=cut

*/

char *trim_left(char *str)
{
	char *s;
	size_t len;

	if (!str)
		return set_errnull(EINVAL);

	for (s = str; is_space(*s); ++s)
	{}

	len = strlen(s);

	if (s > str)
		memmove(str, s, len + 1);

	return str;
}

/*

=item C<String *str_trim_right(String *str)>

Trims trailing whitespace from C<str>. On success, returns C<str>. On error,
returns C<null> with C<errno> set appropriately.

=cut

*/

String *str_trim_right(String *str)
{
	String *ret;
	int err;

	if (!str)
		return set_errnull(EINVAL);

	if ((err = str_wrlock(str)))
		return set_errnull(err);

	ret = str_trim_right_unlocked(str);

	if ((err = str_unlock(str)))
		return set_errnull(err);

	return ret;
}

/*

=item C<String *str_trim_right_unlocked(String *str)>

Equivalent to I<str_trim_right(3)> except that C<str> is not write-locked.

=cut

*/

String *str_trim_right_unlocked(String *str)
{
	char *s;

	if (!str)
		return set_errnull(EINVAL);

	for (s = str->str + str->length - 1; s > str->str && is_space(s[-1]); --s)
	{}

	if (is_space(*s))
		if (!str_remove_range_unlocked(str, s - str->str, str->length - 1 - (s - str->str)))
			return NULL;

	return str;
}

/*

=item C<char *trim_right(char *str)>

Equivalent to I<str_trim_right(3)> but works on an ordinary I<C> string.

=cut

*/

char *trim_right(char *str)
{
	char *s;
	size_t len;

	if (!str)
		return set_errnull(EINVAL);

	len = strlen(str);

	for (s = str + len; s > str && is_space(*--s); )
		*s = '\0';

	return str;
}

/*

=item C<String *str_squeeze(String *str)>

Trims leading and trailing whitespace from C<str> and replaces all other
sequences of whitespace with a single space. On success, returns C<str>. On
error, returns C<null> with C<errno> set appropriately.

=cut

*/

String *str_squeeze(String *str)
{
	String *ret;
	int err;

	if (!str)
		return set_errnull(EINVAL);

	if ((err = str_wrlock(str)))
		return set_errnull(err);

	ret = str_squeeze_unlocked(str);

	if ((err = str_unlock(str)))
		return set_errnull(err);

	return ret;
}

/*

=item C<String *str_squeeze_unlocked(String *str)>

Equivalent to I<str_squeeze(3)> except that C<str> is not write-locked.

=cut

*/

String *str_squeeze_unlocked(String *str)
{
	char *s, *r;
	int started = 0;
	int was_space = 0;

	if (!str)
		return set_errnull(EINVAL);

	for (r = s = str->str; s - str->str < str->length - 1; ++s)
	{
		if (!is_space(*s))
		{
			if (was_space && started)
				*r++ = ' ';
			*r++ = *s;
			started = 1;
		}

		was_space = is_space(*s);
	}

	if (r - str->str < str->length)
		if (!str_remove_range_unlocked(str, r - str->str, str->length - 1 - (r - str->str)))
			return NULL;

	return str;
}

/*

=item C<char *squeeze(char *str)>

Equivalent to I<str_squeeze(3)> but works on an ordinary I<C> string.

=cut

*/

char *squeeze(char *str)
{
	char *s, *r;
	int started = 0;
	int was_space = 0;

	if (!str)
		return set_errnull(EINVAL);

	for (r = s = str; *s; ++s)
	{
		if (!is_space(*s))
		{
			if (was_space && started)
				*r++ = ' ';
			*r++ = *s;
			started = 1;
		}

		was_space = is_space(*s);
	}

	*r = '\0';

	return str;
}

/*

=item C<String *str_quote(const String *str, const char *quotable, char quote_char)>

Creates a new I<String> containing C<str> with every occurrence of any
character in C<quotable> preceded by C<quote_char>. On success, returns the
new string. It is the caller's responsibility to deallocate the new string
with I<str_release(3)> or I<str_destroy(3)>. On error, returns C<null> with
C<errno> set appropriately.

=cut

*/

String *str_quote(const String *str, const char *quotable, char quote_char)
{
	return str_quote_with_locker(NULL, str, quotable, quote_char);
}

/*

=item C<String *str_quote_unlocked(const String *str, const char *quotable, char quote_char)>

Equivalent to I<str_quote(3)> except that C<str> is not read-locked.

=cut

*/

String *str_quote_unlocked(const String *str, const char *quotable, char quote_char)
{
	return str_quote_with_locker_unlocked(NULL, str, quotable, quote_char);
}

/*

=item C<String *str_quote_with_locker(Locker *locker, const String *str, const char *quotable, char quote_char)>

Equivalent to I<str_quote(3)> except that multiple threads accessing the new
string will be synchronised by C<locker>.

=cut

*/

String *str_quote_with_locker(Locker *locker, const String *str, const char *quotable, char quote_char)
{
	String *ret;
	size_t i;

	if (!str || !quotable)
		return set_errnull(EINVAL);

	if (!(ret = str_copy_with_locker(locker, str)))
		return NULL;

	for (i = 0; i < ret->length - 1; ++i)
	{
		if (ret->str[i] && strchr(quotable, ret->str[i]))
		{
			if (!str_insert(ret, i++, "%c", quote_char))
			{
				str_release(ret);
				return NULL;
			}
		}
	}

	return ret;
}

/*

=item C<String *str_quote_with_locker_unlocked(Locker *locker, const String *str, const char *quotable, char quote_char)>

Equivalent to I<str_quote_with_locker(3)> except that C<str> is not
read-locked.

=cut

*/

String *str_quote_with_locker_unlocked(Locker *locker, const String *str, const char *quotable, char quote_char)
{
	String *ret;
	size_t i;

	if (!str || !quotable)
		return set_errnull(EINVAL);

	if (!(ret = str_copy_with_locker_unlocked(locker, str)))
		return NULL;

	for (i = 0; i < ret->length - 1; ++i)
	{
		if (ret->str[i] && strchr(quotable, ret->str[i]))
		{
			if (!str_insert(ret, i++, "%c", quote_char))
			{
				str_release(ret);
				return NULL;
			}
		}
	}

	return ret;
}

/*

=item C<String *quote(const char *str, const char *quotable, char quote_char)>

Equivalent to I<str_quote(3)> but works on an ordinary I<C> string.

=cut

*/

String *quote(const char *str, const char *quotable, char quote_char)
{
	return quote_with_locker(NULL, str, quotable, quote_char);
}

/*

=item C<String *quote_with_locker(Locker *locker, const char *str, const char *quotable, char quote_char)>

Equivalent to I<quote(3)> except that multiple threads accessing the new
string will be synchronised by C<locker>.

=cut

*/

String *quote_with_locker(Locker *locker, const char *str, const char *quotable, char quote_char)
{
	String *ret;
	size_t i;

	if (!str || !quotable)
		return set_errnull(EINVAL);

	if (!(ret = str_create_with_locker(locker, "%s", str)))
		return NULL;

	for (i = 0; i < ret->length - 1; ++i)
	{
		if (strchr(quotable, ret->str[i]))
		{
			if (!str_insert(ret, i++, "%c", quote_char))
			{
				str_release(ret);
				return NULL;
			}
		}
	}

	return ret;
}

/*

=item C<String *str_unquote(const String *str, const char *quotable, char quote_char)>

Creates a new string containing C<str> with every occurrence of
C<quote_char> that is followed by any character in C<quotable> removed. On
success, returns the new I<String>. It is the caller's responsibility to
deallocate the new string with I<str_release(3)> or I<str_destroy(3)>. On
error, returns C<null> with C<errno> set appropriately.

=cut

*/

String *str_unquote(const String *str, const char *quotable, char quote_char)
{
	return str_unquote_with_locker(NULL, str, quotable, quote_char);
}

/*

=item C<String *str_unquote_unlocked(const String *str, const char *quotable, char quote_char)>

Equivalent to I<str_unquote(3)> except that C<str> is not read-locked.

=cut

*/

String *str_unquote_unlocked(const String *str, const char *quotable, char quote_char)
{
	return str_unquote_with_locker_unlocked(NULL, str, quotable, quote_char);
}

/*

=item C<String *str_unquote_with_locker(Locker *locker, const String *str, const char *quotable, char quote_char)>

Equivalent to I<str_unquote(3)> except that multiple threads accessing the
new string will be synchronised by C<locker>.

=cut

*/

String *str_unquote_with_locker(Locker *locker, const String *str, const char *quotable, char quote_char)
{
	String *ret;
	int i;

	if (!str || !quotable)
		return set_errnull(EINVAL);

	if (!(ret = str_copy_with_locker(locker, str)))
		return NULL;

	for (i = 0; i < (int)ret->length - 2; ++i)
	{
		if (ret->str[i] == quote_char && ret->str[i + 1] && strchr(quotable, ret->str[i + 1]))
		{
			if (!str_remove(ret, i))
			{
				str_release(ret);
				return NULL;
			}
		}
	}

	return ret;
}

/*

=item C<String *str_unquote_with_locker_unlocked(Locker *locker, const String *str, const char *quotable, char quote_char)>

Equivalent to I<str_unquote_with_locker(3)> except that C<str> is not
read-locked.

=cut

*/

String *str_unquote_with_locker_unlocked(Locker *locker, const String *str, const char *quotable, char quote_char)
{
	String *ret;
	int i;

	if (!str || !quotable)
		return set_errnull(EINVAL);

	if (!(ret = str_copy_with_locker_unlocked(locker, str)))
		return NULL;

	for (i = 0; i < (int)ret->length - 2; ++i)
	{
		if (ret->str[i] == quote_char && ret->str[i + 1] && strchr(quotable, ret->str[i + 1]))
		{
			if (!str_remove(ret, i))
			{
				str_release(ret);
				return NULL;
			}
		}
	}

	return ret;
}

/*

=item C<String *unquote(const char *str, const char *quotable, char quote_char)>

Equivalent to I<str_unquote(3)> but works on an ordinary I<C> string.

=cut

*/

String *unquote(const char *str, const char *quotable, char quote_char)
{
	return unquote_with_locker(NULL, str, quotable, quote_char);
}

/*

=item C<String *unquote_with_locker(Locker *locker, const char *str, const char *quotable, char quote_char)>

Equivalent to I<unquote(3)> except that multiple threads accessing the new
string will be synchronised by C<locker>.

=cut

*/

String *unquote_with_locker(Locker *locker, const char *str, const char *quotable, char quote_char)
{
	String *ret;
	int i;

	if (!str || !quotable)
		return set_errnull(EINVAL);

	if (!(ret = str_create_with_locker(locker, "%s", str)))
		return NULL;

	for (i = 0; i < (int)ret->length - 2; ++i)
	{
		if (ret->str[i] == quote_char && strchr(quotable, ret->str[i + 1]))
		{
			if (!str_remove(ret, i))
			{
				str_release(ret);
				return NULL;
			}
		}
	}

	return ret;
}

/*

C<static String *do_encode_with_locker(Locker *locker, const char *str, ssize_t length, const char *uncoded, const char *coded, char quote_char, int printable)>

Performs encoding as described in I<str_encode(3)>.

*/

static String *do_encode_with_locker(Locker *locker, const char *str, size_t length, const char *uncoded, const char *coded, char quote_char, int printable)
{
	static const char hex[] = "0123456789abcdef";
	String *encoded;
	const char *target;
	const char *s;

	if (!str || !uncoded || !coded)
		return set_errnull(EINVAL);

	if (!(encoded = str_create_with_locker_sized(locker, length * 4 + 1, "")))
		return NULL;

	for (s = str; s - str < length; ++s)
	{
		if (*s && (target = strchr(uncoded, (unsigned char)*s)))
		{
			if (!str_append(encoded, "%c%c", (unsigned char)quote_char, coded[target - uncoded]))
			{
				str_release(encoded);
				return NULL;
			}
		}
		else if (printable && !is_print(*s))
		{
			if (!str_append(encoded, "%cx%c%c", (unsigned char)quote_char, hex[(unsigned char)*s >> 4], hex[(unsigned char)*s & 0x0f]))
			{
				str_release(encoded);
				return NULL;
			}
		}
		else
		{
			if (!str_append(encoded, "%c", (unsigned char)*s))
			{
				str_release(encoded);
				return NULL;
			}
		}
	}

	return encoded;
}

/*

C<static String *do_decode_with_locker(Locker *locker, const char *str, ssize_t length, const char *uncoded, const char *coded, char quote_char, int printable)>

Performs decoding as described in I<str_decode(3)>.

*/

static String *do_decode_with_locker(Locker *locker, const char *str, size_t length, const char *uncoded, const char *coded, char quote_char, int printable)
{
	String *decoded;
	const char *start;
	const char *slosh;
	char *target;

	if (!str || !uncoded || !coded)
		return set_errnull(EINVAL);

	if (!(decoded = str_create_with_locker_sized(locker, length + 1, "")))
		return NULL;

	for (start = str; start - str < length; start = slosh + 1)
	{
		for (slosh = start; slosh - str < length; ++slosh)
			if (*slosh == quote_char)
				break;

		if (slosh - str == length)
			break;

		if (printable)
		{
			int digits = 0;
			const char *s = slosh + 1;
			char c = '\0';

			if (is_digit(*s) && *s <= '7')
			{
				--s;

				do
				{
					++digits;
					c <<= 3, c |= *++s - '0';
				}
				while (digits < 3 && is_digit(s[1]) && s[1] <= '7');
			}
			else if (*s == 'x' && is_xdigit(s[1]))
			{
				do
				{
					++digits;
					c <<= 4;

					switch (*++s)
					{
						case '0': case '1': case '2': case '3': case '4':
						case '5': case '6': case '7': case '8': case '9':
							c |= *s - '0';
							break;
						case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
							c |= *s - 'a' + 10;
							break;
						case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
							c |= *s - 'A' + 10;
							break;
					}
				}
				while (digits < 2 && is_xdigit(s[1]));
			}

			if (digits)
			{
				if (!str_append(decoded, "%*.*s%c", slosh - start, slosh - start, start, c))
				{
					str_release(decoded);
					return NULL;
				}

				slosh = s; /* Skip over ASCII code */
				continue;
			}
		}

		if (!slosh[1] || !(target = strchr(coded, slosh[1])))
		{
			if (!str_append(decoded, "%*.*s%c", slosh - start, slosh - start, start, quote_char))
			{
				str_release(decoded);
				return NULL;
			}

			continue;
		}

		if (!str_append(decoded, "%*.*s%c", slosh - start, slosh - start, start, uncoded[target - coded]))
		{
			str_release(decoded);
			return NULL;
		}

		++slosh; /* Skip over quoted char */
	}

	if (!str_append(decoded, "%s", start))
	{
		str_release(decoded);
		return NULL;
	}

	return decoded;
}

/*

=item C<String *str_encode(const String *str, const char *uncoded, const char *coded, char quote_char, int printable)>

Returns a copy of C<str> with every occurrence in C<str> of characters in
C<uncoded> replaced with C<quote_char> followed by the corresponding (by
position) character in C<coded>. If C<printable> is non-zero, other
non-printable characters are replaced with their I<ASCII> codes in
hexadecimal. It is the caller's responsibility to deallocate the new string
with I<str_release(3)> or I<str_destroy(3)>. It is strongly recommended to
use I<str_destroy(3)>, because it also sets the pointer variable to C<null>.
On error, returns C<null> with C<errno> set appropriately.

Example:

    // Encode a string into a C string literal
    str_encode(str, "\a\b\t\n\v\f\r\\", "abtnvfr\\", '\\', 1);

    // Decode a C string literal
    str_decode(str, "\a\b\t\n\v\f\r\\", "abtnvfr\\", '\\', 1);

=cut

*/

String *str_encode(const String *str, const char *uncoded, const char *coded, char quote_char, int printable)
{
	return str_encode_with_locker(NULL, str, uncoded, coded, quote_char, printable);
}

/*

=item C<String *str_encode_unlocked(const String *str, const char *uncoded, const char *coded, char quote_char, int printable)>

Equivalent to I<str_encode(3)> except that C<str> is not read-locked.

=cut

*/

String *str_encode_unlocked(const String *str, const char *uncoded, const char *coded, char quote_char, int printable)
{
	return str_encode_with_locker_unlocked(NULL, str, uncoded, coded, quote_char, printable);
}

/*

=item C<String *str_encode_with_locker(Locker *locker, const String *str, const char *uncoded, const char *coded, char quote_char, int printable)>

Equivalent to I<str_encode(3)> except that multiple threads accessing the new
string will be synchronised by C<locker>.

=cut

*/

String *str_encode_with_locker(Locker *locker, const String *str, const char *uncoded, const char *coded, char quote_char, int printable)
{
	String *ret;
	int err;

	if (!str || !uncoded || !coded)
		return set_errnull(EINVAL);

	if ((err = str_rdlock(str)))
		return set_errnull(err);

	ret = str_encode_with_locker_unlocked(locker, str, uncoded, coded, quote_char, printable);

	if ((err = str_unlock(str)))
	{
		str_release(ret);
		return set_errnull(err);
	}

	return ret;
}

/*

=item C<String *str_encode_with_locker_unlocked(Locker *locker, const String *str, const char *uncoded, const char *coded, char quote_char, int printable)>

Equivalent to I<str_encode_with_locker(3)> except that C<str> is not
read-locked.

=cut

*/

String *str_encode_with_locker_unlocked(Locker *locker, const String *str, const char *uncoded, const char *coded, char quote_char, int printable)
{
	if (!str || !uncoded || !coded)
		return set_errnull(EINVAL);

	return do_encode_with_locker(locker, str->str, str->length - 1, uncoded, coded, quote_char, printable);
}

/*

=item C<String *str_decode(const String *str, const char *uncoded, const char *coded, char quote_char, int printable)>

Returns a copy of C<str> with every occurrence in C<str> of C<quote_char>
followed by a character in C<coded> replaced with the corresponding (by
position) character in C<uncoded>. If C<printable> is non-zero, every
occurrence in C<str> of an I<ASCII> code in octal or hexadecimal (i.e.
"\ooo" or "\xhh") is replaced with the corresponding I<ASCII> character. It
is the caller's responsibility to deallocate the new string with
I<str_release(3)> or I<str_destroy(3)>. It is strongly recommended to use
I<str_destroy(3)>, because it also sets the pointer variable to C<null>. On
error, returns C<null> with C<errno> set appropriately.

=cut

*/

String *str_decode(const String *str, const char *uncoded, const char *coded, char quote_char, int printable)
{
	return str_decode_with_locker(NULL, str, uncoded, coded, quote_char, printable);
}

/*

=item C<String *str_decode_unlocked(const String *str, const char *uncoded, const char *coded, char quote_char, int printable)>

Equivalent to I<str_decode(3)> except that C<str> is not read-locked.

=cut

*/

String *str_decode_unlocked(const String *str, const char *uncoded, const char *coded, char quote_char, int printable)
{
	return str_decode_with_locker_unlocked(NULL, str, uncoded, coded, quote_char, printable);
}

/*

=item C<String *str_decode_with_locker(Locker *locker, const String *str, const char *uncoded, const char *coded, char quote_char, int printable)>

Equivalent to I<str_decode(3)> except that multiple threads accessing the
new string will be synchronised by C<locker>.

=cut

*/

String *str_decode_with_locker(Locker *locker, const String *str, const char *uncoded, const char *coded, char quote_char, int printable)
{
	String *ret;
	int err;

	if (!str || !uncoded || !coded)
		return set_errnull(EINVAL);

	if ((err = str_rdlock(str)))
		return set_errnull(err);

	ret = str_decode_with_locker_unlocked(locker, str, uncoded, coded, quote_char, printable);

	if ((err = str_unlock(str)))
	{
		str_release(ret);
		return set_errnull(err);
	}

	return ret;
}

/*

=item C<String *str_decode_with_locker_unlocked(Locker *locker, const String *str, const char *uncoded, const char *coded, char quote_char, int printable)>

Equivalent to I<str_decode_with_locker(3)> except that C<str> is not
read-locked.

=cut

*/

String *str_decode_with_locker_unlocked(Locker *locker, const String *str, const char *uncoded, const char *coded, char quote_char, int printable)
{
	if (!str || !uncoded || !coded)
		return set_errnull(EINVAL);

	return do_decode_with_locker(locker, str->str, str->length - 1, uncoded, coded, quote_char, printable);
}

/*

=item C<String *encode(const char *str, const char *uncoded, const char *coded, char quote_char, int printable)>

Equivalent to I<str_encode(3)> but works on an ordinary I<C> string.

=cut

*/

String *encode(const char *str, const char *uncoded, const char *coded, char quote_char, int printable)
{
	return encode_with_locker(NULL, str, uncoded, coded, quote_char, printable);
}

/*

=item C<String *encode_with_locker(Locker *locker, const char *str, const char *uncoded, const char *coded, char quote_char, int printable)>

Equivalent to I<encode(3)> except that multiple threads accessing the new
string will be synchronised by C<locker>.

=cut

*/

String *encode_with_locker(Locker *locker, const char *str, const char *uncoded, const char *coded, char quote_char, int printable)
{
	if (!str || !uncoded || !coded)
		return set_errnull(EINVAL);

	return do_encode_with_locker(locker, str, strlen(str), uncoded, coded, quote_char, printable);
}

/*

=item C<String *decode(const char *str, const char *uncoded, const char *coded, char quote_char, int printable)>

Equivalent to I<str_decode(3)> but works on an ordinary I<C> string.

=cut

*/

String *decode(const char *str, const char *uncoded, const char *coded, char quote_char, int printable)
{
	return decode_with_locker(NULL, str, uncoded, coded, quote_char, printable);
}

/*

=item C<String *decode_with_locker(Locker *locker, const char *str, const char *uncoded, const char *coded, char quote_char, int printable)>

Equivalent to I<decode(3)> except that multiple threads accessing the new
string will be synchronised by C<locker>.

=cut

*/

String *decode_with_locker(Locker *locker, const char *str, const char *uncoded, const char *coded, char quote_char, int printable)
{
	if (!str || !uncoded || !coded)
		return set_errnull(EINVAL);

	return do_decode_with_locker(locker, str, strlen(str), uncoded, coded, quote_char, printable);
}

/*

=item C<String *str_lc(String *str)>

Converts C<str> into lower case. On success, returns C<str>. On error,
returns C<null> with C<errno> set appropriately.

=cut

*/

String *str_lc(String *str)
{
	String *ret;
	int err;

	if (!str)
		return set_errnull(EINVAL);

	if ((err = str_wrlock(str)))
		return set_errnull(err);

	ret = str_lc_unlocked(str);

	if ((err = str_unlock(str)))
		return set_errnull(err);

	return ret;
}

/*

=item C<String *str_lc_unlocked(String *str)>

Equivalent to I<str_lc(3)> except that C<str> is not write-locked.

=cut

*/

String *str_lc_unlocked(String *str)
{
	size_t i;

	if (!str)
		return set_errnull(EINVAL);

	for (i = 0; i < str->length - 1; ++i)
		str->str[i] = to_lower(str->str[i]);

	return str;
}

/*

=item C<char *lc(char *str)>

Converts C<str> into lower case. On success, returns C<str>. On error,
returns C<null> with C<errno> set appropriately.

=cut

*/

char *lc(char *str)
{
	char *s;

	if (!str)
		return set_errnull(EINVAL);

	for (s = str; *s; ++s)
		*s = to_lower(*s);

	return str;
}

/*

=item C<String *str_lcfirst(String *str)>

Converts the first character in C<str> into lower case. On success, returns
C<str>. On error, returns C<null> with C<errno> set appropriately.

=cut

*/

String *str_lcfirst(String *str)
{
	String *ret;
	int err;

	if (!str)
		return set_errnull(EINVAL);

	if ((err = str_wrlock(str)))
		return set_errnull(err);

	ret = str_lcfirst_unlocked(str);

	if ((err = str_unlock(str)))
		return set_errnull(err);

	return ret;
}

/*

=item C<String *str_lcfirst_unlocked(String *str)>

Equivalent to I<str_lcfirst(3)> except that C<str> is not write-locked.

=cut

*/

String *str_lcfirst_unlocked(String *str)
{
	if (!str)
		return set_errnull(EINVAL);

	if (str->length > 1)
		*str->str = to_lower(*str->str);

	return str;
}

/*

=item C<char *lcfirst(char *str)>

Converts the first character in C<str> into lower case. On success, returns
C<str>. On error, returns C<null> with C<errno> set appropriately.

=cut

*/

char *lcfirst(char *str)
{
	if (!str)
		return set_errnull(EINVAL);

	*str = to_lower(*str);

	return str;
}

/*

=item C<String *str_uc(String *str)>

Converts C<str> into upper case. On success, returns C<str>. On error,
returns C<null> with C<errno> set appropriately.

=cut

*/

String *str_uc(String *str)
{
	String *ret;
	int err;

	if (!str)
		return set_errnull(EINVAL);

	if ((err = str_wrlock(str)))
		return set_errnull(err);

	ret = str_uc_unlocked(str);

	if ((err = str_unlock(str)))
		return set_errnull(err);

	return ret;
}

/*

=item C<String *str_uc_unlocked(String *str)>

Equivalent to I<str_uc(3)> except that C<str> is not write-locked.

=cut

*/

String *str_uc_unlocked(String *str)
{
	size_t i;

	if (!str)
		return set_errnull(EINVAL);

	for (i = 0; i < str->length - 1; ++i)
		str->str[i] = to_upper(str->str[i]);

	return str;
}

/*

=item C<char *uc(char *str)>

Converts C<str> into upper case. On success, returns C<str>. On error,
returns C<null> with C<errno> set appropriately.

=cut

*/

char *uc(char *str)
{
	char *s;

	if (!str)
		return set_errnull(EINVAL);

	for (s = str; *s; ++s)
		*s = to_upper(*s);

	return str;
}

/*

=item C<String *str_ucfirst(String *str)>

Converts the first character in C<str> into upper case. On success, returns
C<str>. On error, returns C<null> with C<errno> set appropriately.

=cut

*/

String *str_ucfirst(String *str)
{
	String *ret;
	int err;

	if (!str)
		return set_errnull(EINVAL);

	if ((err = str_wrlock(str)))
		return set_errnull(err);

	ret = str_ucfirst_unlocked(str);

	if ((err = str_unlock(str)))
		return set_errnull(err);

	return ret;
}

/*

=item C<String *str_ucfirst_unlocked(String *str)>

Equivalent to I<str_ucfirst(3)> except that C<str> is not write-locked.

=cut

*/

String *str_ucfirst_unlocked(String *str)
{
	if (!str)
		return set_errnull(EINVAL);

	if (str->length > 1)
		*str->str = to_upper(*str->str);

	return str;
}

/*

=item C<char *ucfirst(char *str)>

Converts the first character in C<str> into upper case. On success, returns
C<str>. On error, returns C<null> with C<errno> set appropriately.

=cut

*/

char *ucfirst(char *str)
{
	if (!str)
		return set_errnull(EINVAL);

	*str = to_upper(*str);

	return str;
}

/*

=item C<int str_chop(String *str)>

Removes a character from the end of C<str>. On success, returns the
character that was removed. On error, returns C<-1> with C<errno> set
appropriately.

=cut

*/

int str_chop(String *str)
{
	int ret;
	int err;

	if (!str)
		return set_errno(EINVAL);

	if ((err = str_wrlock(str)))
		return set_errno(err);

	ret = str_chop_unlocked(str);

	if ((err = str_unlock(str)))
		return set_errno(err);

	return ret;
}

/*

=item C<int str_chop_unlocked(String *str)>

Equivalent to I<str_chop(3)> except that C<str> is not write-locked.

=cut

*/

int str_chop_unlocked(String *str)
{
	int ret;

	if (!str)
		return set_errno(EINVAL);

	if (str->length == 1)
		return set_errno(EINVAL);

	ret = str->str[str->length - 2];

	if (contract(str, str->length - 2, 1) == -1)
		return -1;

	return ret;
}

/*

=item C<int chop(char *str)>

Removes a character from the end of C<str>. On success, returns the
character that was removed. On error, returns C<-1> with C<errno> set
appropriately.

=cut

*/

int chop(char *str)
{
	int ret;

	if (!str || !*str)
		return set_errno(EINVAL);

	while (str[1])
		++str;

	ret = (int)*str;
	*str = '\0';

	return ret;
}

/*

=item C<int str_chomp(String *str)>

Removes line ending characters (i.e. C<'\n'> and C<'\r'>) from the end of
C<str>. On success, returns the number of characters that were removed. On
error, returns C<-1> with C<errno> set appropriately.

=cut

*/

int str_chomp(String *str)
{
	int ret;
	int err;

	if (!str)
		return set_errno(EINVAL);

	if ((err = str_wrlock(str)))
		return set_errno(err);

	ret = str_chomp_unlocked(str);

	if ((err = str_unlock(str)))
		return set_errno(err);

	return ret;
}

/*

=item C<int str_chomp_unlocked(String *str)>

Equivalent to I<str_chomp(3)> except that C<str> is not write-locked.

=cut

*/

int str_chomp_unlocked(String *str)
{
	char *s;
	size_t length;

	if (!str)
		return set_errno(EINVAL);

	if (str->length == 1)
		return 0;

	length = str->length;

	for (s = str->str + str->length - 2; *s == '\n' || *s == '\r'; --s)
	{
		if (contract(str, str->length - 2, 1) == -1)
			return -1;
	}

	return length - str->length;
}

/*

=item C<int chomp(char *str)>

Removes line ending characters (i.e. C<'\n'> and C<'\r'>) from the end of
C<str>. On success, returns the number of characters that were removed. On
error, returns C<-1> with C<errno> set appropriately.

=cut

*/

int chomp(char *str)
{
	char *s;

	if (!str)
		return set_errno(EINVAL);

	if (str[0] == '\0')
		return 0;

	while (str[1])
		++str;

	for (s = str; *s == '\n' || *s == '\r'; --s)
		*s = '\0';

	return str - s;
}

/*

=item C<int str_bin(const String *str)>

Returns the integer specified by the binary string, C<str>. C<str> can
either be a string of C<[0-1]>, or C<"0b"> followed by a string of C<[0-1]>.
On error, returns C<-1> with C<errno> set appropriately.

=cut

*/

int str_bin(const String *str)
{
	int ret;
	int err;

	if (!str)
		return set_errno(EINVAL);

	if ((err = str_rdlock(str)))
		return set_errno(err);

	ret = str_bin_unlocked(str);

	if ((err = str_unlock(str)))
		return set_errno(err);

	return ret;
}

/*

=item C<int str_bin_unlocked(const String *str)>

Equivalent to I<str_bin(3)> except that C<str> is not read-locked.

=cut

*/

int str_bin_unlocked(const String *str)
{
	if (!str)
		return set_errno(EINVAL);

	return bin(str->str);
}

/*

=item C<int bin(const char *str)>

Returns the integer specified by the binary string, C<str>. C<str> can
either be a string of C<[0-1]>, or C<"0b"> followed by a string of C<[0-1]>.
On error, returns C<-1> with C<errno> set appropriately.

=cut

*/

int bin(const char *str)
{
	int ret = 0;

	if (!str)
		return set_errno(EINVAL);

	if (str[0] == '0' && str[1] == 'b')
		str += 2;

	for (; *str; ++str)
	{
		ret <<= 1;

		switch (*str)
		{
			case '0': break;
			case '1': ret |= 1; break;
			default:  return set_errno(EINVAL);
		}
	}

	return ret;
}

/*

=item C<int str_hex(const String *str)>

Returns the integer specified by the hexadecimal string, C<str>. C<str> can
either be a string of C<[0-9a-fA-F]>, or C<"0x"> followed by a string of
C<[0-9a-fA-f]>. On error, returns C<-1> with C<errno> set appropriately.

=cut

*/

int str_hex(const String *str)
{
	int ret;
	int err;

	if (!str)
		return set_errno(EINVAL);

	if ((err = str_rdlock(str)))
		return set_errno(err);

	ret = str_hex_unlocked(str);

	if ((err = str_unlock(str)))
		return set_errno(err);

	return ret;
}

/*

=item C<int str_hex_unlocked(const String *str)>

Equivalent to I<str_hex(3)> except that C<str> is not read-locked.

=cut

*/

int str_hex_unlocked(const String *str)
{
	if (!str)
		return set_errno(EINVAL);

	return hex(str->str);
}

/*

=item C<int hex(const char *str)>

Returns the integer specified by the hexadecimal string, C<str>. C<str> can
either be a string of C<[0-9a-fA-F]>, or C<"0x"> followed by a string of
C<[0-9a-fA-f]>. On error, returns C<-1> with C<errno> set appropriately.

=cut

*/

int hex(const char *str)
{
	int ret = 0;

	if (!str)
		return set_errno(EINVAL);

	if (str[0] == '0' && str[1] == 'x')
		str += 2;

	for (; *str; ++str)
	{
		ret <<= 4;

		switch (*str)
		{
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				ret |= *str - '0';
				break;
			case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
				ret |= *str - 'a' + 10;
				break;
			case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
				ret |= *str - 'A' + 10;
				break;
			default:
				return set_errno(EINVAL);
		}
	}

	return ret;
}

/*

=item C<int str_oct(const String *str)>

Returns the integer specified by the binary, octal or hexadecimal string,
C<str>. C<str> can either be C<"0x"> followed by a string of C<[0-9a-fA-F]>
(hexadecimal), C<"0b"> followed by a string of C<[0-1]> (binary), or C<"0">
followed by a string of C<[0-7]> (octal). On error, returns C<-1> with
C<errno> set appropriately.

=cut

*/

int str_oct(const String *str)
{
	int ret;
	int err;

	if (!str)
		return set_errno(EINVAL);

	if ((err = str_rdlock(str)))
		return set_errno(err);

	ret = str_oct_unlocked(str);

	if ((err = str_unlock(str)))
		return set_errno(err);

	return ret;
}

/*

=item C<int str_oct_unlocked(const String *str)>

Equivalent to I<str_oct(3)> except that C<str> is not read-locked.

=cut

*/

int str_oct_unlocked(const String *str)
{
	if (!str)
		return set_errno(EINVAL);

	return oct(str->str);
}

/*

=item C<int oct(const char *str)>

Returns the integer specified by the binary, octal or hexadecimal string,
C<str>. C<str> can either be C<"0x"> followed by a string of C<[0-9a-fA-F]>
(hexadecimal), C<"0b"> followed by a string of C<[0-1]> (binary), or C<"0">
followed by a string of C<[0-7]> (octal). On error, returns C<-1> with
C<errno> set appropriately.

=cut

*/

int oct(const char *str)
{
	int ret = 0;

	if (!str || str[0] != '0')
		return set_errno(EINVAL);

	if (str[1] == 'b')
		return bin(str);

	if (str[1] == 'x')
		return hex(str);

	for (++str; *str; ++str)
	{
		ret <<= 3;

		switch (*str)
		{
			case '0': case '1': case '2': case '3':
			case '4': case '5': case '6': case '7':
				ret |= *str - '0';
				break;
			default:
				return set_errno(EINVAL);
		}
	}

	return ret;
}

#ifndef HAVE_STRCASECMP

/*

=item I<int strcasecmp(const char *s1, const char *s2)>

Compares two strings, C<s1> and C<s2>, ignoring the case of the characters.
It returns an integer less than, equal to, or greater than zero if C<s1> is
found to be less than, equal to, or greater than C<s2>, respectively.

=cut

*/

int strcasecmp(const char *s1, const char *s2)
{
	while (*s1 && *s2)
	{
		int c1 = to_lower(*s1++);
		int c2 = to_lower(*s2++);

		if (c1 != c2)
			return c1 - c2;
	}

	return to_lower(*s1) - to_lower(*s2);
}

#endif

#ifndef HAVE_STRNCASECMP

/*

=item I<int strncasecmp(const char *s1, const char *s2, size_t n)>

Equivalent to I<strcasecmp(3)> except that it only compares the first C<n>
characters.

=cut

*/

int strncasecmp(const char *s1, const char *s2, size_t n)
{
	while (n--)
	{
		int c1 = to_lower(*s1++);
		int c2 = to_lower(*s2++);

		if (c1 != c2 || !c1 || !c2)
			return c1 - c2;
	}

	return 0;
}

#endif

#ifndef HAVE_STRLCPY

/*

=item I<size_t strlcpy(char *dst, const char *src, size_t size)>

Copies C<src> into C<dst> (which is C<size> bytes long). The result, C<dst>,
will be no longer than C<size - 1> bytes, and will be C<nul>-terminated
(unless C<size> is zero). This is similar to I<strncpy(3)> except that it
always terminates the string with a C<nul> byte (so it's safer), and it
doesn't fill the remainder of the buffer with C<nul> bytes (so it's faster).
Returns the length of C<src> (If this is >= C<size>, truncation occurred).
Use this rather than I<strcpy(3)> or I<strncpy(3)>.

=cut

*/

size_t strlcpy(char *dst, const char *src, size_t size)
{
	const char *s = src;
	char *d = dst;
	size_t n = size;

	if (n)
		while (--n && (*d++ = *s++))
		{}

	if (n == 0)
	{
		if (size)
			*d = '\0';

		while (*s++)
		{}
	}

	return s - src - 1;
}

#endif

#ifndef HAVE_STRLCAT

/*

=item I<size_t strlcat(char *dst, const char *src, size_t size)>

Appends C<src> to C<dst> (which is C<size> bytes long). The result, C<dst>,
will be no longer than C<size - 1> bytes, and will be C<nul>-terminated
(unless C<size> is zero). This is similar to I<strncat(3)> except that the
last argument is the size of the buffer, not the amount of space available
(so it's more intuitive and hence safer). Returns the sum of the lengths of
C<src> and C<dst> (If this is >= C<size>, truncation occurred). Use this
rather than I<strcat(3)> or I<strncat(3)>.

=cut

*/

size_t strlcat(char *dst, const char *src, size_t size)
{
	const char *s = src;
	char *d = dst;
	size_t n = size;
	size_t dlen = 0;

	while (n-- && *d)
		++d;

	dlen = d - dst;

	if (++n == 0)
	{
		while (*s++)
		{}

		return dlen + s - src - 1;
	}

	for (; *s; ++s)
		if (n - 1)
			--n, *d++ = *s;

	*d = '\0';

	return dlen + s - src;
}

#endif

/*

=item C<char *cstrcpy(char *dst, const char *src)>

Copies the string pointed to by C<src> (including the terminating C<nul>
character) to the array pointed to by C<dst>. The memory areas must not
overlap. B<The array C<dst> must be large enough to store the copy. Unless
you know that this is the case, use I<strlcpy(3)> instead.> This is just
like I<strcpy(3)> except that instead of returning C<dst> (which you already
know), this function returns the address of the terminating C<nul> character
(C<dst + strlen(src)>).

=cut

*/

char *cstrcpy(char *dst, const char *src)
{
	while ((*dst++ = *src++))
	{}

	return dst - 1;
}

/*

=item C<char *cstrcat(char *dst, const char *src)>

Appends the string C<src> to the string C<dst>. The strings must not
overlap. B<The string C<dst> must be large enough to store the appended copy
of C<src>. Unless you know that this is the case, use I<strlcat(3)>
instead.> This is just like I<strcat(3)> except that, instead of returning
C<dst> (which you already know), this function returns the address of the
terminating C<nul> character (C<dst + strlen(dst) + strlen(src)>).

=cut

*/

char *cstrcat(char *dst, const char *src)
{
	while (*dst)
		++dst;

	while ((*dst++ = *src++))
	{}

	return dst - 1;
}

/*

=item C<char *cstrchr(const char *str, int c)>

Scans the string C<str> looking for the character C<c>. Returns a pointer to
the first occurrence of the character C<c> in the string C<str>. This is
just like I<strchr(3)> except that, instead of returning C<null> when C<c>
does not appear in C<str>, this function returns the address of the
terminating C<nul> character (C<str + strlen(str)>).

=cut

*/

char *cstrchr(const char *str, int c)
{
	while (*str && *str != (char)c)
		++str;

	return (char *)str;
}

/*

=item C<char *cstrpbrk(const char *str, const char *brk)>

Scans the string C<str> looking for any of the characters in C<brk>. Returns
a pointer to the first occurrence of any character in C<brk> in the string
C<str>. This is just like I<strpbrk(3)> except that, instead of returning
C<null> when no match is found in C<str>, this function returns the address
of the terminating C<nul> character (C<str + strlen(str)>).

=cut

*/

char *cstrpbrk(const char *str, const char *brk)
{
	const char *b;

	for (; *str; ++str)
		for (b = brk; *b; ++b)
			if (*str == *b)
				return (char *)str;

	return (char *)str;
}

/*

=item C<char *cstrrchr(const char *str, int c)>

Scans the string C<str> looking for the character C<c>. Returns a pointer to
the last occurrence of the character C<c> in the string C<str>. This is just
like I<strrchr(3)> except that, instead of returning C<null> when C<c> does
not appear in C<str>, this function returns the address of the terminating
C<nul> character (C<str + strlen(str)>).

=cut

*/

char *cstrrchr(const char *str, int c)
{
	char *match = NULL;

	for (; *str; ++str)
		if (*str == (char)c)
			match = (char *)str;

	return (match) ? match : (char *)str;
}

/*

=item C<char *cstrstr(const char *str, const char *srch)>

Scans the string C<str> looking for the string C<srch>. Returns a pointer to
the first occurrence of the string C<srch> in the string C<str>. This is
just like I<strstr(3)> except that, instead of returning C<null> when
C<srch> does not appear in C<str>, this function returns the address of the
terminating C<nul> character (C<str + strlen(str)>).

=cut

*/

char *cstrstr(const char *str, const char *srch)
{
	if (*srch == '\0')
		return (char *)str;

	for (; *(str = cstrchr(str, *srch)); ++str)
	{
		char *s = (char *)str;
		char *r = (char *)srch;

		for (;;)
		{
			if (*++r == '\0')
				return (char *)str;
			if (*++s != *r)
				break;
		}
	}

	return (char *)str;
}

#ifndef HAVE_ASPRINTF

/*

=item I<int asprintf(char **str, const char *format, ...)>

Equivalent to I<sprintf(3)> except that, instead of formatting C<format> and
subsequent arguments into a buffer supplied by the caller, they are
formatted into a buffer that is internally allocated and stored in C<*str>.
On success, returns the number of bytes stored in C<*str> excluding the
terminating C<nul> character. On error, returns C<-1> and stores C<null> in
C<*str>.

=cut

*/

int asprintf(char **str, const char *format, ...)
{
	int ret;
	va_list args;
	va_start(args, format);
	ret = vasprintf(str, format, args);
	va_end(args);

	return ret;
}

#endif

#ifndef HAVE_VASPRINTF

/*

=item I<int vasprintf(char **str, const char *format, va_list args)>

Equivalent to I<asprintf(3)> with the variable argument list specified
directly as for I<vprintf(3)>.

=cut

*/

int vasprintf(char **str, const char *format, va_list args)
{
	String *tmp;
	int len;

	if (!(tmp = str_vcreate(format, args)))
	{
		*str = NULL;
		return -1;
	}

	if (str)
		*str = cstr(tmp);
	len = str_length(tmp);
	free(tmp);

	return len;
}

#endif

/*

=back

=head1 ERRORS

On error, C<errno> is set either by an underlying function, or as follows:

=over 4

=item C<EINVAL>

When arguments to any of the functions are invalid.

=back

=head1 MT-Level

I<MT-Disciplined>

By default, I<String>s are not I<MT-Safe> because most programs are
single-threaded and synchronisation doesn't come for free. Even in
multi-threaded programs, not all I<String>s are necessarily shared between
multiple threads.

When a I<String> is shared between multiple threads which need to be
synchronised, the method of synchronisation must be carefully selected by
the client code. There are tradeoffs between concurrency and overhead. The
greater the concurrency, the greater the overhead. More locks give greater
concurrency, but have greater overhead. Readers/Writer locks can give
greater concurrency than Mutex locks, but have greater overhead. One lock
for each I<String> might be required, or one lock for all (or a set of)
I<String>s might be more appropriate.

Generally, the best synchronisation strategy for a given application can
only be determined by testing/benchmarking the written application. It is
important to be able to experiment with the synchronisation strategy at this
stage of development without pain.

To facilitate this, I<String>s can be created with
I<string_create_with_locker(3)> which takes a I<Locker> argument. The
I<Locker> specifies a lock and a set of functions for manipulating the lock.
Each I<String> can have its own lock by creating a separate I<Locker> for
each I<String>. Multiple I<String>s can share the same lock by sharing the
same I<Locker>. Only the application developer can determine what is
appropriate for each application on a string by string basis.

I<MT-Disciplined> means that the application developer has a mechanism for
specifying the synchronisation requirements to be applied to library code.

I<MT-Safe> - I<str_fgetline(3)>

I<Mac OS X> doesn't have I<flockfile(3)>, I<funlockfile(3)> or
I<getc_unlocked(3)>. I<fgetline(3)> is not I<MT-Safe> on such platforms. You
must guard all I<stdio> calls with explicit synchronisation variables.

=head1 EXAMPLES

Create and manipulate strings:

    #include <slack/std.h>
    #include <slack/str.h>

    int main()
    {
        String *str1 = str_create("%s %d", "string", 1);
        String *str2 = str_copy(str1);
        String *str3;
        String *str4;
        String *str5;
        String *str6;

        str_remove(str1, 6);
        str_remove_range(str2, 6, 2);
        str_clear(str1);
        str_insert(str1, 0, "%d", 123);
        str_insert_str(str1, 1, str2);
        str_append(str2, "abc");
        str_append_str(str2, str1);
        str_prepend(str1, "abc");
        str_prepend_str(str1, str2);
        str_replace(str1, 1, -2, "abc");
        str_replace_str(str2, 1, 2, str1);

        str3 = str_substr(str1, 1, 3);
        str4 = substr("abc", 1, 1);
        str5 = str_splice(str3, 1, 1);
        str6 = str_repeat(3, "%c", ' ');

        *cstr(str5) = '\0';
        str_set_length(str5, 0);
        str_recalc_length(str5);

        printf("str1 = '%s' %d\n", cstr(str1), str_length(str1));
        printf("str2 = '%s' %d\n", cstr(str2), str_length(str2));
        printf("str3 = '%s' %d\n", cstr(str3), str_length(str3));
        printf("str4 = '%s' %d\n", cstr(str4), str_length(str4));
        printf("str5 = '%s' %d\n", cstr(str5), str_length(str5));
        printf("str6 = '%s' %d\n", cstr(str6), str_length(str6));

        str_destroy(&str1);
        str_destroy(&str2);
        str_destroy(&str3);
        str_destroy(&str4);
        str_destroy(&str5);
        str_destroy(&str6);

        return EXIT_SUCCESS;
    }

Convert a text file from any system into the local text file format:

    #include <slack/std.h>
    #include <slack/str.h>

    int main()
    {
        String *line;

        while (line = str_fgetline(stdin))
        {
            printf("%s", cstr(line));
            str_destroy(&line);
        }

        return EXIT_SUCCESS;
    }

Perform character translation with a pre-compiled translation table
to rot13 the input:

    #include <slack/std.h>
    #include <slack/str.h>

    int main()
    {
        StringTR *trtable = tr_compile("a-zA-Z", "n-za-mN-ZA-M", 0);
        String *line;

        while (line = str_fgetline(stdin))
        {
            str_tr_compiled(line, trtable);
            printf("%s", cstr(line));
            str_destroy(&line);
        }

        tr_destroy(&trtable);

        return EXIT_SUCCESS;
    }

The same as above but using ordinary I<C> strings:

    #include <slack/std.h>
    #include <slack/str.h>

    int main()
    {
        StringTR *trtable = tr_compile("a-zA-Z", "n-za-mN-ZA-M", 0);
        char line[BUFSIZ];

        while (fgets(line, BUFSIZ, stdin))
        {
            int count = tr_compiled(line, trtable);
            printf("%s", line);
        }

        tr_destroy(&trtable);

        return EXIT_SUCCESS;
    }

Perform regular expression matching and substitution:

    #include <slack/std.h>
    #include <slack/str.h>

    int main()
    {
        // Find matches in a String object

        String *str = str_create("abcabcabc");
        List *list = str_regexpr("a((.*)a(.*))a", str, 0, 0);
        regex_t compiled[1];

        while (list_has_next(list))
            printf("%s\n", cstr(list_next(list)));

        str_destroy(&str);
        list_destroy(&list);

        // Find matches in an ordinary C string

        list = regexpr("a((.*)a(.*))a", "abcabcabc", 0, 0);

        while (list_has_next(list) == 1)
            printf("%s\n", cstr(list_next(list)));

        list_destroy(&list);

        // Use a pre-compiled regular expression on a String object

        str = str_create("abcabcabc");

        if (!regexpr_compile(compiled, "a((.*)a(.*))a", 0))
        {
            list = str_regexpr_compiled(compiled, str, 0);
            regexpr_release(compiled);
        }

        while (list_has_next(list) == 1)
            printf("%s\n", cstr(list_next(list)));

        str_destroy(&str);
        list_destroy(&list);

        // Use a pre-compiled regular expression on an ordinary C string

        if (!regexpr_compile(compiled, "a((.*)a(.*))a", 0))
        {
            list = regexpr_compiled(compiled, "abcabcabc", 0);
            regexpr_release(compiled);
        }

        while (list_has_next(list) == 1)
            printf("%s\n", cstr(list_next(list)));

        list_destroy(&list);

        // Perform regular expression substitution on a String object

        str = str_create("abcabcabc");
        str_regsub("a", "z", str, 0, 0, 1);
        printf("%s\n", cstr(str));

        str_destroy(&str);

        // Perform regular expression substitution with a pre-compiled pattern

        str = str_create("abcabcabc");

        if (!regexpr_compile(compiled, "a", 0))
        {
            str_regsub_compiled(compiled, "z", str, 0, 1);
            printf("%s\n", cstr(str));
            regexpr_release(compiled);
        }

        str_destroy(&str);

        return EXIT_SUCCESS;
    }

Format some text in a I<String> object in several different ways:

    #include <slack/std.h>
    #include <slack/str.h>

    int main()
    {
        String *text = str_create("This is a string with\na few little words");
        List *left = str_fmt(text, 20, ALIGN_LEFT);     // or '<'
        List *right = str_fmt(text, 20, ALIGN_RIGHT);   // or '>'
        List *centre = str_fmt(text, 20, ALIGN_CENTRE); // or '|' or ALIGN_CENTER
        List *full = str_fmt(text, 20, ALIGN_FULL);     // or '='

        printf("Left:\n");
        while (list_has_next(left))
            printf("%s\n", cstr(list_next(left)));

        printf("Right:\n");
        while (list_has_next(right))
            printf("%s\n", cstr(list_next(right)));

        printf("Centre:\n");
        while (list_has_next(centre))
            printf("%s\n", cstr(list_next(centre)));

        printf("Full:\n");
        while (list_has_next(full))
            printf("%s\n", cstr(list_next(full)));

        str_destroy(&text);
        list_destroy(&left);
        list_destroy(&right);
        list_destroy(&centre);
        list_destroy(&full);

        return EXIT_SUCCESS;
    }

Perform the same formatting but on an ordinary I<C> string:

    #include <slack/std.h>
    #include <slack/str.h>

    int main()
    {
        char *text = "This is a string with\na few little words";
        List *left = fmt(text, 20, ALIGN_LEFT);     // or '<'
        List *right = fmt(text, 20, ALIGN_RIGHT);   // or '>'
        List *centre = fmt(text, 20, ALIGN_CENTRE); // or '|' or ALIGN_CENTER
        List *full = fmt(text, 20, ALIGN_FULL);     // or '='

        printf("Left:\n");
        while (list_has_next(left))
            printf("%s\n", cstr(list_next(left)));

        printf("Right:\n");
        while (list_has_next(right))
            printf("%s\n", cstr(list_next(right)));

        printf("Centre:\n");
        while (list_has_next(centre))
            printf("%s\n", cstr(list_next(centre)));

        printf("Full:\n");
        while (list_has_next(full))
            printf("%s\n", cstr(list_next(full)));

        list_destroy(&left);
        list_destroy(&right);
        list_destroy(&centre);
        list_destroy(&full);

        return EXIT_SUCCESS;
    }

Split and join a I<String> object without using regular expressions:

    #include <slack/std.h>
    #include <slack/str.h>

    int main()
    {
        String *text = str_create("line1\nline2\nline3\n");
        List *lines = str_split(text, "\n");
        String *copy;

        while (list_has_next(lines))
            printf("%s\n", cstr(list_next(lines)));

        copy = str_join(lines, "\n");
        printf("%s\n", cstr(copy));

        str_destroy(&text);
        str_destroy(&copy);
        list_destroy(&lines);

        return EXIT_SUCCESS;
    }

Split an ordinary I<C> string without using regular expressions:

    #include <slack/std.h>
    #include <slack/str.h>

    int main()
    {
        char *text = "line1\nline2\nline3\n";
        List *lines = split(text, "\n");

        while (list_has_next(lines))
            printf("%s\n", cstr(list_next(lines)));

        list_destroy(&lines);

        return EXIT_SUCCESS;
    }

Split a I<String> object using regular expressions:

    #include <slack/std.h>
    #include <slack/str.h>

    int main()
    {
        String *text = str_create("line1\rline2\r\nline3\n");
        List *lines = str_regexpr_split(text, "(\n|\r|\r\n)", 0, 0);

        while (list_has_next(lines))
            printf("%s\n", cstr(list_next(lines)));

        str_destroy(&text);
        list_destroy(&lines);

        return EXIT_SUCCESS;
    }

Split an ordinary I<C> string using regular expressions:

    #include <slack/std.h>
    #include <slack/str.h>

    int main()
    {
        char *text = "line1\rline2\r\nline3\n";
        List *lines = regexpr_split(text, "(\n|\r|\r\n)", 0, 0);

        while (list_has_next(lines))
            printf("%s\n", cstr(list_next(lines)));

        list_destroy(&lines);

        return EXIT_SUCCESS;
    }

Trim and squeeze I<String> objects:

    #include <slack/std.h>
    #include <slack/str.h>

    int main()
    {
        String *str1 = str_create("   a  b  c   ");
        String *str2 = str_create("   a  b  c   ");
        String *str3 = str_create("   a  b  c   ");
        String *str4 = str_create("   a  b  c   ");

        str_trim(str1);
        str_trim_left(str2);
        str_trim_right(str3);
        str_squeeze(str4);

        printf("'%s'\n", cstr(str1));
        printf("'%s'\n", cstr(str2));
        printf("'%s'\n", cstr(str3));
        printf("'%s'\n", cstr(str4));

        str_destroy(&str1);
        str_destroy(&str2);
        str_destroy(&str3);
        str_destroy(&str4);

        return EXIT_SUCCESS;
    }

Trim and squeeze ordinary I<C> strings:

    #include <slack/std.h>
    #include <slack/str.h>

    int main()
    {
        char *str1 = strdup("   a  b  c   ");
        char *str2 = strdup("   a  b  c   ");
        char *str3 = strdup("   a  b  c   ");
        char *str4 = strdup("   a  b  c   ");

        trim(str1);
        trim_left(str2);
        trim_right(str3);
        squeeze(str4);

        printf("'%s'\n", str1);
        printf("'%s'\n", str2);
        printf("'%s'\n", str3);
        printf("'%s'\n", str4);

        free(str1);
        free(str2);
        free(str3);
        free(str4);

        return EXIT_SUCCESS;
    }

Quote whitespace in a I<String> object:

    #include <slack/std.h>
    #include <slack/str.h>

    int main()
    {
        String *str = str_create("this is\ta\nstring with  whitespaces");
        String *quoted = str_quote(str, " \t\n", '\\');
        String *unquoted = str_unquote(quoted, " \t\n", '\\');

        printf("'%s'\n", cstr(str));
        printf("'%s'\n", cstr(quoted));
        printf("'%s'\n", cstr(unquoted));

        str_destroy(&str);
        str_destroy(&quoted);
        str_destroy(&unquoted);

        return EXIT_SUCCESS;
    }

Quote whitespace in an ordinary I<C> string:

    #include <slack/std.h>
    #include <slack/str.h>

    int main()
    {
        char *str = "this is\ta\nstring with  whitespaces";
        String *quoted = quote(str, " \t\n", '\\');
        String *unquoted = unquote(cstr(quoted), " \t\n", '\\');

        printf("'%s'\n", str);
        printf("'%s'\n", cstr(quoted));
        printf("'%s'\n", cstr(unquoted));

        str_destroy(&quoted);
        str_destroy(&unquoted);

        return EXIT_SUCCESS;
    }

Apply I<C> string literal encoding and decoded to a I<String> object:

    #include <slack/std.h>
    #include <slack/str.h>

    int main()
    {
        String *str = str_create("\a\b\t\n\v\f\rabc123\x16\034");
        String *encoded = str_encode(str, "\a\b\t\n\v\f\r", "abtnvfr", '\\', 1);
        String *decoded = str_decode(str, "\a\b\t\n\v\f\r", "abtnvfr", '\\', 1);

        printf("'%s'\n", cstr(str));
        printf("'%s'\n", cstr(encoded));
        printf("'%s'\n", cstr(decoded));

        str_destroy(&str);
        str_destroy(&encoded);
        str_destroy(&decoded);

        return EXIT_SUCCESS;
    }

Apply I<C> string literal encoding and decoded to an ordinary I<C> string:

    #include <slack/std.h>
    #include <slack/str.h>

    int main()
    {
        char *str = "\a\b\t\n\v\f\rabc123\x16\034";
        String *encoded = encode(str, "\a\b\t\n\v\f\r", "abtnvfr", '\\', 1);
        String *decoded = decode(str, "\a\b\t\n\v\f\r", "abtnvfr", '\\', 1);

        printf("'%s'\n", str);
        printf("'%s'\n", cstr(encoded));
        printf("'%s'\n", cstr(decoded));

        str_destroy(&encoded);
        str_destroy(&decoded);

        return EXIT_SUCCESS;
    }

Get the soundex code of a I<String> object:

    #include <slack/std.h>
    #include <slack/str.h>

    int main()
    {
        String *smith = str_create("Smith");

        printf("%s %d\n", cstr(smith), str_soundex(smith));

        str_destroy(&smith);

        return EXIT_SUCCESS;
    }

Get the soundex code of an ordinary I<C> string:

    #include <slack/std.h>
    #include <slack/str.h>

    int main()
    {
        char *smith = "Smith";

        printf("%s %d\n", smith, soundex(smith));

        return EXIT_SUCCESS;
    }

Convert between upper and lower case in a I<String> object:

    #include <slack/std.h>
    #include <slack/str.h>

    int main()
    {
        String *str = str_create("smith");

        printf("%s\n", cstr(str));

        str_ucfirst(str);
        printf("%s\n", cstr(str));

        str_uc(str);
        printf("%s\n", cstr(str));

        str_lcfirst(str);
        printf("%s\n", cstr(str));

        str_lc(str);
        printf("%s\n", cstr(str));

        str_destroy(&str);

        return EXIT_SUCCESS;
    }

Convert between upper and lower case in an ordinary I<C> string:

    #include <slack/std.h>
    #include <slack/str.h>

    int main()
    {
        char *str = strdup("smith");
        printf("%s\n", str);
        ucfirst(str);
        printf("%s\n", str);
        uc(str);
        printf("%s\n", str);
        lcfirst(str);
        printf("%s\n", str);
        lc(str);
        printf("%s\n", str);

        free(str);

        return EXIT_SUCCESS;
    }

Chomp line ending characters off the end of a I<String> object, and chop a
character off the end of a I<String> object:

    #include <slack/std.h>
    #include <slack/str.h>

    int main()
    {
        String *str = str_create("aaa\r\n");
        int bytes = str_chomp(str);

        printf("'%s' %d\n", cstr(str), bytes);
        bytes = str_chop(str);
        printf("'%s' '%c'\n", cstr(str), bytes);

        str_destroy(&str);

        return EXIT_SUCCESS;
    }

Chomp line ending characters off the end of an ordinary I<C> string, and
chop a character off the end of an ordinary I<C> string:

    #include <slack/std.h>
    #include <slack/str.h>

    int main()
    {
        char *str = strdup("aaa\r\n");
        int bytes = chomp(str);

        printf("'%s' %d\n", str, bytes);
        bytes = chop(str);
        printf("'%s' '%c'\n", str, bytes);

        free(str);

        return EXIT_SUCCESS;
    }

Parse binary, octal, and hexadecimal integers in I<String> objects:

    #include <slack/std.h>
    #include <slack/str.h>

    int main()
    {
        String *b = str_create("0b1010");
        String *h = str_create("0x0a1b2c3d");
        String *o = str_create("0177");

        printf("%d\n", str_bin(b));
        printf("%d\n", str_hex(h));
        printf("%d\n", str_oct(o));
        printf("%d\n", str_oct(h));
        printf("%d\n", str_oct(b));

        str_destroy(&b);
        str_destroy(&h);
        str_destroy(&o);

        return EXIT_SUCCESS;
    }

Parse binary, octal and hexadecimal integers in ordinary I<C> strings:

    #include <slack/std.h>
    #include <slack/str.h>

    int main()
    {
        char *b = "0b1010";
        char *h = "0x0a1b2c3d";
        char *o = "0177";

        printf("%d\n", bin(b));
        printf("%d\n", hex(h));
        printf("%d\n", oct(o));
        printf("%d\n", oct(h));
        printf("%d\n", oct(b));

        return EXIT_SUCCESS;
    }

Examples of versions of some standard string functions with more informative
interfaces:

    #include <slack/std.h>
    #include <slack/str.h>

    int main()
    {
        char *src = "text";
        char dst[BUFSIZ];
        char *pos;

        char *eos = cstrcpy(dst, src);
        printf("length '%s' = %d\n", dst, eos - dst);

        eos = cstrcat(dst, src);
        printf("length '%s' = %d\n", dst, eos - dst);

        eos = cstrchr(dst, 'z');
        printf("length '%s' = %d\n", dst, eos - dst);

        for (pos = dst; *(pos = cstrpbrk(pos, "xyz")); ++pos)
            printf("x|y|z at pos %d in %s\n", pos - dst, dst);

        if (*(pos = cstrrchr(dst, 'x')))
            printf("last x in %s at pos %d\n", dst, pos - dst);
        else
            printf("there is no x in %s\n", dst);

        for (pos = dst; *(pos = cstrstr(pos, "text")); ++pos)
            printf("text at pos %d in %s\n", pos - dst, dst);

        return EXIT_SUCCESS;
    }

Examples of string functions that are supplied if they are not already
present on the local system.

    #include <slack/std.h>
    #include <slack/str.h>

    int main()
    {
        char *str1 = "smith, john";
        char *str2 = "Smith, Mary";
        char *str3 = NULL;
        char buf[16];
        size_t len;

        printf("%d\n", strcasecmp(str1, str2));
        printf("%d\n", strncasecmp(str1, str2, 5));
        printf("%d\n", strncasecmp(str1, str2, 8));

        if (strlcpy(buf, str1, 16) >= 16)
            printf("truncation occurred\n");
        printf("%s\n", buf);

        if (strlcat(buf, str2, 16) >= 16)
            printf("truncation occurred\n");
        printf("%s\n", buf);

        if (strlcpy(buf, str1, 1) >= 1)
            printf("truncation occurred\n");
        printf("%s\n", buf);

        len = strlcpy(NULL, str1, 0);
        printf("%d\n", len);

        len = asprintf(&str3, "test");
        printf("%s %d\n", str3, len);
        free(str3);

        return EXIT_SUCCESS;
    }

=head1 CAVEAT

The C<delim> parameter to the I<split(3)> and I<join(3)> functions is an
ordinary I<C> string, so it can't contain C<nul> characters.

The C<quotable> parameter to the I<quote(3)> and I<unquote(3)> functions is
an ordinary I<C> string, so it can't contain C<nul> characters.

The C<uncoded> and C<coded> parameters to the I<str_encode(3)> and
I<str_decode(3)> functions are ordinary I<C> strings, so they can't contain
C<nul> characters.

=head1 BUGS

Doesn't support multi-byte/widechar strings, UTF8, UNICODE or ISO 10646 but
support can probably be layered over the top of I<String>.

Uses I<malloc(3)> directly. The type of memory used and the allocation
strategy should be decoupled from this code.

=head1 SEE ALSO

I<libslack(3)>,
I<locker(3)>,
I<string(3)>,
I<regcomp(3)>,
I<regexec(3)>,
I<regerror(3)>,
I<regfree(3)>,
I<perlfunc(1)>,
I<perlop(1)>

=head1 AUTHOR

20210220 raf <raf@raf.org>

=cut

*/

#endif

#ifdef TEST

static void str_print(const char *str, size_t length)
{
	const char * const encoded = "\a\b\t\n\v\f\r\\";
	const char * const decoded = "abtnvfr\\";
	const char *code;
	int i;

	printf("\"");

	for (i = 0; i < length + 1; ++i)
	{
		/* printf("[%02x]", (unsigned char)str[i]); */
		if (str[i] && (code = strchr(encoded, (unsigned char)str[i])))
			printf("\\%c", decoded[code - encoded]);
		else
			printf(is_print(str[i]) ? "%c" : "\\%03o", (unsigned char)str[i]);
	}

	printf("\"");
}

#define RD 0
#define WR 1
String *mtstr = NULL;
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
pthread_mutex_t errors_lock = PTHREAD_MUTEX_INITIALIZER;
int debug = 0;
int errors = 0;

int mterror()
{
	pthread_mutex_lock(&mutex);
	++errors;
	pthread_mutex_unlock(&mutex);
	return -1;
}

void *produce(void *arg)
{
	int test = *(int *)arg >> 16;
	int id = *(int *)arg & 0x0000ffff;
	int i;

	if (debug)
		printf("p%d: loop\n", id);

	for (i = 0; i <= lim; ++i)
	{
		char c = 'a' + (i % 26);

		if (debug)
			printf("p%d: str_append %d\n", id, i);

		if (!str_append(mtstr, "%c", c))
			mterror(), printf("Test%d: str_append(mtstr, '\\%o'), failed\n", test, c);

		write(length[WR], "", 1);
	}

	write(barrier[WR], "", 1);
	return NULL;
}

void *consume(void *arg)
{
	int test = *(int *)arg >> 16;
	int id = *(int *)arg & 0x0000ffff;
	int i;
	char ack;

	if (debug)
		printf("c%d: loop\n", id);

	for (i = 0; i < lim; ++i)
	{
		while (read(length[RD], &ack, 1) == -1 && errno == EINTR)
		{}

		if (debug)
			printf("c%d: str_remove\n", id);

		if (!str_remove(mtstr, 0))
			mterror(), printf("Test%d: str_remove(mtstr, 0) failed\n", test);
	}

	if (i != lim)
		mterror(), printf("Test%d: consumer read %d items, not %d\n", test, i, lim);

	write(barrier[WR], "", 1);
	return NULL;
}

void *writer(void *arg)
{
	int test = *(int *)arg >> 16;
	int id = *(int *)arg & 0x0000ffff;
	int i;

	if (debug)
		printf("w%d: loop\n", id);

	for (i = 0; i < lim; ++i)
	{
		String *str;

		if (debug)
			printf("w%d: loop %d/%d\n", id, i, lim / 10);

		if (debug)
			printf("w%d: loop %d/%d wrlock/cstr/set_length/recalc_length\n", id, i, lim / 10);

		if (str_wrlock(mtstr) == -1)
			mterror(), printf("Test%d: str_wrlock(mtstr) failed (%s)\n", test, strerror(errno));
		else
		{
			char *str = cstr(mtstr);
			size_t len = str_length_unlocked(mtstr);

			str[0] = 'a';

			if (str_set_length_unlocked(mtstr, len) == -1)
				mterror(), printf("Test%d: str_set_length_unlocked(mtstr, %d) failed\n", test, (int)len);

			if (str_recalc_length_unlocked(mtstr) == -1)
				mterror(), printf("Test%d: str_recalc_length_unlocked(mtstr) failed\n", test);

			if (str_unlock(mtstr) == -1)
				mterror(), printf("Test%d: str_unlock(mtstr) failed (%s)\n", test, strerror(errno));
		}

		if (debug)
			printf("w%d: loop %d/%d replace\n", id, i, lim / 10);

		if (!str_replace(mtstr, 0, -1, "abc"))
			mterror(), printf("Test%d: str_replace(mtstr, 0, -1, \"abc\") failed\n", test);

		if (debug)
			printf("w%d: loop %d/%d splice\n", id, i, lim / 10);

		if (!(str = str_splice(mtstr, 0, 0)))
			mterror(), printf("Test%d: str_splice(mtstr, 0, 0) failed\n", test);
		else
			str_destroy(&str);

		if (debug)
			printf("w%d: loop %d/%d tr\n", id, i, lim / 10);

		if (str_tr(mtstr, "a-z", "A-Z", 0) == -1)
			mterror(), printf("Test%d: str_tr(mtstr, \"a-z\", \"A-Z\", 0) failed\n", test);

		if (debug)
			printf("w%d: loop %d/%d regsub\n", id, i, lim / 10);

		str_regsub("[a-z]", "A", mtstr, 0, 0, 1);

		if (debug)
			printf("w%d: loop %d/%d trim\n", id, i, lim / 10);

		if (!str_trim(mtstr))
			mterror(), printf("Test%d: str_trim(mtstr) failed\n", test);
		if (!str_trim_left(mtstr))
			mterror(), printf("Test%d: str_trim_left(mtstr) failed\n", test);
		if (!str_trim_right(mtstr))
			mterror(), printf("Test%d: str_trim_right(mtstr) failed\n", test);

		if (debug)
			printf("w%d: loop %d/%d squeeze\n", id, i, lim / 10);

		if (!str_squeeze(mtstr))
			mterror(), printf("Test%d: str_squeeze(mtstr) failed\n", test);

		if (debug)
			printf("w%d: loop %d/%d lc/uc/lcfirst/ucfirst\n", id, i, lim / 10);

		if (!str_lc(mtstr))
			mterror(), printf("Test%d: str_lc(mtstr) failed\n", test);
		if (!str_uc(mtstr))
			mterror(), printf("Test%d: str_uc(mtstr) failed\n", test);
		if (!str_lcfirst(mtstr))
			mterror(), printf("Test%d: str_lcfirst(mtstr) failed\n", test);
		if (!str_ucfirst(mtstr))
			mterror(), printf("Test%d: str_ucfirst(mtstr) failed\n", test);

		if (debug)
			printf("w%d: loop %d/%d chop/chomp\n", id, i, lim / 10);

		if (str_chop(mtstr) == -1)
			mterror(), printf("Test%d: str_chop(mtstr) failed\n", test);
		if (str_chomp(mtstr) == -1)
			mterror(), printf("Test%d: str_chomp(mtstr) failed\n", test);
	}

	write(barrier[WR], "", 1);
	return NULL;
}

void *reader(void *arg)
{
	int test = *(int *)arg >> 16;
	int id = *(int *)arg & 0x0000ffff;
	int i;

	if (debug)
		printf("r%d: loop\n", id);

	for (i = 0; i < lim / 10; ++i)
	{
		String *str;
		List *list;
		size_t length;
		int empty;

		str = str_copy(mtstr);
		if (debug)
			printf("r%d: loop %d/%d str = '%s'\n", id, i, lim / 10, cstr(str));
		str_destroy(&str);

		empty = str_empty(mtstr);
		if (debug)
			printf("r%d: loop %d/%d empty = %d\n", id, i, lim / 10, empty);

		length = str_length(mtstr);
		if (debug)
			printf("r%d: loop %d/%d length = %d\n", id, i, lim / 10, (int)length);

		if (str_rdlock(mtstr) == -1)
			mterror(), printf("Test%d: str_rdlock(mtstr) failed (%s)\n", test, strerror(errno));
		else
		{
			const char *s = cstr(mtstr);

			if (debug)
				printf("r%d: loop %d/%d cstr = \"%s\"\n", id, i, lim / 10, s);

			if (str_unlock(mtstr) == -1)
				mterror(), printf("Test%d: str_unlock(mtstr) failed (%s)\n", test, strerror(errno));
		}

		if (!(str = str_create("")))
			mterror(), printf("Test%d: str_create() failed\n", test);
		else
		{
			if (!str_insert_str(str, 0, mtstr))
				mterror(), printf("Test%d: str_insert_str(str, 0, mtstr) failed\n", test);

			if (!str_replace_str(str, 0, 0, mtstr))
				mterror(), printf("Test%d: str_replace_str(str, 0, 0, mtstr) failed\n", test);

			str_destroy(&str);
		}

		if (!(list = str_fmt(mtstr, 10, ALIGN_FULL)))
			mterror(), printf("Test%d: str_fmt(mtstr, 10, ALIGN_FULL) failed\n", test);
		else
			list_destroy(&list);

		if (!(list = str_split(mtstr, "")))
			mterror(), printf("Test%d: str_split(mtstr, \"\") failed\n", test);
		else
			list_destroy(&list);

		if (!(list = str_regexpr("[a-z]?", mtstr, 0, 0)))
			mterror(), printf("Test%d: str_regexpr(\"[a-z]?\", mtstr, 0, 0) failed\n", test);
		else
			list_destroy(&list);

		if (!(list = str_regexpr_split(mtstr, "()", 0, 0)))
			mterror(), printf("Test%d: str_regexpr_split(mtstr, \"()\") failed\n", test);
		else
			list_destroy(&list);

		if (!(str = str_encode(mtstr, "abcdef", "abcdef", '\\', 1)))
			mterror(), printf("Test%d: str_encode(mtstr, \"abcdef\", \"abcdef\", '\\') failed\n", test);
		else
			str_destroy(&str);

		if (!(str = str_decode(mtstr, "abcdef", "abcdef", '\\', 1)))
			mterror(), printf("Test%d: str_decode(mtstr, \"abcdef\", \"abcdef\", '\\') failed\n", test);
		else
			str_destroy(&str);

		str_bin(mtstr);
		str_hex(mtstr);
		str_oct(mtstr);
	}

	write(barrier[WR], "", 1);
	return NULL;
}

void mt_test(int test, Locker *locker)
{
	if (!(mtstr = str_create_with_locker(locker, NULL)))
		mterror(), printf("Test%d: str_create_with_locker(NULL) failed\n", test);
	else
	{
		static int iid[12] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
		pthread_attr_t attr;
		pthread_t id;
		int i;
		char ack;

		if (pipe(length) == -1 || pipe(barrier) == -1)
		{
			++errors, printf("Test%d: failed to perform test: pipe() failed\n", test);
			return;
		}

		for (i = 0; i < 12; ++i)
		{
			iid[i] &= 0x0000ffff;
			iid[i] |= test << 16;
		}

		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		pthread_create(&id, &attr, produce, iid + 0);
		pthread_create(&id, &attr, produce, iid + 1);
		pthread_create(&id, &attr, produce, iid + 2);
		pthread_create(&id, &attr, consume, iid + 3);
		pthread_create(&id, &attr, consume, iid + 4);
		pthread_create(&id, &attr, consume, iid + 5);
		/*
		pthread_create(&id, &attr, writer,  iid + 6);
		pthread_create(&id, &attr, writer,  iid + 7);
		pthread_create(&id, &attr, writer,  iid + 8);
		*/
		pthread_create(&id, &attr, reader,  iid + 9);
		pthread_create(&id, &attr, reader,  iid + 10);
		pthread_create(&id, &attr, reader,  iid + 11);
		pthread_attr_destroy(&attr);

		for (i = 0; i < 9 /* 12 */; ++i)
			while (read(barrier[RD], &ack, 1) == -1 && errno == EINTR)
			{}

		str_destroy(&mtstr);
		if (mtstr)
			mterror(), printf("Test%d: str_destroy(&mtstr) failed\n", test);

		close(length[RD]);
		close(length[WR]);
		close(barrier[RD]);
		close(barrier[WR]);
	}
}

int main(int ac, char **av)
{
	const char * const testfile = "str_fgetline.test";
	String *a, *b, *c;
	char tst[BUFSIZ], *t;
	List *list;
	StringTR *trtable;
	int i, big, rc;
	FILE *stream;
#ifdef HAVE_REGEX_H
	regex_t re[1];
#endif

	if (ac == 2 && !strcmp(av[1], "help"))
	{
		printf("usage: %s [debug]\n", *av);
		return EXIT_SUCCESS;
	}

	printf("Testing: %s\n", "str");

#define TEST_ACT(i, action) \
	if (!(action)) \
		++errors, printf("Test%d: %s failed\n", (i), (#action));

#define TEST_EQ(i, action, eq) \
	rc = (action); \
	if (!(rc == (eq))) \
		++errors, printf("Test%d: %s failed: returned %x, not %x\n", (i), (#action), rc, (eq));

#define TEST_NE(i, action, ne) \
	if ((action) == (ne)) \
		++errors, printf("Test%d: %s failed: returned %p\n", (i), (#action), (ne));

#define TEST_STR(i, action, str, length, value) \
	TEST_ACT(i, action) \
	CHECK_STR(i, action, str, length, value)

#define TEST_CSTR(i, action, str, length, value) \
	TEST_ACT(i, action) \
	CHECK_CSTR(i, action, str, length, value)

#define TEST_LEN(i, action, str, length) \
	TEST_ACT(i, action) \
	CHECK_LEN(i, action, str, length)

#define CHECK_LEN(i, action, str, length) \
	if (str_length(str) != (length)) \
		++errors, printf("Test%d: %s failed: length %u (not %u)\n", (i), (#action), (unsigned int)str_length(str), (unsigned int)(length));

#define CHECK_LEN2(i, action, str, length, length2) \
	if (str_length(str) != (length) && str_length(str) != (length2)) \
		++errors, printf("Test%d: %s failed: length %u (not %u or %u)\n", (i), (#action), (unsigned int)str_length(str), (unsigned int)(length), (unsigned int)(length2));

#define CHECK_CLEN(i, action, str, length) \
	if (strlen(str) != (length)) \
		++errors, printf("Test%d: %s failed: length %u (not %u)\n", (i), (#action), (unsigned int)strlen(str), (unsigned int)(length));

#define CHECK_VAL(i, action, str, length, value) \
	if (memcmp(cstr(str), (value), str_length(str) + 1)) \
	{ \
		++errors, printf("Test%d: %s failed: returned:\n", (i), (#action)); \
		str_print(cstr(str), str_length(str)); \
		printf("\nnot:\n"); \
		str_print(value, length); \
		printf("\n"); \
	}

#define CHECK_VAL2(i, action, str, length, value, length2, value2) \
	if (memcmp(cstr(str), (value), str_length(str) + 1) && \
		memcmp(cstr(str), (value2), str_length(str) + 1)) \
	{ \
		++errors, printf("Test%d: %s failed: returned:\n", (i), (#action)); \
		str_print(cstr(str), str_length(str)); \
		printf("\nnot:\n"); \
		str_print(value, length); \
		printf("\nor:\n"); \
		str_print(value2, length2); \
		printf("\n"); \
	}

#define CHECK_CVAL(i, action, str, length, value) \
	if (strcmp(str, (value))) \
	{ \
		++errors, printf("Test%d: %s failed: returned:\n", (i), (#action)); \
		str_print(str, length); \
		printf("\nnot:\n"); \
		str_print(value, length); \
		printf("\n"); \
	}

#define CHECK_STR(i, action, str, length, value) \
	CHECK_LEN(i, action, str, length) \
	CHECK_VAL(i, action, str, length, value)

#define CHECK_STR2(i, action, str, length, value, length2, value2) \
	CHECK_LEN2(i, action, str, length, length2) \
	CHECK_VAL2(i, action, str, length, value, length2, value2)

#define CHECK_CSTR(i, action, str, length, value) \
	CHECK_CLEN(i, action, str, length) \
	CHECK_CVAL(i, action, str, length, value)

#define CHECK_LIST_LENGTH(i, action, list, len) \
	if (list_length(list) != (len)) \
		++errors, printf("Test%d: %s failed (length %d, not %d)\n", (i), #action, (int)list_length(list), (len));

#define CHECK_LIST_ITEM(i, action, index, tok) \
	if (list_item(list, index) && memcmp(cstr((String *)list_item(list, index)), tok, str_length((String *)list_item(list, index)) + 1)) \
	{ \
		++errors; \
		printf("Test%d: %s failed (item %d is \"%s\", not \"%s\")\n", i, #action, index, cstr((String *)list_item(list, index)), tok); \
	}

	/* Test create, empty, length, clear, insert, append, prepend */

	TEST_STR(1, a = str_create("This is a test string %s %c %d\n", "abc", 'x', 37), a, 31, "This is a test string abc x 37\n")
	TEST_STR(2, b = str_create(NULL), b, 0, "")
	TEST_ACT(3, str_empty(b))
	TEST_STR(4, str_append(b, "abc"), b, 3, "abc")
	TEST_ACT(5, !str_empty(b))
	TEST_STR(6, str_clear(b), b, 0, "")
	TEST_ACT(7, str_empty(b))
	TEST_STR(8, str_append(b, "abc"), b, 3, "abc")
	TEST_STR(9, str_prepend(b, "def"), b, 6, "defabc")
	TEST_STR(10, str_insert(b, 1, "ghi"), b, 9, "dghiefabc")
	TEST_ACT(11, str_empty(NULL))

	/* Test string copying and destruction */

	TEST_STR(12, c = str_copy(a), c, 31, "This is a test string abc x 37\n")
	TEST_ACT(13, !str_destroy(&c))
	TEST_ACT(14, !str_copy(NULL))

	/* Test str_fgetline() */

#define TEST_FGETLINE1(test_num, test_length, eol) \
	TEST_ACT((test_num), stream = fopen(testfile, "wb")) \
	else \
	{ \
		const size_t length = (test_length); \
		for (i = 0; i < length - 1; ++i) \
			fputc('a' + i % 26, stream); \
		fputs((eol), stream); \
		fclose(stream); \
		TEST_ACT((test_num), stream = fopen(testfile, "r")) \
		else \
		{ \
			String *line = str_fgetline(stream); \
			fclose(stream); \
			if (!line) \
				++errors, printf("Test%d: str_fgetline() failed: returned NULL\n", (test_num)); \
			else \
			{ \
				if (str_length(line) != length) \
					++errors, printf("Test%d: str_fgetline() failed: length is %d, not %d\n", (test_num), (int)str_length(line), (int)length); \
				else \
				{ \
					for (i = 0; i < length - 1; ++i) \
						if (cstr(line)[i] != 'a' + i % 26) \
						{ \
							++errors, printf("Test%d: str_fgetline() failed: pos %d contains '\\x%02x', not '\\x%02x'\n", (test_num), i, cstr(line)[i], 'a' + i % 26); \
							break; \
						} \
					if (cstr(line)[length - 1] != '\n') \
						++errors, printf("Test%d: str_fgetline() failed: last char is '\\x%02x', not '\\x%02x'\n", (test_num), cstr(line)[length - 1], '\n'); \
				} \
				str_destroy(&line); \
			} \
		} \
		unlink(testfile); \
	}

#define TEST_FGETLINE(test_num, test_length) \
	TEST_FGETLINE1((test_num), (test_length), "\n") \
	TEST_FGETLINE1((test_num), (test_length), "\r\n") \
	TEST_FGETLINE1((test_num), (test_length), "\r")

	TEST_FGETLINE(15, 127)
	TEST_FGETLINE(16, BUFSIZ - 1)
	TEST_FGETLINE(17, BUFSIZ)
	TEST_FGETLINE(18, BUFSIZ + 1)
	TEST_FGETLINE(19, (BUFSIZ - 20) * 4)

	TEST_ACT(20, stream = fopen(testfile, "wb"))
	else
	{
		fprintf(stream, "123\n456\r\n789\rabc\r");
		fclose(stream);
		TEST_ACT(20, stream = fopen(testfile, "rb"))
		else
		{
			String *line;
			TEST_ACT(20, line = str_fgetline(stream))
			TEST_ACT(20, strcmp(cstr(line), "123\n") == 0)
			TEST_ACT(20, line = str_fgetline(stream))
			TEST_ACT(20, strcmp(cstr(line), "456\n") == 0)
			TEST_ACT(20, line = str_fgetline(stream))
			TEST_ACT(20, strcmp(cstr(line), "789\n") == 0)
			TEST_ACT(20, line = str_fgetline(stream))
			TEST_ACT(20, strcmp(cstr(line), "abc\n") == 0)
			TEST_ACT(20, !(line = str_fgetline(stream)))
			fclose(stream);
		}
		unlink(testfile);
	}

	/* Test insert, append, prepend, remove, replace */

	TEST_STR(21, str_remove(a, 30), a, 30, "This is a test string abc x 37")
	TEST_STR(22, str_remove(a, 0), a, 29, "his is a test string abc x 37")
	TEST_STR(23, str_remove(a, 10), a, 28, "his is a tst string abc x 37")
	TEST_STR(24, str_replace(a, 0, 0, "123"), a, 31, "123his is a tst string abc x 37")
	TEST_STR(25, str_replace(a, 1, 1, "123"), a, 33, "11233his is a tst string abc x 37")
	TEST_STR(26, str_replace(a, 0, 5, "456"), a, 31, "456his is a tst string abc x 37")
	TEST_STR(27, str_replace(a, 30, 1, "789"), a, 33, "456his is a tst string abc x 3789")
	TEST_STR(28, str_replace(a, 33, 0, "a"), a, 34, "456his is a tst string abc x 3789a")
	TEST_STR(29, str_remove_range(a, 0, 0), a, 34, "456his is a tst string abc x 3789a")
	TEST_STR(30, str_remove_range(a, 0, 3), a, 31, "his is a tst string abc x 3789a")
	TEST_STR(31, str_remove_range(a, 13, 0), a, 31, "his is a tst string abc x 3789a")
	TEST_STR(32, str_remove_range(a, 13, 7), a, 24, "his is a tst abc x 3789a")
	TEST_STR(33, str_remove_range(a, 23, 0), a, 24, "his is a tst abc x 3789a")
	TEST_STR(34, str_remove_range(a, 21, 2), a, 22, "his is a tst abc x 37a")

	TEST_ACT(35, c = str_create("__test__"))

	TEST_STR(36, str_prepend_str(a, c), a, 30, "__test__his is a tst abc x 37a")
	TEST_STR(37, str_insert_str(b, 1, c), b, 17, "d__test__ghiefabc")
	TEST_STR(38, str_append_str(a, b), a, 47, "__test__his is a tst abc x 37ad__test__ghiefabc")

	TEST_STR(39, str_replace_str(a, 1, 2, b), a, 62,  "_d__test__ghiefabcest__his is a tst abc x 37ad__test__ghiefabc")
	TEST_STR(40, str_replace_str(a, 60, 2, b), a, 77, "_d__test__ghiefabcest__his is a tst abc x 37ad__test__ghiefad__test__ghiefabc")
	TEST_STR(41, str_remove_range(b, 1, 3), b, 14, "dest__ghiefabc")

	TEST_STR(42, str_prepend(b, NULL), b, 14, "dest__ghiefabc")
	TEST_STR(43, str_insert(b, 5, NULL), b, 14, "dest__ghiefabc")
	TEST_STR(44, str_append(b, NULL), b, 14, "dest__ghiefabc")
	TEST_STR(45, str_prepend(b, ""), b, 14, "dest__ghiefabc")
	TEST_STR(46, str_insert(b, 5, ""), b, 14, "dest__ghiefabc")
	TEST_STR(47, str_append(b, ""), b, 14, "dest__ghiefabc")
	str_destroy(&a);
	str_destroy(&b);
	str_destroy(&c);

	/* Test relative index/range */

	TEST_STR(48, a = str_create("0123456789"), a, 10, "0123456789")
	TEST_STR(49, str_remove_range(a, -5, -1), a, 6, "012345")
	TEST_STR(50, str_remove_range(a, -1, -1), a, 6, "012345")
	TEST_STR(51, str_remove_range(a, -3, -2), a, 5, "01235")
	TEST_STR(52, str_insert(a, -1, "abc"), a, 8, "01235abc")
	TEST_STR(53, str_replace(a, -5, -2, "XYZ"), a, 8, "0123XYZc")
	TEST_STR(54, b = str_substr(a, -4, -2), b, 2, "YZ")
	TEST_STR(55, c = substr(cstr(a), -4, -2), c, 2, "YZ")
	str_destroy(&a);
	str_destroy(&b);
	str_destroy(&c);

	/* Test str_repeat() */

	TEST_STR(56, c = str_repeat(0, ""), c, 0, "")
	str_destroy(&c);
	TEST_STR(57, c = str_repeat(10, ""), c, 0, "")
	str_destroy(&c);
	TEST_STR(58, c = str_repeat(0, "%d", 11 * 11), c, 0, "")
	str_destroy(&c);
	TEST_STR(59, c = str_repeat(10, "%d", 11 * 11), c, 30, "121121121121121121121121121121")
	str_destroy(&c);
	TEST_STR(60, c = str_repeat(10, " "), c, 10, "          ")
	str_destroy(&c);

	/* Test big string creation and big string growth (big KB) */

	big = 64;
	TEST_LEN(61, a = str_create("%*s", 1024 * big, ""), a, 1024 * big)
	str_destroy(&a);

	TEST_ACT(62, a = str_create(NULL))

	for (i = 0; i < big; ++i)
	{
		if (!str_append(a, "%1024s", ""))
		{
			++errors, printf("Test62: str_append(\"%%1024s\", \"\") failed (on iteration %d)\n", i);
			break;
		}

		if (str_length(a) != ((i + 1) * 1024))
		{
			++errors, printf("Test62: str_append(\"%%1024s\", \"\") failed: (on iteration %d) length %d, not %d\n", i, (int)str_length(a), (i + 1) * 1024);
			break;
		}
	}

	str_destroy(&a);

	/* Test big string sized creation and big string growth (big KB) */

	TEST_LEN(63, a = str_create_sized(1024 * big, "%*s", 1024 * big, ""), a, 1024 * big)
	str_destroy(&a);

	TEST_ACT(64, a = str_create_sized(1024 * big, NULL))

	for (i = 0; i < big; ++i)
	{
		if (!str_append(a, "%1024s", ""))
		{
			++errors, printf("Test64: str_append(\"%%1024s\", \"\") failed (on iteration %d)\n", i);
			break;
		}

		if (str_length(a) != ((i + 1) * 1024))
		{
			++errors, printf("Test64: str_append(\"%%1024s\", \"\") failed: (on iteration %d) length %d, not %d\n", i, (int)str_length(a), (i + 1) * 1024);
			break;
		}
	}

	str_destroy(&a);

	/* Test substr */

	TEST_ACT(65, a = str_create("abcdefghijkl"))
	TEST_STR(66, b = str_substr(a, 0, 0), b, 0, "")
	str_destroy(&b);
	TEST_STR(67, c = str_substr(a, 0, 3), c, 3, "abc")
	str_destroy(&c);
	TEST_STR(68, b = str_substr(a, 6, 0), b, 0, "")
	str_destroy(&b);
	TEST_STR(69, c = str_substr(a, 6, 3), c, 3, "ghi")
	str_destroy(&c);
	TEST_STR(70, b = str_substr(a, 9, 0), b, 0, "")
	str_destroy(&b);
	TEST_STR(71, c = str_substr(a, 9, 3), c, 3, "jkl")
	str_destroy(&c);
	TEST_STR(72, c = str_substr(a, 0, 12), c, 12, "abcdefghijkl")
	str_destroy(&c);
	str_destroy(&a);

	TEST_STR(73, b = substr("abcdefghijkl", 0, 0), b, 0, "")
	str_destroy(&b);
	TEST_STR(74, c = substr("abcdefghijkl", 0, 3), c, 3, "abc")
	str_destroy(&c);
	TEST_STR(75, b = substr("abcdefghijkl", 6, 0), b, 0, "")
	str_destroy(&b);
	TEST_STR(76, c = substr("abcdefghijkl", 6, 3), c, 3, "ghi")
	str_destroy(&c);
	TEST_STR(77, b = substr("abcdefghijkl", 9, 0), b, 0, "")
	str_destroy(&b);
	TEST_STR(78, c = substr("abcdefghijkl", 9, 3), c, 3, "jkl")
	str_destroy(&c);
	TEST_STR(79, c = substr("abcdefghijkl", 0, 12), c, 12, "abcdefghijkl")
	str_destroy(&c);
	str_destroy(&a);

	/* Test splice */

#define TEST_SPLICE(i, action, ostr, olen, oval, nstr, nlen, nval) \
	TEST_ACT(i, action) \
	CHECK_STR(i, action, ostr, olen, oval) \
	CHECK_STR(i, action, nstr, nlen, nval) \
	str_destroy(&(nstr));

	TEST_ACT   (80, a = str_create("aaabbbcccdddeeefffggghhhiiijjjkkklll"))
	TEST_SPLICE(81, b = str_splice(a, 0, 0), a, 36, "aaabbbcccdddeeefffggghhhiiijjjkkklll", b, 0, "")
	TEST_SPLICE(82, b = str_splice(a, 0, 3), a, 33, "bbbcccdddeeefffggghhhiiijjjkkklll", b, 3, "aaa")
	TEST_SPLICE(83, b = str_splice(a, 3, 0), a, 33, "bbbcccdddeeefffggghhhiiijjjkkklll", b, 0, "")
	TEST_SPLICE(84, b = str_splice(a, 3, 6), a, 27, "bbbeeefffggghhhiiijjjkkklll", b, 6, "cccddd")
	TEST_SPLICE(85, b = str_splice(a, 3, 0), a, 27, "bbbeeefffggghhhiiijjjkkklll", b, 0, "")
	TEST_SPLICE(86, b = str_splice(a, 24, 3), a, 24, "bbbeeefffggghhhiiijjjkkk", b, 3, "lll")
	str_destroy(&a);

	/* Test tr */

#define TEST_TR(i, action, orig, str, bytes, length, value) \
	TEST_ACT(i, str = str_copy(orig)) \
	TEST_EQ(i, action, bytes) \
	CHECK_STR(i, action, str, length, value) \
	str_destroy(&(str));

	TEST_ACT(87, a = str_create(" .,;'/0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"))
	TEST_ACT(88, b = str_create("bookkeeper"))
	TEST_TR (89, str_tr(c, "A-Z", "a-z", 0), a, c, 26, 68, " .,;'/0123456789abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz")
	TEST_TR (90, str_tr(c, "a-z", "A-Z", 0), a, c, 26, 68, " .,;'/0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ")
	TEST_TR (91, str_tr(c, "a", "a", 0), a, c, 1, 68, " .,;'/0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ")
	TEST_TR (92, str_tr(c, "0-9", NULL, 0), a, c, 10, 68, " .,;'/0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ")
	TEST_TR (93, str_tr(c, "a-zA-Z", NULL, TR_SQUASH), b, c, 10, 7, "bokeper")
	TEST_TR (94, str_tr(c, "a-zA-Z", " ", TR_SQUASH), b, c, 10, 1, " ")
	TEST_TR (95, str_tr(c, "a-zA-Z", " ", TR_COMPLEMENT | TR_SQUASH), a, c, 16, 53, " abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ")
	TEST_TR (96, str_tr(c, "AAA", "XYZ", 0), a, c, 1, 68, " .,;'/0123456789abcdefghijklmnopqrstuvwxyzXBCDEFGHIJKLMNOPQRSTUVWXYZ")
	TEST_TR (97, str_tr(c, "a-z", "*", 0), a, c, 26, 68, " .,;'/0123456789**************************ABCDEFGHIJKLMNOPQRSTUVWXYZ")
	TEST_TR (98, str_tr(c, "a-z", "*", TR_COMPLEMENT), a, c, 42, 68, "****************abcdefghijklmnopqrstuvwxyz**************************")
	TEST_TR (99, str_tr(c, "a-z", " ", TR_SQUASH), a, c, 26, 43, " .,;'/0123456789 ABCDEFGHIJKLMNOPQRSTUVWXYZ")
	TEST_TR (100, str_tr(c, "a-z", " ", TR_SQUASH | TR_COMPLEMENT), a, c, 42, 28, " abcdefghijklmnopqrstuvwxyz ")
	TEST_TR (101, str_tr(c, "a-z", "x-z", TR_DELETE), a, c, 26, 45, " .,;'/0123456789xyzABCDEFGHIJKLMNOPQRSTUVWXYZ")
	TEST_TR (102, str_tr(c, "a-z", "", TR_DELETE), a, c, 26, 42, " .,;'/0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ")
	TEST_TR (103, str_tr(c, "a-z", "   ", TR_DELETE), a, c, 26, 45, " .,;'/0123456789   ABCDEFGHIJKLMNOPQRSTUVWXYZ")
	TEST_TR (104, str_tr(c, "a-z", "", TR_DELETE | TR_COMPLEMENT), a, c, 42, 26, "abcdefghijklmnopqrstuvwxyz")
	TEST_TR (105, str_tr(c, "a-z", "                                                           ", TR_DELETE | TR_COMPLEMENT), a, c, 42, 41, "               abcdefghijklmnopqrstuvwxyz")
	TEST_TR (106, str_tr(c, "a-z", "                                                           ", TR_DELETE | TR_COMPLEMENT | TR_SQUASH), a, c, 42, 27, " abcdefghijklmnopqrstuvwxyz")
	TEST_TR (107, str_tr(c, "a-z", "             ", TR_DELETE), a, c, 26, 55, " .,;'/0123456789             ABCDEFGHIJKLMNOPQRSTUVWXYZ")
	TEST_TR (108, str_tr(c, "a-z", "             ", TR_DELETE | TR_SQUASH), a, c, 26, 43, " .,;'/0123456789 ABCDEFGHIJKLMNOPQRSTUVWXYZ")
	TEST_TR (109, str_tr(c, "a-z", "a-b", 0), a, c, 26, 68, " .,;'/0123456789abbbbbbbbbbbbbbbbbbbbbbbbbABCDEFGHIJKLMNOPQRSTUVWXYZ")
	TEST_TR (110, str_tr(c, "a-zA-Z", "A-Za-z", 0), a, c, 52, 68, " .,;'/0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz")
	TEST_TR (111, str_tr(c, "a-zA-Z", "A-Za-z", TR_COMPLEMENT), a, c, 16, 68, "gusznvwxyzzzzzzzabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ")
	TEST_TR (112, str_tr(c, "A-Z", "a-m", 0), a, c, 26, 68, " .,;'/0123456789abcdefghijklmnopqrstuvwxyzabcdefghijklmmmmmmmmmmmmmm")
	TEST_TR (113, str_tr(c, "A-Z", "a-m", TR_SQUASH), a, c, 26, 55, " .,;'/0123456789abcdefghijklmnopqrstuvwxyzabcdefghijklm")
	TEST_TR (114, str_tr(c, "a-zA-Z", "n-za-mN-ZA-M", 0), a, c, 52, 68, " .,;'/0123456789nopqrstuvwxyzabcdefghijklmNOPQRSTUVWXYZABCDEFGHIJKLM")
	str_destroy(&a);
	str_destroy(&b);

#define TEST_TR_DIRECT(i, action, str, bytes, length, value) \
	TEST_EQ(i, action, bytes) \
	CHECK_STR(i, action, str, length, value)

	big = 1024;
	TEST_ACT(115, a = str_create("abcDEFghiJKLmnoPQRstuVWXyz"))
	TEST_ACT(115, trtable = tr_compile("a-zA-Z", "A-Za-z", 0))
	for (i = 0; i < big; ++i)
	{
		TEST_TR_DIRECT(115, tr_compiled(a->str, trtable), a, 26, 26, "ABCdefGHIjklMNOpqrSTUvwxYZ")
		TEST_TR_DIRECT(115, tr_compiled(a->str, trtable), a, 26, 26, "abcDEFghiJKLmnoPQRstuVWXyz")
	}
	TEST_ACT(115, !tr_destroy(&trtable))
	str_destroy(&a);

	TEST_ACT(116, a = str_create("\170\171\172\173\174\175\176\177\200\201\202\203\204\205\206\207\210"))
	TEST_ACT(116, b = str_create("\200-\377"))
	TEST_ACT(116, c = str_create("%c-\177", '\000'))
	TEST_TR_DIRECT(116, str_tr_str(a, b, c, 0), a, 9, 17, "\170\171\172\173\174\175\176\177\000\001\002\003\004\005\006\007\010")
	str_destroy(&a);
	str_destroy(&b);
	str_destroy(&c);

#ifdef HAVE_REGEX_H

	/* Test str_regexpr */

	TEST_ACT(117, a = str_create("abcabcabc"))
	else
	{
		TEST_ACT(117, list = str_regexpr("a((.*)a(.*))a", a, 0, 0))
		else
		{
			CHECK_LIST_LENGTH(117, str_regexpr(), list, 4)
			CHECK_LIST_ITEM(117, str_regexpr("a((.*)a(.*))a", "abcabcabc"), 0, "abcabca")
			CHECK_LIST_ITEM(117, str_regexpr("a((.*)a(.*))a", "abcabcabc"), 1, "bcabc")
			CHECK_LIST_ITEM(117, str_regexpr("a((.*)a(.*))a", "abcabcabc"), 2, "bc")
			CHECK_LIST_ITEM(117, str_regexpr("a((.*)a(.*))a", "abcabcabc"), 3, "bc")
			list_destroy(&list);
		}

		str_destroy(&a);
	}

	/* Test str_regsub */

#define TEST_REGSUB(i, str, pat, rep, cflags, eflags, all, len, res) \
	TEST_ACT((i), a = str_create("%s", (str))) \
	else \
	{ \
		TEST_STR((i), str_regsub((pat), (rep), a, (cflags), (eflags), (all)), a, (len), (res)) \
		str_destroy(&a); \
	}

	TEST_REGSUB(118, "xabcabcabcx", "a((.*)a(.*))a", "$0", 0, 0, 0, 11, "xabcabcabcx")
	TEST_REGSUB(119, "xabcabcabcx", "a((.*)a(.*))a", "$$$2${3}!", 0, 0, 0, 10, "x$bcbc!bcx")
	TEST_REGSUB(120, "xabcabcabcx", "a((.*)a(.*))a", "$$$2$31!", 0, 0, 0, 11, "x$bcbc1!bcx")
	TEST_REGSUB(121, "xabcabcabcx", "a", "z", 0, 0, 0, 11, "xzbcabcabcx")
	TEST_REGSUB(122, "xabcabcabcx", "a", "z", 0, 0, 1, 11, "xzbczbczbcx")
	TEST_REGSUB(123, "aba", "a", "z", 0, 0, 0, 3, "zba")
	TEST_REGSUB(124, "aba", "a", "z", 0, 0, 1, 3, "zbz")
	TEST_REGSUB(125, "xabcabcabcx", "((((((((((((((((((((((((((((((((a))))))))))))))))))))))))))))))))", "!${32}!", 0, 0, 0, 13, "x!a!bcabcabcx")
	TEST_REGSUB(126, "xabcabcabcx", "((((((((((((((((((((((((((((((((a))))))))))))))))))))))))))))))))", "!${32}!", 0, 0, 1, 17, "x!a!bc!a!bc!a!bcx")
	TEST_REGSUB(127, "\\a:b:c:d:e:f:G:H:I:", "(...)(..)(..)(..)(..)(..)(..)(..)(..)", "$1\\U$2\\Q$3\\l$4\\E$5\\E$6\\L$7\\Q\\u$8\\E\\E$9\\\\l", 0, 0, 0, 24, "\\a:B:C\\:d\\:E:f:g:H\\:I:\\l")
	TEST_REGSUB(128, "abcdef", "()", "-", 0, 0, 0, 7, "-abcdef")
	TEST_REGSUB(129, "abcdef", "()", "-", 0, 0, 1, 13, "-a-b-c-d-e-f-")

#endif

	/* Test fmt */

	/* Opening paragraphs from Silas Marner by George Eliot */

	TEST_ACT(130, a = str_create(
		"In the days when the spinning wheels hummed busily in the\n"
		"farmhouses and even the great ladies, clothed in silk and\n"
		"thread lace, had their toy spinning wheels of polished oak,\n"
		"there might be seen in districts far away among the lanes,\n"
		"or deep in the bosom of the hills, certain pallid undersized\n"
		"men, who, by the size of the brawny country folk, looked like\n"
		"the remnants of a disinherited race. The shepherd's dog barked\n"
		"fiercely when one of these alien-looking men appeared on the\n"
		"upland, dark against the early winter sunset; for what dog\n"
		"likes a figure bent under a heavy bag? And these pale men rarely\n"
		"stirred abroad without that mysterious burden. The shepherd\n"
		"himself, though he had good reason to believe that the bag held\n"
		"nothing but flaxen thread, or else the long rolls of strong linen\n"
		"spun from that thread, was not quite sure that this trade of\n"
		"weaving, indispensable though it was, could be carried on\n"
		"entirely without the help of the Evil One.\n"
	))

	TEST_ACT(130, list = str_fmt(a, 70, ALIGN_LEFT))
	CHECK_LIST_LENGTH(130, str_fmt(a, 70, ALIGN_LEFT), list, 14)
	CHECK_LIST_ITEM(130, str_fmt(a, 70, ALIGN_LEFT), 0,  "In the days when the spinning wheels hummed busily in the farmhouses")
	CHECK_LIST_ITEM(130, str_fmt(a, 70, ALIGN_LEFT), 1,  "and even the great ladies, clothed in silk and thread lace, had their")
	CHECK_LIST_ITEM(130, str_fmt(a, 70, ALIGN_LEFT), 2,  "toy spinning wheels of polished oak, there might be seen in districts")
	CHECK_LIST_ITEM(130, str_fmt(a, 70, ALIGN_LEFT), 3,  "far away among the lanes, or deep in the bosom of the hills, certain")
	CHECK_LIST_ITEM(130, str_fmt(a, 70, ALIGN_LEFT), 4,  "pallid undersized men, who, by the size of the brawny country folk,")
	CHECK_LIST_ITEM(130, str_fmt(a, 70, ALIGN_LEFT), 5,  "looked like the remnants of a disinherited race. The shepherd's dog")
	CHECK_LIST_ITEM(130, str_fmt(a, 70, ALIGN_LEFT), 6,  "barked fiercely when one of these alien-looking men appeared on the")
	CHECK_LIST_ITEM(130, str_fmt(a, 70, ALIGN_LEFT), 7,  "upland, dark against the early winter sunset; for what dog likes a")
	CHECK_LIST_ITEM(130, str_fmt(a, 70, ALIGN_LEFT), 8,  "figure bent under a heavy bag? And these pale men rarely stirred")
	CHECK_LIST_ITEM(130, str_fmt(a, 70, ALIGN_LEFT), 9,  "abroad without that mysterious burden. The shepherd himself, though he")
	CHECK_LIST_ITEM(130, str_fmt(a, 70, ALIGN_LEFT), 10, "had good reason to believe that the bag held nothing but flaxen")
	CHECK_LIST_ITEM(130, str_fmt(a, 70, ALIGN_LEFT), 11, "thread, or else the long rolls of strong linen spun from that thread,")
	CHECK_LIST_ITEM(130, str_fmt(a, 70, ALIGN_LEFT), 12, "was not quite sure that this trade of weaving, indispensable though it")
	CHECK_LIST_ITEM(130, str_fmt(a, 70, ALIGN_LEFT), 13, "was, could be carried on entirely without the help of the Evil One.")
	list_destroy(&list);

	TEST_ACT(131, list = str_fmt(a, 70, ALIGN_RIGHT))
	CHECK_LIST_LENGTH(131, str_fmt(a, 70, ALIGN_RIGHT), list, 14)
	CHECK_LIST_ITEM(131, str_fmt(a, 70, ALIGN_RIGHT), 0,  "  In the days when the spinning wheels hummed busily in the farmhouses")
	CHECK_LIST_ITEM(131, str_fmt(a, 70, ALIGN_RIGHT), 1,  " and even the great ladies, clothed in silk and thread lace, had their")
	CHECK_LIST_ITEM(131, str_fmt(a, 70, ALIGN_RIGHT), 2,  " toy spinning wheels of polished oak, there might be seen in districts")
	CHECK_LIST_ITEM(131, str_fmt(a, 70, ALIGN_RIGHT), 3,  "  far away among the lanes, or deep in the bosom of the hills, certain")
	CHECK_LIST_ITEM(131, str_fmt(a, 70, ALIGN_RIGHT), 4,  "   pallid undersized men, who, by the size of the brawny country folk,")
	CHECK_LIST_ITEM(131, str_fmt(a, 70, ALIGN_RIGHT), 5,  "   looked like the remnants of a disinherited race. The shepherd's dog")
	CHECK_LIST_ITEM(131, str_fmt(a, 70, ALIGN_RIGHT), 6,  "   barked fiercely when one of these alien-looking men appeared on the")
	CHECK_LIST_ITEM(131, str_fmt(a, 70, ALIGN_RIGHT), 7,  "    upland, dark against the early winter sunset; for what dog likes a")
	CHECK_LIST_ITEM(131, str_fmt(a, 70, ALIGN_RIGHT), 8,  "      figure bent under a heavy bag? And these pale men rarely stirred")
	CHECK_LIST_ITEM(131, str_fmt(a, 70, ALIGN_RIGHT), 9,  "abroad without that mysterious burden. The shepherd himself, though he")
	CHECK_LIST_ITEM(131, str_fmt(a, 70, ALIGN_RIGHT), 10, "       had good reason to believe that the bag held nothing but flaxen")
	CHECK_LIST_ITEM(131, str_fmt(a, 70, ALIGN_RIGHT), 11, " thread, or else the long rolls of strong linen spun from that thread,")
	CHECK_LIST_ITEM(131, str_fmt(a, 70, ALIGN_RIGHT), 12, "was not quite sure that this trade of weaving, indispensable though it")
	CHECK_LIST_ITEM(131, str_fmt(a, 70, ALIGN_RIGHT), 13, "   was, could be carried on entirely without the help of the Evil One.")
	list_destroy(&list);

	TEST_ACT(132, list = str_fmt(a, 70, ALIGN_CENTRE));
	CHECK_LIST_LENGTH(132, str_fmt(a, 70, ALIGN_CENTRE), list, 16)
	CHECK_LIST_ITEM(132, str_fmt(a, 70, ALIGN_CENTRE), 0,  "      In the days when the spinning wheels hummed busily in the")
	CHECK_LIST_ITEM(132, str_fmt(a, 70, ALIGN_CENTRE), 1,  "      farmhouses and even the great ladies, clothed in silk and")
	CHECK_LIST_ITEM(132, str_fmt(a, 70, ALIGN_CENTRE), 2,  "     thread lace, had their toy spinning wheels of polished oak,")
	CHECK_LIST_ITEM(132, str_fmt(a, 70, ALIGN_CENTRE), 3,  "      there might be seen in districts far away among the lanes,")
	CHECK_LIST_ITEM(132, str_fmt(a, 70, ALIGN_CENTRE), 4,  "     or deep in the bosom of the hills, certain pallid undersized")
	CHECK_LIST_ITEM(132, str_fmt(a, 70, ALIGN_CENTRE), 5,  "    men, who, by the size of the brawny country folk, looked like")
	CHECK_LIST_ITEM(132, str_fmt(a, 70, ALIGN_CENTRE), 6,  "    the remnants of a disinherited race. The shepherd's dog barked")
	CHECK_LIST_ITEM(132, str_fmt(a, 70, ALIGN_CENTRE), 7,  "     fiercely when one of these alien-looking men appeared on the")
	CHECK_LIST_ITEM(132, str_fmt(a, 70, ALIGN_CENTRE), 8,  "      upland, dark against the early winter sunset; for what dog")
	CHECK_LIST_ITEM(132, str_fmt(a, 70, ALIGN_CENTRE), 9,  "   likes a figure bent under a heavy bag? And these pale men rarely")
	CHECK_LIST_ITEM(132, str_fmt(a, 70, ALIGN_CENTRE), 10, "     stirred abroad without that mysterious burden. The shepherd")
	CHECK_LIST_ITEM(132, str_fmt(a, 70, ALIGN_CENTRE), 11, "   himself, though he had good reason to believe that the bag held")
	CHECK_LIST_ITEM(132, str_fmt(a, 70, ALIGN_CENTRE), 12, "  nothing but flaxen thread, or else the long rolls of strong linen")
	CHECK_LIST_ITEM(132, str_fmt(a, 70, ALIGN_CENTRE), 13, "     spun from that thread, was not quite sure that this trade of")
	CHECK_LIST_ITEM(132, str_fmt(a, 70, ALIGN_CENTRE), 14, "      weaving, indispensable though it was, could be carried on")
	CHECK_LIST_ITEM(132, str_fmt(a, 70, ALIGN_CENTRE), 15, "              entirely without the help of the Evil One.")
	list_destroy(&list);

	TEST_ACT(133, list = str_fmt(a, 70, ALIGN_FULL))
	CHECK_LIST_LENGTH(133, str_fmt(a, 70, ALIGN_FULL), list, 14)
	CHECK_LIST_ITEM(133, str_fmt(a, 70, ALIGN_FULL), 0,  "In the days when the spinning wheels hummed busily in  the  farmhouses")
	CHECK_LIST_ITEM(133, str_fmt(a, 70, ALIGN_FULL), 1,  "and even the great ladies, clothed in silk and thread lace, had  their")
	CHECK_LIST_ITEM(133, str_fmt(a, 70, ALIGN_FULL), 2,  "toy spinning wheels of polished oak, there might be seen in  districts")
	CHECK_LIST_ITEM(133, str_fmt(a, 70, ALIGN_FULL), 3,  "far away among the lanes, or deep in the bosom of the  hills,  certain")
	CHECK_LIST_ITEM(133, str_fmt(a, 70, ALIGN_FULL), 4,  "pallid undersized men, who, by the size of the  brawny  country  folk,")
	CHECK_LIST_ITEM(133, str_fmt(a, 70, ALIGN_FULL), 5,  "looked like the remnants of a disinherited race.  The  shepherd's  dog")
	CHECK_LIST_ITEM(133, str_fmt(a, 70, ALIGN_FULL), 6,  "barked fiercely when one of these alien-looking men  appeared  on  the")
	CHECK_LIST_ITEM(133, str_fmt(a, 70, ALIGN_FULL), 7,  "upland, dark against the early winter sunset; for  what  dog  likes  a")
	CHECK_LIST_ITEM(133, str_fmt(a, 70, ALIGN_FULL), 8,  "figure bent under a heavy bag?  And  these  pale  men  rarely  stirred")
	CHECK_LIST_ITEM(133, str_fmt(a, 70, ALIGN_FULL), 9,  "abroad without that mysterious burden. The shepherd himself, though he")
	CHECK_LIST_ITEM(133, str_fmt(a, 70, ALIGN_FULL), 10, "had good reason to believe  that  the  bag  held  nothing  but  flaxen")
	CHECK_LIST_ITEM(133, str_fmt(a, 70, ALIGN_FULL), 11, "thread, or else the long rolls of strong linen spun from that  thread,")
	CHECK_LIST_ITEM(133, str_fmt(a, 70, ALIGN_FULL), 12, "was not quite sure that this trade of weaving, indispensable though it")
	CHECK_LIST_ITEM(133, str_fmt(a, 70, ALIGN_FULL), 13, "was, could be carried on entirely without the help of the Evil One.")
	list_destroy(&list);
	str_destroy(&a);

#define TEST_FMT3(i, alignment, width, line1, line2, line3) \
	TEST_ACT(i, list = str_fmt(a, width, alignment)) \
	CHECK_LIST_LENGTH(i, str_fmt(a, width, alignment), list, 3) \
	CHECK_LIST_ITEM(i, str_fmt(a, width, alignment), 0, line1) \
	CHECK_LIST_ITEM(i, str_fmt(a, width, alignment), 1, line2) \
	CHECK_LIST_ITEM(i, str_fmt(a, width, alignment), 2, line3) \
	list_destroy(&list);

	TEST_ACT(134, a = str_create("123456789\n1234567890\n12345678901"))
	TEST_FMT3(135, '<', 10, "123456789", "1234567890", "12345678901")
	TEST_FMT3(136, '>', 10, " 123456789", "1234567890", "12345678901")
	TEST_FMT3(137, '=', 10, "123456789", "1234567890", "12345678901")
	TEST_FMT3(138, '|', 10, "123456789", "1234567890", "12345678901")
	str_destroy(&a);

	TEST_ACT(139, a = str_create("12345678901\n123456789\n1234567890"))
	TEST_FMT3(140, '<', 10, "12345678901", "123456789", "1234567890")
	TEST_FMT3(141, '>', 10, "12345678901", " 123456789", "1234567890")
	TEST_FMT3(142, '=', 10, "12345678901", "123456789", "1234567890")
	TEST_FMT3(143, '|', 10, "12345678901", "123456789", "1234567890")
	str_destroy(&a);

	TEST_ACT(144, a = str_create("1234567890\n12345678901\n123456789"))
	TEST_FMT3(145, '<', 10, "1234567890", "12345678901", "123456789")
	TEST_FMT3(146, '>', 10, "1234567890", "12345678901", " 123456789")
	TEST_FMT3(147, '=', 10, "1234567890", "12345678901", "123456789")
	TEST_FMT3(148, '|', 10, "1234567890", "12345678901", "123456789")
	str_destroy(&a);

	TEST_ACT(149, a = str_create("12345678901\n1234567890\n123456789"))
	TEST_FMT3(150, '<', 10, "12345678901", "1234567890", "123456789")
	TEST_FMT3(151, '>', 10, "12345678901", "1234567890", " 123456789")
	TEST_FMT3(152, '=', 10, "12345678901", "1234567890", "123456789")
	TEST_FMT3(153, '|', 10, "12345678901", "1234567890", "123456789")
	str_destroy(&a);

	TEST_ACT(154, a = str_create("1234567890\n123456789\n12345678901"))
	TEST_FMT3(155, '<', 10, "1234567890", "123456789", "12345678901")
	TEST_FMT3(156, '>', 10, "1234567890", " 123456789", "12345678901")
	TEST_FMT3(157, '=', 10, "1234567890", "123456789", "12345678901")
	TEST_FMT3(158, '|', 10, "1234567890", "123456789", "12345678901")
	str_destroy(&a);

	TEST_ACT(159, a = str_create("123456789\n12345678901\n1234567890"))
	TEST_FMT3(160, '<', 10, "123456789", "12345678901", "1234567890")
	TEST_FMT3(161, '>', 10, " 123456789", "12345678901", "1234567890")
	TEST_FMT3(162, '=', 10, "123456789", "12345678901", "1234567890")
	TEST_FMT3(163, '|', 10, "123456789", "12345678901", "1234567890")
	str_destroy(&a);

	TEST_ACT(164, a = str_create(
		"http://www.zip.com.au/~joe/fairly/long/testing/url/\n"
		"hello there\n"
		"http://www.zip.com.au/~joe/fairly/long/testing/url/\n"
	))
	TEST_FMT3(165, '<', 30, "http://www.zip.com.au/~joe/fairly/long/testing/url/", "hello there", "http://www.zip.com.au/~joe/fairly/long/testing/url/")
	TEST_FMT3(166, '>', 30, "http://www.zip.com.au/~joe/fairly/long/testing/url/", "                   hello there", "http://www.zip.com.au/~joe/fairly/long/testing/url/")
	TEST_FMT3(167, '=', 30, "http://www.zip.com.au/~joe/fairly/long/testing/url/", "hello                    there", "http://www.zip.com.au/~joe/fairly/long/testing/url/")
	TEST_FMT3(168, '|', 30, "http://www.zip.com.au/~joe/fairly/long/testing/url/", "         hello there", "http://www.zip.com.au/~joe/fairly/long/testing/url/")
	str_destroy(&a);

#define TEST_FMT0(i, alignment, width) \
	TEST_ACT(i, list = str_fmt(a, width, alignment)) \
	CHECK_LIST_LENGTH(i, str_fmt(a, width, alignment), list, 0) \
	list_destroy(&list);

	TEST_ACT(169, a = str_create(""))
	TEST_FMT0(170, '<', 10)
	TEST_FMT0(171, '>', 10)
	TEST_FMT0(172, '=', 10)
	TEST_FMT0(173, '|', 10)
	str_destroy(&a);

	/* Test split */

#define CHECK_SPLIT_ITEM(i, func, str, delim, index, tok) \
	if (list_item(list, index) && memcmp(cstr((String *)list_item(list, index)), tok, str_length((String *)list_item(list, index)) + 1)) \
	{ \
		++errors; \
		printf("Test%d: %s(", i, #func); \
		str_print(str, strlen(str)); \
		printf(", "); \
		str_print(delim, strlen(delim)); \
		printf(") failed (token %d is ", index); \
		str_print(cstr((String *)list_item(list, index)), str_length((String *)list_item(list, index))); \
		printf(", not \"%s\")\n", tok); \
	}

#define TEST_SSPLIT(i, func, str, delim, tok1, tok2, tok3, tok4) \
	TEST_ACT(i, a = str_create(str)) \
	TEST_ACT(i, list = func(a, delim)) \
	CHECK_LIST_LENGTH(i, str_split(str, delim), list, 4) \
	CHECK_SPLIT_ITEM(i, func, str, delim, 0, tok1) \
	CHECK_SPLIT_ITEM(i, func, str, delim, 1, tok2) \
	CHECK_SPLIT_ITEM(i, func, str, delim, 2, tok3) \
	CHECK_SPLIT_ITEM(i, func, str, delim, 3, tok4) \
	str_destroy(&a); \
	list_destroy(&list);

#define TEST_SPLIT(i, func, str, delim, tok1, tok2, tok3, tok4) \
	TEST_ACT(i, list = func(str, delim)) \
	CHECK_LIST_LENGTH(i, split(str, delim), list, 4) \
	CHECK_SPLIT_ITEM(i, func, str, delim, 0, tok1) \
	CHECK_SPLIT_ITEM(i, func, str, delim, 1, tok2) \
	CHECK_SPLIT_ITEM(i, func, str, delim, 2, tok3) \
	CHECK_SPLIT_ITEM(i, func, str, delim, 3, tok4) \
	list_destroy(&list);

	TEST_SSPLIT(174, str_split, "a,b.c;d", " ,.;:", "a", "b", "c", "d")
	TEST_SSPLIT(175, str_split, " a , b . c ; d ", " ,.;:", "a", "b", "c", "d")
	TEST_SSPLIT(176, str_split, " a ,b . c;d: ", " ,.;:", "a", "b", "c", "d")
	TEST_SSPLIT(177, str_split, "a,b.c;d:", " ,.;:", "a", "b", "c", "d")
	TEST_SSPLIT(178, str_split, " a,b.c;d:", " ,.;:", "a", "b", "c", "d")
	TEST_SSPLIT(179, str_split, "aa,bb.cc;dd", " ,.;:", "aa", "bb", "cc", "dd")
	TEST_SSPLIT(180, str_split, " aa , bb . cc ; dd ", " ,.;:", "aa", "bb", "cc", "dd")
	TEST_SSPLIT(181, str_split, " aa ,bb . cc;dd: ", " ,.;:", "aa", "bb", "cc", "dd")
	TEST_SSPLIT(182, str_split, "aa,bb.cc;dd:", " ,.;:", "aa", "bb", "cc", "dd")
	TEST_SSPLIT(183, str_split, " aa,bb.cc;dd:", " ,.;:", "aa", "bb", "cc", "dd")
	TEST_SSPLIT(184, str_split, "abcd", "", "a", "b", "c", "d")

	TEST_SPLIT(185, split, "a,b.c;d", " ,.;:", "a", "b", "c", "d")
	TEST_SPLIT(186, split, " a , b . c ; d ", " ,.;:", "a", "b", "c", "d")
	TEST_SPLIT(187, split, " a ,b . c;d: ", " ,.;:", "a", "b", "c", "d")
	TEST_SPLIT(188, split, "a,b.c;d:", " ,.;:", "a", "b", "c", "d")
	TEST_SPLIT(189, split, " a,b.c;d:", " ,.;:", "a", "b", "c", "d")
	TEST_SPLIT(190, split, "aa,bb.cc;dd", " ,.;:", "aa", "bb", "cc", "dd")
	TEST_SPLIT(191, split, " aa , bb . cc ; dd ", " ,.;:", "aa", "bb", "cc", "dd")
	TEST_SPLIT(192, split, " aa ,bb . cc;dd: ", " ,.;:", "aa", "bb", "cc", "dd")
	TEST_SPLIT(193, split, "aa,bb.cc;dd:", " ,.;:", "aa", "bb", "cc", "dd")
	TEST_SPLIT(194, split, " aa,bb.cc;dd:", " ,.;:", "aa", "bb", "cc", "dd")
	TEST_SPLIT(195, split, "abcd", "", "a", "b", "c", "d")

#ifdef HAVE_REGEX_H

	/* Test regexpr_split */

#define TEST_RE_SSPLIT(i, func, str, delim, tok1, tok2, tok3, tok4) \
	TEST_ACT(i, a = str_create(str)) \
	TEST_ACT(i, list = func(a, delim, 0, 0)) \
	CHECK_LIST_LENGTH(i, str_split(str, delim), list, 4) \
	CHECK_SPLIT_ITEM(i, func, str, delim, 0, tok1) \
	CHECK_SPLIT_ITEM(i, func, str, delim, 1, tok2) \
	CHECK_SPLIT_ITEM(i, func, str, delim, 2, tok3) \
	CHECK_SPLIT_ITEM(i, func, str, delim, 3, tok4) \
	str_destroy(&a); \
	list_destroy(&list);

#define TEST_RE_SPLIT(i, func, str, delim, tok1, tok2, tok3, tok4) \
	TEST_ACT(i, list = func(str, delim, 0, 0)) \
	CHECK_LIST_LENGTH(i, split(str, delim), list, 4) \
	CHECK_SPLIT_ITEM(i, func, str, delim, 0, tok1) \
	CHECK_SPLIT_ITEM(i, func, str, delim, 1, tok2) \
	CHECK_SPLIT_ITEM(i, func, str, delim, 2, tok3) \
	CHECK_SPLIT_ITEM(i, func, str, delim, 3, tok4) \
	list_destroy(&list);

	TEST_RE_SSPLIT(196, str_regexpr_split, "a,b.c;d", "[ ,.;:]+", "a", "b", "c", "d")
	TEST_RE_SSPLIT(197, str_regexpr_split, " a , b . c ; d ", "[ ,.;:]+", "a", "b", "c", "d")
	TEST_RE_SSPLIT(198, str_regexpr_split, " a ,b . c;d: ", "[ ,.;:]+", "a", "b", "c", "d")
	TEST_RE_SSPLIT(199, str_regexpr_split, "a,b.c;d:", "[ ,.;:]+", "a", "b", "c", "d")
	TEST_RE_SSPLIT(200, str_regexpr_split, " a,b.c;d:", "[ ,.;:]+", "a", "b", "c", "d")
	TEST_RE_SSPLIT(201, str_regexpr_split, "aa,bb.cc;dd", "[ ,.;:]+", "aa", "bb", "cc", "dd")
	TEST_RE_SSPLIT(202, str_regexpr_split, " aa , bb . cc ; dd ", "[ ,.;:]+", "aa", "bb", "cc", "dd")
	TEST_RE_SSPLIT(203, str_regexpr_split, " aa ,bb . cc;dd: ", "[ ,.;:]+", "aa", "bb", "cc", "dd")
	TEST_RE_SSPLIT(204, str_regexpr_split, "aa,bb.cc;dd:", "[ ,.;:]+", "aa", "bb", "cc", "dd")
	TEST_RE_SSPLIT(205, str_regexpr_split, " aa,bb.cc;dd:", "[ ,.;:]+", "aa", "bb", "cc", "dd")
	TEST_RE_SSPLIT(206, str_regexpr_split, "abcd", "()", "a", "b", "c", "d")

	TEST_RE_SPLIT(207, regexpr_split, "a,b.c;d", "[ ,.;:]+", "a", "b", "c", "d")
	TEST_RE_SPLIT(208, regexpr_split, " a , b . c ; d ", "[ ,.;:]+", "a", "b", "c", "d")
	TEST_RE_SPLIT(209, regexpr_split, " a ,b . c;d: ", "[ ,.;:]+", "a", "b", "c", "d")
	TEST_RE_SPLIT(210, regexpr_split, "a,b.c;d:", "[ ,.;:]+", "a", "b", "c", "d")
	TEST_RE_SPLIT(211, regexpr_split, " a,b.c;d:", "[ ,.;:]+", "a", "b", "c", "d")
	TEST_RE_SPLIT(212, regexpr_split, "aa,bb.cc;dd", "[ ,.;:]+", "aa", "bb", "cc", "dd")
	TEST_RE_SPLIT(213, regexpr_split, " aa , bb . cc ; dd ", "[ ,.;:]+", "aa", "bb", "cc", "dd")
	TEST_RE_SPLIT(214, regexpr_split, " aa ,bb . cc;dd: ", "[ ,.;:]+", "aa", "bb", "cc", "dd")
	TEST_RE_SPLIT(215, regexpr_split, "aa,bb.cc;dd:", "[ ,.;:]+", "aa", "bb", "cc", "dd")
	TEST_RE_SPLIT(216, regexpr_split, " aa,bb.cc;dd:", "[ ,.;:]+", "aa", "bb", "cc", "dd")
	TEST_RE_SPLIT(217, regexpr_split, "abcd", "()", "a", "b", "c", "d")

#endif

	/* Test join */

#define TEST_JOIN(i, action, str, len, value) \
	TEST_STR(i, action, str, len, value) \
	str_destroy(&str);

	TEST_ACT (218, list = list_make((list_release_t *)str_release, str_create("aaa"), str_create("bbb"), str_create("ccc"), str_create("ddd"), str_create("eee"), str_create("fff"), NULL))
	TEST_JOIN(219, a = str_join(list, NULL), a, 18, "aaabbbcccdddeeefff")
	TEST_JOIN(220, a = str_join(list, ""), a, 18, "aaabbbcccdddeeefff")
	TEST_JOIN(221, a = str_join(list, " "), a, 23, "aaa bbb ccc ddd eee fff")
	TEST_JOIN(222, a = str_join(list, ", "), a, 28, "aaa, bbb, ccc, ddd, eee, fff")
	list_destroy(&list);

	TEST_ACT (223, list = list_make((list_release_t *)str_release, str_create("aaa"), str_create("ccc"), str_create("eee"), NULL))
	TEST_ACT (224, list_insert(list, 0, NULL))
	TEST_ACT (225, list_insert(list, 2, NULL))
	TEST_ACT (226, list_insert(list, 5, NULL))
	TEST_JOIN(227, a = str_join(list, NULL), a, 9, "aaaccceee")
	TEST_JOIN(228, a = str_join(list, ""), a, 9, "aaaccceee")
	TEST_JOIN(229, a = str_join(list, " "), a, 14, " aaa  ccc eee ")
	TEST_JOIN(230, a = str_join(list, ", "), a, 19, ", aaa, , ccc, eee, ")
	list_destroy(&list);

	TEST_ACT (231, list = list_make(NULL, "aaa", "bbb", "ccc", "ddd", "eee", "fff", NULL))
	TEST_JOIN(232, a = join(list, NULL), a, 18, "aaabbbcccdddeeefff")
	TEST_JOIN(233, a = join(list, ""), a, 18, "aaabbbcccdddeeefff")
	TEST_JOIN(234, a = join(list, " "), a, 23, "aaa bbb ccc ddd eee fff")
	TEST_JOIN(235, a = join(list, ", "), a, 28, "aaa, bbb, ccc, ddd, eee, fff")
	list_destroy(&list);

	TEST_ACT (236, list = list_make(NULL, "aaa", "ccc", "eee", NULL))
	TEST_ACT (237, list_insert(list, 0, NULL))
	TEST_ACT (238, list_insert(list, 2, NULL))
	TEST_ACT (239, list_insert(list, 5, NULL))
	TEST_JOIN(240, a = join(list, NULL), a, 9, "aaaccceee")
	TEST_JOIN(241, a = join(list, ""), a, 9, "aaaccceee")
	TEST_JOIN(242, a = join(list, " "), a, 14, " aaa  ccc eee ")
	TEST_JOIN(243, a = join(list, ", "), a, 19, ", aaa, , ccc, eee, ")
	list_destroy(&list);

	/* Test soundex */

	if (sizeof(int) < 4)
		++errors, printf("Test244: Assumption failed: sizeof(int) < 4 bytes! - soundex() won't work\n");

	TEST_EQ(245, soundex("cat"),        0x43333030) /* C300 */
	TEST_EQ(246, soundex("dog"),        0x44323030) /* D200 */
	TEST_EQ(247, soundex("elephant"),   0x45343135) /* E415 */
	TEST_EQ(248, soundex("lion"),       0x4c353030) /* L500 */
	TEST_EQ(249, soundex("wolf"),       0x57343130) /* W410 */
	TEST_EQ(250, soundex("elliot"),     0x45343330) /* E430 */
	TEST_EQ(251, soundex("wordsworth"), 0x57363332) /* W632 */
	TEST_EQ(252, soundex("smith"),      0x53353330) /* S530 */
	TEST_EQ(253, soundex("smythe"),     0x53353330) /* S530 */

	/* Test trim */

#define TEST_SFUNC(i, func, str, len, val) \
	TEST_ACT((i), a = str_create("%s", str)) \
	TEST_ACT((i), func(a)) \
	CHECK_STR((i), func(str), a, len, val) \
	str_destroy(&a);

#define TEST_FUNC(i, func, str, len, val) \
	strlcpy(tst, (str), BUFSIZ); \
	TEST_ACT((i), func(tst)) \
	CHECK_CSTR((i), func(str), tst, len, val)

	TEST_SFUNC(254, str_trim, "", 0, "")
	TEST_SFUNC(255, str_trim, " ", 0, "")
	TEST_SFUNC(256, str_trim, "  ", 0, "")
	TEST_SFUNC(257, str_trim, " \t ", 0, "")
	TEST_SFUNC(258, str_trim, "\r \t \n", 0, "")
	TEST_SFUNC(259, str_trim, "abcdef", 6, "abcdef")
	TEST_SFUNC(260, str_trim, " abcdef", 6, "abcdef")
	TEST_SFUNC(261, str_trim, "abcdef ", 6, "abcdef")
	TEST_SFUNC(262, str_trim, " abcdef ", 6, "abcdef")
	TEST_SFUNC(263, str_trim, "  abcdef  ", 6, "abcdef")
	TEST_SFUNC(264, str_trim, "  abc def  ", 7, "abc def")
	TEST_SFUNC(265, str_trim, "abc def\v\t\f\n\r", 7, "abc def")
	TEST_SFUNC(266, str_trim, "\v\t\f\n\rabc def", 7, "abc def")

	TEST_FUNC(267, trim, "", 0, "")
	TEST_FUNC(268, trim, " ", 0, "")
	TEST_FUNC(269, trim, "  ", 0, "")
	TEST_FUNC(270, trim, " \t ", 0, "")
	TEST_FUNC(271, trim, "\r \t \n", 0, "")
	TEST_FUNC(272, trim, "abcdef", 6, "abcdef")
	TEST_FUNC(273, trim, " abcdef", 6, "abcdef")
	TEST_FUNC(274, trim, "abcdef ", 6, "abcdef")
	TEST_FUNC(275, trim, " abcdef ", 6, "abcdef")
	TEST_FUNC(276, trim, "  abcdef  ", 6, "abcdef")
	TEST_FUNC(277, trim, "  abc def  ", 7, "abc def")
	TEST_FUNC(278, trim, "abc def\v\t\f\n\r", 7, "abc def")
	TEST_FUNC(279, trim, "\v\t\f\n\rabc def", 7, "abc def")

	TEST_SFUNC(280, str_trim_left, "", 0, "")
	TEST_SFUNC(281, str_trim_left, " ", 0, "")
	TEST_SFUNC(282, str_trim_left, "  ", 0, "")
	TEST_SFUNC(283, str_trim_left, " \t ", 0, "")
	TEST_SFUNC(284, str_trim_left, "\r \t \n", 0, "")
	TEST_SFUNC(285, str_trim_left, "abcdef", 6, "abcdef")
	TEST_SFUNC(286, str_trim_left, " abcdef", 6, "abcdef")
	TEST_SFUNC(287, str_trim_left, "abcdef ", 7, "abcdef ")
	TEST_SFUNC(288, str_trim_left, " abcdef ", 7, "abcdef ")
	TEST_SFUNC(289, str_trim_left, "  abcdef  ", 8, "abcdef  ")
	TEST_SFUNC(290, str_trim_left, "  abc def  ", 9, "abc def  ")
	TEST_SFUNC(291, str_trim_left, "abc def\v\t\f\n\r", 12, "abc def\v\t\f\n\r")
	TEST_SFUNC(292, str_trim_left, "\v\t\f\n\rabc def", 7, "abc def")

	TEST_FUNC(293, trim_left, "", 0, "")
	TEST_FUNC(294, trim_left, " ", 0, "")
	TEST_FUNC(295, trim_left, "  ", 0, "")
	TEST_FUNC(296, trim_left, " \t ", 0, "")
	TEST_FUNC(297, trim_left, "\r \t \n", 0, "")
	TEST_FUNC(298, trim_left, "abcdef", 6, "abcdef")
	TEST_FUNC(299, trim_left, " abcdef", 6, "abcdef")
	TEST_FUNC(300, trim_left, "abcdef ", 7, "abcdef ")
	TEST_FUNC(301, trim_left, " abcdef ", 7, "abcdef ")
	TEST_FUNC(302, trim_left, "  abcdef  ", 8, "abcdef  ")
	TEST_FUNC(303, trim_left, "  abc def  ", 9, "abc def  ")
	TEST_FUNC(304, trim_left, "abc def\v\t\f\n\r", 12, "abc def\v\t\f\n\r")
	TEST_FUNC(305, trim_left, "\v\t\f\n\rabc def", 7, "abc def")

	TEST_SFUNC(306, str_trim_right, "", 0, "")
	TEST_SFUNC(307, str_trim_right, " ", 0, "")
	TEST_SFUNC(308, str_trim_right, "  ", 0, "")
	TEST_SFUNC(309, str_trim_right, " \t ", 0, "")
	TEST_SFUNC(310, str_trim_right, "\r \t \n", 0, "")
	TEST_SFUNC(311, str_trim_right, "abcdef", 6, "abcdef")
	TEST_SFUNC(312, str_trim_right, " abcdef", 7, " abcdef")
	TEST_SFUNC(313, str_trim_right, "abcdef ", 6, "abcdef")
	TEST_SFUNC(314, str_trim_right, " abcdef ", 7, " abcdef")
	TEST_SFUNC(315, str_trim_right, "  abcdef  ", 8, "  abcdef")
	TEST_SFUNC(316, str_trim_right, "  abc def  ", 9, "  abc def")
	TEST_SFUNC(317, str_trim_right, "abc def\v\t\f\n\r", 7, "abc def")
	TEST_SFUNC(318, str_trim_right, "\v\t\f\n\rabc def", 12, "\v\t\f\n\rabc def")

	TEST_FUNC(319, trim_right, "", 0, "")
	TEST_FUNC(320, trim_right, " ", 0, "")
	TEST_FUNC(321, trim_right, "  ", 0, "")
	TEST_FUNC(322, trim_right, " \t ", 0, "")
	TEST_FUNC(323, trim_right, "\r \t \n", 0, "")
	TEST_FUNC(324, trim_right, "abcdef", 6, "abcdef")
	TEST_FUNC(325, trim_right, " abcdef", 7, " abcdef")
	TEST_FUNC(326, trim_right, "abcdef ", 6, "abcdef")
	TEST_FUNC(327, trim_right, " abcdef ", 7, " abcdef")
	TEST_FUNC(328, trim_right, "  abcdef  ", 8, "  abcdef")
	TEST_FUNC(329, trim_right, "  abc def  ", 9, "  abc def")
	TEST_FUNC(330, trim_right, "abc def\v\t\f\n\r", 7, "abc def")
	TEST_FUNC(331, trim_right, "\v\t\f\n\rabc def", 12, "\v\t\f\n\rabc def")

	/* Test squeeze */

	TEST_SFUNC(332, str_squeeze, "", 0, "")
	TEST_SFUNC(333, str_squeeze, " ", 0, "")
	TEST_SFUNC(334, str_squeeze, "  ", 0, "")
	TEST_SFUNC(335, str_squeeze, " \t ", 0, "")
	TEST_SFUNC(336, str_squeeze, "\r \t \n", 0, "")
	TEST_SFUNC(337, str_squeeze, "abcdef", 6, "abcdef")
	TEST_SFUNC(338, str_squeeze, " ab  cd  ef", 8, "ab cd ef")
	TEST_SFUNC(339, str_squeeze, "ab  cd  ef ", 8, "ab cd ef")
	TEST_SFUNC(340, str_squeeze, "   ab   cd   ef   ", 8, "ab cd ef")
	TEST_SFUNC(341, str_squeeze, "  abcdef  ", 6, "abcdef")
	TEST_SFUNC(342, str_squeeze, "  abc  def  ", 7, "abc def")
	TEST_SFUNC(343, str_squeeze, "abc def\v\t\f\n\r", 7, "abc def")
	TEST_SFUNC(344, str_squeeze, "\v\t\f\n\rabc def", 7, "abc def")

	TEST_FUNC(345, squeeze, "", 0, "")
	TEST_FUNC(346, squeeze, " ", 0, "")
	TEST_FUNC(347, squeeze, "  ", 0, "")
	TEST_FUNC(348, squeeze, " \t ", 0, "")
	TEST_FUNC(349, squeeze, "\r \t \n", 0, "")
	TEST_FUNC(350, squeeze, "abcdef", 6, "abcdef")
	TEST_FUNC(351, squeeze, " ab  cd  ef", 8, "ab cd ef")
	TEST_FUNC(352, squeeze, "ab  cd  ef ", 8, "ab cd ef")
	TEST_FUNC(353, squeeze, "   ab   cd   ef   ", 8, "ab cd ef")
	TEST_FUNC(354, squeeze, "  abcdef  ", 6, "abcdef")
	TEST_FUNC(355, squeeze, "  abc  def  ", 7, "abc def")
	TEST_FUNC(356, squeeze, "abc def\v\t\f\n\r", 7, "abc def")
	TEST_FUNC(357, squeeze, "\v\t\f\n\rabc def", 7, "abc def")

	/* Test quote, unquote */

#define TEST_SQUOTE(i, str, quotable, quote_char, len, val) \
	TEST_ACT((i), a = str_create("%s", str)) \
	TEST_ACT((i), b = str_quote(a, quotable, quote_char)) \
	CHECK_STR((i), str_quote(str, quotable, quote_char), b, len, val) \
	TEST_ACT((i), c = str_unquote(b, quotable, quote_char)) \
	CHECK_STR((i), str_unquote(quoted(str), quotable, quote_char), c, strlen(str), str) \
	str_destroy(&c); \
	str_destroy(&b); \
	str_destroy(&a);

#define TEST_QUOTE(i, str, quotable, quote_char, len, val) \
	TEST_ACT((i), a = quote(str, quotable, quote_char)) \
	CHECK_STR((i), quote(str, quotable, quote_char), a, len, val) \
	TEST_ACT((i), b = unquote(cstr(a), quotable, quote_char)) \
	CHECK_STR((i), unquote(quoted(str), quotable, quote_char), b, strlen(str), str) \
	str_destroy(&b); \
	str_destroy(&a);

	TEST_SQUOTE(358, "", "\"", '\\', 0, "")
	TEST_QUOTE (359, "", "\"", '\\', 0, "")
	TEST_SQUOTE(360, "\"hello world\"", "\"", '\\', 15, "\\\"hello world\\\"")
	TEST_QUOTE (361, "\"hello world\"", "\"", '\\', 15, "\\\"hello world\\\"")
	TEST_QUOTE (362, "\"hello world\\\"", "\"\\", '\\', 17, "\\\"hello world\\\\\\\"")
	TEST_SQUOTE(363, "\"hello world\\\"", "\"\\", '\\', 17, "\\\"hello world\\\\\\\"")
	TEST_QUOTE (364, "\\\"hello\\\\world\\\"", "\"\\", '\\', 22, "\\\\\\\"hello\\\\\\\\world\\\\\\\"")
	TEST_SQUOTE(365, "\\\"hello\\\\world\\\"", "\"\\", '\\', 22, "\\\\\\\"hello\\\\\\\\world\\\\\\\"")
	TEST_SQUOTE(366, "\"hello\\ \\world\"", "\"\\", '\\', 19, "\\\"hello\\\\ \\\\world\\\"")
	TEST_QUOTE (367, "\"hello\\ \\world\"", "\"\\", '\\', 19, "\\\"hello\\\\ \\\\world\\\"")
	TEST_SQUOTE(368, "\"hello\\ \\world\"", "\"", '\\', 17, "\\\"hello\\ \\world\\\"")
	TEST_QUOTE (369, "\"hello\\ \\world\"", "\"", '\\', 17, "\\\"hello\\ \\world\\\"")

	/* Test encode, decode */

#define TEST_SENCODE(i, str, uncoded, coded, quote_char, printable, len, val) \
	TEST_ACT((i), a = str_create("%s", str)) \
	TEST_ACT((i), b = str_encode(a, uncoded, coded, quote_char, printable)) \
	CHECK_STR((i), str_encode(str, uncoded, coded, quote_char, printable), b, len, val) \
	TEST_ACT((i), c = str_decode(b, uncoded, coded, quote_char, printable)) \
	CHECK_STR((i), str_decode(encoded(str), uncoded, coded, quote_char, printabtle), c, strlen(str), str) \
	str_destroy(&c); \
	str_destroy(&b); \
	str_destroy(&a);

#define TEST_ENCODE(i, str, uncoded, coded, quote_char, printable, len, val) \
	TEST_ACT((i), a = encode(str, uncoded, coded, quote_char, printable)) \
	CHECK_STR((i), encode(str, uncoded, coded, quote_char, printable), a, len, val) \
	TEST_ACT((i), b = decode(cstr(a), uncoded, coded, quote_char, printable)) \
	CHECK_STR((i), decode(encoded(str), uncoded, coded, quote_char, printable), b, strlen(str), str) \
	str_destroy(&b); \
	str_destroy(&a);

	TEST_SENCODE(370, "", "\a\b\t\n\v\f\r\\", "abtnvfr\\", '\\', 1, 0, "")
	TEST_SENCODE(371, "\a\b\t\n\v\f\r\033\\", "\a\b\t\n\v\f\r\\", "abtnvfr\\", '\\', 1, 20, "\\a\\b\\t\\n\\v\\f\\r\\x1b\\\\")
	TEST_SENCODE(372, "a\a\b\tb\n\v\f\rc\033\\d", "\a\b\t\n\v\f\r\\", "abtnvfr\\", '\\', 1, 24, "a\\a\\b\\tb\\n\\v\\f\\rc\\x1b\\\\d")
	TEST_SENCODE(373, "", "=", "=", '\\', 0, 0, "")
	TEST_SENCODE(374, "a=b", "=", "=", '\\', 0, 4, "a\\=b")

	TEST_ENCODE(375, "", "\a\b\t\n\v\f\r\\", "abtnvfr\\", '\\', 1, 0, "")
	TEST_ENCODE(376, "\a\b\t\n\v\f\r\033\\", "\a\b\t\n\v\f\r\\", "abtnvfr\\", '\\', 1, 20, "\\a\\b\\t\\n\\v\\f\\r\\x1b\\\\")
	TEST_ENCODE(377, "a\a\b\tb\n\v\f\rc\033\\d", "\a\b\t\n\v\f\r\\", "abtnvfr\\", '\\', 1, 24, "a\\a\\b\\tb\\n\\v\\f\\rc\\x1b\\\\d")
	TEST_ENCODE(378, "", "=", "=", '\\', 0, 0, "")
	TEST_ENCODE(379, "a=b", "=", "=", '\\', 0, 4, "a\\=b")

	/* Test lc, lcfirst */

	TEST_SFUNC(380, str_lc, "", 0, "")
	TEST_SFUNC(381, str_lc, "abc", 3, "abc")
	TEST_SFUNC(382, str_lc, " a b c ", 7, " a b c ")
	TEST_SFUNC(383, str_lc, "ABC", 3, "abc")
	TEST_SFUNC(384, str_lc, " A B C ", 7, " a b c ")
	TEST_SFUNC(385, str_lc, "0123456", 7, "0123456")

	TEST_FUNC(386, lc, "", 0, "")
	TEST_FUNC(387, lc, "abc", 3, "abc")
	TEST_FUNC(388, lc, " a b c ", 7, " a b c ")
	TEST_FUNC(389, lc, "ABC", 3, "abc")
	TEST_FUNC(390, lc, " A B C ", 7, " a b c ")
	TEST_FUNC(391, lc, "0123456", 7, "0123456")

	TEST_SFUNC(392, str_lcfirst, "", 0, "")
	TEST_SFUNC(393, str_lcfirst, "abc", 3, "abc")
	TEST_SFUNC(394, str_lcfirst, " a b c ", 7, " a b c ")
	TEST_SFUNC(395, str_lcfirst, "ABC", 3, "aBC")
	TEST_SFUNC(396, str_lcfirst, " A B C ", 7, " A B C ")
	TEST_SFUNC(397, str_lcfirst, "0123456", 7, "0123456")

	TEST_FUNC(398, lcfirst, "", 0, "")
	TEST_FUNC(399, lcfirst, "abc", 3, "abc")
	TEST_FUNC(400, lcfirst, " a b c ", 7, " a b c ")
	TEST_FUNC(401, lcfirst, "ABC", 3, "aBC")
	TEST_FUNC(402, lcfirst, " A B C ", 7, " A B C ")
	TEST_FUNC(403, lcfirst, "0123456", 7, "0123456")

	/* Test uc, ucfirst */

	TEST_SFUNC(404, str_uc, "", 0, "")
	TEST_SFUNC(405, str_uc, "abc", 3, "ABC")
	TEST_SFUNC(406, str_uc, " a b c ", 7, " A B C ")
	TEST_SFUNC(407, str_uc, "ABC", 3, "ABC")
	TEST_SFUNC(408, str_uc, " A B C ", 7, " A B C ")
	TEST_SFUNC(409, str_uc, "0123456", 7, "0123456")

	TEST_FUNC(410, uc, "", 0, "")
	TEST_FUNC(411, uc, "abc", 3, "ABC")
	TEST_FUNC(412, uc, " a b c ", 7, " A B C ")
	TEST_FUNC(413, uc, "ABC", 3, "ABC")
	TEST_FUNC(414, uc, " A B C ", 7, " A B C ")
	TEST_FUNC(415, uc, "0123456", 7, "0123456")

	TEST_SFUNC(416, str_ucfirst, "", 0, "")
	TEST_SFUNC(417, str_ucfirst, "abc", 3, "Abc")
	TEST_SFUNC(418, str_ucfirst, " a b c ", 7, " a b c ")
	TEST_SFUNC(419, str_ucfirst, "ABC", 3, "ABC")
	TEST_SFUNC(420, str_ucfirst, " A B C ", 7, " A B C ")
	TEST_SFUNC(421, str_ucfirst, "0123456", 7, "0123456")

	TEST_FUNC(422, ucfirst, "", 0, "")
	TEST_FUNC(423, ucfirst, "abc", 3, "Abc")
	TEST_FUNC(424, ucfirst, " a b c ", 7, " a b c ")
	TEST_FUNC(425, ucfirst, "ABC", 3, "ABC")
	TEST_FUNC(426, ucfirst, " A B C ", 7, " A B C ")
	TEST_FUNC(427, ucfirst, "0123456", 7, "0123456")

	/* Test chop */

#define TEST_SFUNC_EQ(i, func, str, eq, len, val) \
	TEST_ACT((i), a = str_create("%s", str)) \
	TEST_EQ((i), func(a), eq) \
	CHECK_STR((i), func(str), a, len, val) \
	str_destroy(&a);

#define TEST_FUNC_EQ(i, func, str, eq, len, val) \
	strlcpy(tst, (str), BUFSIZ); \
	TEST_EQ((i), func(tst), eq) \
	CHECK_CSTR((i), func(str), tst, len, val)

	TEST_SFUNC_EQ(428, str_chop, "abcdef", 'f', 5, "abcde")
	TEST_SFUNC_EQ(429, str_chop, "abcde", 'e',4, "abcd")
	TEST_SFUNC_EQ(430, str_chop, "abcd", 'd', 3, "abc")
	TEST_SFUNC_EQ(431, str_chop, "abc", 'c', 2, "ab")
	TEST_SFUNC_EQ(432, str_chop, "ab", 'b', 1, "a")
	TEST_SFUNC_EQ(433, str_chop, "a", 'a', 0, "")
	TEST_SFUNC_EQ(434, str_chop, "", -1, 0, "")

	TEST_FUNC_EQ(435, chop, "abcdef", 'f', 5, "abcde")
	TEST_FUNC_EQ(436, chop, "abcde", 'e', 4, "abcd")
	TEST_FUNC_EQ(437, chop, "abcd", 'd', 3, "abc")
	TEST_FUNC_EQ(438, chop, "abc", 'c', 2, "ab")
	TEST_FUNC_EQ(439, chop, "ab", 'b', 1, "a")
	TEST_FUNC_EQ(440, chop, "a", 'a', 0, "")
	TEST_FUNC_EQ(441, chop, "", -1, 0, "")

	/* Test chomp */

	TEST_SFUNC_EQ(442, str_chomp, "abcdef", 0, 6, "abcdef")
	TEST_SFUNC_EQ(443, str_chomp, "abcdef ", 0, 7, "abcdef ")
	TEST_SFUNC_EQ(444, str_chomp, "abcdef \n", 1, 7, "abcdef ")
	TEST_SFUNC_EQ(445, str_chomp, "abcdef \n\r", 2, 7, "abcdef ")

	TEST_FUNC_EQ(446, chomp, "abcdef", 0, 6, "abcdef")
	TEST_FUNC_EQ(447, chomp, "abcdef ", 0, 7, "abcdef ")
	TEST_FUNC_EQ(448, chomp, "abcdef \n", 1, 7, "abcdef ")
	TEST_FUNC_EQ(449, chomp, "abcdef \n\r", 2, 7, "abcdef ")

	/* Test bin, hex, oct */

#define TEST_SNUM(i, func, str, eq) \
	TEST_ACT((i), a = str_create("%s", str)) \
	TEST_EQ((i), func(a), eq) \
	str_destroy(&a);

#define TEST_NUM(i, func, str, eq) \
	strlcpy(tst, (str), BUFSIZ); \
	TEST_EQ((i), func(tst), eq)

	TEST_SNUM(450, str_bin, "010", 2)
	TEST_SNUM(451, str_oct, "010", 8)
	TEST_SNUM(452, str_hex, "010", 16)

	TEST_NUM(453, bin, "010", 2)
	TEST_NUM(454, oct, "010", 8)
	TEST_NUM(455, hex, "010", 16)

	TEST_SNUM(456, str_bin, "11111111111111111111111111111111", -1)
	TEST_SNUM(457, str_oct, "037777777777", -1)
	TEST_SNUM(458, str_hex, "ffffffff", -1)

	TEST_NUM(459, bin, "11111111111111111111111111111111", -1)
	TEST_NUM(460, oct, "037777777777", -1)
	TEST_NUM(461, hex, "ffffffff", -1)

	TEST_SNUM(462, str_bin, "0b1010", 10)
	TEST_SNUM(463, str_hex, "0xa", 10)
	TEST_SNUM(464, str_oct, "012", 10)
	TEST_SNUM(465, str_oct, "0b1010", 10)
	TEST_SNUM(466, str_oct, "0xa", 10)

	TEST_NUM(467, bin, "0b1010", 10)
	TEST_NUM(468, hex, "0xa", 10)
	TEST_NUM(469, oct, "012", 10)
	TEST_NUM(470, oct, "0b1010", 10)
	TEST_NUM(471, oct, "0xa", 10)

	/* Test cstr, str_set_length and str_recalc_length */

#define TEST_SET(i, action, str, eq, len, val) \
	TEST_EQ((i), action, eq) \
	CHECK_STR((i), action, str, eq, val)

	TEST_ACT(472, a = str_create("0123456789"))
	TEST_SET(473, (cstr(a)[5] = '\0', str_recalc_length(a)), a, 5, 5, "01234")
	TEST_SET(474, (cstr(a)[5] =  '5', str_recalc_length(a)), a, 10, 10, "0123456789")
	TEST_SET(475, (cstr(a)[5] = '\0', str_set_length(a, 8)), a, 8, 8, "01234\00067")
	TEST_SET(476, (cstr(a)[5] =  '5', str_set_length(a, 7)), a, 7, 7, "0123456\000")
	TEST_SET(477, (cstr(a)[5] = '\0', str_set_length(a, 6)), a, 6, 6, "01234\000")
	str_destroy(&a);

	/* Test strcasecmp() and strncasecmp() */

#ifndef HAVE_STRCASECMP
	TEST_ACT(478, strcasecmp("Abc", "abc") == 0)
	TEST_ACT(479, strcasecmp("AbcD", "abc") == 100)
	TEST_ACT(480, strcasecmp("Abc", "abcD") == -100)
	TEST_ACT(481, strcasecmp("Abce", "abcD") == 1)
	TEST_ACT(482, strcasecmp("AbcD", "abce") == -1)
#endif

#ifndef HAVE_STRNCASECMP
	TEST_ACT(483, strncasecmp("Abc", "abc", 2) == 0)
	TEST_ACT(484, strncasecmp("AbC", "abc", 3) == 0)
	TEST_ACT(485, strncasecmp("Abc", "abC", 4) == 0)
	TEST_ACT(486, strncasecmp("AbcD", "abc", 2) == 0)
	TEST_ACT(487, strncasecmp("Abcd", "abc", 3) == 0)
	TEST_ACT(488, strncasecmp("AbcD", "abc", 4) == 100)
	TEST_ACT(489, strncasecmp("Abcd", "abc", 4) == 100)
	TEST_ACT(490, strncasecmp("Abc", "abcd", 2) == 0)
	TEST_ACT(491, strncasecmp("Abc", "abcD", 3) == 0)
	TEST_ACT(492, strncasecmp("Abc", "abcd", 4) == -100)
	TEST_ACT(493, strncasecmp("Abc", "abcD", 5) == -100)
	TEST_ACT(494, strncasecmp("Abce", "abcd", 3) == 0)
	TEST_ACT(495, strncasecmp("Abce", "abcD", 4) == 1)
	TEST_ACT(496, strncasecmp("AbcE", "abcd", 5) == 1)
	TEST_ACT(497, strncasecmp("Abcd", "abce", 3) == 0)
	TEST_ACT(498, strncasecmp("AbcD", "abce", 4) == -1)
	TEST_ACT(499, strncasecmp("Abcd", "abcE", 5) == -1)
#endif

	/* Test strlcpy() and strlcat() */

#define TEST_STRCPY(i, buf, len, act, val) \
	memset((buf), 0xff, (len)); \
	TEST_ACT((i), act) \
	TEST_ACT((i), !strcmp((buf), (val)))

#define TEST_STRCAT(i, buf, act, val) \
	TEST_ACT((i), act) \
	TEST_ACT((i), !strcmp((buf), (val)))

#ifndef HAVE_STRLCPY
	TEST_STRCPY(500, tst, 4, strlcpy(tst, "", 3) == 0, "")
	TEST_STRCPY(501, tst, 4, strlcpy(tst, "a", 3) == 1, "a")
	TEST_STRCPY(502, tst, 4, strlcpy(tst, "ab", 3) == 2, "ab")
	TEST_STRCPY(503, tst, 4, strlcpy(tst, "abc", 3) == 3, "ab")
	TEST_STRCPY(504, tst, 4, strlcpy(tst, "abcd", 3) == 4, "ab")
#endif

#ifndef HAVE_STRLCAT
	tst[0] = '\0';
	TEST_STRCAT(505, tst, strlcat(tst, "", 3) == 0, "")
	TEST_STRCAT(506, tst, strlcat(tst, "a", 3) == 1, "a")
	TEST_STRCAT(507, tst, strlcat(tst, "b", 3) == 2, "ab")
	TEST_STRCAT(508, tst, strlcat(tst, "", 3) == 2, "ab")
	TEST_STRCAT(509, tst, strlcat(tst, "c", 3) == 3, "ab")
	TEST_STRCAT(510, tst, strlcat(tst, "d", 3) == 3, "ab")
#endif

	/* Test cstrcpy() */

	TEST_STRCPY(511, tst, 8, cstrcpy(tst, "") == tst + 0, "")
	TEST_STRCPY(512, tst, 8, cstrcpy(tst, "a") == tst + 1, "a")
	TEST_STRCPY(513, tst, 8, cstrcpy(tst, "ab") == tst + 2, "ab")
	TEST_STRCPY(514, tst, 8, cstrcpy(tst, "abc") == tst + 3, "abc")
	TEST_STRCPY(515, tst, 8, cstrcpy(tst, "abcd") == tst + 4, "abcd")

	/* Test cstrcat() */

	tst[0] = '\0';
	TEST_STRCAT(516, tst, cstrcat(tst, "") == tst + 0, "")
	TEST_STRCAT(517, tst, cstrcat(tst, "a") == tst + 1, "a")
	TEST_STRCAT(518, tst, cstrcat(tst, "b") == tst + 2, "ab")
	TEST_STRCAT(519, tst, cstrcat(tst, "c") == tst + 3, "abc")
	TEST_STRCAT(520, tst, cstrcat(tst, "d") == tst + 4, "abcd")
	TEST_STRCAT(521, tst, cstrcat(tst, "") == tst + 4, "abcd")

	/* Test cstrchr() */

	t = "abcabc";
	TEST_ACT(522, cstrchr(t, 'a') == t + 0)
	TEST_ACT(523, cstrchr(t, 'b') == t + 1)
	TEST_ACT(524, cstrchr(t, 'c') == t + 2)
	TEST_ACT(525, cstrchr(t, 'd') == t + 6)
	TEST_ACT(526, cstrchr(t, 'e') == t + 6)
	TEST_ACT(527, cstrchr(t, '\0') == t + 6)

	/* Test cstrpbrk() */

	t = "abcdef";
	TEST_ACT(528, cstrpbrk(t, "abc") == t + 0)
	TEST_ACT(529, cstrpbrk(t, "bcd") == t + 1)
	TEST_ACT(530, cstrpbrk(t, "cde") == t + 2)
	TEST_ACT(531, cstrpbrk(t, "def") == t + 3)
	TEST_ACT(532, cstrpbrk(t, "efg") == t + 4)
	TEST_ACT(533, cstrpbrk(t, "fgh") == t + 5)
	TEST_ACT(534, cstrpbrk(t, "ghi") == t + 6)
	TEST_ACT(535, cstrpbrk(t, "\0") == t + 6)

	/* Test cstrrchr() */

	t = "abcabc";
	TEST_ACT(536, cstrrchr(t, 'a') == t + 3)
	TEST_ACT(537, cstrrchr(t, 'b') == t + 4)
	TEST_ACT(538, cstrrchr(t, 'c') == t + 5)
	TEST_ACT(539, cstrrchr(t, 'd') == t + 6)
	TEST_ACT(540, cstrrchr(t, 'e') == t + 6)
	TEST_ACT(541, cstrrchr(t, '\0') == t + 6)

	/* Test cstrstr() */

	t = "abcabc";
	TEST_ACT(542, cstrstr(t, "abc") == t + 0)
	TEST_ACT(543, cstrstr(t, "bca") == t + 1)
	TEST_ACT(544, cstrstr(t, "cab") == t + 2)
	TEST_ACT(545, cstrstr(t, "def") == t + 6)
	TEST_ACT(546, cstrstr(t, "ghi") == t + 6)
	TEST_ACT(547, cstrstr(t, "\0") == t + 0)
	t = "texttext";
	TEST_ACT(548, cstrstr(t, "text") == t + 0);
	t = "exttext";
	TEST_ACT(549, cstrstr(t, "text") == t + 3);

	/* Test strings containing nuls and high bit characters */

#define TEST_ZFUNC(i, func, origlen, str, newlen, val) \
	TEST_ACT((i), a = substr(str, 0, origlen)) \
	TEST_ACT((i), func(a)) \
	CHECK_STR((i), func(str), a, newlen, val) \
	str_destroy(&a);

#define TEST_ZSPLIT(i, origlen, str, delim, tok1, tok2, tok3, tok4) \
	TEST_ACT(i, a = substr(str, 0, origlen)) \
	TEST_ACT(i, list = str_split(a, delim)) \
	if (list_length(list) != 4) \
	{ \
		++errors; \
		printf("Test%d: str_split(\"", i); \
		str_print(str, origlen); \
		printf("\", \"%s\") failed (%d tokens, not %d)\n", delim, (int)list_length(list), 4); \
	} \
	CHECK_SPLIT_ITEM(i, str_split, str, delim, 0, tok1) \
	CHECK_SPLIT_ITEM(i, str_split, str, delim, 1, tok2) \
	CHECK_SPLIT_ITEM(i, str_split, str, delim, 2, tok3) \
	CHECK_SPLIT_ITEM(i, str_split, str, delim, 3, tok4) \
	str_destroy(&a); \
	list_destroy(&list);

#define TEST_ZQUOTE(i, origlen, str, quotable, quote_char, len, val) \
	TEST_ACT((i), a = substr(str, 0, origlen)) \
	TEST_ACT((i), b = str_quote(a, quotable, quote_char)) \
	CHECK_STR((i), str_quote(str, quotable, quote_char), b, len, val) \
	TEST_ACT((i), c = str_unquote(b, quotable, quote_char)) \
	CHECK_STR((i), str_unquote(quoted(str), quotable, quote_char), c, origlen, str) \
	str_destroy(&c); \
	str_destroy(&b); \
	str_destroy(&a);

#define TEST_ZENCODE(i, origlen, str, uncoded, coded, quote_char, printable, len, val, len2, val2) \
	TEST_ACT((i), a = substr(str, 0, origlen)) \
	TEST_ACT((i), b = str_encode(a, uncoded, coded, quote_char, printable)) \
	CHECK_STR2((i), str_encode(str, uncoded, coded, quote_char, printable), b, len, val, len2, val2) \
	TEST_ACT((i), c = str_decode(b, uncoded, coded, quote_char, printable)) \
	CHECK_STR((i), str_decode(encoded(str), uncoded, coded, quote_char, printable), c, origlen, str) \
	str_destroy(&c); \
	str_destroy(&b); \
	str_destroy(&a);

#define TEST_ZCHOMP(i, func, origlen, str, eq, newlen, val) \
	TEST_ACT((i), a = substr(str, 0, origlen)) \
	TEST_EQ((i), func(a), eq) \
	CHECK_STR((i), func(str), a, newlen, val) \
	str_destroy(&a);

	TEST_STR(550, a = str_create("%c", '\0'), a, 1, "\000")
	TEST_ACT(551, !str_empty(a))
	TEST_STR(552, b = str_create("abc%cdef\376", '\0'), b, 8, "abc\000def\376")
	TEST_STR(553, c = str_copy(b), c, 8, "abc\000def\376")
	TEST_STR(554, str_remove(b, 0), b, 7, "bc\000def\376")
	TEST_STR(555, str_remove_range(c, 1, 2), c, 6, "a\000def\376")
	TEST_STR(556, str_insert(a, 1, "a%c\376b", '\0'), a, 5, "\000a\000\376b")
	TEST_STR(557, str_insert_str(a, 1, b), a, 12, "\000bc\000def\376a\000\376b")
	TEST_STR(558, str_prepend(a, "a%c\376b", '\0'), a, 16, "a\000\376b\000bc\000def\376a\000\376b")
	TEST_STR(559, str_prepend_str(a, b), a, 23, "bc\000def\376a\000\376b\000bc\000def\376a\000\376b")
	TEST_STR(560, str_append(a, "%c\376", '\0'), a, 25, "bc\000def\376a\000\376b\000bc\000def\376a\000\376b\000\376")
	TEST_STR(561, str_append_str(a, c), a, 31, "bc\000def\376a\000\376b\000bc\000def\376a\000\376b\000\376a\000def\376")
	TEST_STR(562, str_replace(a, 3, 5, "%c\376", '\0'), a, 28, "bc\000\000\376\000\376b\000bc\000def\376a\000\376b\000\376a\000def\376")
	TEST_STR(563, str_replace_str(a, 3, 5, c), a, 29, "bc\000a\000def\376\000bc\000def\376a\000\376b\000\376a\000def\376")
	str_destroy(&c);
	TEST_STR(564, c = str_repeat(10, "1%c1", '\0'), c, 30, "1\00011\00011\00011\00011\00011\00011\00011\00011\00011\0001")
	str_destroy(&c);
	TEST_STR(565, c = str_substr(a, 5, 5), c, 5, "def\376\000")
	str_destroy(&c);
	TEST_STR(566, c = substr(cstr(a), 5, 5), c, 5, "def\376\000")
	str_destroy(&c);
	TEST_STR(567, c = str_splice(a, 5, 5), c, 5, "def\376\000")
	CHECK_STR(568, str_splice(a, 5, 5), a, 24, "bc\000a\000bc\000def\376a\000\376b\000\376a\000def\376")
	str_destroy(&c);
	str_destroy(&b);
	TEST_TR_DIRECT(569, str_tr(a, "a-z", "A-EfG-Z", 0), a, 14, 24, "BC\000A\000BC\000DEf\376A\000\376B\000\376A\000DEf\376")
	TEST_ACT(570, b = str_create("%c", '\0'))
	TEST_ACT(571, c = str_create("%c", '\0'))
	TEST_TR_DIRECT(572, str_tr_str(a, b, c, 0), a, 6, 24, "BC\000A\000BC\000DEf\376A\000\376B\000\376A\000DEf\376")
	str_destroy(&c);
	str_destroy(&b);
	str_destroy(&a);
	TEST_ZSPLIT(573, 9, "\000a\376\000a\376a\000\376", "a", "\000", "\376\000", "\376", "\000\376")
	TEST_ACT(574, list = list_make((list_release_t *)str_release, str_create("%c\376", '\0'), str_create("\376%c", '\0'), str_create("%c%c", '\0', '\0'), NULL))
	TEST_JOIN(575, a = str_join(list, "a"), a, 8, "\000\376a\376\000a\000\000")
	list_destroy(&list);
	TEST_ZFUNC(576, str_trim, 12, "  abc\000\376def  ", 8, "abc\000\376def")
	TEST_ZFUNC(577, str_trim_left, 12, "  abc\000\376def  ", 10, "abc\000\376def  ")
	TEST_ZFUNC(578, str_trim_right, 12, "  abc\000\376def  ", 10, "  abc\000\376def")
	TEST_ZFUNC(579, str_squeeze, 19, "   ab \000 cd\376   ef   ", 11, "ab \000 cd\376 ef")
	TEST_ZQUOTE(580, 16, "\\\"hell\000\\\\w\376rld\\\"", "\"\\", '\\', 22, "\\\\\\\"hell\000\\\\\\\\w\376rld\\\\\\\"")
	TEST_ZENCODE(581, 11, "\a\b\t\n\0\376\v\f\r\033\\", "\a\b\t\n\v\f\r\\", "abtnvfr\\", '\\', 1, 28, "\\a\\b\\t\\n\\x00\\xfe\\v\\f\\r\\x1b\\\\", 25, "\\a\\b\\t\\n\\x00\376\\v\\f\\r\\x1b\\\\")
	TEST_ZFUNC(582, str_lc, 7, "ABC\000DEF", 7, "abc\000def")
	TEST_ZFUNC(583, str_lcfirst, 7, "ABC\000DEF", 7, "aBC\000DEF")
	TEST_ZFUNC(584, str_uc, 7, "abc\000def", 7, "ABC\000DEF")
	TEST_ZFUNC(585, str_ucfirst, 7, "abc\000def", 7, "Abc\000def")
	TEST_ZFUNC(586, str_chop, 7, "abc\000def", 6, "abc\000de")
	TEST_ZCHOMP(587, str_chomp, 7, "abc\000def", 0, 7, "abc\000def")
	TEST_ZCHOMP(588, str_chomp, 7, "abc\000de\n", 1, 6, "abc\000de")

	/* Test error reporting */

	TEST_ACT(589, !str_copy(NULL))
	TEST_ACT(590, !cstr(NULL))
	TEST_EQ (591, str_set_length(NULL, 0), -1)
	TEST_ACT(592, a = str_create(NULL))
	TEST_EQ (593, str_set_length(a, 33), -1)
	TEST_EQ (594, str_set_length(a, 1), -1)
	TEST_EQ (595, str_recalc_length(NULL), -1)
	TEST_ACT(596, !str_remove_range(NULL, 0, 0))
	TEST_ACT(597, !str_remove_range(a, 1, 0))
	TEST_ACT(598, !str_remove_range(a, 0, 1))
	TEST_ACT(599, !str_remove_range(a, -1000, 1))
	TEST_ACT(600, !str_remove_range(a, 1, -1000))
	TEST_ACT(601, !str_remove_range(a, -1000, -1000))
	TEST_ACT(602, !str_insert(NULL, 0, ""))
	TEST_ACT(603, !str_insert(a, 1, ""))
	TEST_ACT(604, !str_insert(a, -1000, ""))
	TEST_ACT(605, !str_insert_str(a, 0, NULL))
	b = str_create(NULL);
	TEST_ACT(606, !str_insert_str(a, -1000, b))
	str_destroy(&b);
	TEST_ACT(607, !str_replace(NULL, 0, 0, ""))
	TEST_ACT(608, !str_replace(a, 0, 1, ""))
	TEST_ACT(609, !str_replace(a, 1, 0, ""))
	TEST_ACT(610, !str_replace(a, -1000, 1, ""))
	TEST_ACT(611, !str_replace(a, 1, -1000, ""))
	TEST_ACT(612, !str_replace_str(NULL, 0, 0, NULL))
	b = str_create(NULL);
	TEST_ACT(613, !str_replace_str(a, -1000, 0, b))
	TEST_ACT(614, !str_replace_str(a, 0, -1000, b))
	TEST_ACT(615, !str_replace_str(a, -1000, -1000, b))
	str_destroy(&b);
	TEST_ACT(616, !str_substr(NULL, 0, 0))
	TEST_ACT(617, !str_substr(a, 0, 1))
	TEST_ACT(618, !str_substr(a, 1, 0))
	TEST_ACT(619, !str_substr(a, -1000, 1))
	TEST_ACT(620, !str_substr(a, 1, -1000))
	TEST_ACT(621, !str_substr(a, -1000, -1000))
	TEST_ACT(622, !substr(NULL, 0, 0))
	TEST_ACT(623, !substr("abc", -1000, 1))
	TEST_ACT(624, !substr("abc", 1, -1000))
	TEST_ACT(625, !substr("abc", -1000, -1000))
	TEST_ACT(626, !str_splice(NULL, 0, 0))
	TEST_ACT(627, !str_splice(a, 0, 1))
	TEST_ACT(628, !str_splice(a, 1, 0))
	TEST_EQ (629, str_tr(NULL, "a-z", "A-Z", 0), -1)
	TEST_EQ (630, str_tr(a, NULL, "A-Z", 0), -1)
	TEST_EQ (631, str_tr(a, "z-a", "A-Z", 0), -1)
	TEST_EQ (632, str_tr(a, "a-z", "Z-A", 0), -1)
	TEST_ACT(633, b = str_create("a-z"))
	TEST_ACT(634, c = str_create("A-Z"))
	TEST_EQ (635, str_tr_str(NULL, b, c, 0), -1)
	TEST_EQ (636, str_tr_str(a, NULL, c, 0), -1)
	str_destroy(&c);
	TEST_ACT(637, c = str_create("Z-A"))
	TEST_EQ (638, str_tr_str(a, b, c, 0), -1)
	str_destroy(&b);
	TEST_ACT(639, b = str_create("z-a"))
	TEST_EQ (640, str_tr_str(a, b, c, 0), -1)
	str_destroy(&b);
	str_destroy(&c);
	TEST_EQ (641, tr(NULL, "a-z", "A-Z", 0), -1)
	TEST_EQ (642, tr(tst, NULL, "A-Z", 0), -1)
	TEST_EQ (643, tr(tst, "z-a", "A-Z", 0), -1)
	TEST_EQ (644, tr(tst, "a-z", "Z-A", 0), -1)
	TEST_EQ (645, tr(tst, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz", "a-z", 0), -1)
	TEST_EQ (646, tr(tst, "a-z", "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz", 0), -1)
	TEST_EQ (647, str_tr_compiled(NULL, NULL), -1)
	TEST_EQ (648, str_tr_compiled(a, NULL), -1)
	TEST_EQ (649, tr_compiled(NULL, NULL), -1)
	TEST_EQ (650, tr_compiled("", NULL), -1)
#ifdef HAVE_REGEX_H
	TEST_ACT(651, !str_regexpr(NULL, a, 0, 0))
	TEST_ACT(652, !str_regexpr("", NULL, 0, 0))
	TEST_ACT(653, !regexpr(NULL, "", 0, 0))
	TEST_ACT(654, !regexpr("", NULL, 0, 0))
	TEST_EQ (655, regexpr_compile(re, NULL, 0), REG_BADPAT)
	TEST_EQ (656, regexpr_compile(NULL, "", 0), REG_BADPAT)
	TEST_ACT(657, !regexpr_compiled(re, NULL, 0))
	TEST_ACT(658, !regexpr_compiled(NULL, "", 0))
	str_destroy(&a);
	TEST_ACT(659, a = str_create("aaa"))
	TEST_ACT(660, !str_regsub(NULL, "", a, 0, 0, 0))
	TEST_ACT(661, !str_regsub("", NULL, a, 0, 0, 0))
	TEST_ACT(662, !str_regsub("", "", NULL, 0, 0, 0))
	TEST_ACT(663, !str_regsub_compiled(NULL, "", a, 0, 0))
	TEST_ACT(664, !str_regsub_compiled(re, NULL, a, 0, 0))
	TEST_EQ (665, regexpr_compile(re, ".+", 0), 0)
	TEST_ACT(666, !str_regsub_compiled(re, "$", NULL, 0, 0))
	TEST_ACT(667, !str_regsub_compiled(re, "$a", NULL, 0, 0))
	TEST_ACT(668, !str_regsub_compiled(re, "${0", NULL, 0, 0))
	TEST_ACT(669, !str_regsub_compiled(re, "${", NULL, 0, 0))
	TEST_ACT(670, !str_regsub_compiled(re, "${33}", NULL, 0, 0))
	TEST_ACT(671, !str_regsub_compiled(re, "${-12}", NULL, 0, 0))
	TEST_ACT(672, !str_regsub_compiled(re, "${a}", NULL, 0, 0))
	TEST_ACT(673, !str_regsub_compiled(re, "\\L\\U\\Q\\L\\U\\Q\\L\\U\\Q\\L\\U\\Q\\L\\U\\Q\\L\\U\\Q\\L\\U\\Q\\L\\U\\Q\\L\\U\\Q\\L\\U\\Q\\L\\U\\Q$0\\E\\E\\E\\E\\E\\E\\E\\E\\E\\E\\E\\E\\E\\E\\E\\E\\E\\E\\E\\E\\E\\E\\E\\E\\E\\E\\E\\E\\E\\E\\E\\E\\E", a, 0, 0))
	TEST_ACT(674, !str_regsub_compiled(re, "\\E", a, 0, 0))
	regexpr_release(re);
#endif
	str_destroy(&a);
	TEST_ACT(675, a = str_create("hello"))
	TEST_ACT(676, !str_fmt(NULL, 10, ALIGN_LEFT))
	TEST_ACT(677, !str_fmt(a, -1, ALIGN_LEFT))
	TEST_ACT(678, !str_fmt(a, 10, '@'))
	str_destroy(&a);
	TEST_ACT(679, !fmt(NULL, 10, ALIGN_LEFT))
	TEST_ACT(680, !fmt("hello", -1, ALIGN_LEFT))
	TEST_ACT(681, !fmt("hello", 10, '@'))
	TEST_ACT(682, !str_split(NULL, ""))
	TEST_ACT(683, !str_split(a, NULL))
	TEST_ACT(684, !split(NULL, ""))
	TEST_ACT(685, !split("", NULL))
#ifdef HAVE_REGEX_H
	TEST_ACT(686, !str_regexpr_split(NULL, "", 0, 0))
	TEST_ACT(687, !str_regexpr_split(a, NULL, 0, 0))
	TEST_ACT(688, !regexpr_split(NULL, "", 0, 0))
	TEST_ACT(689, !regexpr_split("", NULL, 0, 0))
#endif
	TEST_ACT(690, !str_join(NULL, " "))
	TEST_ACT(691, !join(NULL, " "))
	TEST_EQ (692, soundex(NULL), -1)
	TEST_ACT(693, !str_trim(NULL))
	TEST_ACT(694, !trim(NULL))
	TEST_ACT(695, !str_trim_left(NULL))
	TEST_ACT(696, !trim_left(NULL))
	TEST_ACT(697, !str_trim_right(NULL))
	TEST_ACT(698, !trim_right(NULL))
	TEST_ACT(699, !str_squeeze(NULL))
	TEST_ACT(700, !squeeze(NULL))
	TEST_ACT(701, !str_quote(NULL, "", '\\'))
	TEST_ACT(702, !str_quote(a, NULL, '\\'))
	TEST_ACT(703, !quote(NULL, "", '\\'))
	TEST_ACT(704, !quote("", NULL, '\\'))
	TEST_ACT(705, !str_unquote(NULL, "", '\\'))
	TEST_ACT(706, !str_unquote(a, NULL, '\\'))
	TEST_ACT(707, !unquote(NULL, "", '\\'))
	TEST_ACT(708, !unquote("", NULL, '\\'))
	TEST_ACT(709, !str_encode(NULL, "", "", '\\', 0))
	TEST_ACT(710, !str_encode(a, NULL, "", '\\', 0))
	TEST_ACT(711, !str_encode(a, "", NULL, '\\', 0))
	TEST_ACT(712, !encode(NULL, "", "", '\\', 0))
	TEST_ACT(713, !encode("", NULL, "", '\\', 0))
	TEST_ACT(714, !encode("", "", NULL, '\\', 0))
	TEST_ACT(715, !str_decode(NULL, "", "", '\\', 0))
	TEST_ACT(716, !str_decode(a, NULL, "", '\\', 0))
	TEST_ACT(717, !str_decode(a, "", NULL, '\\', 0))
	TEST_ACT(718, !decode(NULL, "", "", '\\', 0))
	TEST_ACT(719, !decode("", NULL, "", '\\', 0))
	TEST_ACT(720, !decode("", "", NULL, '\\', 0))
	TEST_ACT(721, !str_lc(NULL))
	TEST_ACT(722, !lc(NULL))
	TEST_ACT(723, !str_lcfirst(NULL))
	TEST_ACT(724, !lcfirst(NULL))
	TEST_ACT(725, !str_uc(NULL))
	TEST_ACT(726, !uc(NULL))
	TEST_ACT(727, !str_ucfirst(NULL))
	TEST_ACT(728, !ucfirst(NULL))
	TEST_EQ (729, str_chop(NULL), -1)
	TEST_EQ (730, str_chop(a), -1)
	TEST_EQ (731, chop(NULL), -1)
	TEST_EQ (732, chop(""), -1)
	TEST_EQ (733, str_chomp(NULL), -1)
	TEST_EQ (734, chomp(NULL), -1)
	TEST_EQ (735, str_bin(NULL), -1)
	TEST_ACT(736, b = str_create("123456789!"))
	TEST_EQ (737, str_bin(b), -1)
	TEST_EQ (738, bin(NULL), -1)
	TEST_EQ (739, bin("123456789!"), -1)
	TEST_EQ (740, str_hex(NULL), -1)
	TEST_EQ (741, str_hex(b), -1)
	TEST_EQ (742, hex(NULL), -1)
	TEST_EQ (743, hex("123456789!"), -1)
	TEST_EQ (744, str_oct(NULL), -1)
	TEST_EQ (745, str_oct(b), -1)
	TEST_EQ (746, oct(NULL), -1)
	TEST_EQ (747, oct("123456789!"), -1)
	str_destroy(&b);
	str_destroy(&a);

	/* Test asprintf() and vasprintf() */

#define TEST_ASPRINTF(i, act, len, res, val) \
	(res) = NULL; \
	rc = (act); \
	TEST_EQ((i), (len), rc) \
	TEST_NE((i), (res), NULL) \
	if (res) \
		TEST_ACT((i), !strcmp((res), (val))) \
	free(res);

#ifndef HAVE_ASPRINTF
	TEST_ASPRINTF(748, asprintf(&t, ""), 0, t, "")
	TEST_ASPRINTF(749, asprintf(&t, "a"), 1, t, "a")
	TEST_ASPRINTF(750, asprintf(&t, "abc"), 3, t, "abc")
	TEST_ASPRINTF(751, asprintf(&t, "-%d-", 0), 3, t, "-0-")
	TEST_ASPRINTF(752, asprintf(&t, "%s", "123"), 3, t, "123")
	TEST_ASPRINTF(753, asprintf(&t, "%.1f ", 1.5), 4, t, "1.5 ")
	TEST_ASPRINTF(754, asprintf(&t, "%1024s", "*"),  1024, t, "                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               *")
	TEST_ASPRINTF(755, asprintf(&t, "%-1024s", "*"), 1024, t, "*                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               ")
#endif

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
		++errors, printf("Test756: locker_create_rwlock() failed\n");
	else
	{
		mt_test(756, locker);
		locker_destroy(&locker);
	}

	if (debug)
		locker = locker_create_debug_mutex(&mutex);
	else
		locker = locker_create_mutex(&mutex);

	if (!locker)
		++errors, printf("Test757: locker_create_mutex() failed\n");
	else
	{
		mt_test(757, locker);
		locker_destroy(&locker);
	}

	if (errors)
		printf("%d/757 tests failed\n", errors);
	else
		printf("All tests passed\n");

	return (errors == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

#endif

/* vi:set ts=4 sw=4: */
