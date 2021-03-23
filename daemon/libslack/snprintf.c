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
 * Copyright Patrick Powell 1995
 * This code is based on code written by Patrick Powell (papowell@astart.com)
 * It may be used for any purpose as long as this notice remains intact
 * on all source code distributions
 */

/**************************************************************
 * Original:
 * Patrick Powell Tue Apr 11 09:48:21 PDT 1995
 * A bombproof version of doprnt (dopr) included.
 * Sigh.  This sort of thing is always nasty do deal with.  Note that
 * the version here does not include floating point...
 *
 * snprintf() is used instead of sprintf() as it does limit checks
 * for string length.  This covers a nasty loophole.
 *
 * The other functions are there to prevent NULL pointers from
 * causing nasty effects.
 *
 * More Recently:
 *  Brandon Long <blong@fiction.net> 9/15/96 for mutt 0.43
 *  This was ugly.  It is still ugly.  I opted out of floating point
 *  numbers, but the formatter understands just about everything
 *  from the normal C string format, at least as far as I can tell from
 *  the Solaris 2.5 printf(3S) man page.
 *
 *  Brandon Long <blong@fiction.net> 10/22/97 for mutt 0.87.1
 *    Ok, added some minimal floating point support, which means this
 *    probably requires libm on most operating systems.  Don't yet
 *    support the exponent (e,E) and sigfig (g,G).  Also, fmtint()
 *    was pretty badly broken, it just wasn't being exercised in ways
 *    which showed it, so that's been fixed.  Also, formated the code
 *    to mutt conventions, and removed dead code left over from the
 *    original.  Also, there is now a builtin-test, just compile with:
 *           gcc -DTEST_SNPRINTF -o snprintf snprintf.c -lm
 *    and run snprintf for results.
 *
 *  Thomas Roessler <roessler@guug.de> 01/27/98 for mutt 0.89i
 *    The PGP code was using unsigned hexadecimal formats.
 *    Unfortunately, unsigned formats simply didn't work.
 *
 *  Michael Elkins <me@cs.hmc.edu> 03/05/98 for mutt 0.90.8
 *    The original code assumed that both snprintf() and vsnprintf() were
 *    missing.  Some systems only have snprintf() but not vsnprintf(), so
 *    the code is now broken down under HAVE_SNPRINTF and HAVE_VSNPRINTF.
 *
 *  Andrew Tridgell (tridge@samba.org) Oct 1998
 *    fixed handling of %.0f
 *    added test for HAVE_LONG_DOUBLE
 *
 *  raf (raf@raf.org) Sep 2001
 *    improved speed of %[diouxXp]
 *    fixed handling of %#
 *    fixed handling of %[feEgG]
 *    fixed return value (required length, not truncated length)
 *    fixed error reporting (return -1 on format error)
 *    fixed snprintf(NULL, 0, "")
 *    fixed handling of %9.[diouxXp]
 *    fixed handling of %p
 *    fixed handling of flags [+- #0]
 *    fixed handling of precision (%[diouxXps])
 *    fixed handling of field width (%c)
 *    fixed handling of %h[diouxX] (bigendian)
 *    added ability to mimic glibc or solaris behaviour
 *    now ISO C 89 compliant formatting except format is not an mbstr
 *    added manpage in pod format (perldoc -F snprintf.c)
 *    added rigorous testing (145893 tests)
 *
 *  raf (raf@raf.org) May 2002
 *    added solaris 8 bug compatibility
 *
 *  raf (raf@raf.org) Dec 2003
 *    added examples to the manpage
 *
 *  raf (raf@raf.org) Jun 2010
 *    fixed for 64-bit systems
 *
 **************************************************************/

/*

=head1 NAME

I<snprintf(3)> - safe sprintf

=head1 SYNOPSIS

    #include <slack/std.h>
    #ifndef HAVE_SNPRINTF
    #include <slack/snprintf.h>
    #endif

    int snprintf(char *str, size_t size, const char *format, ...);
    int vsnprintf(char *str, size_t size, const char *format, va_list args);

=head1 DESCRIPTION

Safe version of I<sprintf(3)> of that doesn't suffer from buffer overruns.

=over 4

=cut

*/

#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <float.h>
#include <sys/types.h>

#include "snprintf.h"

#ifdef HAVE_LONG_DOUBLE
#define LDOUBLE long double
#else
#define LDOUBLE double
#endif

#ifndef TEST

#if defined __STDC__ || defined __cplusplus
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#if defined __STDC__ || defined __cplusplus
int sprintf(char *str, const char *format, ...);
#else
int sprintf();
#endif

/* Format read states */

#define PARSE_DEFAULT    0
#define PARSE_FLAGS      1
#define PARSE_WIDTH      2
#define PARSE_DOT        3
#define PARSE_PRECISION  4
#define PARSE_SIZE       5
#define PARSE_CONVERSION 6
#define PARSE_DONE       7

/* Format flags */

#define FLAG_MINUS      (1 << 0)
#define FLAG_PLUS       (1 << 1)
#define FLAG_SPACE      (1 << 2)
#define FLAG_ALT        (1 << 3)
#define FLAG_ZERO       (1 << 4)
#define FLAG_UP         (1 << 5)
#define FLAG_UNSIGNED   (1 << 6)
#define FLAG_F          (1 << 7)
#define FLAG_E          (1 << 8)
#define FLAG_G          (1 << 9)
#define FLAG_PTR_SIGNED (1 << 10)
#define FLAG_PTR_NIL    (1 << 11)
#define FLAG_PTR_NOALT  (1 << 12)

/* Conversion flags */

#define SIZE_SHORT   1
#define SIZE_LONG    2
#define SIZE_LDOUBLE 3

#ifndef max
#define max(a, b) ((a >= b) ? a : b)
#endif

/*

=item C<int snprintf(char *str, size_t size, const char *format, ...)>

Writes output to the string C<str>, under control of the format string
C<format>, that specifies how subsequent arguments are converted for output.
It is similar to I<sprintf(3)>, except that C<size> specifies the maximum
number of characters to produce. The trailing C<nul> character is counted
towards this limit, so you must allocate at least C<size> characters for
C<str>.

If C<size> is zero, nothing is written and C<str> may be C<null>. Otherwise,
output characters beyond the C<n-1>st are discarded rather than being
written to C<str>, and a C<nul> character is written at the end of the
characters actually written to C<str>. If copying takes place between
objects that overlap, the behaviour is undefined.

On success, returns the number of characters that would have been written
had C<size> been sufficiently large, not counting the terminating C<nul>
character. Thus, the C<nul>-terminated output has been completely written if
and only if the return value is nonnegative and less than C<size>. On error,
returns C<-1> (i.e. encoding error).

Note that if your system already has I<snprintf(3)> but this implementation
was installed anyway, it's because the system implementation has a broken
return value. Some older implementations (e.g. I<glibc-2.0>) return C<-1>
when the string is truncated rather than returning the number of characters
that would have been written had C<size> been sufficiently large (not
counting the terminating C<nul> character) as required by I<ISO/IEC
9899:1999(E)>.

=cut

*/

#ifndef HAVE_SNPRINTF
#ifdef __STDC__
int snprintf(char *str, size_t size, const char *format, ...)
#else
int snprintf(str, size, format, va_alist)
	char *str;
	size_t size;
	const char *format;
	va_dcl
#endif
{
	int ret;
	va_list args;
#ifdef __STDC__
	va_start(args, format);
#else
	va_start(args);
#endif
	ret = vsnprintf(str, size, format, args);
	va_end(args);

	return ret;
}
#endif

/*

=item C<int vsnprintf(char *str, size_t size, const char *format, va_list args)>

Equivalent to I<snprintf(3)> with the variable argument list specified
directly as for I<vsprintf(3)>.

=cut

*/

#ifndef HAVE_VSNPRINTF
static void outch(char *buffer, size_t *currlen, size_t *reqlen, size_t size, int c)
{
	if (*currlen + 1 < size)
		buffer[(*currlen)++] = (char)c;

	++*reqlen;
}

static void fmtch(char *buffer, size_t *currlen, size_t *reqlen, size_t size, int value, int flags, int width)
{
	int padlen;

	/*
	** A negative field width argument is taken as a - flag followed by a
	** positive field width argument
	*/

	if (width < 0)
	{
		flags |= FLAG_MINUS;
		width = -width;
	}

	padlen = width - 1;
	if (padlen < 0)
		padlen = 0;

	if (flags & FLAG_MINUS)
		padlen = -padlen;

	while (padlen > 0)
	{
		outch(buffer, currlen, reqlen, size, ' ');
		--padlen;
	}

	outch(buffer, currlen, reqlen, size, (unsigned int)value);

	while (padlen < 0)
	{
		outch(buffer, currlen, reqlen, size, ' ');
		++padlen;
	}
}

static void fmtstr(char *buffer, size_t *currlen, size_t *reqlen, size_t size, char *value, int flags, int width, int precision)
{
	int padlen, bytes;
	int cnt = 0;

	/*
	** A negative field width argument is taken as a - flag followed by a
	** positive field width argument
	*/

	if (width < 0)
	{
		flags |= FLAG_MINUS;
		width = -width;
	}

	/* A negative precision argument is taken as if the precision were omitted */

	if (precision < 0)
		precision = -1;

#ifdef HAVE_PRINTF_STR_FMT_NULL
	/* Print NULL as "(null)" if possible, otherwise as "" (like glibc) */

	if (value == NULL)
		value = (precision == -1 || precision >= 6) ? "(null)" : "";
#endif

	for (bytes = 0; (precision == -1 || bytes < precision) && value[bytes]; ++bytes)
	{}

	padlen = width - bytes;
	if (padlen < 0)
		padlen = 0;
	if (flags & FLAG_MINUS)
		padlen = -padlen;

	while (padlen > 0)
	{
		outch(buffer, currlen, reqlen, size, ' ');
		--padlen;
	}

	while (*value && (precision == -1 || (cnt < precision)))
	{
		outch(buffer, currlen, reqlen, size, *value++);
		++cnt;
	}

	while (padlen < 0)
	{
		outch(buffer, currlen, reqlen, size, ' ');
		++padlen;
	}
}

static void fmtint(char *buffer, size_t *currlen, size_t *reqlen, size_t size, long value, int base, int width, int precision, int flags)
{
	int signvalue = 0;
	unsigned long uvalue;
	unsigned char convert[22];
	int place = 0;
	int spadlen = 0;
	int zpadlen = 0;
	char *digits;
	int zextra = 0;
	int sextra = 0;

	/*
	** A negative field width argument is taken as a - flag followed by a
	** positive field width argument
	*/

	if (width < 0)
	{
#ifdef HAVE_PRINTF_WITH_SOLARIS_NEGATIVE_WIDTH_BEHAVIOUR
		if (!(flags & FLAG_ZERO) || precision >= 0)
#endif
			flags |= FLAG_MINUS;
		width = -width;
	}

	/* A negative precision argument is taken as if the precision were omitted */

	if (precision < 0)
		precision = -1;

	/* If the space and + flags both appear, the space flag will be ignored */

	if (flags & FLAG_PLUS)
		flags &= ~FLAG_SPACE;

	/* If the 0 and - flags both appear, the 0 flag will be ignored */

	if (flags & FLAG_MINUS)
		flags &= ~FLAG_ZERO;

	/* If a precision is specified, the 0 flag will be ignored */
	/* The d, i, o, u, x and X conversions have a default precision of 1 */

	if (precision == -1)
		precision = 1;
	else
		flags &= ~FLAG_ZERO;

	/*
	** The + flag: The result of the conversion will always begin with a plus
	** or minus sign. (It will begin with a sign only when a negative value is
	** converted if this flag is not specified.)
	**
	** The space flag: If the first character of a signed conversion is not a
	** sign, or if a signed conversion results in no characters, a space will
	** be prefixed to the result.
	*/

	uvalue = (unsigned long)value;

	if (!(flags & FLAG_UNSIGNED))
	{
		if (value < 0)
		{
			signvalue = '-';
			uvalue = (unsigned long)-value;
		}
		else if (flags & FLAG_PLUS)
			signvalue = '+';
		else if (flags & FLAG_SPACE)
			signvalue = ' ';
	}

#ifdef HAVE_PRINTF_PTR_FMT_SIGNED
	/* Behave like glibc (treat %p as signed (almost)) */
	if (flags & FLAG_PTR_SIGNED && value)
	{
		if (flags & FLAG_PLUS)
			signvalue = '+';
		else if (flags & FLAG_SPACE)
			signvalue = ' ';
	}
#endif

#ifdef HAVE_PRINTF_PTR_FMT_NOALT
	/* Behave like Solaris - %p never produces 0x */
	if (flags & FLAG_PTR_NOALT)
		flags &= ~FLAG_ALT;
#endif

#ifdef HAVE_PRINTF_PTR_FMT_NIL
	if (flags & FLAG_PTR_NIL && !value)
	{
		spadlen = width - 5;
		if (spadlen < 0)
			spadlen = 0;
		if (flags & FLAG_MINUS)
			spadlen = -spadlen;

		while (spadlen > 0)
		{
			outch(buffer, currlen, reqlen, size, ' ');
			--spadlen;
		}

		outch(buffer, currlen, reqlen, size, '(');
		outch(buffer, currlen, reqlen, size, 'n');
		outch(buffer, currlen, reqlen, size, 'i');
		outch(buffer, currlen, reqlen, size, 'l');
		outch(buffer, currlen, reqlen, size, ')');

		while (spadlen < 0)
		{
			outch(buffer, currlen, reqlen, size, ' ');
			++spadlen;
		}

		return;
	}
#endif

	/* %[XEG] produce uppercase alpha characters */

	digits = (flags & FLAG_UP) ? "0123456789ABCDEF" : "0123456789abcdef";

	/*
	** The result of converting a zero value with a precision of zero is no
	** characters
	*/

	if (precision || uvalue)
	{
		do
		{
			convert[place++] = digits[uvalue % base];
			uvalue /= base;
		}
		while (uvalue && (place < 21));
	}

	convert[place] = 0;

	/*
	** The # flag: For o conversion, it increases the precision to force the
	** first digit of the result to be a zero. For x (or X) conversion, a
	** non-zero result will have 0x (or 0X) prefixed to it.
	*/

	if (flags & FLAG_ALT)
	{
		if (base == 16 && value)
			sextra = 2;
#ifdef HAVE_PRINTF_WITH_SOLARIS8_ZERO_PRECISION_ALT_OCTAL_BEHAVIOUR
		else if (base == 8 && precision <= place && (place != 0 && convert[place - 1] != '0'))
			zextra = 1;
#else
		else if (base == 8 && precision <= place && (place == 0 || convert[place - 1] != '0'))
			zextra = 1;
#endif
	}

	/*
	** The 0 flag: Leading zeroes (following any indication of sign or base)
	** are used to pad to the field width; no space padding is performed.
	*/

	zpadlen = precision - place;
	if (zpadlen < 0)
		zpadlen = 0;
	zpadlen += zextra;

	spadlen = width - (place + zpadlen) - (signvalue ? 1 : 0) - sextra;
	if (spadlen < 0)
		spadlen = 0;

	if (flags & FLAG_ZERO)
	{
		zpadlen = max(zpadlen, spadlen);
		spadlen = 0;
	}

	if (flags & FLAG_MINUS)
		spadlen = -spadlen;

	while (spadlen > 0)
	{
		outch(buffer, currlen, reqlen, size, ' ');
		--spadlen;
	}

	if (signvalue)
		outch(buffer, currlen, reqlen, size, signvalue);

	if (flags & FLAG_ALT && base == 16 && sextra == 2)
	{
		outch(buffer, currlen, reqlen, size, '0');
		outch(buffer, currlen, reqlen, size, (flags & FLAG_UP) ? 'X' : 'x');
	}

	while (zpadlen > 0)
	{
		outch(buffer, currlen, reqlen, size, '0');
		--zpadlen;
	}

	while (place > 0)
		outch(buffer, currlen, reqlen, size, convert[--place]);

	while (spadlen < 0)
	{
		outch(buffer, currlen, reqlen, size, ' ');
		++spadlen;
	}
}

#ifndef DBL_MAX_10_EXP
#define DBL_MAX_10_EXP 308
#endif

#ifndef LDBL_MAX_10_EXP
#define LDBL_MAX_10_EXP 4932
#endif

#define DBL_MAX_EXP_DIGITS 3
#define LDBL_MAX_EXP_DIGITS 4

#define MAX_DIGITS(ldbl) \
	((ldbl) ? LDBL_MAX_10_EXP : DBL_MAX_10_EXP)
#define MAX_EXP_DIGITS(ldbl) \
	((ldbl) ? LDBL_MAX_EXP_DIGITS : DBL_MAX_EXP_DIGITS)
#define F_MAX(width, precision, ldbl) \
	max((width), 1 + MAX_DIGITS(ldbl) + 1 + (precision))
#define E_MAX(width, precision, ldbl) \
	max((width), 3 + (precision) + 2 + MAX_EXP_DIGITS(ldbl))
#define G_MAX(width, precision, ldbl) \
	max(F_MAX((width), precision, ldbl), E_MAX((width), precision, ldbl))

static void fmtfp(char *buffer, size_t *currlen, size_t *reqlen, size_t size, LDOUBLE fvalue, int width, int precision, int flags, int cflags)
{
	/* Calculate maximum space required and delegate to sprintf() */

	char buf[512], *convert = buf, *p;
	char format[1 + 5 + 20 + 1 + 20 + 1 + 1 + 1];
	char widstr[21];
	int fpmax;

	/*
	** A negative field width argument is taken as a - flag followed by a
	** positive field width argument
	*/

	if (width < 0)
	{
#ifdef HAVE_PRINTF_WITH_SOLARIS_NEGATIVE_WIDTH_BEHAVIOUR
		if ((flags & FLAG_ZERO) == 0)
#endif
			flags |= FLAG_MINUS;
		width = -width;
	}

	/* A negative precision argument is taken as if the precision were omitted */

	if (precision < 0)
		precision = -1;

	/*
	** The ISO C standard (Section 7.9.6.2, The fscanf function), says that
	** if the precision is missing, the default precision for f, e and E is
	** taken to be 6. The default precision for g and G is taken to be 1.
	** However, many systems use 6, so we'll provide an option.
	*/

	if (precision == -1)
		precision =
#ifdef HAVE_PRINTF_FLT_FMT_G_STD
			(flags & FLAG_G) ? 1 :
#endif
			6;

	/* Calculate maximum possible length */

	if (flags & FLAG_F)
		fpmax = F_MAX(width, precision, (cflags & SIZE_LDOUBLE)) + 1;
	else if (flags & FLAG_E)
		fpmax = E_MAX(width, precision, (cflags & SIZE_LDOUBLE)) + 1;
	else
		fpmax = G_MAX(width, precision, (cflags & SIZE_LDOUBLE)) + 1;

	/* Ensure enough space or bail out */

	if (fpmax > 512 && !(convert = malloc(fpmax)))
		return;

	/* Build the format string (probably should have just copied it) */

	sprintf(widstr, "%d", width);
	sprintf(format, "%%%s%s%s%s%s%s.%d%s%s",
		(flags & FLAG_MINUS) ? "-" : "",
		(flags & FLAG_PLUS)  ? "+" : "",
		(flags & FLAG_SPACE) ? " " : "",
		(flags & FLAG_ALT)   ? "#" : "",
		(flags & FLAG_ZERO)  ? "0" : "",
		(width) ? widstr : "",
		precision,
		(cflags & SIZE_LDOUBLE) ? "L" : "",
		(flags & FLAG_E) ? (flags & FLAG_UP) ? "E" : "e" :
		(flags & FLAG_G) ? (flags & FLAG_UP) ? "G" : "g" : "f"
	);

	/* Delegate to sprintf() */

	if (cflags & SIZE_LDOUBLE)
		sprintf(convert, format, fvalue);
	else
		sprintf(convert, format, (double)fvalue);

	/* Copy to buffer */

	for (p = convert; *p; ++p)
		outch(buffer, currlen, reqlen, size, *p);

	/* Deallocate if necessary */

	if (convert != buf)
		free(convert);
}

int vsnprintf(char *str, size_t size, const char *format, va_list args)
{
	char ch;
	long value;
	LDOUBLE fvalue;
	char *strvalue;
	int width;
	int precision;
	int state;
	int flags;
	int cflags;
	size_t currlen;
	size_t reqlen;

	state = PARSE_DEFAULT;
	currlen = reqlen = flags = cflags = width = 0;
	precision = -1;
	ch = *format++;

	while (state != PARSE_DONE)
	{
		if (ch == '\0')
		{
			if (state == PARSE_DEFAULT)
				state = PARSE_DONE;
			else
				return -1;
		}

		switch (state)
		{
			case PARSE_DEFAULT:
				if (ch == '%')
					state = PARSE_FLAGS;
				else
					outch(str, &currlen, &reqlen, size, ch);
				ch = *format++;
				break;

			case PARSE_FLAGS:
				switch (ch)
				{
					case '-':
						flags |= FLAG_MINUS;
						ch = *format++;
						break;

					case '+':
						flags |= FLAG_PLUS;
						ch = *format++;
						break;

					case ' ':
						flags |= FLAG_SPACE;
						ch = *format++;
						break;

					case '#':
						flags |= FLAG_ALT;
						ch = *format++;
						break;

					case '0':
						flags |= FLAG_ZERO;
						ch = *format++;
						break;

					default:
						state = PARSE_WIDTH;
						break;
				}
				break;

			case PARSE_WIDTH:
				if (isdigit((int)(unsigned char)ch))
				{
					width = 10 * width + ch - '0';
					ch = *format++;
				}
				else if (ch == '*')
				{
					width = va_arg(args, int);
					ch = *format++;
					state = PARSE_DOT;
				}
				else
					state = PARSE_DOT;
				break;

			case PARSE_DOT:
				if (ch == '.')
				{
					precision = 0;
					state = PARSE_PRECISION;
					ch = *format++;
				}
				else
					state = PARSE_SIZE;
				break;

			case PARSE_PRECISION:
				if (isdigit((int)(unsigned char)ch))
				{
					precision = 10 * precision + ch - '0';
					ch = *format++;
				}
				else if (ch == '*')
				{
					precision = va_arg(args, int);
					ch = *format++;
					state = PARSE_SIZE;
				}
				else
					state = PARSE_SIZE;
				break;

			case PARSE_SIZE:
				switch (ch)
				{
					case 'h':
						cflags = SIZE_SHORT;
						ch = *format++;
						break;

					case 'l':
						cflags = SIZE_LONG;
						ch = *format++;
						break;

					case 'L':
						cflags = SIZE_LDOUBLE;
						ch = *format++;
						break;
				}

				state = PARSE_CONVERSION;
				break;

			case PARSE_CONVERSION:
				switch (ch)
				{
					case 'd':
					case 'i':
						if (cflags == SIZE_SHORT)
							value = (short)va_arg(args, int);
						else if (cflags == SIZE_LONG)
							value = (long)va_arg(args, long);
						else
							value = (int)va_arg(args, int);

						fmtint(str, &currlen, &reqlen, size, value, 10, width, precision, flags);
						break;

					case 'o':
						flags |= FLAG_UNSIGNED;
						if (cflags == SIZE_SHORT)
							value = (unsigned short)va_arg(args, int);
						else if (cflags == SIZE_LONG)
							value = (unsigned long)va_arg(args, long);
						else
							value = (unsigned int)va_arg(args, int);

						fmtint(str, &currlen, &reqlen, size, value, 8, width, precision, flags);
						break;

					case 'u':
						flags |= FLAG_UNSIGNED;
						if (cflags == SIZE_SHORT)
							value = (unsigned short)va_arg(args, int);
						else if (cflags == SIZE_LONG)
							value = (unsigned long)va_arg(args, long);
						else
							value = (unsigned int)va_arg(args, int);

						fmtint(str, &currlen, &reqlen, size, value, 10, width, precision, flags);
						break;

					case 'X':
						flags |= FLAG_UP;
					case 'x':
						flags |= FLAG_UNSIGNED;
						if (cflags == SIZE_SHORT)
							value = (unsigned short)va_arg(args, int);
						else if (cflags == SIZE_LONG)
							value = (unsigned long)va_arg(args, long);
						else
							value = (unsigned int)va_arg(args, int);

						fmtint(str, &currlen, &reqlen, size, value, 16, width, precision, flags);
						break;

					case 'f':
						flags |= FLAG_F;
						if (cflags == SIZE_LDOUBLE)
							fvalue = va_arg(args, LDOUBLE);
						else
							fvalue = (LDOUBLE)va_arg(args, double);

						fmtfp(str, &currlen, &reqlen, size, fvalue, width, precision, flags, cflags);
						break;

					case 'E':
						flags |= FLAG_UP;
					case 'e':
						flags |= FLAG_E;
						if (cflags == SIZE_LDOUBLE)
							fvalue = va_arg(args, LDOUBLE);
						else
							fvalue = (LDOUBLE)va_arg(args, double);

						fmtfp(str, &currlen, &reqlen, size, fvalue, width, precision, flags, cflags);
						break;

					case 'G':
						flags |= FLAG_UP;
					case 'g':
						flags |= FLAG_G;
						if (cflags == SIZE_LDOUBLE)
							fvalue = va_arg(args, LDOUBLE);
						else
							fvalue = (LDOUBLE)va_arg(args, double);

						fmtfp(str, &currlen, &reqlen, size, fvalue, width, precision, flags, cflags);
						break;

					case 'c':
						fmtch(str, &currlen, &reqlen, size, va_arg(args, int), flags, width);
						break;

					case 's':
						strvalue = va_arg(args, char *);
						fmtstr(str, &currlen, &reqlen, size, strvalue, flags, width, precision);
						break;

					case 'p':
						flags |= FLAG_UNSIGNED;
#ifdef HAVE_PRINTF_PTR_FMT_ALTERNATE
						flags |= FLAG_ALT;
#endif
#ifdef HAVE_PRINTF_PTR_FMT_SIGNED
						flags |= FLAG_PTR_SIGNED;
#endif
#ifdef HAVE_PRINTF_PTR_FMT_NIL
						flags |= FLAG_PTR_NIL;
#endif
#ifdef HAVE_PRINTF_PTR_FMT_NOALT
						flags |= FLAG_PTR_NOALT;
#endif
						cflags = SIZE_LONG;
						strvalue = va_arg(args, void *);
						fmtint(str, &currlen, &reqlen, size, (long)strvalue, 16, width, precision, flags);
						break;

					case 'n':
						if (cflags == SIZE_SHORT)
						{
							short *num = va_arg(args, short *);
							*num = (short)currlen;
						}
						else if (cflags == SIZE_LONG)
						{
							long *num = va_arg(args, long *);
							*num = (long)currlen;
						}
						else
						{
							int *num = va_arg(args, int *);
							*num = currlen;
						}

						break;

					case '%':
						if (flags != 0 || cflags != 0 || width != 0 || precision != 0)
							return -1;

						outch(str, &currlen, &reqlen, size, ch);
						break;

					default:
						return -1;
				}

				ch = *format++;
				state = PARSE_DEFAULT;
				flags = cflags = width = 0;
				precision = -1;
				break;

			case PARSE_DONE:
				break;
		}
	}

	if (size)
		str[currlen] = '\0';

	return reqlen;
}
#endif

/*

=back

=head1 MT-Level

MT-Safe - provided that the locale is only set by the main thread before
starting any other threads.

=head1 EXAMPLES

How long is a piece of string?

    #include <slack/std.h>
    #ifndef HAVE_SNPRINTF
    #include <slack/snprintf.h>
    #endif

    int main(int ac, char **av)
    {
        char *str;
        int len;

        len = snprintf(NULL, 0, "%s %d", *av, ac);
        printf("this string has length %d\n", len);

        if (!(str = malloc((len + 1) * sizeof(char))))
            return EXIT_FAILURE;

        len = snprintf(str, len + 1, "%s %d", *av, ac);
        printf("%s %d\n", str, len);

        free(str);

        return EXIT_SUCCESS;
    }

Check if truncation occurred:

    #include <slack/std.h>
    #ifndef HAVE_SNPRINTF
    #include <slack/snprintf.h>
    #endif

    int main()
    {
        char str[16];
        int len;

        len = snprintf(str, 16, "%s %d", "hello world", 1000);
        printf("%s\n", str);

        if (len >= 16)
            printf("length truncated (from %d)\n", len);

        return EXIT_SUCCESS;
    }

Allocate memory only when needed to prevent truncation:

    #include <slack/std.h>
    #ifndef HAVE_SNPRINTF
    #include <slack/snprintf.h>
    #endif

    int main(int ac, char **av)
    {
        char buf[16];
        char *str = buf;
        char *extra = NULL;
        int len;

        if (!av[1])
            return EXIT_FAILURE;

        if ((len = snprintf(buf, 16, "%s", av[1])) >= 16)
            if (extra = malloc((len + 1) * sizeof(char)))
                snprintf(str = extra, len + 1, "%s", av[1]);

        printf("%s\n", str);

        if (extra)
            free(extra);

        return EXIT_SUCCESS;
    }

=head1 BUGS

The format control string, C<format>, should be interpreted as a multibyte
character sequence but it is not. Apart from that, these functions comply
with the ISO C 89 formatting requirements specified for the I<fprintf(3)>
function (section 7.9.6.1).

Even though I<snprintf(3)> is an ISO C 99 function (section 7.19.6.5), this
implementation does not support the new ISO C 99 formatting conversions or
length modifiers (i.e. C<%hh[diouxXn]>, C<%ll[diouxXn]>, C<%j[diouxXn]>,
C<%z[diouxXn]>, C<%t[diouxXn]>, C<%ls>, C<%lc> and C<%[aAF]>). The main
reason is that the local system's I<sprintf(3)> function is used to perform
floating point formatting. If the local system can support C<%[aA]>, then
you must have C99 already and so you must also have I<snprintf(3)> already.

If I<snprintf(3)> or I<vsnprintf(3)> require more than 512 bytes of space in
which to format a floating point number, but fail to allocate the required
space, the floating point number will not be formatted at all and processing
will continue. There is no indication to the client that an error occurred.
The chances of this happening are remote. It would take a field width or
precision greater than the available memory to trigger this bug. Since there
are only 15 significant digits in a I<double> and only 18 significant digits
in an 80 bit I<long double> (33 significant digits in a 128 bit long
double), a precision larger than 15/18/33 serves no purpose and a field
width larger than the useful output serves no purpose.

=head1 SEE ALSO

I<printf(3)>,
I<sprintf(3)>,
I<vsprintf(3)>

=head1 AUTHOR

2002-2010 raf <raf@raf.org>,
1998 Andrew Tridgell <tridge@samba.org>,
1998 Michael Elkins <me@cs.hmc.edu>,
1998 Thomas Roessler <roessler@guug.de>,
1996-1997 Brandon Long <blong@fiction.net>,
1995 Patrick Powell <papowell@astart.com>

=cut

*/

#endif

#ifdef TEST_SNPRINTF
#define TEST
#endif

#ifdef TEST

#include <stdio.h>

#ifndef LONG_STRING
#define LONG_STRING (16 * 1024)
#endif

/*
** Note: I wanted to use 10000 (rather than 256) as the "huge" precision
** values in some of the floating point tests below. Under Linux, using
** 10000 worked fine, but under Solaris 2.6, even though we're using the
** system's own sprintf() function to render floating point numbers, 322
** tests failed (badly). Even when the tests were modified to compare
** sprintf() with itself, 198 tests failed. Suffice to say that the Solaris'
** sprintf() is flaky. It can only handle precisions no greater than 260
** (for double) and no greater than 508 (for long double). However, that's
** far greater than the actual number of significant digits in a double (15)
** or a long double (18 digits for 80 bit or 33 digits for 128 bit) so
** there's no problem.
*/

char *fp_fmt[] =
{
	"%f", "%-f", "%+f", "% f", "%#f", "%0f", "%-+ #0f",
	"%9f", "%-9f", "%+9f", "% 9f", "%#9f", "%09f", "%-+ #09f",
	"%.9f", "%-.9f", "%+.9f", "% .9f", "%#.9f", "%0.9f", "%-+ #0.9f",
	"%9.f", "%-9.f", "%+9.f", "% 9.f", "%#9.f", "%09.f", "%-+ #09.f",
	"%9.9f", "%-9.9f", "%+9.9f", "% 9.9f", "%#9.9f", "%09.9f", "%-+ #09.9f",
	"%.0f", "%-.0f", "%+.0f", "% .0f", "%#.0f", "%0.0f", "%-+ #0.0f",
	"%.1f", "%-.1f", "%+.1f", "% .1f", "%#.1f", "%0.1f", "%-+ #0.1f",
	"%.256f", "%-.256f", "%+.256f", "% .256f", "%#.256f", "%0.256f", "%-+ #0.256f",
	"%10000f", "%-10000f", "%+10000f", "% 10000f", "%#10000f", "%010000f", "%-+ #010000f",
	"%10000.256f", "%-10000.256f", "%+10000.256f", "% 10000.256f", "%#10000.256f", "%010000.256f", "%-+ #010000.256f",

	"%e", "%-e", "%+e", "% e", "%#e", "%0e", "%-+ #0e",
	"%9e", "%-9e", "%+9e", "% 9e", "%#9e", "%09e", "%-+ #09e",
	"%.9e", "%-.9e", "%+.9e", "% .9e", "%#.9e", "%0.9e", "%-+ #0.9e",
	"%9.e", "%-9.e", "%+9.e", "% 9.e", "%#9.e", "%09.e", "%-+ #09.e",
	"%9.9e", "%-9.9e", "%+9.9e", "% 9.9e", "%#9.9e", "%09.9e", "%-+ #09.9e",
	"%.0e", "%-.0e", "%+.0e", "% .0e", "%#.0e", "%0.0e", "%-+ #0.0e",
	"%.1e", "%-.1e", "%+.1e", "% .1e", "%#.1e", "%0.1e", "%-+ #0.1e",
	"%.256e", "%-.256e", "%+.256e", "% .256e", "%#.256e", "%0.256e", "%-+ #0.256e",
	"%10000e", "%-10000e", "%+10000e", "% 10000e", "%#10000e", "%010000e", "%-+ #010000e",
	"%10000.256e", "%-10000.256e", "%+10000.256e", "% 10000.256e", "%#10000.256e", "%010000.256e", "%-+ #010000.256e",

	"%E", "%-E", "%+E", "% E", "%#E", "%0E", "%-+ #0E",
	"%9E", "%-9E", "%+9E", "% 9E", "%#9E", "%09E", "%-+ #09E",
	"%.9E", "%-.9E", "%+.9E", "% .9E", "%#.9E", "%0.9E", "%-+ #0.9E",
	"%9.E", "%-9.E", "%+9.E", "% 9.E", "%#9.E", "%09.E", "%-+ #09.E",
	"%9.9E", "%-9.9E", "%+9.9E", "% 9.9E", "%#9.9E", "%09.9E", "%-+ #09.9E",
	"%.0E", "%-.0E", "%+.0E", "% .0E", "%#.0E", "%0.0E", "%-+ #0.0E",
	"%.1E", "%-.1E", "%+.1E", "% .1E", "%#.1E", "%0.1E", "%-+ #0.1E",
	"%.256E", "%-.256E", "%+.256E", "% .256E", "%#.256E", "%0.256E", "%-+ #0.256E",
	"%10000E", "%-10000E", "%+10000E", "% 10000E", "%#10000E", "%010000E", "%-+ #010000E",
	"%10000.256E", "%-10000.256E", "%+10000.256E", "% 10000.256E", "%#10000.256E", "%010000.256E", "%-+ #010000.256E",

	"%g", "%-g", "%+g", "% g", "%#g", "%0g", "%-+ #0g",
	"%9g", "%-9g", "%+9g", "% 9g", "%#9g", "%09g", "%-+ #09g",
	"%.9g", "%-.9g", "%+.9g", "% .9g", "%#.9g", "%0.9g", "%-+ #0.9g",
	"%9.g", "%-9.g", "%+9.g", "% 9.g", "%#9.g", "%09.g", "%-+ #09.g",
	"%9.9g", "%-9.9g", "%+9.9g", "% 9.9g", "%#9.9g", "%09.9g", "%-+ #09.9g",
	"%.0g", "%-.0g", "%+.0g", "% .0g", "%#.0g", "%0.0g", "%-+ #0.0g",
	"%.1g", "%-.1g", "%+.1g", "% .1g", "%#.1g", "%0.1g", "%-+ #0.1g",
	"%.256g", "%-.256g", "%+.256g", "% .256g", "%#.256g", "%0.256g", "%-+ #0.256g",
	"%10000g", "%-10000g", "%+10000g", "% 10000g", "%#10000g", "%010000g", "%-+ #010000g",
	"%10000.256g", "%-10000.256g", "%+10000.256g", "% 10000.256g", "%#10000.256g", "%010000.256g", "%-+ #010000.256g",

	"%G", "%-G", "%+G", "% G", "%#G", "%0G", "%-+ #0G",
	"%9G", "%-9G", "%+9G", "% 9G", "%#9G", "%09G", "%-+ #09G",
	"%.9G", "%-.9G", "%+.9G", "% .9G", "%#.9G", "%0.9G", "%-+ #0.9G",
	"%9.G", "%-9.G", "%+9.G", "% 9.G", "%#9.G", "%09.G", "%-+ #09.G",
	"%9.9G", "%-9.9G", "%+9.9G", "% 9.9G", "%#9.9G", "%09.9G", "%-+ #09.9G",
	"%.0G", "%-.0G", "%+.0G", "% .0G", "%#.0G", "%0.0G", "%-+ #0.0G",
	"%.1G", "%-.1G", "%+.1G", "% .1G", "%#.1G", "%0.1G", "%-+ #0.1G",
	"%.256G", "%-.256G", "%+.256G", "% .256G", "%#.256G", "%0.256G", "%-+ #0.256G",
	"%10000G", "%-10000G", "%+10000G", "% 10000G", "%#10000G", "%010000G", "%-+ #010000G",
	"%10000.256G", "%-10000.256G", "%+10000.256G", "% 10000.256G", "%#10000.256G", "%010000.256G", "%-+ #010000.256G",

	"%-1.5f", "%1.5f", "%123.9f", "%10.5f", "% 10.5f", "%+22.9f", "%+4.9f", "%01.3f", "%4f", "%3.1f", "%3.2f", "%.0f", "%.1f",
	"%-1.5e", "%1.5e", "%123.9e", "%10.5e", "% 10.5e", "%+22.9e", "%+4.9e", "%01.3e", "%4e", "%3.1e", "%3.2e", "%.0e", "%.1e",
	"%-1.5E", "%1.5E", "%123.9E", "%10.5E", "% 10.5E", "%+22.9E", "%+4.9E", "%01.3E", "%4E", "%3.1E", "%3.2E", "%.0E", "%.1E",
	"%-1.5g", "%1.5g", "%123.9g", "%10.5g", "% 10.5g", "%+22.9g", "%+4.9g", "%01.3g", "%4g", "%3.1g", "%3.2g", "%.0g", "%.1g",
	"%-1.5G", "%1.5G", "%123.9G", "%10.5G", "% 10.5G", "%+22.9G", "%+4.9G", "%01.3G", "%4G", "%3.1G", "%3.2G", "%.0G", "%.1G",
	NULL
};

char *fp_fmt2[] =
{
	"%*f", "%-*f", "%+*f", "% *f", "%#*f", "%0*f", "%-+ #0*f",
	"%*.9f", "%-*.9f", "%+*.9f", "% *.9f", "%#*.9f", "%0*.9f", "%-+ #0*.9f",
	"%*e", "%-*e", "%+*e", "% *e", "%#*e", "%0*e", "%-+ #0*e",
	"%*.9e", "%-*.9e", "%+*.9e", "% *.9e", "%#*.9e", "%0*.9e", "%-+ #0*.9e",
	"%*E", "%-*E", "%+*E", "% *E", "%#*E", "%0*E", "%-+ #0*E",
	"%*.9E", "%-*.9E", "%+*.9E", "% *.9E", "%#*.9E", "%0*.9E", "%-+ #0*.9E",
	"%*g", "%-*g", "%+*g", "% *g", "%#*g", "%0*g", "%-+ #0*g",
	"%*.9g", "%-*.9g", "%+*.9g", "% *.9g", "%#*.9g", "%0*.9g", "%-+ #0*.9g",
	"%*G", "%-*G", "%+*G", "% *G", "%#*G", "%0*G", "%-+ #0*G",
	"%*.9G", "%-*.9G", "%+*.9G", "% *.9G", "%#*.9G", "%0*.9G", "%-+ #0*.9G",
	NULL
};

char *fp_fmt3[] =
{
	"%.*f", "%-.*f", "%+.*f", "% .*f", "%#.*f", "%0.*f", "%-+ #0.*f",
	"%9.*f", "%-9.*f", "%+9.*f", "% 9.*f", "%#9.*f", "%09.*f", "%-+ #09.*f",
	"%.*e", "%-.*e", "%+.*e", "% .*e", "%#.*e", "%0.*e", "%-+ #0.*e",
	"%9.*e", "%-9.*e", "%+9.*e", "% 9.*e", "%#9.*e", "%09.*e", "%-+ #09.*e",
	"%.*E", "%-.*E", "%+.*E", "% .*E", "%#.*E", "%0.*E", "%-+ #0.*E",
	"%9.*E", "%-9.*E", "%+9.*E", "% 9.*E", "%#9.*E", "%09.*E", "%-+ #09.*E",
	"%.*g", "%-.*g", "%+.*g", "% .*g", "%#.*g", "%0.*g", "%-+ #0.*g",
	"%9.*g", "%-9.*g", "%+9.*g", "% 9.*g", "%#9.*g", "%09.*g", "%-+ #09.*g",
	"%.*G", "%-.*G", "%+.*G", "% .*G", "%#.*G", "%0.*G", "%-+ #0.*G",
	"%9.*G", "%-9.*G", "%+9.*G", "% 9.*G", "%#9.*G", "%09.*G", "%-+ #09.*G",
	NULL
};

char *fp_fmt4[] =
{
	"%*.*f", "%-*.*f", "%+*.*f", "% *.*f", "%#*.*f", "%0*.*f", "%-+ #0*.*f",
	"%*.*e", "%-*.*e", "%+*.*e", "% *.*e", "%#*.*e", "%0*.*e", "%-+ #0*.*e",
	"%*.*E", "%-*.*E", "%+*.*E", "% *.*E", "%#*.*E", "%0*.*E", "%-+ #0*.*E",
	"%*.*g", "%-*.*g", "%+*.*g", "% *.*g", "%#*.*g", "%0*.*g", "%-+ #0*.*g",
	"%*.*G", "%-*.*G", "%+*.*G", "% *.*G", "%#*.*G", "%0*.*G", "%-+ #0*.*G",
	NULL
};

char *fp_ldbl_fmt[] =
{
	"%Lf", "%-Lf", "%+Lf", "% Lf", "%#Lf", "%0Lf", "%-+ #0Lf",
	"%9Lf", "%-9Lf", "%+9Lf", "% 9Lf", "%#9Lf", "%09Lf", "%-+ #09Lf",
	"%.9Lf", "%-.9Lf", "%+.9Lf", "% .9Lf", "%#.9Lf", "%0.9Lf", "%-+ #0.9Lf",
	"%9.Lf", "%-9.Lf", "%+9.Lf", "% 9.Lf", "%#9.Lf", "%09.Lf", "%-+ #09.Lf",
	"%9.9Lf", "%-9.9Lf", "%+9.9Lf", "% 9.9Lf", "%#9.9Lf", "%09.9Lf", "%-+ #09.9Lf",
	"%.0Lf", "%-.0Lf", "%+.0Lf", "% .0Lf", "%#.0Lf", "%0.0Lf", "%-+ #0.0Lf",
	"%.1Lf", "%-.1Lf", "%+.1Lf", "% .1Lf", "%#.1Lf", "%0.1Lf", "%-+ #0.1Lf",
	"%.256Lf", "%-.256Lf", "%+.256Lf", "% .256Lf", "%#.256Lf", "%0.256Lf", "%-+ #0.256Lf",
	"%10000Lf", "%-10000Lf", "%+10000Lf", "% 10000Lf", "%#10000Lf", "%010000Lf", "%-+ #010000Lf",
	"%10000.256Lf", "%-10000.256Lf", "%+10000.256Lf", "% 10000.256Lf", "%#10000.256Lf", "%010000.256Lf", "%-+ #010000.256Lf",

	"%Le", "%-Le", "%+Le", "% Le", "%#Le", "%0Le", "%-+ #0Le",
	"%9Le", "%-9Le", "%+9Le", "% 9Le", "%#9Le", "%09Le", "%-+ #09Le",
	"%.9Le", "%-.9Le", "%+.9Le", "% .9Le", "%#.9Le", "%0.9Le", "%-+ #0.9Le",
	"%9.Le", "%-9.Le", "%+9.Le", "% 9.Le", "%#9.Le", "%09.Le", "%-+ #09.Le",
	"%9.9Le", "%-9.9Le", "%+9.9Le", "% 9.9Le", "%#9.9Le", "%09.9Le", "%-+ #09.9Le",
	"%.0Le", "%-.0Le", "%+.0Le", "% .0Le", "%#.0Le", "%0.0Le", "%-+ #0.0Le",
	"%.1Le", "%-.1Le", "%+.1Le", "% .1Le", "%#.1Le", "%0.1Le", "%-+ #0.1Le",
	"%.256Le", "%-.256Le", "%+.256Le", "% .256Le", "%#.256Le", "%0.256Le", "%-+ #0.256Le",
	"%10000Le", "%-10000Le", "%+10000Le", "% 10000Le", "%#10000Le", "%010000Le", "%-+ #010000Le",
	"%10000.256Le", "%-10000.256Le", "%+10000.256Le", "% 10000.256Le", "%#10000.256Le", "%010000.256Le", "%-+ #010000.256Le",

	"%LE", "%-LE", "%+LE", "% LE", "%#LE", "%0LE", "%-+ #0LE",
	"%9LE", "%-9LE", "%+9LE", "% 9LE", "%#9LE", "%09LE", "%-+ #09LE",
	"%.9LE", "%-.9LE", "%+.9LE", "% .9LE", "%#.9LE", "%0.9LE", "%-+ #0.9LE",
	"%9.LE", "%-9.LE", "%+9.LE", "% 9.LE", "%#9.LE", "%09.LE", "%-+ #09.LE",
	"%9.9LE", "%-9.9LE", "%+9.9LE", "% 9.9LE", "%#9.9LE", "%09.9LE", "%-+ #09.9LE",
	"%.0LE", "%-.0LE", "%+.0LE", "% .0LE", "%#.0LE", "%0.0LE", "%-+ #0.0LE",
	"%.1LE", "%-.1LE", "%+.1LE", "% .1LE", "%#.1LE", "%0.1LE", "%-+ #0.1LE",
	"%.256LE", "%-.256LE", "%+.256LE", "% .256LE", "%#.256LE", "%0.256LE", "%-+ #0.256LE",
	"%10000LE", "%-10000LE", "%+10000LE", "% 10000LE", "%#10000LE", "%010000LE", "%-+ #010000LE",
	"%10000.256LE", "%-10000.256LE", "%+10000.256LE", "% 10000.256LE", "%#10000.256LE", "%010000.256LE", "%-+ #010000.256LE",

	"%Lg", "%-Lg", "%+Lg", "% Lg", "%#Lg", "%0Lg", "%-+ #0Lg",
	"%9Lg", "%-9Lg", "%+9Lg", "% 9Lg", "%#9Lg", "%09Lg", "%-+ #09Lg",
	"%.9Lg", "%-.9Lg", "%+.9Lg", "% .9Lg", "%#.9Lg", "%0.9Lg", "%-+ #0.9Lg",
	"%9.Lg", "%-9.Lg", "%+9.Lg", "% 9.Lg", "%#9.Lg", "%09.Lg", "%-+ #09.Lg",
	"%9.9Lg", "%-9.9Lg", "%+9.9Lg", "% 9.9Lg", "%#9.9Lg", "%09.9Lg", "%-+ #09.9Lg",
	"%.0Lg", "%-.0Lg", "%+.0Lg", "% .0Lg", "%#.0Lg", "%0.0Lg", "%-+ #0.0Lg",
	"%.1Lg", "%-.1Lg", "%+.1Lg", "% .1Lg", "%#.1Lg", "%0.1Lg", "%-+ #0.1Lg",
	"%.256Lg", "%-.256Lg", "%+.256Lg", "% .256Lg", "%#.256Lg", "%0.256Lg", "%-+ #0.256Lg",
	"%10000Lg", "%-10000Lg", "%+10000Lg", "% 10000Lg", "%#10000Lg", "%010000Lg", "%-+ #010000Lg",
	"%10000.256Lg", "%-10000.256Lg", "%+10000.256Lg", "% 10000.256Lg", "%#10000.256Lg", "%010000.256Lg", "%-+ #010000.256Lg",

	"%LG", "%-LG", "%+LG", "% LG", "%#LG", "%0LG", "%-+ #0LG",
	"%9LG", "%-9LG", "%+9LG", "% 9LG", "%#9LG", "%09LG", "%-+ #09LG",
	"%.9LG", "%-.9LG", "%+.9LG", "% .9LG", "%#.9LG", "%0.9LG", "%-+ #0.9LG",
	"%9.LG", "%-9.LG", "%+9.LG", "% 9.LG", "%#9.LG", "%09.LG", "%-+ #09.LG",
	"%9.9LG", "%-9.9LG", "%+9.9LG", "% 9.9LG", "%#9.9LG", "%09.9LG", "%-+ #09.9LG",
	"%.0LG", "%-.0LG", "%+.0LG", "% .0LG", "%#.0LG", "%0.0LG", "%-+ #0.0LG",
	"%.1LG", "%-.1LG", "%+.1LG", "% .1LG", "%#.1LG", "%0.1LG", "%-+ #0.1LG",
	"%.256LG", "%-.256LG", "%+.256LG", "% .256LG", "%#.256LG", "%0.256LG", "%-+ #0.256LG",
	"%10000LG", "%-10000LG", "%+10000LG", "% 10000LG", "%#10000LG", "%010000LG", "%-+ #010000LG",
	"%10000.256LG", "%-10000.256LG", "%+10000.256LG", "% 10000.256LG", "%#10000.256LG", "%010000.256LG", "%-+ #010000.256LG",

	"%-1.5Lf", "%1.5Lf", "%123.9Lf", "%10.5Lf", "% 10.5Lf", "%+22.9Lf", "%+4.9Lf", "%01.3Lf", "%4Lf", "%3.1Lf", "%3.2Lf", "%.0Lf", "%.1Lf",
	"%-1.5Le", "%1.5Le", "%123.9Le", "%10.5Le", "% 10.5Le", "%+22.9Le", "%+4.9Le", "%01.3Le", "%4Le", "%3.1Le", "%3.2Le", "%.0Le", "%.1Le",
	"%-1.5LE", "%1.5LE", "%123.9LE", "%10.5LE", "% 10.5LE", "%+22.9LE", "%+4.9LE", "%01.3LE", "%4LE", "%3.1LE", "%3.2LE", "%.0LE", "%.1LE",
	"%-1.5Lg", "%1.5Lg", "%123.9Lg", "%10.5Lg", "% 10.5Lg", "%+22.9Lg", "%+4.9Lg", "%01.3Lg", "%4Lg", "%3.1Lg", "%3.2Lg", "%.0Lg", "%.1Lg",
	"%-1.5LG", "%1.5LG", "%123.9LG", "%10.5LG", "% 10.5LG", "%+22.9LG", "%+4.9LG", "%01.3LG", "%4LG", "%3.1LG", "%3.2LG", "%.0LG", "%.1LG",
	NULL
};

char *fp_ldbl_fmt2[] =
{
	"%*Lf", "%-*Lf", "%+*Lf", "% *Lf", "%#*Lf", "%0*Lf", "%-+ #0*Lf",
	"%*Le", "%-*Le", "%+*Le", "% *Le", "%#*Le", "%0*Le", "%-+ #0*Le",
	"%*LE", "%-*LE", "%+*LE", "% *LE", "%#*LE", "%0*LE", "%-+ #0*LE",
	"%*Lg", "%-*Lg", "%+*Lg", "% *Lg", "%#*Lg", "%0*Lg", "%-+ #0*Lg",
	"%*LG", "%-*LG", "%+*LG", "% *LG", "%#*LG", "%0*LG", "%-+ #0*LG",
	NULL
};

char *fp_ldbl_fmt3[] =
{
	"%.*Lf", "%-.*Lf", "%+.*Lf", "% .*Lf", "%#.*Lf", "%0.*Lf", "%-+ #0.*Lf",
	"%.*Le", "%-.*Le", "%+.*Le", "% .*Le", "%#.*Le", "%0.*Le", "%-+ #0.*Le",
	"%.*LE", "%-.*LE", "%+.*LE", "% .*LE", "%#.*LE", "%0.*LE", "%-+ #0.*LE",
	"%.*Lg", "%-.*Lg", "%+.*Lg", "% .*Lg", "%#.*Lg", "%0.*Lg", "%-+ #0.*Lg",
	"%.*LG", "%-.*LG", "%+.*LG", "% .*LG", "%#.*LG", "%0.*LG", "%-+ #0.*LG",
	NULL
};

char *fp_ldbl_fmt4[] =
{
	"%*.*Lf", "%-*.*Lf", "%+*.*Lf", "% *.*Lf", "%#*.*Lf", "%0*.*Lf", "%-+ #0*.*Lf",
	"%*.*Le", "%-*.*Le", "%+*.*Le", "% *.*Le", "%#*.*Le", "%0*.*Le", "%-+ #0*.*Le",
	"%*.*LE", "%-*.*LE", "%+*.*LE", "% *.*LE", "%#*.*LE", "%0*.*LE", "%-+ #0*.*LE",
	"%*.*Lg", "%-*.*Lg", "%+*.*Lg", "% *.*Lg", "%#*.*Lg", "%0*.*Lg", "%-+ #0*.*Lg",
	"%*.*LG", "%-*.*LG", "%+*.*LG", "% *.*LG", "%#*.*LG", "%0*.*LG", "%-+ #0*.*LG",
	NULL
};

LDOUBLE fp_nums[] =
{
	-1.5, 134.21, 91340.2, 341.1234, 0203.9, 0.96, 0.996, 0.9996, 1.996,
	4.136, 0, 1.5, -134.21, -91340.2, -341.1234, -0203.9, -0.96, -0.996,
	-0.9996, -1.996, -4.136, 1.5e-1, -1.5e+250, 203.452e-250
};

char *int_fmt[] =
{
	"%d", "%-d", "%+d", "% d", "%#d", "%0d", "%-+ #0d",
	"%9d", "%-9d", "%+9d", "% 9d", "%#9d", "%09d", "%-+ #09d",
	"%.9d", "%-.9d", "%+.9d", "% .9d", "%#.9d", "%0.9d", "%-+ #0.9d",
	"%9.d", "%-9.d", "%+9.d", "% 9.d", "%#9.d", "%09.d", "%-+ #09.d",
	"%9.9d", "%-9.9d", "%+9.9d", "% 9.9d", "%#9.9d", "%09.9d", "%-+ #09.9d",
	"%.0d", "%-.0d", "%+.0d", "% .0d", "%#.0d", "%0.0d", "%-+ #0.0d",
	"%.1d", "%-.1d", "%+.1d", "% .1d", "%#.1d", "%0.1d", "%-+ #0.1d",
	"%.10000d", "%-.10000d", "%+.10000d", "% .10000d", "%#.10000d", "%0.10000d", "%-+ #0.10000d",
	"%10000d", "%-10000d", "%+10000d", "% 10000d", "%#10000d", "%010000d", "%-+ #010000d",
	"%10000.10000d", "%-10000.10000d", "%+10000.10000d", "% 10000.10000d", "%#10000.10000d", "%010000.10000d", "%-+ #010000.10000d",

	"%i", "%-i", "%+i", "% i", "%#i", "%0i", "%-+ #0i",
	"%9i", "%-9i", "%+9i", "% 9i", "%#9i", "%09i", "%-+ #09i",
	"%.9i", "%-.9i", "%+.9i", "% .9i", "%#.9i", "%0.9i", "%-+ #0.9i",
	"%9.i", "%-9.i", "%+9.i", "% 9.i", "%#9.i", "%09.i", "%-+ #09.i",
	"%9.9i", "%-9.9i", "%+9.9i", "% 9.9i", "%#9.9i", "%09.9i", "%-+ #09.9i",
	"%.0i", "%-.0i", "%+.0i", "% .0i", "%#.0i", "%0.0i", "%-+ #0.0i",
	"%.1i", "%-.1i", "%+.1i", "% .1i", "%#.1i", "%0.1i", "%-+ #0.1i",
	"%.10000i", "%-.10000i", "%+.10000i", "% .10000i", "%#.10000i", "%0.10000i", "%-+ #0.10000i",
	"%10000i", "%-10000i", "%+10000i", "% 10000i", "%#10000i", "%010000i", "%-+ #010000i",
	"%10000.10000i", "%-10000.10000i", "%+10000.10000i", "% 10000.10000i", "%#10000.10000i", "%010000.10000i", "%-+ #010000.10000i",

	"%-1.5d", "%1.5d", "%123.9d", "%5.5d", "%10.5d", "% 10.5d", "%+22.33d", "%01.3d", "%4d",
	"%-1.5i", "%1.5i", "%123.9i", "%5.5i", "%10.5i", "% 10.5i", "%+22.33i", "%01.3i", "%4i",
	NULL
};

char *int_fmt2[] =
{
	"%*d", "%-*d", "%+*d", "% *d", "%#*d", "%0*d", "%-+ #0*d",
	"%*i", "%-*i", "%+*i", "% *i", "%#*i", "%0*i", "%-+ #0*i",
	NULL
};

char *int_fmt3[] =
{
	"%.*d", "%-.*d", "%+.*d", "% .*d", "%#.*d", "%0.*d", "%-+ #0.*d",
	"%.*i", "%-.*i", "%+.*i", "% .*i", "%#.*i", "%0.*i", "%-+ #0.*i",
	NULL
};

char *int_fmt4[] =
{
	"%*.*d", "%-*.*d", "%+*.*d", "% *.*d", "%#*.*d", "%0*.*d", "%-+ #0*.*d",
	"%*.*i", "%-*.*i", "%+*.*i", "% *.*i", "%#*.*i", "%0*.*i", "%-+ #0*.*i",
	NULL
};

char *hint_fmt[] =
{
	"%hd", "%-hd", "%+hd", "% hd", "%#hd", "%0hd", "%-+ #0hd",
	"%9hd", "%-9hd", "%+9hd", "% 9hd", "%#9hd", "%09hd", "%-+ #09hd",
	"%.9hd", "%-.9hd", "%+.9hd", "% .9hd", "%#.9hd", "%0.9hd", "%-+ #0.9hd",
	"%9.hd", "%-9.hd", "%+9.hd", "% 9.hd", "%#9.hd", "%09.hd", "%-+ #09.hd",
	"%9.9hd", "%-9.9hd", "%+9.9hd", "% 9.9hd", "%#9.9hd", "%09.9hd", "%-+ #09.9hd",
	"%.0hd", "%-.0hd", "%+.0hd", "% .0hd", "%#.0hd", "%0.0hd", "%-+ #0.0hd",
	"%.1hd", "%-.1hd", "%+.1hd", "% .1hd", "%#.1hd", "%0.1hd", "%-+ #0.1hd",
	"%.10000hd", "%-.10000hd", "%+.10000hd", "% .10000hd", "%#.10000hd", "%0.10000hd", "%-+ #0.10000hd",
	"%10000hd", "%-10000hd", "%+10000hd", "% 10000hd", "%#10000hd", "%010000hd", "%-+ #010000hd",
	"%10000.10000hd", "%-10000.10000hd", "%+10000.10000hd", "% 10000.10000hd", "%#10000.10000hd", "%010000.10000hd", "%-+ #010000.10000hd",

	"%hi", "%-hi", "%+hi", "% hi", "%#hi", "%0hi", "%-+ #0hi",
	"%9hi", "%-9hi", "%+9hi", "% 9hi", "%#9hi", "%09hi", "%-+ #09hi",
	"%.9hi", "%-.9hi", "%+.9hi", "% .9hi", "%#.9hi", "%0.9hi", "%-+ #0.9hi",
	"%9.hi", "%-9.hi", "%+9.hi", "% 9.hi", "%#9.hi", "%09.hi", "%-+ #09.hi",
	"%9.9hi", "%-9.9hi", "%+9.9hi", "% 9.9hi", "%#9.9hi", "%09.9hi", "%-+ #09.9hi",
	"%.0hi", "%-.0hi", "%+.0hi", "% .0hi", "%#.0hi", "%0.0hi", "%-+ #0.0hi",
	"%.1hi", "%-.1hi", "%+.1hi", "% .1hi", "%#.1hi", "%0.1hi", "%-+ #0.1hi",
	"%.10000hi", "%-.10000hi", "%+.10000hi", "% .10000hi", "%#.10000hi", "%0.10000hi", "%-+ #0.10000hi",
	"%10000hi", "%-10000hi", "%+10000hi", "% 10000hi", "%#10000hi", "%010000hi", "%-+ #010000hi",
	"%10000.10000hi", "%-10000.10000hi", "%+10000.10000hi", "% 10000.10000hi", "%#10000.10000hi", "%010000.10000hi", "%-+ #010000.10000hi",

	"%-1.5hd", "%1.5hd", "%123.9hd", "%5.5hd", "%10.5hd", "% 10.5hd", "%+22.33hd", "%01.3hd", "%4hd",
	"%-1.5hi", "%1.5hi", "%123.9hi", "%5.5hi", "%10.5hi", "% 10.5hi", "%+22.33hi", "%01.3hi", "%4hi",
	NULL
};

char *hint_fmt2[] =
{
	"%*hd", "%-*hd", "%+*hd", "% *hd", "%#*hd", "%0*hd", "%-+ #0*hd",
	"%*hi", "%-*hi", "%+*hi", "% *hi", "%#*hi", "%0*hi", "%-+ #0*hi",
	NULL
};

char *hint_fmt3[] =
{
	"%.*hd", "%-.*hd", "%+.*hd", "% .*hd", "%#.*hd", "%0.*hd", "%-+ #0.*hd",
	"%.*hi", "%-.*hi", "%+.*hi", "% .*hi", "%#.*hi", "%0.*hi", "%-+ #0.*hi",
	NULL
};

char *hint_fmt4[] =
{
	"%*.*hd", "%-*.*hd", "%+*.*hd", "% *.*hd", "%#*.*hd", "%0*.*hd", "%-+ #0*.*hd",
	"%*.*hi", "%-*.*hi", "%+*.*hi", "% *.*hi", "%#*.*hi", "%0*.*hi", "%-+ #0*.*hi",
	NULL
};

char *lint_fmt[] =
{
	"%ld", "%-ld", "%+ld", "% ld", "%#ld", "%0ld", "%-+ #0ld",
	"%9ld", "%-9ld", "%+9ld", "% 9ld", "%#9ld", "%09ld", "%-+ #09ld",
	"%.9ld", "%-.9ld", "%+.9ld", "% .9ld", "%#.9ld", "%0.9ld", "%-+ #0.9ld",
	"%9.ld", "%-9.ld", "%+9.ld", "% 9.ld", "%#9.ld", "%09.ld", "%-+ #09.ld",
	"%9.9ld", "%-9.9ld", "%+9.9ld", "% 9.9ld", "%#9.9ld", "%09.9ld", "%-+ #09.9ld",
	"%.0ld", "%-.0ld", "%+.0ld", "% .0ld", "%#.0ld", "%0.0ld", "%-+ #0.0ld",
	"%.1ld", "%-.1ld", "%+.1ld", "% .1ld", "%#.1ld", "%0.1ld", "%-+ #0.1ld",
	"%.10000ld", "%-.10000ld", "%+.10000ld", "% .10000ld", "%#.10000ld", "%0.10000ld", "%-+ #0.10000ld",
	"%10000ld", "%-10000ld", "%+10000ld", "% 10000ld", "%#10000ld", "%010000ld", "%-+ #010000ld",
	"%10000.10000ld", "%-10000.10000ld", "%+10000.10000ld", "% 10000.10000ld", "%#10000.10000ld", "%010000.10000ld", "%-+ #010000.10000ld",

	"%li", "%-li", "%+li", "% li", "%#li", "%0li", "%-+ #0li",
	"%9li", "%-9li", "%+9li", "% 9li", "%#9li", "%09li", "%-+ #09li",
	"%.9li", "%-.9li", "%+.9li", "% .9li", "%#.9li", "%0.9li", "%-+ #0.9li",
	"%9.li", "%-9.li", "%+9.li", "% 9.li", "%#9.li", "%09.li", "%-+ #09.li",
	"%9.9li", "%-9.9li", "%+9.9li", "% 9.9li", "%#9.9li", "%09.9li", "%-+ #09.9li",
	"%.0li", "%-.0li", "%+.0li", "% .0li", "%#.0li", "%0.0li", "%-+ #0.0li",
	"%.1li", "%-.1li", "%+.1li", "% .1li", "%#.1li", "%0.1li", "%-+ #0.1li",
	"%.10000li", "%-.10000li", "%+.10000li", "% .10000li", "%#.10000li", "%0.10000li", "%-+ #0.10000li",
	"%10000li", "%-10000li", "%+10000li", "% 10000li", "%#10000li", "%010000li", "%-+ #010000li",
	"%10000.10000li", "%-10000.10000li", "%+10000.10000li", "% 10000.10000li", "%#10000.10000li", "%010000.10000li", "%-+ #010000.10000li",

	"%-1.5ld", "%1.5ld", "%123.9ld", "%5.5ld", "%10.5ld", "% 10.5ld", "%+22.33ld", "%01.3ld", "%4ld",
	"%-1.5li", "%1.5li", "%123.9li", "%5.5li", "%10.5li", "% 10.5li", "%+22.33li", "%01.3li", "%4li",
	NULL
};

char *lint_fmt2[] =
{
	"%*ld", "%-*ld", "%+*ld", "% *ld", "%#*ld", "%0*ld", "%-+ #0*ld",
	"%*li", "%-*li", "%+*li", "% *li", "%#*li", "%0*li", "%-+ #0*li",
	NULL
};

char *lint_fmt3[] =
{
	"%.*ld", "%-.*ld", "%+.*ld", "% .*ld", "%#.*ld", "%0.*ld", "%-+ #0.*ld",
	"%.*li", "%-.*li", "%+.*li", "% .*li", "%#.*li", "%0.*li", "%-+ #0.*li",
	NULL
};

char *lint_fmt4[] =
{
	"%*.*ld", "%-*.*ld", "%+*.*ld", "% *.*ld", "%#*.*ld", "%0*.*ld", "%-+ #0*.*ld",
	"%*.*li", "%-*.*li", "%+*.*li", "% *.*li", "%#*.*li", "%0*.*li", "%-+ #0*.*li",
	NULL
};

long int_nums[] = { -1, 134, 91340, 341, 0203, 0, -12345 };

char *uint_fmt[] =
{
	"%o", "%-o", "%+o", "% o", "%#o", "%0o", "%-+ #0o",
	"%9o", "%-9o", "%+9o", "% 9o", "%#9o", "%09o", "%-+ #09o",
	"%.9o", "%-.9o", "%+.9o", "% .9o", "%#.9o", "%0.9o", "%-+ #0.9o",
	"%9.o", "%-9.o", "%+9.o", "% 9.o", "%#9.o", "%09.o", "%-+ #09.o",
	"%9.9o", "%-9.9o", "%+9.9o", "% 9.9o", "%#9.9o", "%09.9o", "%-+ #09.9o",
	"%.0o", "%-.0o", "%+.0o", "% .0o", "%#.0o", "%0.0o", "%-+ #0.0o",
	"%.1o", "%-.1o", "%+.1o", "% .1o", "%#.1o", "%0.1o", "%-+ #0.1o",
	"%.10000o", "%-.10000o", "%+.10000o", "% .10000o", "%#.10000o", "%0.10000o", "%-+ #0.10000o",
	"%10000o", "%-10000o", "%+10000o", "% 10000o", "%#10000o", "%010000o", "%-+ #010000o",
	"%10000.10000o", "%-10000.10000o", "%+10000.10000o", "% 10000.10000o", "%#10000.10000o", "%010000.10000o", "%-+ #010000.10000o",

	"%u", "%-u", "%+u", "% u", "%#u", "%0u", "%-+ #0u",
	"%9u", "%-9u", "%+9u", "% 9u", "%#9u", "%09u", "%-+ #09u",
	"%.9u", "%-.9u", "%+.9u", "% .9u", "%#.9u", "%0.9u", "%-+ #0.9u",
	"%9.u", "%-9.u", "%+9.u", "% 9.u", "%#9.u", "%09.u", "%-+ #09.u",
	"%9.9u", "%-9.9u", "%+9.9u", "% 9.9u", "%#9.9u", "%09.9u", "%-+ #09.9u",
	"%.0u", "%-.0u", "%+.0u", "% .0u", "%#.0u", "%0.0u", "%-+ #0.0u",
	"%.1u", "%-.1u", "%+.1u", "% .1u", "%#.1u", "%0.1u", "%-+ #0.1u",
	"%.10000u", "%-.10000u", "%+.10000u", "% .10000u", "%#.10000u", "%0.10000u", "%-+ #0.10000u",
	"%10000u", "%-10000u", "%+10000u", "% 10000u", "%#10000u", "%010000u", "%-+ #010000u",
	"%10000.10000u", "%-10000.10000u", "%+10000.10000u", "% 10000.10000u", "%#10000.10000u", "%010000.10000u", "%-+ #010000.10000u",

	"%x", "%-x", "%+x", "% x", "%#x", "%0x", "%-+ #0x",
	"%9x", "%-9x", "%+9x", "% 9x", "%#9x", "%09x", "%-+ #09x",
	"%.9x", "%-.9x", "%+.9x", "% .9x", "%#.9x", "%0.9x", "%-+ #0.9x",
	"%9.x", "%-9.x", "%+9.x", "% 9.x", "%#9.x", "%09.x", "%-+ #09.x",
	"%9.9x", "%-9.9x", "%+9.9x", "% 9.9x", "%#9.9x", "%09.9x", "%-+ #09.9x",
	"%.0x", "%-.0x", "%+.0x", "% .0x", "%#.0x", "%0.0x", "%-+ #0.0x",
	"%.1x", "%-.1x", "%+.1x", "% .1x", "%#.1x", "%0.1x", "%-+ #0.1x",
	"%.10000x", "%-.10000x", "%+.10000x", "% .10000x", "%#.10000x", "%0.10000x", "%-+ #0.10000x",
	"%10000x", "%-10000x", "%+10000x", "% 10000x", "%#10000x", "%010000x", "%-+ #010000x",
	"%10000.10000x", "%-10000.10000x", "%+10000.10000x", "% 10000.10000x", "%#10000.10000x", "%010000.10000x", "%-+ #010000.10000x",

	"%X", "%-X", "%+X", "% X", "%#X", "%0X", "%-+ #0X",
	"%9X", "%-9X", "%+9X", "% 9X", "%#9X", "%09X", "%-+ #09X",
	"%.9X", "%-.9X", "%+.9X", "% .9X", "%#.9X", "%0.9X", "%-+ #0.9X",
	"%9.X", "%-9.X", "%+9.X", "% 9.X", "%#9.X", "%09.X", "%-+ #09.X",
	"%9.9X", "%-9.9X", "%+9.9X", "% 9.9X", "%#9.9X", "%09.9X", "%-+ #09.9X",
	"%.0X", "%-.0X", "%+.0X", "% .0X", "%#.0X", "%0.0X", "%-+ #0.0X",
	"%.1X", "%-.1X", "%+.1X", "% .1X", "%#.1X", "%0.1X", "%-+ #0.1X",
	"%.10000X", "%-.10000X", "%+.10000X", "% .10000X", "%#.10000X", "%0.10000X", "%-+ #0.10000X",
	"%10000X", "%-10000X", "%+10000X", "% 10000X", "%#10000X", "%010000X", "%-+ #010000X",
	"%10000.10000X", "%-10000.10000X", "%+10000.10000X", "% 10000.10000X", "%#10000.10000X", "%010000.10000X", "%-+ #010000.10000X",

	"%p", "%-p", "%+p", "% p", "%#p", "%0p", "%-+ #0p",
	"%9p", "%-9p", "%+9p", "% 9p", "%#9p", "%09p", "%-+ #09p",
	"%.9p", "%-.9p", "%+.9p", "% .9p", "%#.9p", "%0.9p", "%-+ #0.9p",
	"%9.p", "%-9.p", "%+9.p", "% 9.p", "%#9.p", "%09.p", "%-+ #09.p",
	"%9.9p", "%-9.9p", "%+9.9p", "% 9.9p", "%#9.9p", "%09.9p", "%-+ #09.9p",
	"%.0p", "%-.0p", "%+.0p", "% .0p", "%#.0p", "%0.0p", "%-+ #0.0p",
	"%.1p", "%-.1p", "%+.1p", "% .1p", "%#.1p", "%0.1p", "%-+ #0.1p",
	"%.10000p", "%-.10000p", "%+.10000p", "% .10000p", "%#.10000p", "%0.10000p", "%-+ #0.10000p",
	"%10000p", "%-10000p", "%+10000p", "% 10000p", "%#10000p", "%010000p", "%-+ #010000p",
	"%10000.10000p", "%-10000.10000p", "%+10000.10000p", "% 10000.10000p", "%#10000.10000p", "%010000.10000p", "%-+ #010000.10000p",

	"%-1.5o", "%1.5o", "%123.9o", "%5.5o", "%10.5o", "% 10.5o", "%+22.33o", "%01.3o", "%4o",
	"%-1.5u", "%1.5u", "%123.9u", "%5.5u", "%10.5u", "% 10.5u", "%+22.33u", "%01.3u", "%4u",
	"%-1.5x", "%1.5x", "%123.9x", "%5.5x", "%10.5x", "% 10.5x", "%+22.33x", "%01.3x", "%4x",
	"%-1.5X", "%1.5X", "%123.9X", "%5.5X", "%10.5X", "% 10.5X", "%+22.33X", "%01.3X", "%4X",
	"%-1.5p", "%1.5p", "%123.9p", "%5.5p", "%10.5p", "% 10.5p", "%+22.33p", "%01.3p", "%4p",
	NULL
};

char *uint_fmt2[] =
{
	"%*o", "%-*o", "%+*o", "% *o", "%#*o", "%0*o", "%-+ #0*o",
	"%*u", "%-*u", "%+*u", "% *u", "%#*u", "%0*u", "%-+ #0*u",
	"%*x", "%-*x", "%+*x", "% *x", "%#*x", "%0*x", "%-+ #0*x",
	"%*X", "%-*X", "%+*X", "% *X", "%#*X", "%0*X", "%-+ #0*X",
	"%*p", "%-*p", "%+*p", "% *p", "%#*p", "%0*p", "%-+ #0*p",
	NULL
};

char *uint_fmt3[] =
{
	"%.*o", "%-.*o", "%+.*o", "% .*o", "%#.*o", "%0.*o", "%-+ #0.*o",
	"%.*u", "%-.*u", "%+.*u", "% .*u", "%#.*u", "%0.*u", "%-+ #0.*u",
	"%.*x", "%-.*x", "%+.*x", "% .*x", "%#.*x", "%0.*x", "%-+ #0.*x",
	"%.*X", "%-.*X", "%+.*X", "% .*X", "%#.*X", "%0.*X", "%-+ #0.*X",
	"%.*p", "%-.*p", "%+.*p", "% .*p", "%#.*p", "%0.*p", "%-+ #0.*p",
	NULL
};

char *uint_fmt4[] =
{
	"%*.*o", "%-*.*o", "%+*.*o", "% *.*o", "%#*.*o", "%0*.*o", "%-+ #0*.*o",
	"%*.*u", "%-*.*u", "%+*.*u", "% *.*u", "%#*.*u", "%0*.*u", "%-+ #0*.*u",
	"%*.*x", "%-*.*x", "%+*.*x", "% *.*x", "%#*.*x", "%0*.*x", "%-+ #0*.*x",
	"%*.*X", "%-*.*X", "%+*.*X", "% *.*X", "%#*.*X", "%0*.*X", "%-+ #0*.*X",
	"%*.*p", "%-*.*p", "%+*.*p", "% *.*p", "%#*.*p", "%0*.*p", "%-+ #0*.*p",
	NULL
};

char *huint_fmt[] =
{
	"%ho", "%-ho", "%+ho", "% ho", "%#ho", "%0ho", "%-+ #0ho",
	"%9ho", "%-9ho", "%+9ho", "% 9ho", "%#9ho", "%09ho", "%-+ #09ho",
	"%.9ho", "%-.9ho", "%+.9ho", "% .9ho", "%#.9ho", "%0.9ho", "%-+ #0.9ho",
	"%9.ho", "%-9.ho", "%+9.ho", "% 9.ho", "%#9.ho", "%09.ho", "%-+ #09.ho",
	"%9.9ho", "%-9.9ho", "%+9.9ho", "% 9.9ho", "%#9.9ho", "%09.9ho", "%-+ #09.9ho",
	"%.0ho", "%-.0ho", "%+.0ho", "% .0ho", "%#.0ho", "%0.0ho", "%-+ #0.0ho",
	"%.1ho", "%-.1ho", "%+.1ho", "% .1ho", "%#.1ho", "%0.1ho", "%-+ #0.1ho",
	"%.10000ho", "%-.10000ho", "%+.10000ho", "% .10000ho", "%#.10000ho", "%0.10000ho", "%-+ #0.10000ho",
	"%10000ho", "%-10000ho", "%+10000ho", "% 10000ho", "%#10000ho", "%010000ho", "%-+ #010000ho",
	"%10000.10000ho", "%-10000.10000ho", "%+10000.10000ho", "% 10000.10000ho", "%#10000.10000ho", "%010000.10000ho", "%-+ #010000.10000ho",

	"%hu", "%-hu", "%+hu", "% hu", "%#hu", "%0hu", "%-+ #0hu",
	"%9hu", "%-9hu", "%+9hu", "% 9hu", "%#9hu", "%09hu", "%-+ #09hu",
	"%.9hu", "%-.9hu", "%+.9hu", "% .9hu", "%#.9hu", "%0.9hu", "%-+ #0.9hu",
	"%9.hu", "%-9.hu", "%+9.hu", "% 9.hu", "%#9.hu", "%09.hu", "%-+ #09.hu",
	"%9.9hu", "%-9.9hu", "%+9.9hu", "% 9.9hu", "%#9.9hu", "%09.9hu", "%-+ #09.9hu",
	"%.0hu", "%-.0hu", "%+.0hu", "% .0hu", "%#.0hu", "%0.0hu", "%-+ #0.0hu",
	"%.1hu", "%-.1hu", "%+.1hu", "% .1hu", "%#.1hu", "%0.1hu", "%-+ #0.1hu",
	"%.10000hu", "%-.10000hu", "%+.10000hu", "% .10000hu", "%#.10000hu", "%0.10000hu", "%-+ #0.10000hu",
	"%10000hu", "%-10000hu", "%+10000hu", "% 10000hu", "%#10000hu", "%010000hu", "%-+ #010000hu",
	"%10000.10000hu", "%-10000.10000hu", "%+10000.10000hu", "% 10000.10000hu", "%#10000.10000hu", "%010000.10000hu", "%-+ #010000.10000hu",

	"%hx", "%-hx", "%+hx", "% hx", "%#hx", "%0hx", "%-+ #0hx",
	"%9hx", "%-9hx", "%+9hx", "% 9hx", "%#9hx", "%09hx", "%-+ #09hx",
	"%.9hx", "%-.9hx", "%+.9hx", "% .9hx", "%#.9hx", "%0.9hx", "%-+ #0.9hx",
	"%9.hx", "%-9.hx", "%+9.hx", "% 9.hx", "%#9.hx", "%09.hx", "%-+ #09.hx",
	"%9.9hx", "%-9.9hx", "%+9.9hx", "% 9.9hx", "%#9.9hx", "%09.9hx", "%-+ #09.9hx",
	"%.0hx", "%-.0hx", "%+.0hx", "% .0hx", "%#.0hx", "%0.0hx", "%-+ #0.0hx",
	"%.1hx", "%-.1hx", "%+.1hx", "% .1hx", "%#.1hx", "%0.1hx", "%-+ #0.1hx",
	"%.10000hx", "%-.10000hx", "%+.10000hx", "% .10000hx", "%#.10000hx", "%0.10000hx", "%-+ #0.10000hx",
	"%10000hx", "%-10000hx", "%+10000hx", "% 10000hx", "%#10000hx", "%010000hx", "%-+ #010000hx",
	"%10000.10000hx", "%-10000.10000hx", "%+10000.10000hx", "% 10000.10000hx", "%#10000.10000hx", "%010000.10000hx", "%-+ #010000.10000hx",

	"%hX", "%-hX", "%+hX", "% hX", "%#hX", "%0hX", "%-+ #0hX",
	"%9hX", "%-9hX", "%+9hX", "% 9hX", "%#9hX", "%09hX", "%-+ #09hX",
	"%.9hX", "%-.9hX", "%+.9hX", "% .9hX", "%#.9hX", "%0.9hX", "%-+ #0.9hX",
	"%9.hX", "%-9.hX", "%+9.hX", "% 9.hX", "%#9.hX", "%09.hX", "%-+ #09.hX",
	"%9.9hX", "%-9.9hX", "%+9.9hX", "% 9.9hX", "%#9.9hX", "%09.9hX", "%-+ #09.9hX",
	"%.0hX", "%-.0hX", "%+.0hX", "% .0hX", "%#.0hX", "%0.0hX", "%-+ #0.0hX",
	"%.1hX", "%-.1hX", "%+.1hX", "% .1hX", "%#.1hX", "%0.1hX", "%-+ #0.1hX",
	"%.10000hX", "%-.10000hX", "%+.10000hX", "% .10000hX", "%#.10000hX", "%0.10000hX", "%-+ #0.10000hX",
	"%10000hX", "%-10000hX", "%+10000hX", "% 10000hX", "%#10000hX", "%010000hX", "%-+ #010000hX",
	"%10000.10000hX", "%-10000.10000hX", "%+10000.10000hX", "% 10000.10000hX", "%#10000.10000hX", "%010000.10000hX", "%-+ #010000.10000hX",

	"%-1.5ho", "%1.5ho", "%123.9ho", "%5.5ho", "%10.5ho", "% 10.5ho", "%+22.33ho", "%01.3ho", "%4ho",
	"%-1.5hu", "%1.5hu", "%123.9hu", "%5.5hu", "%10.5hu", "% 10.5hu", "%+22.33hu", "%01.3hu", "%4hu",
	"%-1.5hx", "%1.5hx", "%123.9hx", "%5.5hx", "%10.5hx", "% 10.5hx", "%+22.33hx", "%01.3hx", "%4hx",
	"%-1.5hX", "%1.5hX", "%123.9hX", "%5.5hX", "%10.5hX", "% 10.5hX", "%+22.33hX", "%01.3hX", "%4hX",
	NULL
};

char *huint_fmt2[] =
{
	"%*ho", "%-*ho", "%+*ho", "% *ho", "%#*ho", "%0*ho", "%-+ #0*ho",
	"%*hu", "%-*hu", "%+*hu", "% *hu", "%#*hu", "%0*hu", "%-+ #0*hu",
	"%*hx", "%-*hx", "%+*hx", "% *hx", "%#*hx", "%0*hx", "%-+ #0*hx",
	"%*hX", "%-*hX", "%+*hX", "% *hX", "%#*hX", "%0*hX", "%-+ #0*hX",
	NULL
};

char *huint_fmt3[] =
{
	"%.*ho", "%-.*ho", "%+.*ho", "% .*ho", "%#.*ho", "%0.*ho", "%-+ #0.*ho",
	"%.*hu", "%-.*hu", "%+.*hu", "% .*hu", "%#.*hu", "%0.*hu", "%-+ #0.*hu",
	"%.*hx", "%-.*hx", "%+.*hx", "% .*hx", "%#.*hx", "%0.*hx", "%-+ #0.*hx",
	"%.*hX", "%-.*hX", "%+.*hX", "% .*hX", "%#.*hX", "%0.*hX", "%-+ #0.*hX",
	NULL
};

char *huint_fmt4[] =
{
	"%*.*ho", "%-*.*ho", "%+*.*ho", "% *.*ho", "%#*.*ho", "%0*.*ho", "%-+ #0*.*ho",
	"%*.*hu", "%-*.*hu", "%+*.*hu", "% *.*hu", "%#*.*hu", "%0*.*hu", "%-+ #0*.*hu",
	"%*.*hx", "%-*.*hx", "%+*.*hx", "% *.*hx", "%#*.*hx", "%0*.*hx", "%-+ #0*.*hx",
	"%*.*hX", "%-*.*hX", "%+*.*hX", "% *.*hX", "%#*.*hX", "%0*.*hX", "%-+ #0*.*hX",
	NULL
};

char *luint_fmt[] =
{
	"%lo", "%-lo", "%+lo", "% lo", "%#lo", "%0lo", "%-+ #0lo",
	"%9lo", "%-9lo", "%+9lo", "% 9lo", "%#9lo", "%09lo", "%-+ #09lo",
	"%.9lo", "%-.9lo", "%+.9lo", "% .9lo", "%#.9lo", "%0.9lo", "%-+ #0.9lo",
	"%9.lo", "%-9.lo", "%+9.lo", "% 9.lo", "%#9.lo", "%09.lo", "%-+ #09.lo",
	"%9.9lo", "%-9.9lo", "%+9.9lo", "% 9.9lo", "%#9.9lo", "%09.9lo", "%-+ #09.9lo",
	"%.0lo", "%-.0lo", "%+.0lo", "% .0lo", "%#.0lo", "%0.0lo", "%-+ #0.0lo",
	"%.1lo", "%-.1lo", "%+.1lo", "% .1lo", "%#.1lo", "%0.1lo", "%-+ #0.1lo",
	"%.10000lo", "%-.10000lo", "%+.10000lo", "% .10000lo", "%#.10000lo", "%0.10000lo", "%-+ #0.10000lo",
	"%10000lo", "%-10000lo", "%+10000lo", "% 10000lo", "%#10000lo", "%010000lo", "%-+ #010000lo",
	"%10000.10000lo", "%-10000.10000lo", "%+10000.10000lo", "% 10000.10000lo", "%#10000.10000lo", "%010000.10000lo", "%-+ #010000.10000lo",

	"%lu", "%-lu", "%+lu", "% lu", "%#lu", "%0lu", "%-+ #0lu",
	"%9lu", "%-9lu", "%+9lu", "% 9lu", "%#9lu", "%09lu", "%-+ #09lu",
	"%.9lu", "%-.9lu", "%+.9lu", "% .9lu", "%#.9lu", "%0.9lu", "%-+ #0.9lu",
	"%9.lu", "%-9.lu", "%+9.lu", "% 9.lu", "%#9.lu", "%09.lu", "%-+ #09.lu",
	"%9.9lu", "%-9.9lu", "%+9.9lu", "% 9.9lu", "%#9.9lu", "%09.9lu", "%-+ #09.9lu",
	"%.0lu", "%-.0lu", "%+.0lu", "% .0lu", "%#.0lu", "%0.0lu", "%-+ #0.0lu",
	"%.1lu", "%-.1lu", "%+.1lu", "% .1lu", "%#.1lu", "%0.1lu", "%-+ #0.1lu",
	"%.10000lu", "%-.10000lu", "%+.10000lu", "% .10000lu", "%#.10000lu", "%0.10000lu", "%-+ #0.10000lu",
	"%10000lu", "%-10000lu", "%+10000lu", "% 10000lu", "%#10000lu", "%010000lu", "%-+ #010000lu",
	"%10000.10000lu", "%-10000.10000lu", "%+10000.10000lu", "% 10000.10000lu", "%#10000.10000lu", "%010000.10000lu", "%-+ #010000.10000lu",

	"%lx", "%-lx", "%+lx", "% lx", "%#lx", "%0lx", "%-+ #0lx",
	"%9lx", "%-9lx", "%+9lx", "% 9lx", "%#9lx", "%09lx", "%-+ #09lx",
	"%.9lx", "%-.9lx", "%+.9lx", "% .9lx", "%#.9lx", "%0.9lx", "%-+ #0.9lx",
	"%9.lx", "%-9.lx", "%+9.lx", "% 9.lx", "%#9.lx", "%09.lx", "%-+ #09.lx",
	"%9.9lx", "%-9.9lx", "%+9.9lx", "% 9.9lx", "%#9.9lx", "%09.9lx", "%-+ #09.9lx",
	"%.0lx", "%-.0lx", "%+.0lx", "% .0lx", "%#.0lx", "%0.0lx", "%-+ #0.0lx",
	"%.1lx", "%-.1lx", "%+.1lx", "% .1lx", "%#.1lx", "%0.1lx", "%-+ #0.1lx",
	"%.10000lx", "%-.10000lx", "%+.10000lx", "% .10000lx", "%#.10000lx", "%0.10000lx", "%-+ #0.10000lx",
	"%10000lx", "%-10000lx", "%+10000lx", "% 10000lx", "%#10000lx", "%010000lx", "%-+ #010000lx",
	"%10000.10000lx", "%-10000.10000lx", "%+10000.10000lx", "% 10000.10000lx", "%#10000.10000lx", "%010000.10000lx", "%-+ #010000.10000lx",

	"%lX", "%-lX", "%+lX", "% lX", "%#lX", "%0lX", "%-+ #0lX",
	"%9lX", "%-9lX", "%+9lX", "% 9lX", "%#9lX", "%09lX", "%-+ #09lX",
	"%.9lX", "%-.9lX", "%+.9lX", "% .9lX", "%#.9lX", "%0.9lX", "%-+ #0.9lX",
	"%9.lX", "%-9.lX", "%+9.lX", "% 9.lX", "%#9.lX", "%09.lX", "%-+ #09.lX",
	"%9.9lX", "%-9.9lX", "%+9.9lX", "% 9.9lX", "%#9.9lX", "%09.9lX", "%-+ #09.9lX",
	"%.0lX", "%-.0lX", "%+.0lX", "% .0lX", "%#.0lX", "%0.0lX", "%-+ #0.0lX",
	"%.1lX", "%-.1lX", "%+.1lX", "% .1lX", "%#.1lX", "%0.1lX", "%-+ #0.1lX",
	"%.10000lX", "%-.10000lX", "%+.10000lX", "% .10000lX", "%#.10000lX", "%0.10000lX", "%-+ #0.10000lX",
	"%10000lX", "%-10000lX", "%+10000lX", "% 10000lX", "%#10000lX", "%010000lX", "%-+ #010000lX",
	"%10000.10000lX", "%-10000.10000lX", "%+10000.10000lX", "% 10000.10000lX", "%#10000.10000lX", "%010000.10000lX", "%-+ #010000.10000lX",

	"%-1.5lo", "%1.5lo", "%123.9lo", "%5.5lo", "%10.5lo", "% 10.5lo", "%+22.33lo", "%01.3lo", "%4lo",
	"%-1.5lu", "%1.5lu", "%123.9lu", "%5.5lu", "%10.5lu", "% 10.5lu", "%+22.33lu", "%01.3lu", "%4lu",
	"%-1.5lx", "%1.5lx", "%123.9lx", "%5.5lx", "%10.5lx", "% 10.5lx", "%+22.33lx", "%01.3lx", "%4lx",
	"%-1.5lX", "%1.5lX", "%123.9lX", "%5.5lX", "%10.5lX", "% 10.5lX", "%+22.33lX", "%01.3lX", "%4lX",
	NULL
};

char *luint_fmt2[] =
{
	"%*lo", "%-*lo", "%+*lo", "% *lo", "%#*lo", "%0*lo", "%-+ #0*lo",
	"%*lu", "%-*lu", "%+*lu", "% *lu", "%#*lu", "%0*lu", "%-+ #0*lu",
	"%*lx", "%-*lx", "%+*lx", "% *lx", "%#*lx", "%0*lx", "%-+ #0*lx",
	"%*lX", "%-*lX", "%+*lX", "% *lX", "%#*lX", "%0*lX", "%-+ #0*lX",
	NULL
};

char *luint_fmt3[] =
{
	"%.*lo", "%-.*lo", "%+.*lo", "% .*lo", "%#.*lo", "%0.*lo", "%-+ #0.*lo",
	"%.*lu", "%-.*lu", "%+.*lu", "% .*lu", "%#.*lu", "%0.*lu", "%-+ #0.*lu",
	"%.*lx", "%-.*lx", "%+.*lx", "% .*lx", "%#.*lx", "%0.*lx", "%-+ #0.*lx",
	"%.*lX", "%-.*lX", "%+.*lX", "% .*lX", "%#.*lX", "%0.*lX", "%-+ #0.*lX",
	NULL
};

char *luint_fmt4[] =
{
	"%*.*lo", "%-*.*lo", "%+*.*lo", "% *.*lo", "%#*.*lo", "%0*.*lo", "%-+ #0*.*lo",
	"%*.*lu", "%-*.*lu", "%+*.*lu", "% *.*lu", "%#*.*lu", "%0*.*lu", "%-+ #0*.*lu",
	"%*.*lx", "%-*.*lx", "%+*.*lx", "% *.*lx", "%#*.*lx", "%0*.*lx", "%-+ #0*.*lx",
	"%*.*lX", "%-*.*lX", "%+*.*lX", "% *.*lX", "%#*.*lX", "%0*.*lX", "%-+ #0*.*lX",
	NULL
};

unsigned long uint_nums[] =
{
	0xffffffff, 134, 91340, 341, 0203, 0, 0xffff3333
};

char *str_fmt[] =
{
	"%s", "%-s",
	"%9s", "%-9s",
	"%.9s", "%-.9s",
	"%9.s", "%-9.s",
	"%9.9s", "%-9.9s",
	"%.0s", "%-.0s",
	"%.1s", "%-.1s",
	"%.10000s", "%-.10000s",
	"%10000s", "%-10000s",
	"%10000.10000s", "%-10000.10000s",
	NULL
};

char *str_fmt2[] =
{
	"%*s", "%-*s",
	NULL
};

char *str_fmt3[] =
{
	"%.*s", "%-.*s",
	NULL
};

char *str_fmt4[] =
{
	"%*.*s", "%-*.*s",
	NULL
};

char *str_vals[] =
{
#ifdef HAVE_PRINTF_STR_FMT_NULL
	NULL,
#endif
	"", "a", "ab", "abc", "abcdefgh", "abcdefghi",
	"abcdefghij", "abcdefghijk", "abcdef\000ghijk"
};

char *char_fmt[] =
{
	"%c", "%-c",
	"%1c", "%-1c",
	"%2c", "%-2c",
	"%3c", "%-3c",
	"%4c", "%-4c",
	NULL
};

char *char_fmt2[] =
{
	"%*c", "%-*c",
	NULL
};

char *char_fmt3[] =
{
	"%.*c", "%-.*c",
	NULL
};

char *char_fmt4[] =
{
	"%*.*c", "%-*.*c",
	NULL
};

char char_vals[] =
{
	'a', '\0', '\377'
};

int widths[] =     { 0,  0, 15, 15, 1,  0, 80, 80,   0, -15, -15, -80 };
int precisions[] = { 0, 15,  0, 15, 1, 80,  0, 80, -15,   0, -15, -80 };

/*
** Note: Some of the following may mean something on your system so we don't
** compare this snprintf() with the system's sprintf(). We just check that
** snprintf() returns -1 for these formats.
*/

char *err_fmt[] =
{
	"%h%", "%l%", "%L%", "%9%", "%.9%", "%9.9%", "%-%", "%+%", "% %", "%#%", "%0%",
	"%a", "%A", "%b", "%B", "%C", "%D", "%F", "%h", "%H", "%I", "%j", "%J",
	"%k", "%K", "%l", "%L", "%m", "%M", "%N", "%O", "%P", "%q", "%Q", "%r",
	"%R", "%S", "%t", "%T", "%U", "%v", "%V", "%w", "%W", "%y", "%Y", "%z",
	"%Z",
	NULL
};

int main(int ac, char **av)
{
	char buf1[LONG_STRING];
	char buf2[LONG_STRING];
	int verbose = 0;
	int x, y, z;
	int tests = 0;
	int errors = 0;
	int len1;
	int len2;
	int rc;
	short hn;
	int n;
	long ln;

	if (ac == 2 && !strcmp(av[1], "help"))
	{
		printf("usage: %s [verbose]\n", *av);
		return EXIT_SUCCESS;
	}

	if (ac == 2 && !strcmp(av[1], "verbose"))
		verbose = 1;

	printf("Testing: %s\n", "snprintf");

	/*_
	** Note that, in the macros below, the assignment of sprintf()'s
	** return value to a variable called "suppress_optimization" is
	** is extremely important when using recent versions of gcc.
	** Without it, gcc would optimize the call to sprintf() into a
	** call to strcpy() in the case when the format is "%s".
	** That would crash while the actual sprintf() in glibc would work
	** as expected and put "(null)" into the buffer. Note that this only
	** matters when the value is NULL which is only included in the tests
	** when HAVE_PRINTF_STR_FMT_NULL is defined. Presumably, at some point,
	** gcc's optimizer will remove the unused assignment and then this
	** problem will resurface. There's a pretense at a use which seems to
	** make gcc behave itself for now (but it might get smarter one day
	** and these tests might break again).
	*/

#define TEST_SNPRINTF1(format, val, vfmt) \
	{ \
		char valstr[1024]; \
		int suppress_optimization = sprintf(valstr, (vfmt), (val)); \
		if (suppress_optimization) \
			/* suppress gcc warning */; \
		++tests; \
		len1 = snprintf(buf1, sizeof(buf1), (format), (val)); \
		len2 = sprintf(buf2, (format), (val)); \
		if (len1 != len2) \
			++errors, printf("Test%d: snprintf(%s, %s) failed (%s, not %s)\n", tests, (format), valstr, buf1, buf2); \
		else if (memcmp(buf1, buf2, len1 + 1)) \
			++errors, printf("Test%d: snprintf(%s, %s) failed (%s, not %s)\n", tests, (format), valstr, buf1, buf2); \
		else if (verbose) \
			printf("Test%d: snprintf(%s, %s) succeeded (%s)\n", tests, (format), valstr, buf1); \
	}

#define TEST_SNPRINTF2(format, width, precision, val, vfmt) \
	{ \
		char valstr[1024]; \
		int suppress_optimization = sprintf(valstr, (vfmt), (val)); \
		if (suppress_optimization) \
			/* suppress gcc warning */; \
		++tests; \
		len1 = snprintf(buf1, sizeof(buf1), (format), (width), (precision), (val)); \
		len2 = sprintf(buf2, (format), (width), (precision), (val)); \
		if (len1 != len2) \
			++errors, printf("Test%d: snprintf(%s, %d, %d, %s) failed (%s, not %s)\n", tests, (format), (width), (precision), valstr, buf1, buf2); \
		else if (memcmp(buf1, buf2, len1 + 1)) \
			++errors, printf("Test%d: snprintf(%s, %d, %d, %s) failed (%s, not %s)\n", tests, (format), (width), (precision), valstr, buf1, buf2); \
		else if (verbose) \
			printf("Test%d: snprintf(%s, %d, %d, %s) succeeded (%s)\n", tests, (format), (width), (precision), valstr, buf1); \
	}

#define TEST_SNPRINTF3(format, arg, val, vfmt) \
	{ \
		char valstr[1024]; \
		int suppress_optimization = sprintf(valstr, (vfmt), (val)); \
		if (suppress_optimization) \
			/* suppress gcc warning */; \
		++tests; \
		len1 = snprintf(buf1, sizeof(buf1), (format), (arg), (val)); \
		len2 = sprintf(buf2, (format), (arg), (val)); \
		if (len1 != len2) \
			++errors, printf("Test%d: snprintf(%s, %d, %s) failed (%s, not %s)\n", tests, (format), (arg), valstr, buf1, buf2); \
		else if (memcmp(buf1, buf2, len1 + 1)) \
			++errors, printf("Test%d: snprintf(%s, %d, %s) failed (%s, not %s)\n", tests, (format), (arg), valstr, buf1, buf2); \
		else if (verbose) \
			printf("Test%d: snprintf(%s, %d, %s) succeeded (%s)\n", tests, (format), (arg), valstr, buf1); \
	}

	/* Test int */

	for (x = 0; int_fmt[x]; ++x)
		for (y = 0; y < sizeof(int_nums) / sizeof(*int_nums); ++y)
			TEST_SNPRINTF1(int_fmt[x], (int)int_nums[y], "%d")

	for (x = 0; int_fmt2[x]; ++x)
		for (y = 0; y < sizeof(int_nums) / sizeof(*int_nums); ++y)
			for (z = 0; z < sizeof(widths) / sizeof(*widths); ++z)
				TEST_SNPRINTF3(int_fmt2[x], widths[z], (int)int_nums[y], "%d")

	for (x = 0; int_fmt3[x]; ++x)
		for (y = 0; y < sizeof(int_nums) / sizeof(*int_nums); ++y)
			for (z = 0; z < sizeof(widths) / sizeof(*widths); ++z)
				TEST_SNPRINTF3(int_fmt3[x], precisions[z], (int)int_nums[y], "%d")

	for (x = 0; int_fmt4[x]; ++x)
		for (y = 0; y < sizeof(int_nums) / sizeof(*int_nums); ++y)
			for (z = 0; z < sizeof(widths) / sizeof(*widths); ++z)
				TEST_SNPRINTF2(int_fmt4[x], widths[z], precisions[z], (int)int_nums[y], "%d")

	/* Test short int */

	for (x = 0; hint_fmt[x]; ++x)
		for (y = 0; y < sizeof(int_nums) / sizeof(*int_nums); ++y)
			TEST_SNPRINTF1(hint_fmt[x], (short)int_nums[y], "%hd")

	for (x = 0; hint_fmt2[x]; ++x)
		for (y = 0; y < sizeof(int_nums) / sizeof(*int_nums); ++y)
			for (z = 0; z < sizeof(widths) / sizeof(*widths); ++z)
				TEST_SNPRINTF3(hint_fmt2[x], widths[z], (short)int_nums[y], "%hd")

	for (x = 0; hint_fmt3[x]; ++x)
		for (y = 0; y < sizeof(int_nums) / sizeof(*int_nums); ++y)
			for (z = 0; z < sizeof(widths) / sizeof(*widths); ++z)
				TEST_SNPRINTF3(hint_fmt3[x], precisions[z], (short)int_nums[y], "%hd")

	for (x = 0; hint_fmt4[x]; ++x)
		for (y = 0; y < sizeof(int_nums) / sizeof(*int_nums); ++y)
			for (z = 0; z < sizeof(widths) / sizeof(*widths); ++z)
				TEST_SNPRINTF2(hint_fmt4[x], widths[z], precisions[z], (short)int_nums[y], "%hd")

	/* Test long int */

	for (x = 0; lint_fmt[x]; ++x)
		for (y = 0; y < sizeof(int_nums) / sizeof(*int_nums); ++y)
			TEST_SNPRINTF1(lint_fmt[x], int_nums[y], "%ld")

	for (x = 0; lint_fmt2[x]; ++x)
		for (y = 0; y < sizeof(int_nums) / sizeof(*int_nums); ++y)
			for (z = 0; z < sizeof(widths) / sizeof(*widths); ++z)
				TEST_SNPRINTF3(lint_fmt2[x], widths[z], int_nums[y], "%ld")

	for (x = 0; lint_fmt3[x]; ++x)
		for (y = 0; y < sizeof(int_nums) / sizeof(*int_nums); ++y)
			for (z = 0; z < sizeof(widths) / sizeof(*widths); ++z)
				TEST_SNPRINTF3(lint_fmt3[x], precisions[z], int_nums[y], "%ld")

	for (x = 0; lint_fmt4[x]; ++x)
		for (y = 0; y < sizeof(int_nums) / sizeof(*int_nums); ++y)
			for (z = 0; z < sizeof(widths) / sizeof(*widths); ++z)
				TEST_SNPRINTF2(lint_fmt4[x], widths[z], precisions[z], int_nums[y], "%ld")

	/* Test unsigned int */

	for (x = 0; uint_fmt[x]; ++x)
		for (y = 0; y < sizeof(uint_nums) / sizeof(*uint_nums); ++y)
			TEST_SNPRINTF1(uint_fmt[x], (unsigned int)uint_nums[y], "%u")

	for (x = 0; uint_fmt2[x]; ++x)
		for (y = 0; y < sizeof(uint_nums) / sizeof(*uint_nums); ++y)
			for (z = 0; z < sizeof(widths) / sizeof(*widths); ++z)
				TEST_SNPRINTF3(uint_fmt2[x], widths[z], (unsigned int)uint_nums[y], "%u")

	for (x = 0; uint_fmt3[x]; ++x)
		for (y = 0; y < sizeof(uint_nums) / sizeof(*uint_nums); ++y)
			for (z = 0; z < sizeof(widths) / sizeof(*widths); ++z)
				TEST_SNPRINTF3(uint_fmt3[x], precisions[z], (unsigned int)uint_nums[y], "%u")

	for (x = 0; uint_fmt4[x]; ++x)
		for (y = 0; y < sizeof(uint_nums) / sizeof(*uint_nums); ++y)
			for (z = 0; z < sizeof(widths) / sizeof(*widths); ++z)
				TEST_SNPRINTF2(uint_fmt4[x], widths[z], precisions[z], (unsigned int)uint_nums[y], "%u")

	/* Test unsigned short int */

	for (x = 0; huint_fmt[x]; ++x)
		for (y = 0; y < sizeof(uint_nums) / sizeof(*uint_nums); ++y)
			TEST_SNPRINTF1(huint_fmt[x], (unsigned short)uint_nums[y], "%hu")

	for (x = 0; huint_fmt2[x]; ++x)
		for (y = 0; y < sizeof(uint_nums) / sizeof(*uint_nums); ++y)
			for (z = 0; z < sizeof(widths) / sizeof(*widths); ++z)
				TEST_SNPRINTF3(huint_fmt2[x], widths[z], (unsigned short)uint_nums[y], "%hu")

	for (x = 0; huint_fmt3[x]; ++x)
		for (y = 0; y < sizeof(uint_nums) / sizeof(*uint_nums); ++y)
			for (z = 0; z < sizeof(widths) / sizeof(*widths); ++z)
				TEST_SNPRINTF3(huint_fmt3[x], precisions[z], (unsigned short)uint_nums[y], "%hu")

	for (x = 0; huint_fmt4[x]; ++x)
		for (y = 0; y < sizeof(uint_nums) / sizeof(*uint_nums); ++y)
			for (z = 0; z < sizeof(widths) / sizeof(*widths); ++z)
				TEST_SNPRINTF2(huint_fmt4[x], widths[z], precisions[z], (unsigned short)uint_nums[y], "%hu")

	/* Test unsigned long int */

	for (x = 0; luint_fmt[x]; ++x)
		for (y = 0; y < sizeof(uint_nums) / sizeof(*uint_nums); ++y)
			TEST_SNPRINTF1(luint_fmt[x], (unsigned long)uint_nums[y], "%lu")

	for (x = 0; luint_fmt2[x]; ++x)
		for (y = 0; y < sizeof(uint_nums) / sizeof(*uint_nums); ++y)
			for (z = 0; z < sizeof(widths) / sizeof(*widths); ++z)
				TEST_SNPRINTF3(luint_fmt2[x], widths[z], (unsigned long)uint_nums[y], "%lu")

	for (x = 0; luint_fmt3[x]; ++x)
		for (y = 0; y < sizeof(uint_nums) / sizeof(*uint_nums); ++y)
			for (z = 0; z < sizeof(widths) / sizeof(*widths); ++z)
				TEST_SNPRINTF3(luint_fmt3[x], precisions[z], (unsigned long)uint_nums[y], "%lu")

	for (x = 0; luint_fmt4[x]; ++x)
		for (y = 0; y < sizeof(uint_nums) / sizeof(*uint_nums); ++y)
			for (z = 0; z < sizeof(widths) / sizeof(*widths); ++z)
				TEST_SNPRINTF2(luint_fmt4[x], widths[z], precisions[z], (unsigned long)uint_nums[y], "%lu")

	/* Test double */

	for (x = 0; fp_fmt[x]; ++x)
		for (y = 0; y < sizeof(fp_nums) / sizeof(*fp_nums); ++y)
			TEST_SNPRINTF1(fp_fmt[x], (double)fp_nums[y], "%g")

	for (x = 0; fp_fmt2[x]; ++x)
		for (y = 0; y < sizeof(fp_nums) / sizeof(*fp_nums); ++y)
			for (z = 0; z < sizeof(widths) / sizeof(*widths); ++z)
				TEST_SNPRINTF3(fp_fmt2[x], widths[z], (double)fp_nums[y], "%g")

	for (x = 0; fp_fmt3[x]; ++x)
		for (y = 0; y < sizeof(fp_nums) / sizeof(*fp_nums); ++y)
			for (z = 0; z < sizeof(widths) / sizeof(*widths); ++z)
				TEST_SNPRINTF3(fp_fmt3[x], precisions[z], (double)fp_nums[y], "%g")

	for (x = 0; fp_fmt4[x]; ++x)
		for (y = 0; y < sizeof(fp_nums) / sizeof(*fp_nums); ++y)
			for (z = 0; z < sizeof(widths) / sizeof(*widths); ++z)
				TEST_SNPRINTF2(fp_fmt4[x], widths[z], precisions[z], (double)fp_nums[y], "%g")

	/* Test long double */

	for (x = 0; fp_ldbl_fmt[x]; ++x)
		for (y = 0; y < sizeof(fp_nums) / sizeof(*fp_nums); ++y)
			TEST_SNPRINTF1(fp_ldbl_fmt[x], fp_nums[y], "%Lg")

	for (x = 0; fp_ldbl_fmt2[x]; ++x)
		for (y = 0; y < sizeof(fp_nums) / sizeof(*fp_nums); ++y)
			for (z = 0; z < sizeof(widths) / sizeof(*widths); ++z)
				TEST_SNPRINTF3(fp_ldbl_fmt2[x], widths[z], fp_nums[y], "%Lg")

	for (x = 0; fp_ldbl_fmt3[x]; ++x)
		for (y = 0; y < sizeof(fp_nums) / sizeof(*fp_nums); ++y)
			for (z = 0; z < sizeof(widths) / sizeof(*widths); ++z)
				TEST_SNPRINTF3(fp_ldbl_fmt3[x], precisions[z], fp_nums[y], "%Lg")

	for (x = 0; fp_ldbl_fmt4[x]; ++x)
		for (y = 0; y < sizeof(fp_nums) / sizeof(*fp_nums); ++y)
			for (z = 0; z < sizeof(widths) / sizeof(*widths); ++z)
				TEST_SNPRINTF2(fp_ldbl_fmt4[x], widths[z], precisions[z], fp_nums[y], "%Lg")

	/* Test strings */

	for (x = 0; str_fmt[x]; ++x)
		for (y = 0; y < sizeof(str_vals) / sizeof(*str_vals); ++y)
			TEST_SNPRINTF1(str_fmt[x], str_vals[y], "%s")

	for (x = 0; str_fmt2[x]; ++x)
		for (y = 0; y < sizeof(str_vals) / sizeof(*str_vals); ++y)
			for (z = 0; z < sizeof(widths) / sizeof(*widths); ++z)
				TEST_SNPRINTF3(str_fmt2[x], widths[z], str_vals[y], "%s")

	for (x = 0; str_fmt3[x]; ++x)
		for (y = 0; y < sizeof(str_vals) / sizeof(*str_vals); ++y)
			for (z = 0; z < sizeof(widths) / sizeof(*widths); ++z)
				TEST_SNPRINTF3(str_fmt3[x], precisions[z], str_vals[y], "%s")

	for (x = 0; str_fmt4[x]; ++x)
		for (y = 0; y < sizeof(str_vals) / sizeof(*str_vals); ++y)
			for (z = 0; z < sizeof(widths) / sizeof(*widths); ++z)
				TEST_SNPRINTF2(str_fmt4[x], widths[z], precisions[z], str_vals[y], "%s")

	/* Test characters */

	for (x = 0; char_fmt[x]; ++x)
		for (y = 0; y < sizeof(char_vals) / sizeof(*char_vals); ++y)
			TEST_SNPRINTF1(char_fmt[x], char_vals[y], "%c")

	for (x = 0; char_fmt2[x]; ++x)
		for (y = 0; y < sizeof(char_vals) / sizeof(*char_vals); ++y)
			for (z = 0; z < sizeof(widths) / sizeof(*widths); ++z)
				TEST_SNPRINTF3(char_fmt2[x], widths[z], char_vals[y], "%c")

	for (x = 0; char_fmt3[x]; ++x)
		for (y = 0; y < sizeof(char_vals) / sizeof(*char_vals); ++y)
			for (z = 0; z < sizeof(widths) / sizeof(*widths); ++z)
				TEST_SNPRINTF3(char_fmt3[x], precisions[z], char_vals[y], "%c")

	for (x = 0; char_fmt4[x]; ++x)
		for (y = 0; y < sizeof(char_vals) / sizeof(*char_vals); ++y)
			for (z = 0; z < sizeof(widths) / sizeof(*widths); ++z)
				TEST_SNPRINTF2(char_fmt4[x], widths[z], precisions[z], char_vals[y], "%c")

	/* Test %[n%] */

#define TEST_SNPRINTF4(action, eq, buf, val, size, n, nval) \
	if (++tests, (rc = (action)) != (eq)) \
		++errors, printf("Test%d: %s failed (returned %d, not %d)\n", tests, (#action), rc, (eq)); \
	else if ((n) != (nval)) \
		++errors, printf("Test%d: %s failed (output %ld, not %ld)\n", tests, (#action), (long)(n), (long)(nval)); \
	else if (buf && val && (size) && memcmp((buf), (val), (size))) \
		++errors, printf("Test%d: %s failed (%s, not %s)\n", tests, (#action), (char *)(buf), (char *)(val)); \
	else if (verbose) \
		printf("Test%d: %s succeeded (%s = %ld)\n", tests, (#action), (#n), (long)n);

	TEST_SNPRINTF4(snprintf(buf1, sizeof(buf1), "%n", &n), 0, buf1, "", 1, n, 0)
	TEST_SNPRINTF4(snprintf(buf1, sizeof(buf1), "%n1", &n), 1, buf1, "1", 2, n, 0)
	TEST_SNPRINTF4(snprintf(buf1, sizeof(buf1), "a%n", &n), 1, buf1, "a", 2, n, 1)
	TEST_SNPRINTF4(snprintf(buf1, sizeof(buf1), "a%nb", &n), 2, buf1, "ab", 3, n, 1)
	TEST_SNPRINTF4(snprintf(buf1, sizeof(buf1), "ab%n", &n), 2, buf1, "ab", 3, n, 2)
	TEST_SNPRINTF4(snprintf(buf1, sizeof(buf1), "ab%nc", &n), 3, buf1, "abc", 4, n, 2)
	TEST_SNPRINTF4(snprintf(buf1, sizeof(buf1), "abc%n", &n), 3, buf1, "abc", 4, n, 3)
	TEST_SNPRINTF4(snprintf(buf1, sizeof(buf1), "abc%ndef", &n), 6, buf1, "abcdef", 7, n, 3)
	TEST_SNPRINTF4(snprintf(buf1, sizeof(buf1), "%d %d %d%n %d", 1, 1, 1, &n, 1), 7, buf1, "1 1 1 1", 8, n, 5)
	TEST_SNPRINTF4(snprintf(buf1, sizeof(buf1), "%g%n", 1.5, &n), 3, buf1, "1.5", 4, n, 3)
	TEST_SNPRINTF4(snprintf(buf1, sizeof(buf1), "%g%n1", 1.5, &n), 4, buf1, "1.51", 5, n, 3)
	TEST_SNPRINTF4(snprintf(buf1, sizeof(buf1), "%s%n", "a string", &n), 8, buf1, "a string", 9, n, 8)
	TEST_SNPRINTF4(snprintf(buf1, sizeof(buf1), "%s%n1", "a string", &n), 9, buf1, "a string1", 10, n, 8)

	TEST_SNPRINTF4(snprintf(buf1, sizeof(buf1), "%hn", &hn), 0, buf1, "", 1, hn, 0)
	TEST_SNPRINTF4(snprintf(buf1, sizeof(buf1), "%hn1", &hn), 1, buf1, "1", 2, hn, 0)
	TEST_SNPRINTF4(snprintf(buf1, sizeof(buf1), "a%hn", &hn), 1, buf1, "a", 2, hn, 1)
	TEST_SNPRINTF4(snprintf(buf1, sizeof(buf1), "a%hnb", &hn), 2, buf1, "ab", 3, hn, 1)
	TEST_SNPRINTF4(snprintf(buf1, sizeof(buf1), "ab%hn", &hn), 2, buf1, "ab", 3, hn, 2)
	TEST_SNPRINTF4(snprintf(buf1, sizeof(buf1), "ab%hnc", &hn), 3, buf1, "abc", 4, hn, 2)
	TEST_SNPRINTF4(snprintf(buf1, sizeof(buf1), "abc%hn", &hn), 3, buf1, "abc", 4, hn, 3)
	TEST_SNPRINTF4(snprintf(buf1, sizeof(buf1), "abc%hndef", &hn), 6, buf1, "abcdef", 7, hn, 3)
	TEST_SNPRINTF4(snprintf(buf1, sizeof(buf1), "%d %d %d%hn %d", 1, 1, 1, &hn, 1), 7, buf1, "1 1 1 1", 8, hn, 5)
	TEST_SNPRINTF4(snprintf(buf1, sizeof(buf1), "%g%hn", 1.5, &hn), 3, buf1, "1.5", 4, hn, 3)
	TEST_SNPRINTF4(snprintf(buf1, sizeof(buf1), "%g%hn1", 1.5, &hn), 4, buf1, "1.51", 5, hn, 3)
	TEST_SNPRINTF4(snprintf(buf1, sizeof(buf1), "%s%hn", "a string", &hn), 8, buf1, "a string", 9, hn, 8)
	TEST_SNPRINTF4(snprintf(buf1, sizeof(buf1), "%s%hn1", "a string", &hn), 9, buf1, "a string1", 10, hn, 8)

	TEST_SNPRINTF4(snprintf(buf1, sizeof(buf1), "%ln", &ln), 0, buf1, "", 1, ln, 0)
	TEST_SNPRINTF4(snprintf(buf1, sizeof(buf1), "%ln1", &ln), 1, buf1, "1", 2, ln, 0)
	TEST_SNPRINTF4(snprintf(buf1, sizeof(buf1), "a%ln", &ln), 1, buf1, "a", 2, ln, 1)
	TEST_SNPRINTF4(snprintf(buf1, sizeof(buf1), "a%lnb", &ln), 2, buf1, "ab", 3, ln, 1)
	TEST_SNPRINTF4(snprintf(buf1, sizeof(buf1), "ab%ln", &ln), 2, buf1, "ab", 3, ln, 2)
	TEST_SNPRINTF4(snprintf(buf1, sizeof(buf1), "ab%lnc", &ln), 3, buf1, "abc", 4, ln, 2)
	TEST_SNPRINTF4(snprintf(buf1, sizeof(buf1), "abc%ln", &ln), 3, buf1, "abc", 4, ln, 3)
	TEST_SNPRINTF4(snprintf(buf1, sizeof(buf1), "abc%lndef", &ln), 6, buf1, "abcdef", 7, ln, 3)
	TEST_SNPRINTF4(snprintf(buf1, sizeof(buf1), "%d %d %d%ln %d", 1, 1, 1, &ln, 1), 7, buf1, "1 1 1 1", 8, ln, 5)
	TEST_SNPRINTF4(snprintf(buf1, sizeof(buf1), "%g%ln", 1.5, &ln), 3, buf1, "1.5", 4, ln, 3)
	TEST_SNPRINTF4(snprintf(buf1, sizeof(buf1), "%g%ln1", 1.5, &ln), 4, buf1, "1.51", 5, ln, 3)
	TEST_SNPRINTF4(snprintf(buf1, sizeof(buf1), "%s%ln", "a string", &ln), 8, buf1, "a string", 9, ln, 8)
	TEST_SNPRINTF4(snprintf(buf1, sizeof(buf1), "%s%ln1", "a string", &ln), 9, buf1, "a string1", 10, ln, 8)

	TEST_SNPRINTF4(snprintf(NULL, 0, "%n", &n), 0, NULL, NULL, 0, n, 0)
	TEST_SNPRINTF4(snprintf(NULL, 0, "%n1", &n), 1, NULL, NULL, 0, n, 0)
	TEST_SNPRINTF4(snprintf(NULL, 0, "a%n", &n), 1, NULL, NULL, 0, n, 0)
	TEST_SNPRINTF4(snprintf(NULL, 0, "a%nb", &n), 2, NULL, NULL, 0, n, 0)
	TEST_SNPRINTF4(snprintf(NULL, 0, "ab%n", &n), 2, NULL, NULL, 0, n, 0)
	TEST_SNPRINTF4(snprintf(NULL, 0, "ab%nc", &n), 3, NULL, NULL, 0, n, 0)
	TEST_SNPRINTF4(snprintf(NULL, 0, "abc%n", &n), 3, NULL, NULL, 0, n, 0)
	TEST_SNPRINTF4(snprintf(NULL, 0, "abc%ndef", &n), 6, NULL, NULL, 0, n, 0)
	TEST_SNPRINTF4(snprintf(NULL, 0, "%d %d %d%n %d", 1, 1, 1, &n, 1), 7, NULL, NULL, 0, n, 0)
	TEST_SNPRINTF4(snprintf(NULL, 0, "%g%n", 1.5, &n), 3, NULL, NULL, 0, n, 0)
	TEST_SNPRINTF4(snprintf(NULL, 0, "%g%n1", 1.5, &n), 4, NULL, NULL, 0, n, 0)
	TEST_SNPRINTF4(snprintf(NULL, 0, "%s%n", "a string", &n), 8, NULL, NULL, 0, n, 0)
	TEST_SNPRINTF4(snprintf(NULL, 0, "%s%n1", "a string", &n), 9, NULL, NULL, 0, n, 0)

	TEST_SNPRINTF4(snprintf(NULL, 0, "%hn", &hn), 0, NULL, NULL, 0, hn, 0)
	TEST_SNPRINTF4(snprintf(NULL, 0, "%hn1", &hn), 1, NULL, NULL, 0, hn, 0)
	TEST_SNPRINTF4(snprintf(NULL, 0, "a%hn", &hn), 1, NULL, NULL, 0, hn, 0)
	TEST_SNPRINTF4(snprintf(NULL, 0, "a%hnb", &hn), 2, NULL, NULL, 0, hn, 0)
	TEST_SNPRINTF4(snprintf(NULL, 0, "ab%hn", &hn), 2, NULL, NULL, 0, hn, 0)
	TEST_SNPRINTF4(snprintf(NULL, 0, "ab%hnc", &hn), 3, NULL, NULL, 0, hn, 0)
	TEST_SNPRINTF4(snprintf(NULL, 0, "abc%hn", &hn), 3, NULL, NULL, 0, hn, 0)
	TEST_SNPRINTF4(snprintf(NULL, 0, "abc%hndef", &hn), 6, NULL, NULL, 0, hn, 0)
	TEST_SNPRINTF4(snprintf(NULL, 0, "%d %d %d%hn %d", 1, 1, 1, &hn, 1), 7, NULL, NULL, 0, hn, 0)
	TEST_SNPRINTF4(snprintf(NULL, 0, "%g%hn", 1.5, &hn), 3, NULL, NULL, 0, hn, 0)
	TEST_SNPRINTF4(snprintf(NULL, 0, "%g%hn1", 1.5, &hn), 4, NULL, NULL, 0, hn, 0)
	TEST_SNPRINTF4(snprintf(NULL, 0, "%s%hn", "a string", &hn), 8, NULL, NULL, 0, hn, 0)
	TEST_SNPRINTF4(snprintf(NULL, 0, "%s%hn1", "a string", &hn), 9, NULL, NULL, 0, hn, 0)

	TEST_SNPRINTF4(snprintf(NULL, 0, "%ln", &ln), 0, NULL, NULL, 0, ln, 0)
	TEST_SNPRINTF4(snprintf(NULL, 0, "%ln1", &ln), 1, NULL, NULL, 0, ln, 0)
	TEST_SNPRINTF4(snprintf(NULL, 0, "a%ln", &ln), 1, NULL, NULL, 0, ln, 0)
	TEST_SNPRINTF4(snprintf(NULL, 0, "a%lnb", &ln), 2, NULL, NULL, 0, ln, 0)
	TEST_SNPRINTF4(snprintf(NULL, 0, "ab%ln", &ln), 2, NULL, NULL, 0, ln, 0)
	TEST_SNPRINTF4(snprintf(NULL, 0, "ab%lnc", &ln), 3, NULL, NULL, 0, ln, 0)
	TEST_SNPRINTF4(snprintf(NULL, 0, "abc%ln", &ln), 3, NULL, NULL, 0, ln, 0)
	TEST_SNPRINTF4(snprintf(NULL, 0, "abc%lndef", &ln), 6, NULL, NULL, 0, ln, 0)
	TEST_SNPRINTF4(snprintf(NULL, 0, "%d %d %d%ln %d", 1, 1, 1, &ln, 1), 7, NULL, NULL, 0, ln, 0)
	TEST_SNPRINTF4(snprintf(NULL, 0, "%g%ln", 1.5, &ln), 3, NULL, NULL, 0, ln, 0)
	TEST_SNPRINTF4(snprintf(NULL, 0, "%g%ln1", 1.5, &ln), 4, NULL, NULL, 0, ln, 0)
	TEST_SNPRINTF4(snprintf(NULL, 0, "%s%ln", "a string", &ln), 8, NULL, NULL, 0, ln, 0)
	TEST_SNPRINTF4(snprintf(NULL, 0, "%s%ln1", "a string", &ln), 9, NULL, NULL, 0, ln, 0)

	/* Test truncation and return value */

#define TEST_SNPRINTF5(action, eq, buf, val, size) \
	if (++tests, (rc = (action)) != (eq)) \
		++errors, printf("Test%d: %s failed (returned %d, not %d)\n", tests, (#action), rc, (eq)); \
	else if (buf && val && (size) && memcmp((buf), (val), (size))) \
		++errors, printf("Test%d: %s failed (%s, not %s)\n", tests, (#action), (char *)(buf), (char *)(val));

	TEST_SNPRINTF5(snprintf(NULL, 0, ""), 0, NULL, NULL, 0)
	TEST_SNPRINTF5(snprintf(NULL, 0, "abc"), 3, NULL, NULL, 0)
	TEST_SNPRINTF5(snprintf(buf1, 1, "abc"), 3, buf1, "", 1)
	TEST_SNPRINTF5(snprintf(buf1, 1, "%s", "abc"), 3, buf1, "", 1)
	TEST_SNPRINTF5(snprintf(buf1, 1, "%d", 10), 2, buf1, "", 1)
	TEST_SNPRINTF5(snprintf(buf1, 1, "%i", 10), 2, buf1, "", 1)
	TEST_SNPRINTF5(snprintf(buf1, 1, "%o", 10), 2, buf1, "", 1)
	TEST_SNPRINTF5(snprintf(buf1, 1, "%u", 10), 2, buf1, "", 1)
	TEST_SNPRINTF5(snprintf(buf1, 1, "%x", 10), 1, buf1, "", 1)
	TEST_SNPRINTF5(snprintf(buf1, 1, "%x", 16), 2, buf1, "", 1)
	TEST_SNPRINTF5(snprintf(buf1, 1, "%X", 16), 2, buf1, "", 1)
	TEST_SNPRINTF5(snprintf(buf1, 2, "%d", 16), 2, buf1, "1", 2)
	TEST_SNPRINTF5(snprintf(buf1, 2, "%.1f", -1.5), 4, buf1, "-", 2)
	TEST_SNPRINTF5(snprintf(buf1, 2, "%.1e", -1.5), 8, buf1, "-", 2)
	TEST_SNPRINTF5(snprintf(buf1, 2, "%.1E", -1.5), 8, buf1, "-", 2)
	TEST_SNPRINTF5(snprintf(buf1, 2, "%.2g", -1.5), 4, buf1, "-", 2)
	TEST_SNPRINTF5(snprintf(buf1, 2, "%.2G", -1.5), 4, buf1, "-", 2)
	TEST_SNPRINTF5(snprintf(buf1, 3, "%s", "abcd"), 4, buf1, "ab", 3)

	/* Test error reporting */

#define TEST_SNPRINTF6(action, format) \
	if (++tests, (rc = (action)) != -1) \
		++errors, printf("Test%d: snprintf(NULL, 0, \"%s\") failed (returned %d, not %d)\n", tests, (format), rc, -1); \
	else if (verbose) \
		printf("Test%d: snprintf(NULL, 0, \"%s\") succeeded (returned -1)\n", tests, (format));

	for (x = 0; err_fmt[x]; ++x)
		TEST_SNPRINTF6(snprintf(NULL, 0, err_fmt[x]), err_fmt[x])

	if (errors)
		printf("%d/%d tests failed\n", errors, tests);
	else
		printf("All tests passed\n");

	if (errors)
	{
		printf("\n");
		printf("    Note: You can ignore any errors involving %%p or NULL pointers (%%p or %%s).\n");
		printf("    They are undefined by the ISO C standard so any behaviour is correct.\n");

		printf("\n");
		printf("    Note: There are some tests for combinations of flags and conversions that\n");
		printf("    are undefined by the ISO C standard so any behaviour is correct.\n");
		printf("    This implementation can be configured to behave like Linux or Solaris.\n");
		printf("    On other systems, these tests may fail.\n");

		printf("\n");
		printf("    Note: There may also be failures that indicate that this system's sprintf()\n");
		printf("    is not ISO C compliant since most of the tests just compare the output of\n");
		printf("    snprintf() with this system's sprintf().\n");
	}

	return (errors == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
#endif

/* vi:set ts=4 sw=4: */
