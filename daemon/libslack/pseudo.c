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
 * Author: Tatu Ylonen <ylo@cs.hut.fi>
 * Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
 *                    All rights reserved
 * Allocating a pseudo-terminal, and making it the controlling tty.
 *
 * As far as I am concerned, the code I have written for this software
 * can be used freely for any purpose.  Any derived versions of this
 * software must be clearly marked as such, and if the derived work is
 * incompatible with the protocol description in the RFC file, it must be
 * called by a name other than "ssh" or "Secure Shell".
 */

/*
 * 20010930 raf <raf@raf.org>
 * Librarified the code (no calls to fatal(), return errors instead)
 * Made pty device name truncation an error (rather than ignoring it)
 * Made it thread safe on some systems (use ttyname_r/ptsname_r)
 * Added a manpage (perldoc -F pseudo.c)
 * Made pty_allocate() more like openpty() but safe (called it pty_open())
 * Added safe version of forkpty() (called it pty_fork())
 *
 * 20100612 raf <raf@raf.org>
 * pty_open: pty_user_fd often has ECHO on by default these days so turn it off
 *
 * 20201111 raf <raf@raf.org>
 * pty_make_controlling_tty(): Removed vhangup() which caused problems when
 * it succeeded (i.e. when run as root). It broke test code that used pty_fork().
 * Replaced racist pty jargon in code and documentation with descriptive terms.
 */

/*

=head1 NAME

I<libslack(pseudo)> - pseudo terminal module

=head1 SYNOPSIS

    #include <slack/std.h>
    #include <slack/pseudo.h>

    int pty_open(int *pty_user_fd, int *pty_process_fd, char *pty_device_name, size_t pty_device_name_size, const struct termios *pty_device_termios, const struct winsize *pty_device_winsize);
    int pty_release(const char *pty_device_name);
    int pty_set_owner(const char *pty_device_name, uid_t uid);
    int pty_make_controlling_tty(int *pty_process_fd, const char *pty_device_name);
    int pty_change_window_size(int pty_user_fd, int row, int col, int xpixel, int ypixel);
    pid_t pty_fork(int *pty_user_fd, char *pty_device_name, size_t pty_device_name_size, const struct termios *pty_device_termios, const struct winsize *pty_device_winsize);

=head1 DESCRIPTION

This module provides functions for opening pseudo terminals, changing their
ownership, making them the controlling terminal, changing their window size
and forking a new process whose standard input, output, and error are
attached to a pseudo terminal which is made the controlling terminal.

=over 4

=cut

*/

#ifndef _BSD_SOURCE
#define _BSD_SOURCE /* For strlcpy() on OpenBSD-4.7 */
#endif

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE /* New name for _BSD_SOURCE */
#endif

#ifndef __BSD_VISIBLE
#define __BSD_VISIBLE 1 /* For strlcpy() on FreeBSD-8.0 */
#endif

#ifndef _GNU_SOURCE
#define _GNU_SOURCE /* For ptsname_r() on Linux */
#endif

#ifndef _NETBSD_SOURCE
#define _NETBSD_SOURCE /* For strlcpy() and so <utmpx.h> (included by <util.h>) isn't broken on NetBSD-5.0.2 */
#endif

#include "config.h"
#include "std.h"

#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#ifdef HAVE_PTY_H
#include <pty.h>
#endif
#ifdef HAVE_UTIL_H
#include <util.h>
#endif
#ifdef HAVE_LIBUTIL_H
#include <libutil.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include "pseudo.h"

#ifndef HAVE_STRLCPY
#include "str.h"
#endif

#ifndef TEST

/* Pty allocated with _getpty gets broken if we do I_PUSH:es to it */
#if defined(HAVE__GETPTY) || defined(HAVE_OPENPTY)
#undef HAVE_DEV_PTMX
#endif

#ifdef HAVE_PTY_H
#include <pty.h>
#endif
#if defined(HAVE_DEV_PTMX) && defined(HAVE_SYS_STROPTS_H)
#include <sys/stropts.h>
#endif

#if 0
/*
** Don't use vhangup(). It is only available on Linux anyway
** and it requires root privileges (CAP_SYS_TTY_CONFIG).
** And, when it works, it causes some coproc module tests
** that use pty_fork() to fail when run as root (but only
** since we started turning off echo mode on pty_user_fd).
** And it doesn't make sense to virtually hangup the terminal
** as soon as we've made it our controlling terminal. I don't
** understand what the point ever was. The documentation for
** vhangup() says that it arranges for other users to have a
** "clean" terminal at login time without explaining what that
** means. I doubt that it's relevant here.
*/
#if defined(HAVE_VHANGUP) && !defined(HAVE_DEV_PTMX)
#define USE_VHANGUP
#endif
#endif

#ifndef O_NOCTTY
#define O_NOCTTY 0
#endif

#ifndef PATH_TTY
#define PATH_TTY "/dev/tty"
#endif

#define set_errno(errnum) (errno = (errnum), -1)

/*

=item C<int pty_open(int *pty_user_fd, int *pty_process_fd, char *pty_device_name, size_t pty_device_name_size, const struct termios *pty_device_termios, const struct winsize *pty_device_winsize)>

A safe version of I<openpty(3)>. Allocates and opens a pseudo terminal. The
new file descriptor for the user (or controlling process) side of the pseudo
terminal is stored in C<*pty_user_fd>. The new file descriptor for the
process side of the pseudo terminal is stored in C<*pty_process_fd>. The
device name of the process side of the pseudo terminal is stored in the
buffer pointed to by C<pty_device_name> which must be able to hold at least
C<64> characters. C<pty_device_name_size> is the size of the buffer pointed
to by C<pty_device_name>. No more than C<pty_device_name_size> bytes will be
written into the buffer pointed to by C<pty_device_name>, including the
terminating C<nul> byte. If C<pty_device_termios> is not null, it is passed
to I<tcsetattr(3)> with the command C<TCSANOW> to set the terminal
attributes of the device on the process side of the pseudo terminal. If
C<pty_device_winsize> is not null, it is passed to I<ioctl(2)> with the
command C<TIOCSWINSZ> to set the window size of the device on the process
side of the pseudo terminal. On success, returns C<0>. On error, returns
C<-1> with C<errno> set appropriately.

=cut

*/

static int groupname2gid(const char *groupname)
{
	FILE *group = fopen("/etc/group", "r");
	char line[BUFSIZ], *gid;
	int ret = -1;

	while (fgets(line, BUFSIZ, group))
	{
		if (!strncmp(line, "tty:", 4))
		{
			if ((gid = strchr(line + 4, ':')))
				ret = atoi(gid + 1);

			break;
		}
	}

	fclose(group);

	return ret;
}

static int uid2gid(uid_t uid)
{
	FILE *passwd = fopen("/etc/passwd", "r");
	char line[BUFSIZ], *ptr;
	int ret = -1;

	while (fgets(line, BUFSIZ, passwd))
	{
		if ((ptr = strchr(line, ':')) && (ptr = strchr(ptr + 1, ':')) &&
			atoi(ptr + 1) == (int)uid && (ptr = strchr(ptr + 1, ':')))
		{
			ret = atoi(ptr + 1);
			break;
		}
	}

	fclose(passwd);

	return ret;
}

int pty_open(int *pty_user_fd, int *pty_process_fd, char *pty_device_name, size_t pty_device_name_size, const struct termios *pty_device_termios, const struct winsize *pty_device_winsize)
{
	struct termios pty_user_termios[1];

#if defined(HAVE_OPENPTY) || defined(BSD4_4)

	/* openpty(3) exists in OSF/1 and some other os'es */

#ifdef HAVE_TTYNAME_R
	char buf[64], *name = buf;
	int err;
#else
	char *name;
#endif

	if (!pty_user_fd || !pty_process_fd || !pty_device_name || pty_device_name_size < 64)
		return set_errno(EINVAL);

	/* Open both pty descriptors, set ownership and permissions */

	if (openpty(pty_user_fd, pty_process_fd, NULL, NULL, NULL) == -1)
		return -1;

	/* Retrieve the device name of the process side of the pty */

#ifdef HAVE_TTYNAME_R
	if ((err = ttyname_r(*pty_process_fd, buf, 64)))
	{
		close(*pty_user_fd);
		close(*pty_process_fd);
		return set_errno(err);
	}
#else
	if (!(name = ttyname(*pty_process_fd)))
	{
		close(*pty_user_fd);
		close(*pty_process_fd);
		return set_errno(ENOTTY);
	}
#endif

	/* Return it to the caller */

	if (strlcpy(pty_device_name, name, pty_device_name_size) >= pty_device_name_size)
	{
		close(*pty_user_fd);
		close(*pty_process_fd);
		return set_errno(ENOSPC);
	}

#else /* HAVE_OPENPTY */
#ifdef HAVE__GETPTY

	/*
	 * _getpty(3) exists in SGI Irix 4.x, 5.x & 6.x -- it generates more
	 * pty's automagically when needed
	 */

	char *name;

	if (!pty_user_fd || !pty_process_fd || !pty_device_name || pty_device_name_size < 64)
		return set_errno(EINVAL);

	/* Open the pty user file descriptor and get the pty's device name */

	if (!(name = _getpty(pty_user_fd, O_RDWR, 0622, 0)))
		return -1;

	/* Return it to the caller */

	if (strlcpy(pty_device_name, name, pty_device_name_size) >= pty_device_name_size)
	{
		close(*pty_user_fd);
		return set_errno(ENOSPC);
	}

	/* Open the pty process file descriptor */

	if ((*pty_process_fd = open(pty_device_name, O_RDWR | O_NOCTTY)) == -1)
	{
		close(*pty_user_fd);
		return -1;
	}

#else /* HAVE__GETPTY */
#if defined(HAVE_DEV_PTMX)

	/*
	 * This code is used e.g. on Solaris 2.x.  (Note that Solaris 2.3
	 * also has bsd-style ptys, but they simply do not work.)
	 */

#ifdef HAVE_PTSNAME_R
	char buf[64], *name = buf;
	int err;
#else
	char *name;
#endif

	if (!pty_user_fd || !pty_process_fd || !pty_device_name || pty_device_name_size < 64)
		return set_errno(EINVAL);

	/* Open the pty user file descriptor */

	if ((*pty_user_fd = open("/dev/ptmx", O_RDWR | O_NOCTTY)) == -1)
		return -1;

	/* Set pty device ownership and permissions to the real uid of the process */

	if (grantpt(*pty_user_fd) == -1)
	{
		close(*pty_user_fd);
		return -1;
	}

	/* Unlock the pty device so it can be opened */

	if (unlockpt(*pty_user_fd) == -1)
	{
		close(*pty_user_fd);
		return -1;
	}

	/* Retrieve the device name of the process side of the pty */

#ifdef HAVE_PTSNAME_R
	if ((err = ptsname_r(*pty_user_fd, buf, 64)))
	{
		close(*pty_user_fd);
		return set_errno(err);
	}
#else
	if (!(name = ptsname(*pty_user_fd)))
	{
		close(*pty_user_fd);
		return set_errno(ENOTTY);
	}
#endif

	/* Return it to the caller */

	if (strlcpy(pty_device_name, name, pty_device_name_size) >= pty_device_name_size)
	{
		close(*pty_user_fd);
		return set_errno(ENOSPC);
	}

	/* Open the pty process file descriptor */

	if ((*pty_process_fd = open(pty_device_name, O_RDWR | O_NOCTTY)) == -1)
	{
		close(*pty_user_fd);
		return -1;
	}

	/* Turn the pty process file descriptor into a terminal */

#ifndef HAVE_CYGWIN
	/*
	 * Push the appropriate streams modules, as described in Solaris pts(7).
	 * HP-UX pts(7) doesn't have ttcompat module.
	 */
	if (ioctl(*pty_process_fd, I_PUSH, "ptem") == -1)
	{
		close(*pty_user_fd);
		close(*pty_process_fd);
		return -1;
	}

	if (ioctl(*pty_process_fd, I_PUSH, "ldterm") == -1)
	{
		close(*pty_user_fd);
		close(*pty_process_fd);
		return -1;
	}

#ifndef __hpux
	if (ioctl(*pty_process_fd, I_PUSH, "ttcompat") == -1)
	{
		close(*pty_user_fd);
		close(*pty_process_fd);
		return -1;
	}
#endif
#endif

#else /* HAVE_DEV_PTMX */
#ifdef HAVE_DEV_PTS_AND_PTC

	/* AIX-style pty code */

#ifdef HAVE_TTYNAME_R
	char buf[64], *name = buf;
	int err;
#else
	char *name;
#endif

	if (!pty_user_fd || !pty_process_fd || !pty_device_name || pty_device_name_size < 64)
		return set_errno(EINVAL);

	/* Open the pty user file descriptor */

	if ((*pty_user_fd = open("/dev/ptc", O_RDWR | O_NOCTTY)) == -1)
		return -1;

	/* Retrieve the device name of the process side of the pty */

#ifdef HAVE_TTYNAME_R
	if ((err = ttyname_r(*pty_user_fd, buf, 64)))
	{
		close(*pty_user_fd);
		return set_errno(err);
	}
#else
	if (!(name = ttyname(*pty_user_fd)))
	{
		close(*pty_user_fd);
		return set_errno(ENOTTY);
	}
#endif

	/* Return it to the caller */

	if (strlcpy(pty_device_name, name, pty_device_name_size) >= pty_device_name_size)
	{
		close(*pty_user_fd);
		return set_errno(ENOSPC);
	}

	/* Open the pty process file descriptor */

	if ((*pty_process_fd = open(name, O_RDWR | O_NOCTTY)) == -1)
	{
		close(*pty_user_fd);
		return -1;
	}

#else /* HAVE_DEV_PTS_AND_PTC */

	/* BSD-style pty code */
	const char * const ptymajors = "pqrstuvwxyzabcdefghijklmnoABCDEFGHIJKLMNOPQRSTUVWXYZ";
	const char * const ptyminors = "0123456789abcdef";
	int num_minors = strlen(ptyminors);
	int num_ptys = strlen(ptymajors) * num_minors;
	char buf[64];
	int found = 0;
	int i;

	/* Identify the first available pty user device */

	for (i = 0; !found && i < num_ptys; i++)
	{
		snprintf(buf, sizeof buf, "/dev/pty%c%c", ptymajors[i / num_minors], ptyminors[i % num_minors]);
		snprintf(pty_device_name, pty_device_name_size, "/dev/tty%c%c", ptymajors[i / num_minors], ptyminors[i % num_minors]);

		/* Open the pty user file descriptor */

		if ((*pty_user_fd = open(buf, O_RDWR | O_NOCTTY)) == -1)
		{
			/* Try SCO style naming */
			snprintf(buf, sizeof buf, "/dev/ptyp%d", i);
			snprintf(pty_device_name, pty_device_name_size, "/dev/ttyp%d", i);

			if ((*pty_user_fd = open(buf, O_RDWR | O_NOCTTY)) == -1)
				continue;
		}

		/* Set pty device ownership and permissions to the real uid of the process */

		pty_set_owner(pty_device_name, getuid());

		/* Open the pty process file descriptor */

		if ((*pty_process_fd = open(pty_device_name, O_RDWR | O_NOCTTY)) == -1)
		{
			close(*pty_user_fd);
			return -1;
		}

		found = 1;
	}

	if (!found)
		return set_errno(ENOENT);
}

#endif /* HAVE_DEV_PTS_AND_PTC */
#endif /* HAVE_DEV_PTMX */
#endif /* HAVE__GETPTY */
#endif /* HAVE_OPENPTY */

	/* The pty user terminal may have echo on. Turn it off. */
	/* But this returns EINVAL on Solaris-10 so let it fail. */

	/* Turning echo off causes some coproc module tests to fail
	** on Linux after coproc_pty_open() is called. We write to
	** it and then call read_timeout() and nothing happens.
	** But I'd expect /bin/cat to write what it has read.
	** The terminal echo shouldn't be required. And anyway,
	** shouldn't the process end be doing the echo? And the tests
	** only fail when run as root. What's going on here?
	** Calling vhangup() in pty_make_controlling_tty() was
	** causing the problem, but only when we turn off echo.
	** Without vhangup(), it's fine.
	*/

	if (tcgetattr(*pty_user_fd, pty_user_termios) != -1)
	{
		if (pty_user_termios->c_lflag & ECHO)
		{
			pty_user_termios->c_lflag &= ~ECHO;

			if (tcsetattr(*pty_user_fd, TCSANOW, pty_user_termios) == -1)
			{
				close(*pty_user_fd);
				close(*pty_process_fd);
				return -1;
			}
		}
	}

	/* Set the pty process file descriptor's terminal attributes if requested */

	if (pty_device_termios && tcsetattr(*pty_process_fd, TCSANOW, pty_device_termios) == -1)
	{
		close(*pty_user_fd);
		close(*pty_process_fd);
		return -1;
	}

	/* Set the pty process file descriptor's window size if required */

	if (pty_device_winsize && ioctl(*pty_process_fd, TIOCSWINSZ, pty_device_winsize) == -1)
	{
		close(*pty_user_fd);
		close(*pty_process_fd);
		return -1;
	}

	return 0;
}

/*

=item C<int pty_release(const char *pty_device_name)>

Releases the pseudo terminal device whose name is in C<pty_device_name>. Its
ownership is returned to I<root>, and its permissions are set to
C<rw-rw-rw->. Note that only I<root> can execute this function successfully
on most systems. On success, returns C<0>. On error, returns C<-1> with
C<errno> set appropriately.

=cut

*/

int pty_release(const char *pty_device_name)
{
	if (!pty_device_name)
		return set_errno(EINVAL);

	if (chown(pty_device_name, (uid_t)0, (gid_t)0) == -1)
		return -1;

	if (chmod(pty_device_name, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH) == -1)
		return -1;

	return 0;
}

/*

=item C<int pty_set_owner(const char *pty_device_name, uid_t uid)>

Changes the ownership of the pseudo terminal device referred to by
C<pty_device_name> to the user id, C<uid>. Group ownership of the device
will be changed to the C<tty> group if it exists. Otherwise, it will be
changed to the given user's primary group. The device's permissions will be
set to C<rw--w---->. Note that only I<root> can execute this function
successfully on most systems. Also note that the ownership of the device is
automatically set to the real uid of the process by I<pty_open(3)> and
I<pty_fork(3)>. The permissions are also set automatically by these
functions. So I<pty_set_owner(3)> is only needed when the device needs to be
owned by some user other than the real user. On success, returns C<0>. On
error, returns C<-1> with C<errno> set appropriately.

=cut

*/

int pty_set_owner(const char *pty_device_name, uid_t uid)
{
	mode_t mode = S_IRUSR | S_IWUSR | S_IWGRP;
	struct stat status[1];
	int gid;

	if (stat(pty_device_name, status) == -1)
		return -1;

	if ((gid = groupname2gid("tty")) == -1)
	{
		gid = uid2gid(uid);
		mode |= S_IWOTH;
	}

	if (status->st_uid != uid || status->st_gid != gid)
		if (chown(pty_device_name, uid, gid) == -1)
			if (errno != EROFS || status->st_uid != uid)
				return -1;

	if ((status->st_mode & (S_IRWXU | S_IRWXG | S_IRWXO)) != mode)
		if (chmod(pty_device_name, mode) == -1)
			if (errno != EROFS || (status->st_mode & (S_IRGRP | S_IROTH)))
				return -1;

	return 0;
}

/*

=item C<int pty_make_controlling_tty(int *pty_process_fd, const char *pty_device_name)>

Makes the pseudo terminal's process file descriptor the controlling
terminal. C<*pty_process_fd> contains the file descriptor for the process
side of a pseudo terminal. The file descriptor of the resulting controlling
terminal will be stored in C<*pty_process_fd>. C<pty_device_name> is the
device name of the process side of the pseudo terminal. On success, returns
C<0>. On error, returns C<-1> with C<errno> set appropriately.

=cut

*/

int pty_make_controlling_tty(int *pty_process_fd, const char *pty_device_name)
{
	int fd;
#ifdef USE_VHANGUP
	void (*old)(int);
#endif /* USE_VHANGUP */

	if (!pty_process_fd || *pty_process_fd < 0 || !pty_device_name)
		return set_errno(EINVAL);

	/* First disconnect from the old controlling tty */
#ifdef TIOCNOTTY
	if ((fd = open(PATH_TTY, O_RDWR | O_NOCTTY)) >= 0)
	{
		ioctl(fd, TIOCNOTTY, NULL);
		close(fd);
	}
#endif /* TIOCNOTTY */

	setsid();

	/*
	 * Verify that we are successfully disconnected from the controlling
	 * tty.
	 */
#if 0
	if ((fd = open(PATH_TTY, O_RDWR | O_NOCTTY)) >= 0)
	{
		close(fd);
		return set_errno(ENXIO);
	}
#endif

	/* Make it our controlling tty */
#ifdef TIOCSCTTY
	if (ioctl(*pty_process_fd, TIOCSCTTY, NULL) == -1)
		return -1;
#endif /* TIOCSCTTY */
#ifdef HAVE_NEWS4
	setpgrp(0, 0);
#endif /* HAVE_NEWS4 */
#ifdef USE_VHANGUP
	old = signal(SIGHUP, SIG_IGN);
	vhangup();
	signal(SIGHUP, old);
#endif /* USE_VHANGUP */
	/* Why do this? */
	if ((fd = open(pty_device_name, O_RDWR)) >= 0)
	{
#ifdef USE_VHANGUP
	close(*pty_process_fd);
	*pty_process_fd = fd;
#else /* USE_VHANGUP */
	close(fd);
#endif /* USE_VHANGUP */
	}

	/* Verify that we now have a controlling tty */
	if ((fd = open(PATH_TTY, O_RDWR)) == -1)
		return -1;

	close(fd);

	return 0;
}

/*

=item C<int pty_change_window_size(int pty_user_fd, int row, int col, int xpixel, int ypixel)>

Changes the window size associated with the pseudo terminal referred to by
C<pty_user_fd>. The C<row>, C<col>, C<xpixel>, and C<ypixel> parameters
specify the new window size. On success, returns C<0>. On error, returns
C<-1> with C<errno> set appropriately.

=cut

*/

int pty_change_window_size(int pty_user_fd, int row, int col, int xpixel, int ypixel)
{
	struct winsize win;

	if (pty_user_fd < 0 || row < 0 || col < 0 || xpixel < 0 || ypixel < 0)
		return set_errno(EINVAL);

	win.ws_row = row;
	win.ws_col = col;
	win.ws_xpixel = xpixel;
	win.ws_ypixel = ypixel;

	return ioctl(pty_user_fd, TIOCSWINSZ, &win);
}

/*

=item C<pid_t pty_fork(int *pty_user_fd, char *pty_device_name, size_t pty_device_name_size, const struct termios *pty_device_termios, const struct winsize *pty_device_winsize)>

A safe version of I<forkpty(3)>. Creates a pseudo terminal, and then calls
I<fork(2)>. In the parent process, the process side of the pseudo terminal
is closed. In the child process, the user side of the pseudo terminal is
closed, and the process side is made the controlling terminal. It is
duplicated onto standard input, output, and error, and is then closed. The
user (or controlling process) side of the pseudo terminal is stored in
C<*pty_user_fd> for the parent process. The device name of the process side
of the pseudo terminal is stored in the buffer pointed to by
C<pty_device_name> which must be able to hold at least C<64> bytes.
C<pty_device_name_size> is the size of the buffer pointed to by
C<pty_device_name>. No more than C<pty_device_name_size> bytes will be
written to C<pty_device_name>, including the terminating C<nul> byte. If
C<pty_device_termios> is not null, it is passed to I<tcsetattr(3)> with the
command C<TCSANOW> to set the terminal attributes of the process side of the
pseudo terminal device. If C<pty_device_winsize> is not null, it is passed
to I<ioctl(2)> with the command C<TIOCSWINSZ> to set the window size of the
process side of the pseudo terminal device. On success, returns C<0> to the
child process and returns the process id of the child process to the parent
process. On error, returns C<-1> with C<errno> set appropriately.

=cut

*/

pid_t pty_fork(int *pty_user_fd, char *pty_device_name, size_t pty_device_name_size, const struct termios *pty_device_termios, const struct winsize *pty_device_winsize)
{
	int pty_process_fd;
	pid_t pid;

	/*
	** Note: we don't use forkpty() because it closes the pty user side file
	** descriptor in the child process before making the process side of the
	** pseudo terminal the controlling terminal of the child process and
	** this can prevent the pty process side file descriptor from becoming
	** the controlling terminal (but I have no idea why).
	*/

	if (pty_open(pty_user_fd, &pty_process_fd, pty_device_name, pty_device_name_size, pty_device_termios, pty_device_winsize) == -1)
		return -1;

	switch (pid = fork())
	{
		case -1:
			pty_release(pty_device_name);
			close(pty_process_fd);
			close(*pty_user_fd);
			return -1;

		case 0:
		{
			/* Make the process side of the pty our controlling tty */

			if (pty_make_controlling_tty(&pty_process_fd, pty_device_name) == -1)
				_exit(EXIT_FAILURE);

			/* Redirect stdin, stdout and stderr from the pseudo tty */

			if (pty_process_fd != STDIN_FILENO && dup2(pty_process_fd, STDIN_FILENO) == -1)
				_exit(EXIT_FAILURE);

			if (pty_process_fd != STDOUT_FILENO && dup2(pty_process_fd, STDOUT_FILENO) == -1)
				_exit(EXIT_FAILURE);

			if (pty_process_fd != STDERR_FILENO && dup2(pty_process_fd, STDERR_FILENO) == -1)
				_exit(EXIT_FAILURE);

			/* Close the extra descriptor for the pseudo tty */

			if (pty_process_fd != STDIN_FILENO && pty_process_fd != STDOUT_FILENO && pty_process_fd != STDERR_FILENO)
				close(pty_process_fd);

			/* Close the user side of the pseudo tty in the child */

			close(*pty_user_fd);

			return 0;
		}

		default:
		{
			/* Close the process side of the pseudo tty in the parent */

			close(pty_process_fd);

			return pid;
		}
	}
}

/*

=back

=head1 ERRORS

Additional errors may be generated and returned from the underlying system
calls. See their manual pages.

=over 4

=item C<EINVAL>

Invalid arguments were passed to one of the functions.

=item C<ENOTTY>

I<openpty(3)> or I<open("/dev/ptc")> returned a pty process file descriptor
for which I<ttyname(3)> failed to return the pty device name.
I<open("/dev/ptmx")> returned a pty user file descriptor for which
I<ptsname(3)> failed to return the pty device name.

=item C<ENOENT>

The old I<BSD>-style pty device search failed to locate an available pseudo
terminal.

=item C<ENOSPC>

The device name of the process side of the pseudo terminal was too large to
fit in the C<pty_device_name> buffer passed to I<pty_open(3)> or I<pty_fork(3)>.

=item C<ENXIO>

I<pty_make_controlling_tty(3)> failed to disconnect from the controlling
terminal.

=back

=head1 MT-Level

I<MT-Safe> if and only if I<ttyname_r(3)> or I<ptsname_r(3)> are available
when needed. On systems that have I<openpty(3)> or C<"/dev/ptc">,
I<ttyname_r(3)> is required, otherwise the unsafe I<ttyname(3)> will be
used. On systems that have C<"/dev/ptmx">, I<ptsname_r(3)> is required,
otherwise the unsafe I<ptsname(3)> will be used. On systems that have
I<_getpty(2)>, I<pty_open(3)> is unsafe because I<_getpty(2)> is unsafe. In
short, it's I<MT-Safe> under I<Linux>, Unsafe under I<Solaris> and
I<OpenBSD>.

=head1 EXAMPLE

A very simple pty program:

    #include <slack/std.h>
    #include <slack/pseudo.h>
    #include <slack/sig.h>

    #include <sys/select.h>
    #include <sys/wait.h>

    struct termios stdin_termios;
    struct winsize stdin_winsize;
    int havewin = 0;
    char pty_device_name[64];
    int pty_user_fd;
    pid_t pid;

    int tty_raw(int fd)
    {
        struct termios attr[1];

        if (tcgetattr(fd, attr) == -1)
            return -1;

        attr->c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
        attr->c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
        attr->c_cflag &= ~(CSIZE | PARENB);
        attr->c_cflag |= (CS8);
        attr->c_oflag &= ~(OPOST);
        attr->c_cc[VMIN] = 1;
        attr->c_cc[VTIME] = 0;

        return tcsetattr(fd, TCSANOW, attr);
    }

    void restore_stdin(void)
    {
        if (tcsetattr(STDIN_FILENO, TCSANOW, &stdin_termios) == -1)
            errorsys("failed to restore stdin terminal attributes");
    }

    void winch(int signo)
    {
        struct winsize winsize;

        if (ioctl(STDIN_FILENO, TIOCGWINSZ, &winsize) != -1)
            ioctl(pty_user_fd, TIOCSWINSZ, &winsize);
    }

    int main(int ac, char **av)
    {
        if (ac == 1)
        {
            fprintf(stderr, "usage: pty command [arg...]\n");
            return EXIT_FAILURE;
        }

        if (isatty(STDIN_FILENO))
            havewin = ioctl(STDIN_FILENO, TIOCGWINSZ, &stdin_winsize) != -1;

        switch (pid = pty_fork(&pty_user_fd, pty_device_name, sizeof pty_device_name, NULL, havewin ? &stdin_winsize : NULL))
        {
            case -1:
                fprintf(stderr, "pty: pty_fork() failed (%s)\n", strerror(errno));
                pty_release(pty_device_name);
                return EXIT_FAILURE;

            case 0:
                execvp(av[1], av + 1);
                return EXIT_FAILURE;

            default:
            {
                int in_fd = STDIN_FILENO;
                int status;

                if (isatty(STDIN_FILENO))
                {
                    if (tcgetattr(STDIN_FILENO, &stdin_termios) != -1)
                        atexit((void (*)(void))restore_stdin);

                    tty_raw(STDIN_FILENO);

                    signal_set_handler(SIGWINCH, 0, winch);
                }

                while (pty_user_fd != -1)
                {
                    fd_set readfds[1];
                    int maxfd;
                    char buf[BUFSIZ];
                    ssize_t bytes;
                    int n;

                    FD_ZERO(readfds);

                    if (in_fd != -1)
                        FD_SET(in_fd, readfds);

                    if (pty_user_fd != -1)
                        FD_SET(pty_user_fd, readfds);

                    maxfd = (pty_user_fd > in_fd) ? pty_user_fd : in_fd;

                    signal_handle_all();

                    if ((n = select(maxfd + 1, readfds, NULL, NULL, NULL)) == -1 && errno != EINTR)
                        break;

                    if (n == -1 && errno == EINTR)
                        continue;

                    if (in_fd != -1 && FD_ISSET(in_fd, readfds))
                    {
                        if ((bytes = read(in_fd, buf, BUFSIZ)) > 0)
                        {
                            if (pty_user_fd != -1 && write(pty_user_fd, buf, bytes) == -1)
                                break;
                        }
                        else if (n == -1 && errno == EINTR)
                        {
                            continue;
                        }
                        else
                        {
                            in_fd = -1;
                            continue;
                        }
                    }

                    if (pty_user_fd != -1 && FD_ISSET(pty_user_fd, readfds))
                    {
                        if ((bytes = read(pty_user_fd, buf, BUFSIZ)) > 0)
                        {
                            if (write(STDOUT_FILENO, buf, bytes) == -1)
                                break;
                        }
                        else if (n == -1 && errno == EINTR)
                        {
                            continue;
                        }
                        else
                        {
                            close(pty_user_fd);
                            pty_user_fd = -1;
                            continue;
                        }
                    }
                }

                if (waitpid(pid, &status, 0) == -1)
                {
                    fprintf(stderr, "pty: waitpid(%d) failed (%s)\n", (int)pid, strerror(errno));
                    pty_release(pty_device_name);
                    return EXIT_FAILURE;
                }
            }
        }

        pty_release(pty_device_name);

        if (pty_user_fd != -1)
            close(pty_user_fd);

        return EXIT_SUCCESS;
    }

=head1 SEE ALSO

I<libslack(3)>,
I<openpty(3)>,
I<forkpty(3)>
I<open(2)>,
I<close(2)>,
I<grantpt(3)>,
I<unlockpt(3)>,
I<ioctl(2)>,
I<ttyname(3)>,
I<ttyname_r(3)>,
I<ptsname(3)>,
I<ptsname_r(3)>,
I<setpgrp(2)>,
I<vhangup(2)>,
I<setsid(2)>,
I<_getpty(2)>,
I<chown(2)>,
I<chmod(2)>,
I<tcsetattr(3)>,
I<setpgrp(2)>,
I<fork(2)>,
I<dup2(2)>

=head1 AUTHOR

1995 Tatu Ylonen <ylo@cs.hut.fi>
2001-2021 raf <raf@raf.org>

=cut

*/

#endif

#ifdef TEST

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <pwd.h>

#include <sys/types.h>
#include <sys/wait.h>

int main(int ac, char **av)
{
	int errors = 0;
	char pty_device_name[64];
	int	pty_user_fd, pty_process_fd;
	uid_t euid = geteuid();
	pid_t pid;

	printf("Testing: %s\n", "pseudo");

	/* Test pty_open() */

	if (pty_open(&pty_user_fd, &pty_process_fd, pty_device_name, sizeof pty_device_name, NULL, NULL) == -1)
		++errors, printf("Test1: pty_open() failed (%s)\n", strerror(errno));
	else
	{
		/* Test pty_set_owner() if root */

		if (euid == 0 && pty_set_owner(pty_device_name, getuid()) == -1)
			++errors, printf("Test2: pty_set_owner() failed (%s)\n", strerror(errno));

		/* Test pty_make_controlling_tty() and pty_change_window_size() */

		switch (pid = fork())
		{
			case -1:
				++errors, printf("Test3: failed to perform test: fork() failed (%s)\n", strerror(errno));
				break;

			case 0:
			{
				errors = 0;
				close(pty_user_fd);

				if (pty_make_controlling_tty(&pty_process_fd, pty_device_name) == -1)
					++errors, printf("Test4: pty_make_controlling_tty() failed (%s)\n", strerror(errno));

				if (pty_change_window_size(pty_process_fd, 80, 24, 800, 240) == -1)
					++errors, printf("Test5: pty_change_window_size() failed (%s)\n", strerror(errno));

				exit(errors);
			}

			default:
			{
				int status;

				if (waitpid(pid, &status, 0) == -1)
					++errors, printf("Test6: failed to evaluate test: waitpid() failed (%s)\n", strerror(errno));
				else if (WIFSIGNALED(status))
					++errors, printf("Test6: failed to evaluate test: child process received signal %d\n", WTERMSIG(status));
				else
					errors += (WEXITSTATUS(status) != EXIT_SUCCESS);

				break;
			}
		}

		/* Test pty_release() if root */

		if (euid == 0 && pty_release(pty_device_name) == -1)
			++errors, printf("Test7: pty_release() failed (%s)\n", strerror(errno));
	}

	/* Test pty_fork() */

	switch (pid = pty_fork(&pty_user_fd, pty_device_name, sizeof pty_device_name, NULL, NULL))
	{
		case -1:
			++errors, printf("Test8: pty_fork() failed (%s)\n", strerror(errno));
			break;

		case 0:
			exit(isatty(STDIN_FILENO) ? EXIT_SUCCESS : EXIT_FAILURE);
			break; /* unreached */

		default:
		{
			int status;

			if (waitpid(pid, &status, 0) == -1)
				++errors, printf("Test9: failed to evaluate test: waitpid() failed (%s)\n", strerror(errno));
			else if (WIFSIGNALED(status))
				++errors, printf("Test9: failed to evaluate test: child process received signal %d\n", WTERMSIG(status));
			else if (WEXITSTATUS(status) != EXIT_SUCCESS)
				++errors, printf("Test9: pty_fork() failed to result in a tty for child\n");

			if (euid == 0 && pty_release(pty_device_name) == -1)
				++errors, printf("Test10: pty_release() failed (%s)\n", strerror(errno));

			break;
		}
	}

	if (errors)
		printf("%d/10 tests failed\n", errors);
	else
		printf("All tests passed\n");

	if (euid)
	{
		printf("\n");
		printf("   Note: Can't test pty_set_owner() or pty_release()\n");
		printf("   On some systems (e.g. Linux) these aren't needed anyway.\n");
		printf("   Audit the code and rerun the test as root (or setuid root)\n");
	}

	return (errors == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

#endif

/* vi:set ts=4 sw=4: */
