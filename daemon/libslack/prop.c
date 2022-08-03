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

I<libslack(prop)> - program properties file module

=head1 SYNOPSIS

    #include <slack/std.h>
    #include <slack/prop.h>

    const char *prop_get(const char *name);
    const char *prop_get_or(const char *name, const char *default_value);
    const char *prop_set(const char *name, const char *value);
    int prop_get_int(const char *name);
    int prop_get_int_or(const char *name, int default_value);
    int prop_set_int(const char *name, int value);
    double prop_get_double(const char *name);
    double prop_get_double_or(const char *name, double default_value);
    double prop_set_double(const char *name, double value);
    int prop_get_bool(const char *name);
    int prop_get_bool_or(const char *name, int default_value);
    int prop_set_bool(const char *name, int value);
    int prop_unset(const char *name);
    int prop_save(void);
    int prop_clear(void);
    int prop_locker(Locker *locker);

=head1 DESCRIPTION

This module provides support for system-wide and user-specific (generic and
program-specific) properties in "well-known" locations:

    /etc/properties/app          - system-wide, generic properties
    ~/.properties/app            - user-defined, generic properties
    /etc/properties/app.progname - system-wide, program-specific properties
    ~/.properties/app.progname   - user-defined, program-specific properties

Note that, depending on how I<libslack> was installed, the system-wide
C</etc/properties> directory might not be under C</etc>. It might be under
another directory, such as C</usr/local/etc> or C</opt/local/etc>. If you
aren't sure where it is expected, run C<libslack-config --cflags> and look
for the definition of C<ETC_DIR>.

When the client first requests, sets, or unsets a property, all properties
relevant to the current program are loaded from these files in the order
given above. This order ensures that program-specific properties override
generic properties and that user-defined properties override system-wide
properties. The client can change properties at runtime and save the current
properties back to disk (to the user-defined, program-specific properties
file).

Program names (as returned by I<prog_name(3)>) are converted into file name
suffixes by replacing every occurrence of the file path separator (C<'/'>)
with a C<'-'>. Properties files consist of one property per line.

Each property is specified by its name, followed by C<'='> followed by its
value. The name must not have a C<'='> in it unless it is quoted with a
preceding C<'\'>. Special characters are expressed in I<C> string literal
notation (e.g. C<\a\b\f\n\r\t\v\x1b>). Spaces immediately before and after
the C<'='> are stripped unless they are quoted with a preceding C<'\'>. The
properties files may also contain blank lines and comments (C<'#'> until the
end of the line).

Boolean property values can be expressed as C<0> or C<1>, C<true> or
C<false>, C<yes> or C<no>, C<on> or C<off> (case insensitive).

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

#include <pwd.h>
#include <sys/stat.h>

#include "prog.h"
#include "daemon.h"
#include "map.h"
#include "err.h"
#include "lim.h"
#include "mem.h"
#include "prop.h"
#include "str.h"
#include "locker.h"

#ifndef HAVE_SNPRINTF
#include "snprintf.h"
#endif

#ifndef isodigit
#define isodigit(c) (isdigit((int)(unsigned char)c) && (unsigned char)(c) < '8')
#endif

#ifndef is_space
#define is_space(c) isspace((int)(unsigned char)(c))
#endif

typedef struct Prop Prop;

struct Prop
{
	Map *map;
	Prop *defaults;
};

#ifndef TEST

static struct
{
	int init;
	Prop *prop;
	char *home;
	int dirty;
	Locker *locker;
}
g = { 0, NULL, NULL, 0, NULL };

/*

C<Prop *prop_create(Map *map, Prop *defaults)>

Creates and returns a I<Prop> containing C<map> and C<defaults>. On error,
returns C<null> with C<errno> set appropriately. The new I<Prop> will
destroy C<map> and C<defaults> when it is destroyed.

*/

static Prop *prop_create(Map *map, Prop *defaults)
{
	Prop *prop;

	if (!(prop = mem_new(Prop)))
		return NULL;

	prop->map = map;
	prop->defaults = defaults;

	return prop;
}

/*

C<void prop_release(Prop *prop)>

Releases (deallocates) C<prop>.

*/

static void prop_release(Prop *prop)
{
	if (!prop)
		return;

	map_release(prop->map);
	prop_release(prop->defaults);
	mem_release(prop);
}

/*

C<int key_cmp(const char **a, const char **b)>

Compares the two strings pointed to by C<a> and C<b> using I<strcmp(3)>.

*/

static int key_cmp(const char **a, const char **b)
{
	return strcmp(*a, *b);
}

static const char *special_code = "abfnrtv";
static const char *special_char = "\a\b\f\n\r\t\v";
static const char *eq = "=";

/*

C<String *quote_special(const char *src)>

Replaces every occurrence in C<src> of special characters (represented in
I<C> by C<"\a">, C<"\b">, C<"\f">, C<"\n">, C<"\r">, C<"\t">, C<"\v">) with
their corresponding I<C> representation. Other non-printable characters are
replaced with their I<ASCII> codes in hexadecimal (i.e. "\xhh"). It is the
caller's responsibility to deallocate the returned I<String> with
I<str_release(3)> or I<str_destroy(3)>. It is strongly recommended to use
I<str_destroy(3)>, because it also sets the pointer variable to C<null>.

*/

static String *quote_special(const char *src)
{
	return encode(src, special_char, special_code, '\\', 1);
}

/*

C<String *unquote_special(const char *src)>

Replaces every occurrence in C<src> of C<"\a">, C<"\b">, C<"\f">, C<"\n">,
C<"\r">, C<"\t"> or C<"\v"> with the corresponding special characters (as
interpreted by I<C>). Ascii codes in octal or hexadecimal (i.e. "\ooo" or
"\xhh") are replaced with their corresponding I<ASCII> characters. It is the
caller's responsibility to deallocate the returned I<String> with
I<str_release(3)> or I<str_destroy(3)>. It is strongly recommended to use
I<str_destroy(3)>, because it also sets the pointer variable to C<null>.

*/

static String *unquote_special(const char *src)
{
	return decode(src, special_char, special_code, '\\', 1);
}

/*

C<String *quote_equals(const char *src)>

Replaces every occurrence in C<src> of C<"="> with C<"\=">. It is the
caller's responsibility to deallocate the returned I<String> with
I<str_release(3)> or I<str_destroy(3)>. It is strongly recommended to use
I<str_destroy(3)>, because it also sets the pointer variable to C<null>.

*/

static String *quote_equals(const char *src)
{
	return encode(src, eq, eq, '\\', 0);
}

/*

C<String *quote_equals(const char *src)>

Replaces every occurrence in C<src> of C<"\="> with C<"=">. It is the
caller's responsibility to deallocate the returned I<String> with
I<str_release(3)> or I<str_destroy(3)>. It is strongly recommended to use
I<str_destroy(3)>, because it also sets the pointer variable to C<null>.

*/

static String *unquote_equals(const char *src)
{
	return decode(src, eq, eq, '\\', 0);
}

/*

C<char *user_home(void)>

Returns the user's home directory (obtained from C</etc/passwd>). The return
value is cached so that any subsequent calls will be faster.

*/

static char *user_home(void)
{
	struct passwd *pwent;
	char *home = NULL;

	if (g.home)
		return g.home;

	if ((pwent = getpwuid(getuid())))
		home = pwent->pw_dir;

	return g.home = (home && strlen(home)) ? mem_strdup(home) : NULL;
}

/*

C<void prop_parse(Map *map, const char *path, char *line, size_t lineno)>

Parses one line from a properties file. C<path> is the path of the
properties file. C<line> is the text to parse. C<lineno> is the current line
number. The property parsed, if any, is added to C<map>. Emits error
messages when syntax errors occur. That's probably a mistake. To suppress
the error messages, call I<prog_err_none(3)> first and restore error
messages afterwards with something like I<prog_err_stderr(3)>.

*/

static void prop_parse(Map *map, const char *path, char *line, size_t lineno)
{
	String *prop, *name;
	char *p, *eq, *value, *val, *key;

	/* Unquote any special characters in the line */

	if (!(prop = unquote_special(line)))
	{
		error("prop: Out of memory");
		return;
	}

	/* Find first unquoted '=' */

	for (p = cstr(prop), eq = strchr(p, '='); eq; eq = strchr(eq + 1, '='))
		if (eq == p || eq[-1] != '\\')
			break;

	if (!eq)
	{
		error("prop: %s line %d: Expected '='\n%s", path, lineno, line);
		str_release(prop);
		set_errno(EINVAL);
		return;
	}

	/* Identify and separate the name and value */

	value = eq + 1;

	while (is_space(value[0]))
		++value;

	while (eq > p && is_space(eq[-1]) && (eq == p + 1 || eq[-2] != '\\'))
		--eq;

	*eq = nul;

	/* Unquote any quoted trailing space in the key */

	if (eq > p + 1 && is_space(eq[-1]) && eq[-2] == '\\')
	{
		eq[-2] = eq[-1];
		eq[-1] = nul;
	}

	/* Unquote any quoted '=' in the name */

	if (!(name = unquote_equals(p)))
	{
		error("prop: Out of memory");
		str_release(prop);
		return;
	}

	key = cstr(name);

	/* Unquote any quoted leading space in the value */

	if (*value == '\\')
		++value;

	/* Add this property to the map */

	if (!(val = mem_strdup(value)))
	{
		error("prop: Out of memory");
		str_release(prop);
		str_release(name);
		return;
	}

	if (map_add(map, key, val) == -1)
	{
		error("prop: %s line %d: Property %s already defined\n%s", path, lineno, name, line);
		mem_release(val);
	}

	str_release(prop);
	str_release(name);
}

/*

C<Prop *prop_load(const char *path, Prop *defaults)>

Creates and returns a new I<Prop> containing C<defaults> and the properties
obtained from the properties file specified by C<path>. On error, returns
C<null> with C<errno> set appropriately.

*/

static Prop *prop_load(const char *path, Prop *defaults)
{
	Prop *prop;
	Map *map;

	if (!(map = map_create((map_release_t *)free)))
		return NULL;

	if (!daemon_parse_config(path, map, (daemon_config_parser_t *)prop_parse))
	{
		map_release(map);
		return NULL;
	}

	if (!(prop = prop_create(map, defaults)))
	{
		map_release(map);
		return NULL;
	}

	return prop;
}

/*

C<int prop_init(void)>

Initialises the I<prop(3)> module. Loads properties from the following locations:

    /etc/properties/app          - system-wide, generic properties
    ~/.properties/app            - user-defined, generic properties
    /etc/properties/app.progname - system-wide, program-specific properties
    ~/.properties/app.progname   - user-defined, program-specific properties

Note that, depending on how I<libslack> was installed, the system-wide
C</etc/properties> directory might not be under C</etc>. It might be under
another directory, such as C</usr/local/etc> or C</opt/local/etc>. If you
aren't sure where it is expected, run C<libslack-config --cflags> and look
for the definition of C<ETC_DIR>.

Properties from the first three files become (nested) defaults. Properties
from the last file becomes the top-level I<Prop>. If the last file doesn't
exist, an empty I<Prop> becomes the top-level I<Prop>.

Called at the start of the first get, set, or unset function called.

*/

static int prop_init(void)
{
	char *path;
	Prop *prop = NULL;
	Prop *prop_next;
	char *home;
	int writable = 0;
	size_t path_len;

	path_len = limit_path();

	if (!(path = mem_create(path_len, char)))
		return -1;

	/* System wide generic properties: /etc/properties/app */

	snprintf(path, path_len, "%s%cproperties%capp", ETC_DIR, PATH_SEP, PATH_SEP);
	if ((prop_next = prop_load(path, prop)))
		prop = prop_next;

	/* User defined generic properties: ~/.properties/app */

	if ((home = user_home()))
	{
		snprintf(path, path_len, "%s%c.properties%capp", home, PATH_SEP, PATH_SEP);
		if ((prop_next = prop_load(path, prop)))
			prop = prop_next;
	}

	if (prog_name())
	{
		char *progname, *sep;

		if (!(progname = mem_strdup(prog_name())))
		{
			mem_release(path);
			prop_release(prop);
			return -1;
		}

		for (sep = strchr(progname, PATH_SEP); sep; sep = strchr(sep, PATH_SEP))
			*sep++ = '-';

		/* System wide program specific properties: /etc/properties/app.progname */

		snprintf(path, path_len, "%s%cproperties%capp.%s", ETC_DIR, PATH_SEP, PATH_SEP, progname);
		if ((prop_next = prop_load(path, prop)))
			prop = prop_next;

		/* User defined program specific properties: ~/.properties/app.progname */

		if (home)
		{
			snprintf(path, path_len, "%s%c.properties%capp.%s", home, PATH_SEP, PATH_SEP, progname);
			if ((prop_next = prop_load(path, prop)))
			{
				prop = prop_next;
				writable = 1;
			}
		}

		free(progname);
	}

	/* Guarantee a user defined program specific property map for prop_set() */

	if (!writable)
	{
		Map *map;

		if (!(map = map_create((map_release_t *)free)))
		{
			mem_release(path);
			prop_release(prop);
			return -1;
		}

		if (!(prop_next = prop_create(map, prop)))
		{
			mem_release(path);
			prop_release(prop);
			map_release(map);
			return -1;
		}

		prop = prop_next;
	}

	mem_release(path);
	g.prop = prop;
	g.init = 1;

	return 0;
}

/*

=item C<const char *prop_get(const char *name)>

Returns the value of the property named C<name>. Returns C<null> if there is
no such property. On error, returns C<null> with C<errno> set appropriately.

=cut

*/

const char *prop_get(const char *name)
{
	Prop *p;
	const char *value = NULL;
	int err;

	if ((err = locker_wrlock(g.locker)))
		return set_errnull(err);

	if (!g.init && prop_init() == -1)
	{
		locker_unlock(g.locker);
		return NULL;
	}

	for (p = g.prop; p; p = p->defaults)
		if ((value = map_get(p->map, name)))
			break;

	if ((err = locker_unlock(g.locker)))
		return set_errnull(err);

	return value;
}

/*

=item C<const char *prop_get_or(const char *name, const char *default_value)>

Returns the value of the property named C<name>. Returns C<default_value> on
error, or if there is no such property.

=cut

*/

const char *prop_get_or(const char *name, const char *default_value)
{
	const char *prop = prop_get(name);

	return prop ? prop : default_value;
}

/*

=item C<const char *prop_set(const char *name, const char *value)>

Sets the property named C<name> to a dynamically allocated copy of C<value>.
If I<prop_save(3)> is called after a call to this function, the new property
will be saved to disk, and will be available the next time this program is
executed. On success, returns the copy of C<value>. On error, returns
C<null> with C<errno> set appropriately.

=cut

*/

const char *prop_set(const char *name, const char *value)
{
	char *val;
	int err;

	if ((err = locker_wrlock(g.locker)))
		return set_errnull(err);

	if (!g.init && prop_init() == -1)
	{
		locker_unlock(g.locker);
		return NULL;
	}

	if (!(val = mem_strdup(value)))
	{
		locker_unlock(g.locker);
		return NULL;
	}

	if (map_put(g.prop->map, name, val) == -1)
	{
		mem_release(val);
		locker_unlock(g.locker);
		return NULL;
	}

	g.dirty = 1;

	if ((err = locker_unlock(g.locker)))
		return set_errnull(err);

	return val;
}

/*

=item C<int prop_get_int(const char *name)>

Returns the value of the property named C<name> as an integer. Returns C<0>
on error, or if there is no such property, or if it is not interpretable as
a decimal integer.

=cut

*/

int prop_get_int(const char *name)
{
	return prop_get_int_or(name, 0);
}

/*

=item C<int prop_get_int_or(const char *name, int default_value)>

Returns the value of the property named C<name> as an integer. Returns
C<default_value> on error, or if there is no such property, or if it is not
interpretable as a decimal integer.

=cut

*/

int prop_get_int_or(const char *name, int default_value)
{
	const char *prop = prop_get(name);
	int val;

	return (prop && sscanf(prop, " %d ", &val)) ? val : default_value;
}

/*

=item C<int prop_set_int(const char *name, int value)>

Sets the property named C<name> to C<value>. If I<prop_save(3)> is called
after a call to this function, the new property will be saved to disk and
will be available the next time this program is executed. On success,
returns C<value>. On error, returns C<0>.

=cut

*/

int prop_set_int(const char *name, int value)
{
	char buf[128];
	snprintf(buf, sizeof(buf), "%d", value);
	return prop_set(name, buf) ? value : 0;
}

/*

=item C<double prop_get_double(const char *name)>

Returns the value of the property named C<name> as a double. Returns C<0.0>
on error, or if there is no such property, or if it is not interpretable as
a floating point number.

=cut

*/

double prop_get_double(const char *name)
{
	return prop_get_double_or(name, 0.0);
}

/*

=item C<double prop_get_double_or(const char *name, double default_value)>

Returns the value of the property named C<name> as a double. Returns
C<default_value> on error, or if there is no such property, or if it is not
interpretable as a floating point number.

=cut

*/

double prop_get_double_or(const char *name, double default_value)
{
	const char *prop = prop_get(name);
	double val;

	return (prop && sscanf(prop, "%lg", &val)) ? val : default_value;
}

/*

=item C<double prop_set_double(const char *name, double value)>

Sets the property named C<name> to C<value>. If I<prop_save(3)> is called
after a call to this function, the new property will be saved to disk and
will be available the next time this program is executed. On success,
returns C<value>. On error, returns C<0.0>.

=cut

*/

double prop_set_double(const char *name, double value)
{
	char buf[128];
	snprintf(buf, 128, "%g", value);
	return prop_set(name, buf) ? value : -1;
}

/*

=item C<int prop_get_bool(const char *name)>

Returns the value of the property named C<name> as a boolean value. Returns
C<0> on error or if there is no such property. The values: C<"true">,
C<"yes">, C<"on"> and C<"1"> are all interpreted as C<true>. All other
values are interpreted as C<false>.

=cut

*/

int prop_get_bool(const char *name)
{
	return prop_get_bool_or(name, 0);
}

/*

=item C<int prop_get_bool_or(const char *name, int default_value)>

Returns the value of the property named C<name> as a boolean value. Returns
C<default_value> on error, or if there is no such property, or if it is not
interpretable as a boolean value. The values: C<"true">, C<"yes">, C<"on">
and C<"1"> are all interpreted as C<true>. The values: C<"false">, C<"no">,
C<"off"> and C<"0"> are all interpreted as C<false>. All other values are
disregarded and will cause C<default_value> to be returned.

=cut

*/

int prop_get_bool_or(const char *name, int default_value)
{
	const char *prop = prop_get(name);
	char buf[128];
	int val;

	if (!prop)
		return default_value;

	if (sscanf(prop, " %d ", &val))
		return val;

	if (sscanf(prop, " %127s ", buf))
	{
		if ((buf[0] == 't' || buf[0] == 'T') &&
			(buf[1] == 'r' || buf[1] == 'R') &&
			(buf[2] == 'u' || buf[2] == 'U') &&
			(buf[3] == 'e' || buf[3] == 'E') &&
			(buf[4] == nul))
			return 1;

		if ((buf[0] == 'f' || buf[0] == 'F') &&
			(buf[1] == 'a' || buf[1] == 'A') &&
			(buf[2] == 'l' || buf[2] == 'L') &&
			(buf[3] == 's' || buf[3] == 'S') &&
			(buf[4] == 'e' || buf[4] == 'E') &&
			(buf[5] == nul))
			return 0;

		if ((buf[0] == 'y' || buf[0] == 'Y') &&
			(buf[1] == 'e' || buf[1] == 'E') &&
			(buf[2] == 's' || buf[2] == 'S') &&
			(buf[3] == nul))
			return 1;

		if ((buf[0] == 'n' || buf[0] == 'N') &&
			(buf[1] == 'o' || buf[1] == 'O') &&
			(buf[2] == nul))
			return 0;

		if ((buf[0] == 'o' || buf[0] == 'O') &&
			(buf[1] == 'n' || buf[1] == 'N') &&
			(buf[2] == nul))
			return 1;

		if ((buf[0] == 'o' || buf[0] == 'O') &&
			(buf[1] == 'f' || buf[1] == 'F') &&
			(buf[2] == 'f' || buf[2] == 'F') &&
			(buf[3] == nul))
			return 0;
	}

	return default_value;
}

/*

=item C<int prop_set_bool(const char *name, int value)>

Sets the property named C<name> to C<value>. If I<prop_save(3)> is called
after a call to this function, the new property will be saved to disk and
will be available the next time this program is executed. On success,
returns C<value>. On error, returns C<0>.

=cut

*/

int prop_set_bool(const char *name, int value)
{
	return prop_set_int(name, value);
}

/*

=item C<int prop_unset(const char *name)>

Removes the property named C<name>. Property removal is only saved to disk
when I<prop_save(3)> is called, if the property existed only in the
user-defined, program-specific properties file, or was created by the
program at runtime. On success, returns C<0>. On error, returns C<-1> with
C<errno> set appropriately.

=cut

*/

int prop_unset(const char *name)
{
	Prop *p;
	int err;

	if ((err = locker_wrlock(g.locker)))
		return set_errno(err);

	if (!g.init && prop_init() == -1)
	{
		locker_unlock(g.locker);
		return -1;
	}

	for (p = g.prop; p; p = p->defaults)
		map_remove(p->map, name);

	g.dirty = 1;

	if ((err = locker_unlock(g.locker)))
		return set_errno(err);

	return 0;
}

/*

=item C<int prop_save(void)>

Saves the program's properties to disk. If the property named C<"save"> is
set to C<"false">, C<"no">, C<"off"> or C<"0">, nothing is written to disk.
If no properties were added, removed or changed, nothing is written to disk.
Only the user-defined, program-specific properties are saved. Generic and
system-wide properties files must be edited by hand. Each program can only
save its own properties. They are saved in the following file:

    ~/.properties/app.progname

Where C<progname> is the name of the program after being translated as
described at the top of the I<DESCRIPTION> section.

The C<~/.properties> directory is created if necessary. On success, returns
C<0>. On error, returns C<-1> with C<errno> set appropriately.

=cut

*/

int prop_save(void)
{
	char *path;
	size_t path_len;
	size_t len;
	char *home;
	char *progname, *sep;
	struct stat status[1];
	List *keys;
	Lister *k;
	FILE *file;
	int err;

	if (!prop_get_bool_or("save", 1))
		return 0;

	if ((err = locker_wrlock(g.locker)))
		return set_errno(err);

	if (!g.dirty)
	{
		if ((err = locker_unlock(g.locker)))
			return set_errno(err);

		return 0;
	}

	if (!prog_name())
	{
		locker_unlock(g.locker);
		return set_errno(EINVAL);
	}

	if (!(home = user_home()))
	{
		locker_unlock(g.locker);
		return set_errno(EINVAL);
	}

	path_len = limit_path();

	if (!(path = mem_create(path_len, char)))
	{
		locker_unlock(g.locker);
		return -1;
	}

	snprintf(path, path_len, "%s%c.properties", home, PATH_SEP);

	if (stat(path, status) == -1 && mkdir(path, S_IRWXU) == -1)
	{
		mem_release(path);
		locker_unlock(g.locker);
		return -1;
	}

	if (stat(path, status) == -1 || S_ISDIR(status->st_mode) == 0)
	{
		mem_release(path);
		locker_unlock(g.locker);
		return set_errno(EINVAL);
	}

	if (!(progname = mem_strdup(prog_name())))
	{
		mem_release(path);
		locker_unlock(g.locker);
		return -1;
	}

	for (sep = strchr(progname, PATH_SEP); sep; sep = strchr(sep, PATH_SEP))
		*sep++ = '-';

	len = strlen(path);
	snprintf(path + len, path_len - len, "%capp.%s", PATH_SEP, progname);
	mem_release(progname);

	file = fopen(path, "w");
	mem_release(path);

	if (!file)
	{
		locker_unlock(g.locker);
		return -1;
	}

	if (!(keys = map_keys(g.prop->map)))
	{
		locker_unlock(g.locker);
		return -1;
	}

	if (!list_sort(keys, (list_cmp_t *)key_cmp))
	{
		list_release(keys);
		locker_unlock(g.locker);
		return -1;
	}

	if (!(k = lister_create(keys)))
	{
		list_release(keys);
		locker_unlock(g.locker);
		return -1;
	}

	while (lister_has_next(k) == 1)
	{
		const char *key = (const char *)lister_next(k);
		const char *value = map_get(g.prop->map, key);
		String *lhs, *rhs, *lhs2;

		/* Quote any '=' in the key */

		if (!(lhs = quote_equals(key)))
		{
			fclose(file);
			lister_release(k);
			list_release(keys);
			locker_unlock(g.locker);
			return -1;
		}

		/* Quote any special chars in the key */

		if (!(lhs2 = quote_special(cstr(lhs))))
		{
			fclose(file);
			lister_release(k);
			list_release(keys);
			str_release(lhs);
			locker_unlock(g.locker);
			return -1;
		}

		str_release(lhs);
		lhs = lhs2;

		/* Quote any trailing space in the key */

		if (str_length(lhs) && is_space(cstr(lhs)[str_length(lhs) - 1]))
		{
			if (!str_insert(lhs, str_length(lhs) - 1, "\\"))
			{
				fclose(file);
				lister_release(k);
				list_release(keys);
				str_release(lhs);
				locker_unlock(g.locker);
				return -1;
			}
		}

		/* Quote any special chars in the value */

		if (!(rhs = quote_special(value)))
		{
			fclose(file);
			lister_release(k);
			list_release(keys);
			str_release(lhs);
			locker_unlock(g.locker);
			return -1;
		}

		/* Quote any leading space in the value */

		if (is_space(cstr(rhs)[0]))
		{
			if (!str_insert(rhs, 0, "\\"))
			{
				fclose(file);
				lister_release(k);
				list_release(keys);
				str_release(lhs);
				locker_unlock(g.locker);
				return -1;
			}
		}

		fprintf(file, "%s=%s\n", cstr(lhs), cstr(rhs));
		str_release(lhs);
		str_release(rhs);
	}

	fclose(file);
	lister_release(k);
	list_release(keys);
	g.dirty = 0;

	if ((err = locker_unlock(g.locker)))
		return set_errno(err);

	return 0;
}

/*

=item C<int prop_clear(void)>

Clears the properties as though they were never initialised. On success,
returns C<0>. On error, returns C<-1> with C<errno> set appropriately.

=cut

*/

int prop_clear(void)
{
	int err;

	if ((err = locker_wrlock(g.locker)))
		return set_errno(err);

	prop_release(g.prop);
	g.prop = NULL;
	g.init = 0;
	g.dirty = 0;

	if ((err = locker_unlock(g.locker)))
		return set_errno(err);

	return 0;
}

/*

=item C<int prop_locker(Locker *locker)>

Sets the locking strategy for the I<prop(3)> module to C<locker>. This is
only needed in multi-threaded programs. It must only be called once, from
the main thread. On success, returns C<0>. On error, returns C<-1> with
C<errno> set appropriately.

=cut

*/

int prop_locker(Locker *locker)
{
	if (g.locker)
		return set_errno(EINVAL);

	g.locker = locker;

	return 0;
}

/*

=back

=head1 ERRORS

On error, C<errno> is set either by an underlying function, or as follows:

=over 4

=item C<EINVAL>

When there is no prog_name or home directory, or when there is a parse error
in a properties file, or if C<~/.properties> exists but is not a directory.
I<prop_locker(3)> sets this when an attempt is made to change the locker
after one has already been set.

=back

=head1 MT-Level

I<MT-Disciplined>

=head1 FILES

    /etc/properties/app
    ~/.properties/app
    /etc/properties/app.*
    ~/.properties/app.*

=head1 EXAMPLE

    #include <slack/std.h>
    #include <slack/prog.h>
    #include <slack/prop.h>
    #include <slack/err.h>

    int main(int ac, char **av)
    {
        const char *s; int i; double d; int b;

        prog_init();
        prog_set_name(*av);

        s = prop_get("string");
        i = prop_get_int_or("int", 1);
        d = prop_get_double_or("double", 1.0);
        b = prop_get_bool("boolean");

        msg("s = '%s'\ni = %d\nd = %g\nb = %s\n", s, i, d, (b) ? "true" : "false");

        prop_set("string", "strung");
        prop_set_int("int", i += 4);
        prop_set_double("double", d *= 1.75);
        prop_set_bool("boolean", !b);
        prop_save();

        return EXIT_SUCCESS;
    }

=head1 BUGS

This only provides coarse-grained persistence. If multiple instances of the
same program are setting properties, the last to exit wins. This can be
overcome by calling I<prop_save()> after setting any property and
I<prop_clear()> before getting any property.

=head1 SEE ALSO

I<libslack(3)>,
I<prog(3)>

=head1 AUTHOR

20210220 raf <raf@raf.org>

=cut

*/

#endif

#ifdef TEST

static const char *user_home()
{
	struct passwd *pwent;
	char *home = NULL;

	if ((pwent = getpwuid(getuid())))
		home = pwent->pw_dir;

	return (home && strlen(home)) ? home : NULL;
}

static int props_exist(void)
{
	struct stat status[1];
	const char *home;
	char *path;
	size_t path_len;

	if (!(home = user_home()))
		return 0;

	path_len = limit_path();

	if (!(path = mem_create(path_len, char)))
		return -1;

	snprintf(path, path_len, "%s%c.properties", home, PATH_SEP);

	if (stat(path, status) == 0 && S_ISDIR(status->st_mode) == 1)
	{
		mem_release(path);
		return 1;
	}

	mem_release(path);
	return 0;
}

static void clean(int has_props)
{
	const char *home;
	char *path;
	char *progname;
	char *sep;
	size_t path_len;
	size_t len;

	if (!(home = user_home()))
		return;

	path_len = limit_path();

	if (!(path = mem_create(path_len, char)))
		return;

	if (!(progname = mem_strdup(prog_name())))
	{
		mem_release(path);
		return;
	}

	for (sep = strchr(progname, PATH_SEP); sep; sep = strchr(sep, PATH_SEP))
		*sep++ = '-';

	snprintf(path, path_len, "%s%c.properties", home, PATH_SEP);
	len = strlen(path);
	snprintf(path + len, path_len - len, "%capp.%s", PATH_SEP, progname);
	unlink(path);
	path[len] = nul;

	if (!has_props)
		rmdir(path);

	mem_release(path);
	mem_release(progname);
}

int main(int ac, char **av)
{
	struct
	{
		const char *key;
		const char *value;
	}
	data[] =
	{
		{ "key", "value" },
		{ " key with spaces ", "value with spaces" },
		{ "key with = sign", " value with leading space" },
		{ "key with newline\n and = two = signs", "value with newline\n!" },
		{ "key with newline,\n = two = signs and an Escape\033!", "value with newline\n and two non printables\001!\002!" },
		{ "", "" },
		{ NULL, NULL }
	};

	const char * const key = "key";
	const char * const value = "value";
	const char * const not_key = "not key";
	int has_props = props_exist();
	const char *val;
	int ival, i;
	int int_val;
	const char *sval;
	double double_val;
	int bool_val;
	int errors = 0;

	if (ac == 2 && !strcmp(av[1], "help"))
	{
		printf("usage: %s\n", *av);
		return EXIT_SUCCESS;
	}

	printf("Testing: %s\n", "prop");
	prog_init();

	val = prop_set(key, value);
	if (strcmp(val, value))
		++errors, printf("Test1: prop_set(key, value) failed (%s not %s)\n", val, value);
	val = prop_get(key);
	if (!val || strcmp(val, value))
		++errors, printf("Test2: prop_get(key) failed (%s not %s)\n", val ? val : "null", value);
	val = prop_get_or(key, NULL);
	if (strcmp(val, value))
		++errors, printf("Test3: prop_get_or(key, NULL) failed (%s not %s)\n", val, value);
	val = prop_get_or(not_key, value);
	if (strcmp(val, value))
		++errors, printf("Test4: prop_get_or(not_key, value) failed (%s not %s)\n", val, value);
	ival = prop_unset(key);
	if (ival != 0)
		++errors, printf("Test5: prop_unset() failed (%d not %d)\n", ival, 0);
	val = prop_get(key);
	if (val != NULL)
		++errors, printf("Test6: prop_get(key) (after unset) failed (%s not NULL)\n", val);
	val = prop_get_or(key, value);
	if (strcmp(val, value))
		++errors, printf("Test7: prop_get_or(key, value) (after unset) failed (%s not %s)\n", val, value);
	ival = prop_save();
	if (ival != -1)
		++errors, printf("Test8: prop_save() (without progname) failed (%d not -1)\n", ival);

	prog_set_name("prop.test");
	prop_clear();

	val = prop_get(not_key);
	if (val != NULL)
		++errors, printf("Test9: prop_get(not_key) failed (%s not NULL)\n", val);
	val = prop_get_or(not_key, value);
	if (val != value)
		++errors, printf("Test10: prop_get_or(not_key, value) failed (%s not %s)\n", val, value);
	val = prop_set(key, value);
	if (strcmp(val, value))
		++errors, printf("Test11: prop_set(key, value) failed (%s not %s)\n", val, value);
	ival = prop_save();
	if (ival != 0)
		++errors, printf("Test12: prop_save() (with progname) failed (%d not 0, errno = %s)\n", ival, strerror(errno));

	clean(has_props);
	prop_clear();

	for (i = 0; data[i].key; ++i)
	{
		val = prop_set(data[i].key, data[i].value);
		if (strcmp(val, data[i].value))
			++errors, printf("Test%d: prop_set('%s', '%s') failed ('%s' not '%s')\n", 13 + 2 * i, data[i].key, data[i].value, val, data[i].value);
		val = prop_get(data[i].key);
		if (!val || strcmp(val, data[i].value))
			++errors, printf("Test%d: prop_get('%s') failed ('%s' not '%s')\n", 14 + 2 * i, data[i].key, val ? val : "null", data[i].value);
	}

	ival = prop_save();
	if (ival != 0)
		++errors, printf("Test25: prop_save() (with progname) failed (%d not 0, errno = %s)\n", ival, strerror(errno));

	prop_clear();

	for (i = 0; data[i].key; ++i)
	{
		val = prop_get(data[i].key);
		if (!val || strcmp(val, data[i].value))
			++errors, printf("Test%d: prop_get('%s') failed ('%s' not '%s')\n", 26 + i, data[i].key, val ? val : "null", data[i].value);
	}

	clean(has_props);
	prop_clear();

	if ((int_val = prop_set_int("i", 37)) != 37)
		++errors, printf("Test32: prop_set_int() failed (%d not 37)\n", int_val);

	if ((int_val = prop_get_int("i")) != 37)
		++errors, printf("Test33: prop_get_int() failed (%d not 37)\n", int_val);

	if ((int_val = prop_get_int_or("i", 13)) != 37)
		++errors, printf("Test34: prop_get_int_or() failed (%d not 37)\n", int_val);

	if ((int_val = prop_get_int_or("j", 13)) != 13)
		++errors, printf("Test35: prop_get_int_or() failed (%d not 13)\n", int_val);

	if ((double_val = prop_set_double("d", 37.0)) != 37.0)
		++errors, printf("Test36: prop_set_double() failed (%g not 37.0)\n", double_val);

	if ((double_val = prop_get_double("d")) != 37)
		++errors, printf("Test37: prop_get_double() failed (%g not 37.0)\n", double_val);

	if ((double_val = prop_get_double_or("d", 13.0)) != 37)
		++errors, printf("Test38: prop_get_double_or() failed (%g not 37.0)\n", double_val);

	if ((double_val = prop_get_double_or("e", 13.0)) != 13.0)
		++errors, printf("Test39: prop_get_double_or() failed (%g not 13.0)\n", double_val);

	if ((bool_val = prop_set_bool("b", 1)) != 1)
		++errors, printf("Test40: prop_set_bool() failed (%d not 1)\n", bool_val);

	if ((bool_val = prop_get_bool("b")) != 1)
		++errors, printf("Test41: prop_get_bool() failed (%d not 1)\n", bool_val);

	if ((bool_val = prop_get_bool_or("b", 0)) != 1)
		++errors, printf("Test42: prop_get_bool_or() failed (%d not 1)\n", bool_val);

	if ((bool_val = prop_get_bool_or("c", 1)) != 1)
		++errors, printf("Test43: prop_get_bool_or() failed (%d not 1)\n", bool_val);

	sval = prop_set("b", "true");
	if (!sval)
		++errors, printf("Test44: prop_set() failed (returned null)\n");
	else if (strcmp(sval, "true"))
		++errors, printf("Test45: prop_set() failed (returned \"%s\" not \"%s\")\n", sval, "true");
	if ((bool_val = prop_get_bool_or("b", 0)) != 1)
		++errors, printf("Test46: prop_get_bool_or() failed (%d not 1)\n", bool_val);

	sval = prop_set("b", "false");
	if (!sval)
		++errors, printf("Test47: prop_set() failed (returned null)\n");
	else if (strcmp(sval, "false"))
		++errors, printf("Test48: prop_set() failed (returned \"%s\" not \"%s\")\n", sval, "true");
	if ((bool_val = prop_get_bool_or("b", 1)) != 0)
		++errors, printf("Test49: prop_get_bool_or() failed (%d not 0)\n", bool_val);

	sval = prop_set("b", "yes");
	if (!sval)
		++errors, printf("Test50: prop_set() failed (returned null)\n");
	else if (strcmp(sval, "yes"))
		++errors, printf("Test51: prop_set() failed (returned \"%s\" not \"%s\")\n", sval, "yes");
	if ((bool_val = prop_get_bool_or("b", 0)) != 1)
		++errors, printf("Test52: prop_get_bool_or() failed (%d not 1)\n", bool_val);

	sval = prop_set("b", "no");
	if (!sval)
		++errors, printf("Test53: prop_set() failed (returned null)\n");
	else if (strcmp(sval, "no"))
		++errors, printf("Test54: prop_set() failed (returned \"%s\" not \"%s\")\n", sval, "no");
	if ((bool_val = prop_get_bool_or("b", 1)) != 0)
		++errors, printf("Test55: prop_get_bool_or() failed (%d not 0)\n", bool_val);

	sval = prop_set("b", "on");
	if (!sval)
		++errors, printf("Test56: prop_set() failed (returned null)\n");
	else if (strcmp(sval, "on"))
		++errors, printf("Test57: prop_set() failed (returned \"%s\" not \"%s\")\n", sval, "on");
	if ((bool_val = prop_get_bool_or("b", 0)) != 1)
		++errors, printf("Test58: prop_get_bool_or() failed (%d not 1)\n", bool_val);

	sval = prop_set("b", "off");
	if (!sval)
		++errors, printf("Test59: prop_set() failed (returned null)\n");
	else if (strcmp(sval, "off"))
		++errors, printf("Test60: prop_set() failed (returned \"%s\" not \"%s\")\n", sval, "off");
	if ((bool_val = prop_get_bool_or("b", 1)) != 0)
		++errors, printf("Test61: prop_get_bool_or() failed (%d not 0)\n", bool_val);

	sval = prop_set("b", "neither");
	if (!sval)
		++errors, printf("Test62: prop_set() failed (returned null)\n");
	else if (strcmp(sval, "neither"))
		++errors, printf("Test63: prop_set() failed (returned \"%s\" not \"%s\")\n", sval, "neither");
	if ((bool_val = prop_get_bool_or("b", 1)) != 1)
		++errors, printf("Test64: prop_get_bool_or() failed (%d not 1)\n", bool_val);
	if ((bool_val = prop_get_bool_or("b", 0)) != 0)
		++errors, printf("Test65: prop_get_bool_or() failed (%d not 0)\n", bool_val);

	if (errors)
		printf("%d/65 tests failed\n", errors);
	else
		printf("All tests passed\n");

	return (errors == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

#endif

/* vi:set ts=4 sw=4: */
