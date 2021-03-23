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

I<libslack(lim)> - POSIX.1 limits module

=head1 SYNOPSIS

    #include <slack/std.h>
    #include <slack/lim.h>

    long limit_arg(void);
    long limit_child(void);
    long limit_tick(void);
    long limit_group(void);
    long limit_open(void);
    long limit_stream(void);
    long limit_tzname(void);
    long limit_job(void);
    long limit_save_ids(void);
    long limit_version(void);
    long limit_pcanon(const char *path);
    long limit_fcanon(int fd);
    long limit_canon(void);
    long limit_pinput(const char *path);
    long limit_finput(int fd);
    long limit_input(void);
    long limit_pvdisable(const char *path);
    long limit_fvdisable(int fd);
    long limit_vdisable(void);
    long limit_plink(const char *path);
    long limit_flink(int fd);
    long limit_link(void);
    long limit_pname(const char *path);
    long limit_fname(int fd);
    long limit_name(void);
    long limit_ppath(const char *path);
    long limit_fpath(int fd);
    long limit_path(void);
    long limit_ppipe(const char *path);
    long limit_fpipe(int fd);
    long limit_pnotrunc(const char *path);
    long limit_fnotrunc(int fd);
    long limit_notrunc(void);
    long limit_pchown(const char *path);
    long limit_fchown(int fd);
    long limit_chown(void);

=head1 DESCRIPTION

This module provides functions for simply and reliably obtaining a POSIX.1
limit for the current system or a usable default when a particular facility
is unlimited on the current system. These functions always return a usable
value.

=over 4

=cut

*/

#include "config.h"
#include "std.h"

#include <limits.h>

#include <sys/resource.h>

#include "lim.h"
#include "err.h"

enum name_t
{
	LIMIT_ARG,
	LIMIT_CHILD,
	LIMIT_TICK,
	LIMIT_GROUP,
	LIMIT_OPEN,
	LIMIT_STREAM,
	LIMIT_TZNAME,
	LIMIT_JOB,
	LIMIT_SAVE_IDS,
	LIMIT_VERSION,
	LIMIT_CANON,
	LIMIT_INPUT,
	LIMIT_VDISABLE,
	LIMIT_LINK,
	LIMIT_NAME,
	LIMIT_PATH,
	LIMIT_PIPE,
	LIMIT_NOTRUNC,
	LIMIT_CHOWN,
	LIMIT_COUNT
};

typedef struct conf_t conf_t;

struct conf_t
{
	const int name;       /* name argument for sysconf, [f]pathconf */
	const long value;     /* limit to use when indeterminate */
	const off_t offset;   /* offset to apply to value when not indeterminate */
};

#ifndef TEST

static const struct
{
	conf_t conf[LIMIT_COUNT];
}
g =
{
	{
		{ _SC_ARG_MAX,     131072, 0 },
		{ _SC_CHILD_MAX,     1024, 0 },
		{ _SC_CLK_TCK,         -1, 0 },
		{ _SC_NGROUPS_MAX,     32, 0 },
		{ _SC_OPEN_MAX,      1024, 0 },
		{ _SC_STREAM_MAX,    1024, 0 },
		{ _SC_TZNAME_MAX,       3, 0 },
		{ _SC_JOB_CONTROL,      0, 0 },
		{ _SC_SAVED_IDS,        0, 0 },
		{ _SC_VERSION,          0, 0 },
		{ _PC_MAX_CANON,      255, 0 },
		{ _PC_MAX_INPUT,      255, 0 },
		{ _PC_VDISABLE,         0, 0 },
		{ _PC_LINK_MAX,     32768, 0 },
		{ _PC_NAME_MAX,      1024, 0 },
		{ _PC_PATH_MAX,      4096, 2 },
		{ _PC_PIPE_BUF,      4096, 0 },
		{ _PC_NO_TRUNC,         0, 0 },
		{ _PC_CHOWN_RESTRICTED, 0, 0 }
	}
};

/*

C<long limit_sysconf(int limit)>

Returns system limits using I<sysconf(3)>. If the limit is indeterminate, a
predetermined default value is returned. Whatever happens, a usable value is
returned.

*/

static long limit_sysconf(int limit)
{
	long value;

	if ((value = sysconf(g.conf[limit].name)) == -1)
		return g.conf[limit].value;

	return value;
}

/*

C<long limit_pathconf(int limit, const char *path)>

Returns system limits using I<pathconf(3)>. If the limit is indeterminate, a
predetermined default value is returned. If the limit is determinate, a
predetermined amount may be added to its value. This is only needed for
C<_PC_PATH_MAX> which is the maximum length of a relative path. To be more
useful, 2 is added to this limit to account for the C<'/'> and C<'\0'> that
will be needed to form an absolute path. Whatever happens, a usable value is
returned.

*/

static long limit_pathconf(int limit, const char *path)
{
	long value;

	if ((value = pathconf(path, g.conf[limit].name)) == -1)
		return g.conf[limit].value;

	return value + g.conf[limit].offset;
}

/*

C<long limit_fpathconf(int limit, int fd)>

Returns system limits using I<fpathconf(3)>. If the limit is indeterminate,
a predetermined default value is returned. If the limit is determinate, a
predetermined amount may be added to its value. This is only needed for
C<_PC_PATH_MAX> which is the maximum length of a relative path. To be more
useful, 2 is added to this limit to account for the C<'/'> and C<'\0'> that
will be needed to form an absolute path. Whatever happens, a usable value is
returned.

*/

static long limit_fpathconf(int limit, int fd)
{
	long value;

	if ((value = fpathconf(fd, g.conf[limit].name)) == -1)
		return g.conf[limit].value;

	return value + g.conf[limit].offset;
}

/*

=item C<long limit_arg(void)>

Returns the maximum length of arguments to the I<exec(2)> family of
functions. If indeterminate, a usable value (131072) is returned.

=cut

*/

long limit_arg(void)
{
	return limit_sysconf(LIMIT_ARG);
}

/*

=item C<long limit_child(void)>

Returns the maximum number of simultaneous processes per user id. If
indeterminate, a usable value (1024) is returned.

=cut

*/

long limit_child(void)
{
	return limit_sysconf(LIMIT_CHILD);
}

/*

=item C<long limit_tick(void)>

Returns the number of clock ticks per second. If indeterminate (which makes
no sense), -1 is returned. This should never happen.

=cut

*/

long limit_tick(void)
{
	return limit_sysconf(LIMIT_TICK);
}

/*

=item C<long limit_group(void)>

Returns the maximum number of groups that a user may belong to. If
indeterminate, a usable value (32) is returned.

=cut

*/

long limit_group(void)
{
	return limit_sysconf(LIMIT_GROUP);
}

/*

=item C<long limit_open(void)>

Returns the maximum number of files that a process can have open at any
time. If indeterminate, a usable value (1024) is returned.

=cut

*/

long limit_open(void)
{
	return limit_sysconf(LIMIT_OPEN);
}

/*

=item C<long limit_stream(void)>

Returns the maximum number of streams that a process can have open at any
time. If indeterminate, a usable value (1024) is returned.

=cut

*/

long limit_stream(void)
{
	return limit_sysconf(LIMIT_STREAM);
}

/*

=item C<long limit_tzname(void)>

Returns the maximum number of bytes in a timezone name. If indeterminate, a
usable value (3) is returned.

=cut

*/

long limit_tzname(void)
{
	return limit_sysconf(LIMIT_TZNAME);
}

/*

=item C<long limit_job(void)>

Returns whether or not job control is supported.

=cut

*/

long limit_job(void)
{
	return limit_sysconf(LIMIT_JOB);
}

/*

=item C<long limit_save_ids(void)>

Returns whether or not a process has a saved set-user-id and a saved
set-group-id.

=cut

*/

long limit_save_ids(void)
{
	return limit_sysconf(LIMIT_SAVE_IDS);
}

/*

=item C<long limit_version(void)>

Returns the year and month the POSIX.1 standard was approved in the format
YYYYMML.

=cut

*/

long limit_version(void)
{
	return limit_sysconf(LIMIT_VERSION);
}

/*

=item C<long limit_pcanon(const char *path)>

Returns the maximum length of a formatted input line for the terminal
referred to by C<path>. If indeterminate, a usable value (255) is returned.

=cut

*/

long limit_pcanon(const char *path)
{
	return limit_pathconf(LIMIT_CANON, path);
}

/*

=item C<long limit_fcanon(int fd)>

Returns the maximum length of a formatted input line for the terminal
referred to by C<fd>. If indeterminate, a usable value (255) is returned.

=cut

*/

long limit_fcanon(int fd)
{
	return limit_fpathconf(LIMIT_CANON, fd);
}

/*

=item C<long limit_canon(void)>

Returns the maximum length of a formatted input line for the controlling
terminal (C</dev/tty>). If indeterminate, a usable value (255) is returned.

=cut

*/

long limit_canon(void)
{
	return limit_pcanon("/dev/tty");
}

/*

=item C<long limit_pinput(const char *path)>

Returns the maximum length of an input line for the terminal referred to by
C<path>. If indeterminate, a usable value (255) is returned.

=cut

*/

long limit_pinput(const char *path)
{
	return limit_pathconf(LIMIT_INPUT, path);
}

/*

=item C<long limit_finput(int fd)>

Returns the maximum length of an input line for the terminal referred to by
C<fd>. If indeterminate, a usable value (255) is returned.

=cut

*/

long limit_finput(int fd)
{
	return limit_fpathconf(LIMIT_INPUT, fd);
}

/*

=item C<long limit_input(void)>

Returns the maximum length of an input line for the controlling terminal
(C</dev/tty>). If indeterminate, a usable value (255) is returned.

=cut

*/

long limit_input(void)
{
	return limit_pinput("/dev/tty");
}

/*

=item C<long limit_pvdisable(const char *path)>

Returns whether or not special character processing can be disabled for the
terminal referred to by C<path>.

=cut

*/

long limit_pvdisable(const char *path)
{
	return limit_pathconf(LIMIT_VDISABLE, path);
}

/*

=item C<long limit_fvdisable(int fd)>

Returns whether or not special character processing can be disabled for the
terminal referred to by C<fd>.

=cut

*/

long limit_fvdisable(int fd)
{
	return limit_fpathconf(LIMIT_VDISABLE, fd);
}

/*

=item C<long limit_vdisable(void)>

Returns whether or not special character processing can be disabled for the
controlling terminal (C</dev/tty>).

=cut

*/

long limit_vdisable(void)
{
	return limit_pvdisable("/dev/tty");
}

/*

=item C<long limit_plink(const char *path)>

Returns the maximum number of links to the file represented by C<path>. If
indeterminate, a usable value (32768) is returned.

=cut

*/

long limit_plink(const char *path)
{
	return limit_pathconf(LIMIT_LINK, path);
}

/*

=item C<long limit_flink(int fd)>

Returns the maximum number of links to the file represented by C<fd>. If
indeterminate, a usable value (32768) is returned.

=cut

*/

long limit_flink(int fd)
{
	return limit_fpathconf(LIMIT_LINK, fd);
}

/*

=item C<long limit_link(void)>

Returns the maximum number of links to the root directory (C</>). If
indeterminate, a usable value (32768) is returned.

=cut

*/

long limit_link(void)
{
	return limit_plink("/");
}

/*

=item C<long limit_pname(const char *path)>

Returns the maximum length of a filename in the directory referred to by
C<path> that the process can create. If indeterminate, a usable value (1024)
is returned.

=cut

*/

long limit_pname(const char *path)
{
	return limit_pathconf(LIMIT_NAME, path);
}

/*

=item C<long limit_fname(int fd)>

Returns the maximum length of a filename in the directory referred to by
C<fd> that the process can create. If indeterminate, a usable value (1024)
is returned.

=cut

*/

long limit_fname(int fd)
{
	return limit_fpathconf(LIMIT_NAME, fd);
}

/*

=item C<long limit_name(void)>

Returns the maximum length of a filename in the root directory (C</>) that
the process can create. If indeterminate, a usable value (1024) is returned.

=cut

*/

long limit_name(void)
{
	return limit_pname("/");
}

/*

=item C<long limit_ppath(const char *path)>

Returns the maximum length of an absolute pathname (including the C<nul>
character) when C<path> is the current directory. If indeterminate, a usable
value (4096) is returned.

=cut

*/

long limit_ppath(const char *path)
{
	return limit_pathconf(LIMIT_PATH, path);
}

/*

=item C<long limit_fpath(int fd)>

Returns the maximum length of an absolute pathname (including the C<nul>
character) when C<fd> refers to the current directory. If indeterminate, a
usable value (4096) is returned.

=cut

*/

long limit_fpath(int fd)
{
	return limit_fpathconf(LIMIT_PATH, fd);
}

/*

=item C<long limit_path(void)>

Returns the maximum length of an absolute pathname (including the C<nul>
character). If indeterminate, a usable value (4096) is returned.

=cut

*/

long limit_path(void)
{
	return limit_ppath("/");
}

/*

=item C<long limit_ppipe(const char *path)>

Returns the size of the pipe buffer for the fifo referred to by C<path>. If
indeterminate, a usable value (4096) is returned.

=cut

*/

long limit_ppipe(const char *path)
{
	return limit_pathconf(LIMIT_PIPE, path);
}

/*

=item C<long limit_fpipe(int fd)>

Returns the size of the pipe buffer for the pipe or fifo referred to by
C<fd>. If indeterminate, a usable value (4096) is returned.

=cut

*/

long limit_fpipe(int fd)
{
	return limit_fpathconf(LIMIT_PIPE, fd);
}

/*

=item C<long limit_pnotrunc(const char *path)>

Returns whether or not an error is generated when accessing filenames longer
than the maximum filename length for the filesystem referred to by C<path>.

=cut

*/

long limit_pnotrunc(const char *path)
{
	return limit_pathconf(LIMIT_NOTRUNC, path);
}

/*

=item C<long limit_fnotrunc(int fd)>

Returns whether or not an error is generated when accessing filenames longer
than the maximum filename length for the filesystem referred to by C<fd>.

=cut

*/

long limit_fnotrunc(int fd)
{
	return limit_fpathconf(LIMIT_NOTRUNC, fd);
}

/*

=item C<long limit_notrunc(void)>

Returns whether or not an error is generated when accessing filenames longer
than the maximum filename length for the root filesystem.

=cut

*/

long limit_notrunc(void)
{
	return limit_pnotrunc("/");
}

/*

=item C<long limit_pchown(const char *path)>

Returns whether or not I<chown(2)> may be called on the file referred to by
C<path> or the files contained in the directory referred to by C<path>.

=cut

*/

long limit_pchown(const char *path)
{
	return limit_pathconf(LIMIT_CHOWN, path);
}

/*

=item C<long limit_fchown(int fd)>

Returns whether or not I<chown(2)> may be called on the file referred to by
C<fd> or the files contained in the directory referred to by C<fd>.

=cut

*/

long limit_fchown(int fd)
{
	return limit_fpathconf(LIMIT_CHOWN, fd);
}

/*

=item C<long limit_chown(void)>

Returns whether or not I<chown(2)> may be called on the files contained in
the root filesystem.

=cut

*/

long limit_chown(void)
{
	return limit_pchown("/");
}

/*

=back

=head1 RETURNS

The functions that return a condition return 1 when the condition is true or
C<0> when it is false. All of the others either return the system limit
indicated or a predetermined, usable value when the indicated limit is
indeterminate. These functions never return C<-1>.

=head1 MT-Level

MT-Safe

=head1 EXAMPLES

Store the current directory into allocated memory:

    #include <slack/std.h>
    #include <slack/lim.h>

    int main()
    {
        long path_size = limit_path();
        char *buf = malloc(path_size * sizeof(char));
        if (!buf)
            return EXIT_FAILURE;

        printf("%s\n", getcwd(buf, path_size));

        return EXIT_SUCCESS;
    }

Close all file descriptors:

    #include <slack/std.h>
    #include <slack/lim.h>

    int main()
    {
        int fd_limit = limit_open();
		int fd;

        for (fd = 0; fd < fd_limit; ++fd)
            close(fd);

        return EXIT_SUCCESS;
    }

=head1 SEE ALSO

I<libslack(3)>,
I<sysconf(2)>,
I<pathconf(2)>,
I<fpathconf(2)>,
I<locker(3)>

=head1 AUTHOR

20201111 raf <raf@raf.org>

=cut

*/

#endif

#ifdef TEST

int main(int ac, char **av)
{
	int fds[2];
	long limit;
	int errors = 0;
	int verbose = (ac >= 2 && !strcmp(av[1], "-v"));

	if (ac == 2 && !strcmp(av[1], "help"))
	{
		printf("usage: %s [-v]\n", *av);
		return EXIT_SUCCESS;
	}

	printf("Testing: %s\n", "lim");

	if ((limit = limit_arg()) == -1)
		++errors, printf("Test1: limit_arg() failed\n");
	else if (verbose)
		printf("arg = %ld\n", limit);

	if ((limit = limit_child()) == -1)
		++errors, printf("Test2: limit_child() failed\n");
	else if (verbose)
		printf("child = %ld\n", limit);

	if ((limit = limit_tick()) == -1)
		++errors, printf("Test3: limit_tick() failed\n");
	else if (verbose)
		printf("tick = %ld\n", limit);

	if ((limit = limit_group()) == -1)
		++errors, printf("Test4: limit_group() failed\n");
	else if (verbose)
		printf("group = %ld\n", limit);

	if ((limit = limit_open()) == -1)
		++errors, printf("Test5: limit_open() failed\n");
	else if (verbose)
		printf("open = %ld\n", limit);

	if ((limit = limit_stream()) == -1)
		++errors, printf("Test6: limit_stream() failed\n");
	else if (verbose)
		printf("stream = %ld\n", limit);

	if ((limit = limit_tzname()) == -1)
		++errors, printf("Test7: limit_tzname() failed\n");
	else if (verbose)
		printf("tzname = %ld\n", limit);

	if ((limit = limit_job()) == -1)
		++errors, printf("Test8: limit_job() failed\n");
	else if (verbose)
		printf("job = %ld\n", limit);

	if ((limit = limit_save_ids()) == -1)
		++errors, printf("Test9: limit_save_ids() failed\n");
	else if (verbose)
		printf("save_ids = %ld\n", limit);

	if ((limit = limit_version()) == -1)
		++errors, printf("Test10: limit_version() failed\n");
	else if (verbose)
		printf("version = %ld\n", limit);

	if ((limit = limit_canon()) == -1)
		++errors, printf("Test11: limit_canon() failed\n");
	else if (verbose)
		printf("canon = %ld\n", limit);

	if ((limit = limit_input()) == -1)
		++errors, printf("Test12: limit_input() failed\n");
	else if (verbose)
		printf("input = %ld\n", limit);

	if ((limit = limit_vdisable()) == -1)
		++errors, printf("Test13: limit_vdisable() failed\n");
	else if (verbose)
		printf("vdisable = %ld\n", limit);

	if ((limit = limit_link()) == -1)
		++errors, printf("Test14: limit_link() failed\n");
	else if (verbose)
		printf("link = %ld\n", limit);

	if ((limit = limit_name()) == -1)
		++errors, printf("Test15: limit_name() failed\n");
	else if (verbose)
		printf("name = %ld\n", limit);

	if ((limit = limit_path()) == -1)
		++errors, printf("Test16: limit_path() failed\n");
	else if (verbose)
		printf("path = %ld\n", limit);

	if (pipe(fds) == -1)
	{
		++errors, printf("Test17: failed to test limit_fpipe() - pipe() failed (%s)\n", strerror(errno));
	}
	else
	{
		if ((limit = limit_fpipe(fds[0])) == -1)
			++errors, printf("Test17: limit_fpipe() failed\n");
		else if (verbose)
			printf("pipe = %ld\n", limit);

		close(fds[0]);
		close(fds[1]);
	}

	if ((limit = limit_notrunc()) == -1)
		++errors, printf("Test18: limit_notrunc() failed\n");
	else if (verbose)
		printf("notrunc = %ld\n", limit);

	if ((limit = limit_chown()) == -1)
		++errors, printf("Test19: limit_chown() failed\n");
	else if (verbose)
		printf("chown = %ld\n", limit);

	if (errors)
		printf("%d/19 tests failed\n", errors);
	else
		printf("All tests passed\n");

	return (errors == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

#endif

/* vi:set ts=4 sw=4: */
