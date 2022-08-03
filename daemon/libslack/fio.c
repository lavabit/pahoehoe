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

=head1 NAME

I<libslack(fio)> - fifo and file control module and some I/O

=head1 SYNOPSIS

    #include <slack/std.h>
    #include <slack/fio.h>

    char *fgetline(char *line, size_t size, FILE *stream);
    char *fgetline_unlocked(char *line, size_t size, FILE *stream);
    int read_timeout(int fd, long sec, long usec);
    int write_timeout(int fd, long sec, long usec);
    int rw_timeout(int fd, long sec, long usec);
    int nap(long sec, long usec);
    int fcntl_set_flag(int fd, int flag);
    int fcntl_clear_flag(int fd, int flag);
    int fcntl_set_fdflag(int fd, int flag);
    int fcntl_clear_fdflag(int fd, int flag);
    int fcntl_lock(int fd, int cmd, int type, int whence, int start, int len);
    int nonblock_set(int fd, int arg);
    int nonblock_on(int fd);
    int nonblock_off(int fd);
    int fifo_exists(const char *path, int prepare);
    int fifo_has_reader(const char *path, int prepare);
    int fifo_open(const char *path, mode_t mode, int lock, int *writefd);

=head1 DESCRIPTION

This module provides various I/O related functions: reading a line of text
no matter what line endings are used; timeouts for read/write operations
without signals; exclusively opening a fifo for reading; and some random
shorthand functions for manipulating file flags and locks.

=over 4

=cut

*/

#include "config.h"

#ifndef NO_POSIX_SOURCE
#define NO_POSIX_SOURCE /* For ETIMEDOUT, EADDRINUSE, EOPNOTSUPP on FreeBSD-8.0 */
#endif

#include "std.h"

#include <fcntl.h>

#include <sys/types.h>
#if HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#include <sys/time.h>
#include <sys/stat.h>

#include "err.h"
#include "fio.h"

#ifndef TEST

void (flockfile)(FILE *stream); /* Missing from old glibc headers */
void (funlockfile)(FILE *stream);

#ifndef HAVE_FLOCKFILE
#define flockfile(stream)
#define funlockfile(stream)
#define getc_unlocked(stream) getc(stream)
#endif

/*

=item C<char *fgetline(char *line, size_t size, FILE *stream)>

Similar to I<fgets(3)> except that it recognises UNIX (C<"\n">), DOS/Windows
(C<"\r\n">) and old Macintosh (C<"\r">) line endings (even different line
endings in the same file). Reads bytes from C<stream> and stores them in the
buffer pointed to by C<line>. Reading stops after an C<EOF>, or the end of
the line is reached, or when C<size - 1> bytes have been stored. If the end
of the line was reached, it is stored as a C<"\n"> byte. A C<nul> byte is
stored after the last byte in the buffer. On success, returns C<line>. On
error, or when the end of file occurs while no bytes have been read, returns
C<null>. Note that even when C<null> is returned, C<line> is modified and
will always be C<nul>-terminated. So it is safe to examine C<line> even when
this function returns C<null>. Calls to this function can be mixed with
calls to other input functions from the I<stdio> library for the same input
stream. This is a drop-in replacement for I<fgets(3)>.

    char line[BUFSIZ];
    while (fgetline(line, BUFSIZ, stdin))
        printf("%s", line);

=cut

*/

char *fgetline(char *line, size_t size, FILE *stream)
{
	char *ret;

	flockfile(stream);
	ret = fgetline_unlocked(line, size, stream);
	funlockfile(stream);

	return ret;
}

/*

=item C<char *fgetline_unlocked(char *line, size_t size, FILE *stream)>

Equivalent to I<fgetline(3)> except that C<stream> is not locked.

=cut

*/

char *fgetline_unlocked(char *line, size_t size, FILE *stream)
{
	char *s = line;
	char *end = line + size - 1;
	int c = '\0', c2;

	if (!s)
		return NULL;

	while (s < end && (c = getc_unlocked(stream)) != EOF)
	{
		if (c == '\n')
		{
			*s++ = c;
			break;
		}
		else if (c == '\r')
		{
			*s++ = '\n';

			if ((c2 = getc_unlocked(stream)) == '\n')
				break;

			ungetc(c2, stream);
			break;
		}
		else
			*s++ = c;
	}

	*s = '\0';

	if (c == EOF && (s == line || ferror(stream)))
		return NULL;

	return line;
}

/*

=item C<int read_timeout(int fd, long sec, long usec)>

Performs a I<select(2)> on a single file descriptor, C<fd>, for reading and
exceptions (i.e. arrival of urgent data), that times out after C<sec>
seconds and C<usec> microseconds. This is just a shorthand function to
provide a simple timed I<read(2)> (or I<readv(2)> or I<accept(2)> or
I<recv(2)> or I<recvfrom(2)> or I<recvmsg(2)> without resorting to
I<alarm(3)> and C<SIGALRM> signals (best avoided). On success, returns C<0>.
On error, returns C<-1> with C<errno> set appropriately (C<ETIMEDOUT> if it
timed out, otherwise set by I<select(2)>). Usage:

    if (read_timeout(fd, 5, 0) == -1 || (bytes = read(fd, buf, count)) == -1)
        return -1;

=cut

*/

int read_timeout(int fd, long sec, long usec)
{
	fd_set readfds[1];
	fd_set exceptfds[1];
	struct timeval timeout[1];

	if (fd < 0 || sec < 0 || usec < 0)
		return set_errno(EINVAL);

	FD_ZERO(readfds);
	FD_SET(fd, readfds);
	*exceptfds = *readfds;
	timeout->tv_sec = sec;
	timeout->tv_usec = usec;

	switch (select(fd + 1, readfds, NULL, exceptfds, timeout))
	{
		case -1:
			return -1;
		case 0:
			return set_errno(ETIMEDOUT);
	}

	return 0;
}

/*

=item C<int write_timeout(int fd, long sec, long usec)>

Performs a I<select(2)> on a single file descriptor, C<fd>, for writing,
that times out after C<sec> seconds and C<usec> microseconds. This is just a
shorthand function to provide a simple timed I<write(2)> (or I<writev(2)> or
I<send(2)> or I<sendto(2)> or I<sendmsg(2)>) without resorting to
I<alarm(3)> and C<SIGALRM> signals (best avoided). On success, returns C<0>.
On error, returns C<-1> with C<errno> set appropriately (C<ETIMEDOUT> if it
timed out, otherwise set by I<select(2)>). Usage:

    if (write_timeout(fd, 5, 0) == -1 || (bytes = write(fd, buf, count)) == -1)
        return -1;

=cut

*/

int write_timeout(int fd, long sec, long usec)
{
	fd_set writefds[1];
	struct timeval timeout[1];

	if (fd < 0 || sec < 0 || usec < 0)
		return set_errno(EINVAL);

	FD_ZERO(writefds);
	FD_SET(fd, writefds);
	timeout->tv_sec = sec;
	timeout->tv_usec = usec;

	switch (select(fd + 1, NULL, writefds, NULL, timeout))
	{
		case -1:
			return -1;
		case 0:
			return set_errno(ETIMEDOUT);
	}

	return 0;
}

/*

=item C<int rw_timeout(int fd, long sec, long usec)>

Performs a I<select(2)> on a single file descriptor, C<fd>, for reading,
writing and exceptions (i.e. arrival of urgent data), that times out after
C<sec> seconds and C<usec> microseconds. This is just a shorthand function
to provide a simple timed I<read(2)> or I<write(2)> without resorting to
I<alarm(3)> and C<SIGALRM> signals (best avoided). On success, returns a bit
mask indicating whether C<fd> is readable (C<R_OK>), writable (C<W_OK>)
and/or has urgent data available (C<X_OK>). On error, returns C<-1> with
C<errno> set appropriately (C<ETIMEDOUT> if it timed out, otherwise set by
I<select(2)>).

    if ((mask = rw_timeout(fd, 5, 0)) == -1)
        return -1;

    if ((mask & W_OK) && (bytes = write(fd, buf, count)) == -1)
        return -1;

    if ((mask & R_OK) && (bytes = read(fd, buf, count)) == -1)
        return -1;

=cut

*/

int rw_timeout(int fd, long sec, long usec)
{
	fd_set readfds[1];
	fd_set writefds[1];
	fd_set exceptfds[1];
	struct timeval timeout[1];
	int rc = 0;

	if (fd < 0 || sec < 0 || usec < 0)
		return set_errno(EINVAL);

	FD_ZERO(readfds);
	FD_SET(fd, readfds);
	*writefds = *readfds;
	*exceptfds = *readfds;
	timeout->tv_sec = sec;
	timeout->tv_usec = usec;

	switch (select(fd + 1, readfds, writefds, exceptfds, timeout))
	{
		case -1:
			return -1;
		case 0:
			return set_errno(ETIMEDOUT);
	}

	if (FD_ISSET(fd, readfds))
		rc |= R_OK;

	if (FD_ISSET(fd, writefds))
		rc |= W_OK;

	if (FD_ISSET(fd, exceptfds))
		rc |= X_OK;

	return rc;
}

/*

=item C<int nap(long sec, long usec)>

Puts the process to sleep for C<sec> seconds and C<usec> microseconds. Note,
however, that many systems' timers only have 10ms resolution. This uses
I<select(3)> to ensure that I<alarm(3)> and C<SIGALRM> signals are not used
(best avoided). On success, returns C<0>. On error, returns C<-1> with
C<errno> set appropriately.

    nap(1, 500000); // Sleep for 1.5 seconds
    nap(0, 100000); // Sleep for 0.1 seconds

=cut

*/

int nap(long sec, long usec)
{
	struct timeval tv[1];

	if (sec < 0 || usec < 0)
		return set_errno(EINVAL);

	tv->tv_sec = sec;
	tv->tv_usec = usec;

	return select(0, NULL, NULL, NULL, tv);
}

/*

=item C<int fcntl_set_flag(int fd, int flag)>

Shorthand for setting the file status flag, C<flag>, on the file descriptor,
C<fd>, using I<fcntl(2)>. All other file status flags are unaffected. On
success, returns C<0>. On error, returns C<-1> with C<errno> set by
I<fcntl(2)> with C<F_GETFL> or C<F_SETFL> as the command. Example file
status flags are C<O_APPEND>, C<O_ASYNC>, C<O_DIRECT>, C<O_NOATIME>, and
C<O_NONBLOCK> depending on the system.

=cut

*/

int fcntl_set_flag(int fd, int flag)
{
	int flags;

	if ((flags = fcntl(fd, F_GETFL, 0)) == -1)
		return -1;

	return fcntl(fd, F_SETFL, flags | flag);
}

/*

=item C<int fcntl_clear_flag(int fd, int flag)>

Shorthand for clearing the file status flag, C<flag>, from the file
descriptor, C<fd>, using I<fcntl(2)>. All other file status flags are
unaffected. On success, returns C<0>. On error, returns C<-1> with C<errno>
set by I<fcntl(2)> with C<F_GETFL> or C<F_SETFL> as the command. Example
file status flags are C<O_APPEND>, C<O_ASYNC>, C<O_DIRECT>, C<O_NOATIME>,
and C<O_NONBLOCK> depending on the system.

=cut

*/

int fcntl_clear_flag(int fd, int flag)
{
	int flags;

	if ((flags = fcntl(fd, F_GETFL, 0)) == -1)
		return -1;

	return fcntl(fd, F_SETFL, flags & ~flag);
}

/*

=item C<int fcntl_set_fdflag(int fd, int flag)>

Shorthand for setting the file descriptor flag, C<flag>, on the file
descriptor, C<fd>, using I<fcntl(2)>. All other file descriptor flags are
unaffected. On success, returns C<0>. On error, returns C<-1> with C<errno>
set by I<fcntl(2)> with C<F_GETFD> or C<F_SETFD> as the command. The only
file descriptor flag at time of writing is C<FD_CLOEXEC>.

=cut

*/

int fcntl_set_fdflag(int fd, int flag)
{
	int flags;

	if ((flags = fcntl(fd, F_GETFD, 0)) == -1)
		return -1;

	return fcntl(fd, F_SETFD, flags | flag);
}

/*

=item C<int fcntl_clear_fdflag(int fd, int flag)>

Shorthand for clearing the file descriptor flag, C<flag>, from the file
descriptor, C<fd>, using I<fcntl(2)>. All other file descriptor flags are
unaffected. On success, returns C<0>. On error, returns C<-1> with C<errno>
set by I<fcntl(2)> with C<F_GETFD> or C<F_SETFD> as the command. The only
file descriptor flag at time of writing is C<FD_CLOEXEC>.

=cut

*/

int fcntl_clear_fdflag(int fd, int flag)
{
	int flags;

	if ((flags = fcntl(fd, F_GETFL, 0)) == -1)
		return -1;

	return fcntl(fd, F_SETFL, flags & ~flag);
}

/*

=item C<int fcntl_lock(int fd, int cmd, int type, int whence, int start, int len)>

Shorthand for performing discretionary file locking operations on the file
descriptor, C<fd>. C<cmd> is the locking command and is passed to
I<fcntl(2)>. C<type>, C<whence>, C<start> and C<len> are used to fill a
I<flock> structure which is passed to I<fcntl(2)>. Returns the same as
I<fcntl(2)> with C<cmd> as the command.

    if (fcntl_lock(fd, F_SETLK, F_WRLCK, SEEK_SET, 0, 0) == -1)
        return -1;

=cut

*/

int fcntl_lock(int fd, int cmd, int type, int whence, int start, int len)
{
	struct flock lock[1];

	lock->l_type = type;
	lock->l_whence = whence;
	lock->l_start = start;
	lock->l_len = len;

	return fcntl(fd, cmd, lock);
}

/*

=item C<int nonblock_set(int fd, int arg)>

Sets non-blocking mode for the file descriptor, C<fd>, if C<arg> is
non-zero. Sets blocking mode if C<arg> is zero. On success, returns C<0>. On
error, returns C<-1> with C<errno> set by I<fcntl(2)> with C<F_GETFL> or
C<F_SETFL> as the command.

=cut

*/

int nonblock_set(int fd, int arg)
{
	return (arg) ? nonblock_on(fd) : nonblock_off(fd);
}

/*

=item C<int nonblock_on(int fd)>

Sets non-blocking mode for the file descriptor, C<fd>. On success, returns
C<0>. On error, returns C<-1> with C<errno> set by I<fcntl(2)> with
C<F_GETFL> or C<F_SETFL> as the command.

=cut

*/

int nonblock_on(int fd)
{
	return fcntl_set_flag(fd, O_NONBLOCK);
}

/*

=item C<int nonblock_off(int fd)>

Sets blocking mode for the file descriptor, C<fd>. On success, returns C<0>.
On error, returns C<-1> with C<errno> set by I<fcntl(2)> with C<F_GETFL> or
C<F_SETFL> as the command.

=cut

*/

int nonblock_off(int fd)
{
	return fcntl_clear_flag(fd, O_NONBLOCK);
}

/*

=item C<int fifo_exists(const char *path, int prepare)>

Determines whether or not C<path> refers to a fifo. Returns C<0> if C<path>
doesn't exist or doesn't refer to a fifo. If C<path> refers to a fifo,
returns 1. If C<prepare> is non-zero, and C<path> refers to a non-fifo, it
will be unlinked. On error, returns C<-1> with C<errno> set by I<stat(2)>.

=cut

*/

int fifo_exists(const char *path, int prepare)
{
	struct stat status[1];

	if (stat(path, status) == -1)
		return (errno == ENOENT) ? 0 : -1;

	if (S_ISFIFO(status->st_mode) == 0)
	{
		if (prepare)
			unlink(path);

		return 0;
	}

	return 1;
}

/*

=item C<int fifo_has_reader(const char *path, int prepare)>

Determines whether or not C<path> refers to a fifo that is being read by
another process. If C<path> does not exist, or does not refer to a fifo, or
if the fifo can't be opened for non-blocking I<write(2)>, returns C<0>. If
C<prepare> is non-zero, and path refers to a non-fifo, it will be unlinked.
On error, returns C<-1> with C<errno> set by I<stat(2)> or I<open(2)>.

=cut

*/

int fifo_has_reader(const char *path, int prepare)
{
	int fd;

	/*
	** Check that fifo exists and is a fifo.
	** If not, there can be no reader process.
	*/

	switch (fifo_exists(path, prepare))
	{
		case  0: return 0;
		case -1: return -1;
	}

	/*
	** Open the fifo for non-blocking write.
	** If there is no reader process, open()
	** will fail with errno == ENXIO.
	*/

	if ((fd = open(path, O_WRONLY | O_NONBLOCK)) == -1)
		return (errno == ENXIO) ? 0 : -1;

	if (close(fd) == -1)
		return -1;

	return 1;
}

/*

=item C<int fifo_open(const char *path, mode_t mode, int lock, int *writefd)>

Creates a fifo named C<path> with creation mode C<mode> for reading. If
C<path> already exists, is a fifo, and has a reader process, returns C<-1>
with C<errno> set to C<EADDRINUSE>. If the fifo is created (or an existing
one can be reused), two file descriptors are opened to the fifo. A read
descriptor and a write descriptor. On success, returns the read descriptor.
The write descriptor only exists to ensure that there is always at least one
writer process for the fifo. This allows a I<read(2)> on the read descriptor
to block until another process writes to the fifo rather than returning an
C<EOF> condition. This is done in a I<POSIX>-compliant way. If C<lock> is
non-zero, the fifo is exclusively locked. If C<writefd> is not C<null>, the
write descriptor is stored there. On error, returns C<-1> with C<errno> set
by I<stat(2)>, I<open(2)>, I<mkfifo(2)>, I<fstat(2)> or I<fcntl(2)>.

    char *fifopath = "/tmp/fifo";
    int fd, wfd;

    if ((fd = fifo_open(fifopath, S_IRUSR | S_IWUSR | S_IWGRP | S_IWOTH, 1, &wfd)) == -1)
        return -1;

    // Read from fd...

    close(fd);
    close(wfd);
    unlink(fifopath);

=cut

*/

int fifo_open(const char *path, mode_t mode, int lock, int *writefd)
{
	struct stat status[1];
	int rfd, wfd, mine = 0;

	/* Don't open the fifo for reading twice. */

	switch (fifo_has_reader(path, 1))
	{
		case  1: return set_errno(EADDRINUSE);
		case -1: return -1;
	}

	/* Create the fifo. */

	if (mkfifo(path, mode) != -1)
		mine = 1;
	else if (errno != EEXIST)
		return -1;

	/*
	** Open the fifo for non-blocking read only.
	** This prevents blocking while waiting for a
	** writer process. We are about to supply our
	** own writer.
	*/

	if ((rfd = open(path, O_RDONLY | O_NONBLOCK)) == -1)
	{
		if (mine)
			unlink(path);
		return -1;
	}

	/*
	** A sanity check to make sure that what we have just
	** opened is really a fifo. Someone may have just replaced
	** the fifo with a file between fifo_has_reader and here.
	*/

	if (fstat(rfd, status) == -1 || S_ISFIFO(status->st_mode) == 0)
	{
		if (mine)
			unlink(path);
		close(rfd);
		return -1;
	}

	/*
	** Open the fifo for write only and leave this fd open.
	** This guarantees that there is always at least one
	** writer process. This prevents EOF indications being
	** returned from read() when there are no other writer
	** processes.
	**
	** Just opening the fifo "rw" should work but it's undefined
	** by POSIX.
	*/

	if ((wfd = open(path, O_WRONLY)) == -1)
	{
		if (mine)
			unlink(path);
		close(rfd);
		return -1;
	}

	/*
	** Exclusively lock the fifo to prevent two invocations
	** deciding that there's no reader and opening this fifo
	** at the same time.
	**
	** Note: some systems (e.g. FreeBSD, Mac OS X) can't lock fifos :(
	*/

/* On MacOSX-10.6 these are different numbers */
#ifndef ENOTSUP
#define ENOTSUP EOPNOTSUPP
#endif
	if (lock && fcntl_lock(wfd, F_SETLK, F_WRLCK, SEEK_SET, 0, 0) == -1 && errno != EOPNOTSUPP && errno != ENOTSUP && errno != EBADF)
	{
		if (mine)
			unlink(path);
		close(rfd);
		close(wfd);
		return (errno == EACCES) ? set_errno(EADDRINUSE) : -1;
	}

	/* A sanity test on the write descriptor we have just opened and locked. */

	if (fstat(wfd, status) == -1 || S_ISFIFO(status->st_mode) == 0)
	{
		if (mine)
			unlink(path);
		close(rfd);
		close(wfd);
		return -1;
	}

	/* Now put the reader into blocking mode. */

	if (nonblock_off(rfd) == -1)
	{
		if (mine)
			unlink(path);
		close(rfd);
		close(wfd);
		return -1;
	}

	/*
	** Flaw: If someone unceremoniously unlinks our fifo, we won't know
	** about it and nothing will stop another invocation from creating a new
	** fifo and handling it. This process would sleep forever in select().
	*/

	if (writefd)
		*writefd = wfd;

	return rfd;
}

/*

=back

=head1 ERRORS

These functions set C<errno> to the following values. C<errno> may also be
set by the underlying system calls. See their manpages for details.

=over 4

=item C<ETIMEDOUT>

The I<read_timeout(3)>, I<write_timeout(3)> and I<rw_timeout(3)> functions
set this when a timeout occurs.

=item C<EADDRINUSE>

I<fifo_open(3)> sets this when the path refers to a fifo that already has
another process reading from it.

=back

=head1 MT-Level

I<MT-Safe>

I<Mac OS X> doesn't have I<flockfile(3)>, I<funlockfile(3)> or
I<getc_unlocked(3)> so I<fgetline(3)> is not I<MT-Safe> on such platforms.
You must guard all I<stdio> calls in multi-threaded programs with explicit
synchronisation variables.

=head1 EXAMPLES

A paranoid I<fgetline()> example:

    #include <slack/std.h>
    #include <slack/fio.h>

    int main()
    {
        char line[BUFSIZ];

        while (fgetline(line, BUFSIZ, stdin))
            printf("%s", line);

        if (ferror(stdin))
        {
            if (!*line)
                printf("%s\n", line);

            return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
    }

Read from stdin but give up after 5 seconds:

    #include <slack/std.h>
    #include <slack/fio.h>

    int main()
    {
        char buf[BUFSIZ];
        ssize_t bytes;

        if (read_timeout(STDIN_FILENO, 5, 0) == -1 ||
            (bytes = read(STDIN_FILENO, buf, BUFSIZ)) == -1 ||
            write(STDOUT_FILENO, buf, bytes) != bytes)
            return EXIT_FAILURE;

        return EXIT_SUCCESS;
    }

A command line sub-second sleep command:

    #include <slack/std.h>
    #include <slack/fio.h>

    int main(int ac, char **av)
    {
        if (ac != 3)
            return EXIT_FAILURE;

        nap(atoi(av[1]), atoi(av[2]));

        return EXIT_SUCCESS;
    }

Setting file flags:

    #include <slack/std.h>
    #include <slack/fio.h>

    int main()
    {
        if (fcntl_set_flag(STDIN_FILENO, O_NONBLOCK | O_ASYNC) == -1)
            return EXIT_FAILURE;

        if (fcntl_set_flag(STDOUT_FILENO, O_APPEND) == -1)
            return EXIT_FAILURE;

        if (fcntl_set_fdflag(STDOUT_FILENO, FD_CLOEXEC) == -1)
            return EXIT_FAILURE;

        if (nonblock_on(STDOUT_FILENO) == -1)
            return EXIT_FAILURE;

        if (fcntl_clear_flag(STDIN_FILENO, O_NONBLOCK | O_ASYNC) == -1)
            return EXIT_FAILURE;

        if (fcntl_clear_flag(STDOUT_FILENO, O_APPEND) == -1)
            return EXIT_FAILURE;

        if (fcntl_clear_fdflag(STDOUT_FILENO, FD_CLOEXEC) == -1)
            return EXIT_FAILURE;

        if (nonblock_off(STDOUT_FILENO) == -1)
            return EXIT_FAILURE;

        return EXIT_SUCCESS;
    }

File locking:

    #include <slack/std.h>
    #include <slack/fio.h>

    int main(int ac, char **av)
    {
        int fd;

        if ((fd = open(av[1], O_RDWR)) == -1)
            return EXIT_FAILURE;

        if (fcntl_lock(fd, F_SETLK, F_WRLCK, SEEK_SET, 0, 0) == -1)
            return close(fd), EXIT_FAILURE;

        // Write to the file...

        if (fcntl_lock(fd, F_SETLK, F_UNLCK, SEEK_SET, 0, 0) == -1)
            return close(fd), EXIT_FAILURE;

        close(fd);

        return EXIT_SUCCESS;
    }

Turn a logfile into a fifo that sends log messages to syslog instead:

	#include <slack/std.h>
	#include <slack/fio.h>
	#include <syslog.h>

	int main()
	{
		char *fifopath = "/tmp/log2syslog";
		char buf[BUFSIZ];
		ssize_t bytes;
		int fd, wfd;

		if ((fd = fifo_open(fifopath, S_IRUSR | S_IWUSR | S_IWGRP | S_IWOTH, 1, &wfd)) == -1)
			return EXIT_FAILURE;

		while ((bytes = read(fd, buf, BUFSIZ)) > 0)
		{
			buf[bytes] = '\0';
			syslog(LOG_DAEMON | LOG_ERR, "%s", buf);
		}

		close(fd);
		close(wfd);
		unlink(fifopath);
	}

=head1 BUGS

Some systems, such as I<Mac OS X>, can't lock fifos. On these systems,
I<fifo_open(3)> ignores the locking failure and returns successfully. This
means that there is no guarantee of a unique reader process on these
systems. You will need to lock an ordinary file yourself to provide this
guarantee.

=head1 SEE ALSO

I<libslack(3)>,
I<fcntl(2)>,
I<stat(2)>,
I<fstat(2)>,
I<open(2)>,
I<write(2)>,
I<read(2)>,
I<mkfifo(2)>

=head1 AUTHOR

20210220 raf <raf@raf.org>

=cut

*/

#endif

#ifdef TEST

#include <slack/fio.h>

int main(int ac, char **av)
{
	const char * const fifoname = "./fio.fifo";
	const char * const filename = "./fio.file";
	const mode_t mode = S_IRUSR | S_IWUSR | S_IWGRP | S_IWOTH;
	FILE *file;
	char line[BUFSIZ];
	const int lock = 1;
	int errors = 0;
	int fd, wfd;

	if (ac == 2 && !strcmp(av[1], "help"))
	{
		printf("usage: %s\n", *av);
		return EXIT_SUCCESS;
	}

	printf("Testing: %s\n", "fio");

	umask(0);

	if ((fd = fifo_open(fifoname, mode, lock, &wfd)) == -1)
	{
		++errors, printf("Test1: fifo_open(\"%s\", %d, %d) failed (%s)\n", fifoname, (int)mode, lock, strerror(errno));
#ifndef HAVE_FCNTL_THAT_CAN_LOCK_FIFOS
		printf("\n      Can your system lock fifos?\n\n");
#endif
	}
	else
	{
		if ((fcntl_lock(fd, F_SETLK, F_WRLCK, SEEK_SET, 0, 0)) != -1)
			++errors, printf("Test2: fcntl_lock(wrlock) failed\n");

		/* Should really test that the following non-blocking changes do occur */

		if (nonblock_on(fd) == -1)
			++errors, printf("Test3: nonblock_on() failed (%s)\n", strerror(errno));

		if (nonblock_off(fd) == -1)
			++errors, printf("Test4: nonblock_off() failed (%s)\n", strerror(errno));

		if (fcntl_set_flag(fd, O_NONBLOCK) == -1)
			++errors, printf("Test5: fcntl_set_flag() failed (%s)\n", strerror(errno));

		if (fcntl_clear_flag(fd, O_NONBLOCK) == -1)
			++errors, printf("Test6: fcntl_clear_flag() failed (%s)\n", strerror(errno));

		if (fcntl_set_fdflag(fd, FD_CLOEXEC) == -1)
			++errors, printf("Test7: fcntl_set_fdflag() failed (%s)\n", strerror(errno));

		if (fcntl_clear_fdflag(fd, FD_CLOEXEC) == -1)
			++errors, printf("Test8: fcntl_clear_fdflag() failed (%s)\n", strerror(errno));

		close(fd);
		close(wfd);
		unlink(fifoname);
	}

#define CHECK_FGETLINE(i, size, expected) \
	if ((expected) && !fgetline(line, (size), file)) \
		++errors, printf("Test%d: fgetline() failed\n", (i)); \
	else if ((expected) && strcmp(line, ((expected) ? (expected) : ""))) \
		++errors, printf("Test%d: fgetline() read \"%s\", not \"%s\"\n", (i), line, (expected ? expected : "(null)"));

#define TEST_FGETLINE(i, buf, size, contents, line1, line2, line3) \
	if (!(file = fopen(filename, "wb"))) \
		++errors, printf("Test%d: failed to run test: failed to create test file\n", (i)); \
	else \
	{ \
		if (fwrite((contents), 1, strlen(contents), file) != strlen(contents)) \
			++errors, printf("Test%d: failed to run test: failed to write to test file\n", (i)); \
		else \
		{ \
			fclose(file); \
			if (!(file = fopen(filename, "r"))) \
				++errors, printf("Test%d: failed to run test: failed to open test file for reading\n", (i)); \
			else \
			{ \
				CHECK_FGETLINE((i), (size), (line1)) \
				CHECK_FGETLINE((i), (size), (line2)) \
				CHECK_FGETLINE((i), (size), (line3)) \
				if (fgetline(buf, BUFSIZ, file)) \
					++errors, printf("Test%d: fgetline() failed to return NULL at end of file\n", (i)); \
			} \
		} \
		fclose(file); \
		unlink(filename); \
	}

	TEST_FGETLINE(9, line, BUFSIZ, "abc\ndef\r\nghi\r", "abc\n", "def\n", "ghi\n")
	TEST_FGETLINE(10, line, BUFSIZ, "abc\rdef\nghi\r\n", "abc\n", "def\n", "ghi\n")
	TEST_FGETLINE(11, line, BUFSIZ, "abc\r\ndef\rghi\n", "abc\n", "def\n", "ghi\n")
	TEST_FGETLINE(12, line, BUFSIZ, "abc\ndef\rghi", "abc\n", "def\n", "ghi")
	TEST_FGETLINE(13, line, BUFSIZ, "", (char *)NULL, (char *)NULL, (char *)NULL)
	TEST_FGETLINE(14, line, 5, "abc", "abc", (char *)NULL, (char *)NULL)
	TEST_FGETLINE(15, line, 5, "abc\n", "abc\n", (char *)NULL, (char *)NULL)
	TEST_FGETLINE(16, line, 5, "abc\r\n", "abc\n", (char *)NULL, (char *)NULL)
	TEST_FGETLINE(17, line, 5, "abc\r", "abc\n", (char *)NULL, (char *)NULL)
	TEST_FGETLINE(18, line, 3, "abc\r", "ab", "c\n", (char *)NULL)
	TEST_FGETLINE(19, NULL, 0, "abc\r", (char *)NULL, (char *)NULL, (char *)NULL)

	/* Test read_timeout() and write_timeout() */

	if ((fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC | O_NONBLOCK, S_IRUSR | S_IWUSR)) == -1)
		++errors, printf("Test20: failed to create %s (%s)\n", filename, strerror(errno));
	else
	{
		char buf[12] = "0123456789\n";

		if (write_timeout(fd, 1, 0) == -1)
			++errors, printf("Test21: write_timeout(fd, 1, 0) failed (%s)\n", strerror(errno));
		else if (write(fd, buf, 11) != 11)
			++errors, printf("Test22: write(fd, \"0123456789\\n\", 11) failed (%s)\n", strerror(errno));
		else
		{
			close(fd);

			if ((fd = open(filename, O_RDONLY | O_NONBLOCK)) == -1)
				++errors, printf("Test23: failed to open %s for reading (%s)\n", filename, strerror(errno));
			else if (read_timeout(fd, 1, 0) == -1)
				++errors, printf("Test24: read_timeout(fd, 1, 0) failed (%s)\n", strerror(errno));
			else if (read(fd, buf, 11) != 11)
				++errors, printf("Test25: read(fd) failed (%s)\n", strerror(errno));
		}

		close(fd);
	}

	unlink(filename);

	/* Test error handling */

#define TEST_ERR(i, func) \
	if ((func) != -1) \
		++errors, printf("Test%d: %s failed to return -1\n", (i), (#func)); \
	else if (errno != EINVAL) \
		++errors, printf("Test%d: %s failed (errno = %s, not %s)\n", (i), (#func), strerror(errno), strerror(EINVAL));

	TEST_ERR(26, read_timeout(-1, 0, 0))
	TEST_ERR(27, read_timeout(0, -1, 0))
	TEST_ERR(28, read_timeout(0, 0, -1))
	TEST_ERR(29, write_timeout(-1, 0, 0))
	TEST_ERR(30, write_timeout(0, -1, 0))
	TEST_ERR(31, write_timeout(0, 0, -1))
	TEST_ERR(32, rw_timeout(-1, 0, 0))
	TEST_ERR(33, rw_timeout(0, -1, 0))
	TEST_ERR(34, rw_timeout(0, 0, -1))
	TEST_ERR(35, nap(-1, 0))
	TEST_ERR(36, nap(0, -1))

	if (errors)
		printf("%d/36 tests failed\n", errors);
	else
		printf("All tests passed\n");

#ifndef HAVE_FCNTL_THAT_CAN_LOCK_FIFOS
	printf("\n");
	printf("    Note: Some systems (e.g. FreeBSD) can't lock fifos so fifo_open()\n");
	printf("    can't guarantee a unique reader.\n");
#endif

	return (errors == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

#endif

/* vi:set ts=4 sw=4: */
