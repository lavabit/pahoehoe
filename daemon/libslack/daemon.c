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

I<libslack(daemon)> - daemon module

=head1 SYNOPSIS

    #include <slack/std.h>
    #include <slack/daemon.h>

    typedef void daemon_config_parser_t(void *obj, const char *path, char *line, size_t lineno);

    int daemon_started_by_init(void);
    int daemon_started_by_inetd(void);
    int daemon_prevent_core(void);
    int daemon_revoke_privileges(void);
    int daemon_become_user(uid_t uid, gid_t gid, char *user);
    char *daemon_absolute_path(const char *path);
    int daemon_path_is_safe(const char *path, char *explanation, size_t explanation_size);
    void *daemon_parse_config(const char *path, void *obj, daemon_config_parser_t *parser);
    int daemon_pidfile(const char *name);
    int daemon_init(const char *name);
    int daemon_close(void);
    pid_t daemon_getpid(const char *name);
    int daemon_is_running(const char *name);
    int daemon_stop(const char *name);

=head1 DESCRIPTION

This module provides functions for writing daemons. There are many tasks
that need to be performed to correctly set up a daemon process. This can be
tedious. These functions perform these tasks for you.

=over 4

=cut

*/

#include "config.h"

#ifndef NO_POSIX_SOURCE
#define NO_POSIX_SOURCE /* For ELOOP on FreeBSD-8.0 */
#endif

#ifndef _BSD_SOURCE
#define _BSD_SOURCE /* For setgroups(2) and S_ISLNK(2) on Linux */
#endif

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE /* New name for _BSD_SOURCE */
#endif

#ifndef __BSD_VISIBLE
#define __BSD_VISIBLE 1 /* For setgroups(2) and initgroups(2) on FreeBSD-8.0 */
#endif

#ifndef _NETBSD_SOURCE
#define _NETBSD_SOURCE /* For endpwent, endgrent, setgroups, initgroups, lstat, readlink on NetBSD-5.0.2 */
#endif

#include "std.h"

#include <fcntl.h>
#include <pwd.h>
#include <grp.h>

#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/resource.h>

#include "daemon.h"
#include "mem.h"
#include "err.h"
#include "lim.h"
#include "fio.h"

#ifndef HAVE_SNPRINTF
#include "snprintf.h"
#endif

#ifndef TEST

static struct
{
	pthread_mutex_t lock; /* Mutex lock for structure */
	char *pidfile;        /* Name of the locked pid file */
}
g =
{
	PTHREAD_MUTEX_INITIALIZER,
	NULL
};

#define ptry(action) { int err = (action); if (err) return set_errno(err); }

/*

=item C<int daemon_started_by_init(void)>

If this process was started by I<init(8)>, returns 1. If not, returns C<0>.
If it was, we might be getting respawned so I<fork(2)> and I<exit(2)> would
be a big mistake (and unnecessary anyway since there is no controlling
terminal). On error, returns C<-1> with C<errno> set appropriately.

=cut

*/

int daemon_started_by_init(void)
{
	return (getppid() == 1);
}

/*

=item C<int daemon_started_by_inetd(void)>

If this process was started by I<inetd(8)>, returns C<1>. If not, returns
C<0>. On error, returns C<-1> with C<errno> set appropriately. If it was,
C<stdin>, C<stdout> and C<stderr> would be opened to a socket. Closing them
would be a big mistake. We also would not need to I<fork(2)> and I<exit(2)>
because there is no controlling terminal.

=cut

*/

int daemon_started_by_inetd(void)
{
	size_t optlen = sizeof(int);
	int optval;

	return (getsockopt(STDIN_FILENO, SOL_SOCKET, SO_TYPE, &optval, (void *)&optlen) == 0);
}

/*

=item C<int daemon_prevent_core(void)>

Prevents core files from being generated. This is used to prevent leaking
sensitive information in daemons run by root. On success, returns C<0>. On
error, returns C<-1> with C<errno> set appropriately.

=cut

*/

int daemon_prevent_core(void)
{
	struct rlimit limit[1] = {{ 0, 0 }};

	if (getrlimit(RLIMIT_CORE, limit) == -1)
		return -1;

	limit->rlim_cur = 0;

	return setrlimit(RLIMIT_CORE, limit);
}

/*

=item C<int daemon_revoke_privileges(void)>

Revokes setuid and setgid privileges. Useful when your program does not
require any special privileges and may become unsafe if incorrectly
installed with special privileges. Also useful when your program only
requires special privileges upon startup (e.g. binding to a privileged
socket). Performs the following: Sets the effective gid to the real gid if
they differ. Checks that they no longer differ. Sets the effective uid to
the real uid if they differ. Checks that they no longer differ. Also closes
/etc/passwd and /etc/group in case they were opened by root and give access
to user and group passwords. On success, returns C<0>. On error, returns
C<-1> with C<errno> set appropriately.

=cut

*/

int daemon_revoke_privileges(void)
{
	uid_t uid = getuid();
	gid_t gid = getgid();
	uid_t euid = geteuid();
	gid_t egid = getegid();

	if (egid != gid && (setgid(gid) == -1 || getegid() != getgid()))
		return -1;

	if (euid != uid && (setuid(uid) == -1 || geteuid() != getuid()))
		return -1;

	endpwent();
	endgrent();

	return 0;
}

/*

=item C<int daemon_become_user(uid_t uid, gid_t gid, char *user)>

Changes the owner and group of the process to C<uid> and C<gid>
respectively. If C<user> is not null, the supplementary group list will be
initialised with I<initgroups(3)>. Otherwise, the supplementary group list
will be cleared of all groups. On success, returns 0. On error, returns -1.
Only root can use this function.

=cut

*/

int daemon_become_user(uid_t uid, gid_t gid, char *user)
{
	gid_t gids[10];
	int g = 0;

	if (setgroups(0, NULL) == -1 || (g = getgroups(0, NULL)) != 0)
	{
		/* FreeBSD always returns the primary group */

		if (g != 1 || getgroups(10, gids) != 1 || gids[0] != getgid())
			return -1;
	}

	if (setgid(gid) == -1 || getgid() != gid || getegid() != gid)
		return -1;

	if (user && initgroups(user, gid) == -1)
		return -1;

	if (setuid(uid) == -1 || getuid() != uid || geteuid() != uid)
		return -1;

	return 0;
}

/*

=item C<char *daemon_absolute_path(const char *path)>

Returns C<path> converted into an absolute path. Cleans up any C<.> and
C<..> and C<//> and trailing C</> found in the returned path. Note that the
returned path looks canonical but isn't because symbolic links are not
followed and expanded. It is the caller's responsibility to deallocate the
path returned with I<mem_release(3)> or I<free(3)>. On success, returns the
absolute path. On error, returns C<null> with C<errno> set appropriately.

=cut

*/

char *daemon_absolute_path(const char *path)
{
	size_t path_len;
	char *abs_path;
	char *p;

	if (!path)
		return set_errnull(EINVAL);

	/* Make path absolute and mostly canonical (don't follow symbolic links) */

	if (*path != PATH_SEP)
	{
		long lim = limit_path();
		char *cwd = mem_create(lim, char);
		size_t cwd_len;
		int rc;

		if (!cwd)
			return NULL;

		if (!getcwd(cwd, lim))
		{
			mem_release(cwd);
			return NULL;
		}

		cwd_len = strlen(cwd);
		if (cwd_len + 1 + strlen(path) >= lim)
		{
			mem_release(cwd);
			return set_errnull(ENAMETOOLONG);
		}

		rc = snprintf(cwd + cwd_len, lim - cwd_len, "%c%s", PATH_SEP, path);
		if (rc == -1 || rc >= lim - cwd_len)
		{
			mem_release(cwd);
			return set_errnull(ENAMETOOLONG);
		}

		abs_path = cwd;
	}
	else
	{
		if (!(abs_path = mem_strdup(path)))
			return NULL;
	}

	/* Clean up any // and . and .. in the absolute path */

	path_len = strlen(abs_path);

	for (p = abs_path; *p; ++p)
	{
		if (p[0] == PATH_SEP)
		{
			if (p[1] == PATH_SEP)
			{
				memmove(p, p + 1, path_len + 1 - (p + 1 - abs_path));
				--path_len;
				--p;
			}
			else if (p[1] == '.')
			{
				if (p[2] == PATH_SEP || p[2] == nul)
				{
					int keep_sep = (p == abs_path && p[2] == nul);
					memmove(p + keep_sep, p + 2, path_len + 1 - (p + 2 - abs_path));
					path_len -= 2 - keep_sep;
					--p;
				}
				else if (p[2] == '.' && (p[3] == PATH_SEP || p[3] == nul))
				{
					char *scan, *parent;
					int keep_sep;

					for (scan = parent = p; scan > abs_path; )
					{
						if (*--scan == PATH_SEP)
						{
							parent = scan;
							break;
						}
					}

					keep_sep = (parent == abs_path && p[3] == nul);
					memmove(parent + keep_sep, p + 3, path_len + 1 - (p + 3 - abs_path));
					path_len -= p + 3 - parent;
					p = parent - 1;
				}
			}
		}
	}

	/* Strip off any trailing / */

	while (path_len > 1 && abs_path[path_len - 1] == PATH_SEP)
		abs_path[--path_len] = nul;

	return abs_path;
}

/*

=item C<int daemon_path_is_safe(const char *path, char *explanation, size_t explanation_size)>

Checks that the file referred to by C<path> is not group or world writable.
Also checks that the containing directories are not group or world writable,
following symbolic links. Useful when you need to know whether or not you
can trust a user supplied configuration/command file before reading and
acting upon its contents. On success, returns 1 if C<path> is safe or 0 if
it is not. When the path is not safe, an explanation is written to the
C<explanation> buffer (if it is not C<null>). No more than
C<explanation_size> bytes including the terminating C<nul> byte will be
written to the C<explanation> buffer. On error, returns C<-1> with C<errno>
set appropriately.

=cut

*/

static int daemon_check_path(char *path, char *explanation, size_t explanation_size, int level)
{
	struct stat status[1];
	char *sep;
	int rc;

	if (level > 16)
		return set_errno(ELOOP);

	for (sep = path + strlen(path); sep; sep = strrchr(path, PATH_SEP))
	{
		sep[sep == path] = nul;

		if (lstat(path, status) == -1)
			return -1;

		if (S_ISLNK(status->st_mode))
		{
			size_t lim;
			char *sym_linked;
			char *tmp;

			lim = limit_path();
			if (!(sym_linked = mem_create(lim, char)))
				return -1;

			memset(sym_linked, 0, lim);

			if (readlink(path, sym_linked, lim) == -1)
			{
				mem_release(sym_linked);
				return -1;
			}

			if (*sym_linked != PATH_SEP)
			{
				if (!(tmp = mem_create(lim, char)))
				{
					mem_release(sym_linked);
					return -1;
				}

				rc = snprintf(tmp, lim, "%s%c..%c%s", path, PATH_SEP, PATH_SEP, sym_linked);
				if (rc == -1 || rc >= lim)
				{
					mem_release(sym_linked);
					mem_release(tmp);
					return set_errno(ENAMETOOLONG);
				}

				rc = snprintf(sym_linked, lim, "%s", tmp);
				mem_release(tmp);
				if (rc == -1 || rc >= lim)
				{
					mem_release(sym_linked);
					return set_errno(ENAMETOOLONG);
				}
			}

			tmp = daemon_absolute_path(sym_linked);
			mem_release(sym_linked);
			if (!(sym_linked = tmp))
				return -1;

			rc = daemon_check_path(sym_linked, explanation, explanation_size, level + 1);
			mem_release(sym_linked);

			switch (rc)
			{
				case -1: return -1;
				case  0: return 0;
				case  1: break;
			}
		}
		else if (status->st_mode & (S_IWGRP | S_IWOTH))
		{
			if (explanation)
			{
				snprintf(explanation, explanation_size, "%s is %s%s%s writable", path,
					(status->st_mode & S_IWGRP) ? "group" : "",
					((status->st_mode & (S_IWGRP | S_IWOTH)) == (S_IWGRP | S_IWOTH)) ? " and " : "",
					(status->st_mode & S_IWOTH) ? "world" : ""
				);
			}

			return 0;
		}

		if (sep == path)
			break;
	}

	return 1;
}

int daemon_path_is_safe(const char *path, char *explanation, size_t explanation_size)
{
	char *abs_path;
	int rc;

	if (!path)
		return set_errno(EINVAL);

	abs_path = daemon_absolute_path(path);
	if (!abs_path)
		return -1;

	rc = daemon_check_path(abs_path, explanation, explanation_size, 0);
	mem_release(abs_path);

	return rc;
}

/*

=item C<void *daemon_parse_config(const char *path, void *obj, daemon_config_parser_t *parser)>

Parses the text configuration file named C<path>. Blank lines are ignored.
Comments (C<'#'> to end of line) are ignored. Lines that end with C<'\'> are
joined with the following line. There may be whitespace and even a comment
after the C<'\'> character but nothing else. The C<parser> function is
called with the client supplied C<obj>, the file name, the line and the line
number as arguments. On success, returns C<obj>. On error, returns C<null>
(i.e. if the configuration file could not be read). Note: Don't parse config
files unless they are "safe" as determined by I<daemon_path_is_safe(3)>.

=cut

*/

void *daemon_parse_config(const char *path, void *obj, daemon_config_parser_t *parser)
{
	FILE *conf;
	char line[BUFSIZ];
	char buf[BUFSIZ];
	int lineno;
	int rc;

	if (!(conf = fopen(path, "r")))
		return NULL;

	line[0] = nul;

	for (lineno = 1; fgets(buf, BUFSIZ, conf); ++lineno)
	{
		char *start = buf;
		char *end;
		size_t length;
		int cont_line;

		/* Strip trailing comments */

		if ((end = strchr(start, '#')))
			*end = nul;
		else
			end = start + strlen(start);

		/* Skip trailing spaces (allows comments after line continuation) */

		while (end > start && isspace((int)(unsigned char)end[-1]))
			--end;

		/* Skip empty lines */

		if (*start == nul || start == end)
			continue;

		/* Perform line continuation */

		if ((cont_line = (end[-1] == '\\')))
			--end;

		length = strlen(line);
		rc = snprintf(line + length, BUFSIZ - length, "%*.*s", (int)(end - start), (int)(end - start), start);
		if (rc == -1 || rc >= BUFSIZ - length)
			return NULL;

		if (cont_line)
			continue;

		/* Parse the resulting line */

		parser(obj, path, line, lineno);
		line[0] = nul;
	}

	fclose(conf);

	return obj;
}

/*

C<int daemon_construct_pidfile(const char *name, char **pidfile)>

Constructs the pidfile for the given C<name> in C<pidfile>. If C<name> is
already an absolute path, it is just copied into the new buffer directly. On
success, returns C<0> and the resulting buffer in C<pidfile> must be
deallocated by the caller. On error, returns C<-1> with C<errno> set
appropriately.

*/

static int daemon_construct_pidfile(const char *name, char **pidfile)
{
	long path_len;
	const char *pid_dir;
	char *suffix = ".pid";
	size_t size;

	path_len = limit_path();
	pid_dir = (getuid()) ? USER_PID_DIR : ROOT_PID_DIR;
	size = ((*name == PATH_SEP) ? strlen(name) : sizeof(pid_dir) + 1 + strlen(name) + strlen(suffix)) + 1;

	if (size > path_len)
		return set_errno(ENAMETOOLONG);

	if (!*pidfile && !(*pidfile = mem_create(path_len, char)))
		return -1;

	if (*name == PATH_SEP)
		snprintf(*pidfile, path_len, "%s", name);
	else
		snprintf(*pidfile, path_len, "%s%c%s%s", pid_dir, PATH_SEP, name, suffix);

	return 0;
}

/*

C<int daemon_lock_pidfile(const char *pidfile)>

Open and lock the file referred to by C<pidfile>. On success, returns the
file descriptor of the opened and locked file. On error, returns C<-1> with
C<errno> set appropriately.

*/

static int daemon_lock_pidfile(char *pidfile)
{
	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
	struct stat statbuf_fd[1], statbuf_fs[1];
	int pid_fd;
	start:

	/* This is broken over NFS (Linux). So pidfiles must reside locally. */

	if ((pid_fd = open(pidfile, O_RDWR | O_CREAT | O_EXCL, mode)) == -1)
	{
		if (errno != EEXIST)
			return -1;

		/*
		** The pidfile already exists. Is it locked?
		** If so, another invocation is still alive.
		** If not, the invocation that created it has died.
		** Open the pidfile to attempt a lock.
		*/

		if ((pid_fd = open(pidfile, O_RDWR)) == -1)
		{
			/*
			** We couldn't open the file. That means that it existed
			** a moment ago but has been since been deleted. Maybe if
			** we try again now, it'll work (unless another process is
			** about to re-create it before we do, that is).
			*/

			if (errno == ENOENT)
				goto start;

			return -1;
		}
	}

	if (fcntl_lock(pid_fd, F_SETLK, F_WRLCK, SEEK_SET, 0, 0) == -1)
	{
		close(pid_fd);
		return -1;
	}

	/*
	** The pidfile may have been unlinked, after we opened it, by another daemon
	** process that was dying between the last open() and the fcntl(). There's
	** no use hanging on to a locked file that doesn't exist (and there's
	** nothing to stop another daemon process from creating and locking a
	** new instance of the file. So, if the pidfile has been deleted, we
	** abandon this lock and start again. Note that it may have been deleted
	** and subsequently re-created by yet another daemon process just starting
	** up so check that that hasn't happened as well by comparing inode
	** numbers. If it has, we also abandon this lock and start again.
	*/

	if (fstat(pid_fd, statbuf_fd) == -1)
	{
		/* This shouldn't happen */
		close(pid_fd);
		return -1;
	}

	if (stat(pidfile, statbuf_fs) == -1)
	{
		/* The pidfile has been unlinked so we start again */

		if (errno == ENOENT)
		{
			close(pid_fd);
			goto start;
		}

		close(pid_fd);
		return -1;
	}
	else if (statbuf_fd->st_ino != statbuf_fs->st_ino)
	{
		/* The pidfile has been unlinked and re-created so we start again */

		close(pid_fd);
		goto start;
	}

	/* Prevent leaking this file descriptor into child processes. */

	fcntl_set_fdflag(pid_fd, FD_CLOEXEC);

	return pid_fd;
}

/*

C<int daemon_pidfile_unlocked(const char *name)>

Equivalent to I<daemon_pidfile(3)> except that the daemon module's mutex is
not locked and unlocked.

*/

static int daemon_pidfile_unlocked(const char *name)
{
	char pid[32];
	int pid_fd;

	/* Check argument */

	if (!name)
		return set_errno(EINVAL);

	/* Build the pidfile path */

	if (daemon_construct_pidfile(name, &g.pidfile) == -1)
		return -1;

	/* Open it and lock it (if possible) */

	if ((pid_fd = daemon_lock_pidfile(g.pidfile)) == -1)
	{
		mem_destroy(&g.pidfile);
		return -1;
	}

	/* Store our pid */

	snprintf(pid, 32, "%d\n", (int)getpid());

	if (write(pid_fd, pid, strlen(pid)) != strlen(pid))
	{
		daemon_close();
		return -1;
	}

	/*
	** Flaw: If someone unceremoniously unlinks the pidfile,
	** we won't know about it and nothing will stop another
	** invocation from starting up.
	*/

	return 0;
}

/*

=item C<int daemon_pidfile(const char *name)>

Creates a pid file for a daemon and locks it. The file has one line
containing the process id of the daemon. The well-known locations for the
file is defined in C<ROOT_PID_DIR> for root (C<"/var/run"> by default) and
C<USER_PID_DIR> for all other users (C<"/tmp"> by default). The name of the
file is the name of the daemon (given by the I<name> argument) followed by
C<".pid"> (If I<name> is an absolute file path, it is used as is). The
presence of this file will prevent two daemons with the same name from
running at the same time. On success, returns C<0>. On error, returns C<-1>
with C<errno> set appropriately. B<Note:> This is called by
I<daemon_init(3)> so there is usually no need to call this function
directly.

=cut

*/

int daemon_pidfile(const char *name)
{
	int rc;

	ptry(pthread_mutex_lock(&g.lock))
	rc = daemon_pidfile_unlocked(name);
	ptry(pthread_mutex_unlock(&g.lock))

	return rc;
}

/*

=item C<int daemon_init(const char *name)>

Initialises a daemon by performing the following tasks:

=over 4

=item *

If the process was not invoked by I<init(8)> (i.e. pid 1) or I<inetd(8)>
(i.e. C<stdin> is a socket):

=over 4

=item *

Ignore C<SIGHUP> signals in case the current process session leader
terminates while attached to a controlling terminal causing us to
receive a C<SIGHUP> signal before we start our own process session below.

This can happen when the process that calls I<daemon_init(3)> was itself
invoked interactively via the shell builtin C<exec>. When this initial
process terminates below, the terminal emulator that invoked the shell also
terminates.

=item *

Background the process to lose process group leadership.

=item *

Start a new process session.

=item *

Background the process again to lose process session leadership. Under
C<SVR4> this prevents the process from ever gaining a controlling terminal.
This is only necessary under C<SVR4> but is always done for simplicity. Note
that ignoring C<SIGHUP> signals earlier means that when the newly created
process session leader terminates, then even if it has a controlling
terminal open, the newly backgrounded process won't receive the
corresponding C<SIGHUP> signal that is sent to all processes in the process
session's foreground process group because it inherited signal dispositions
from the initial process.

=back

=item *

Change directory to the root directory so as not to hamper umounts.

=item *

Clear the umask to enable explicit file creation modes.

=item *

Close all open file descriptors. If the process was invoked by I<inetd(8)>,
C<stdin>, C<stdout> and C<stderr> are left open since they are open to a
socket.

=item *

Open C<stdin>, C<stdout> and C<stderr> to C</dev/null> in case something
requires them to be open. Of course, this is not done if the process was
invoked by I<inetd(8)>.

=item *

If C<name> is non-null, create and lock a file containing the process id of
the process. The presence of this locked file prevents two instances of a
daemon with the same name from running at the same time. The default
location of the pidfile is C</var/run> for I<root> or C</tmp> for ordinary
users.

=back

On success, returns C<0>. On error, returns C<-1> with C<errno> set
appropriately.

=cut

*/

int daemon_init(const char *name)
{
	pid_t pid;
	long nopen;
	int fd;

	/*
	** Don't setup a daemon-friendly process context
	** if started by init(8) or inetd(8).
	*/

	if (!(daemon_started_by_init() || daemon_started_by_inetd()))
	{
		/*
		** Ignore SIGHUP signals in case the current process session leader
		** terminates while attached to a controlling terminal causing us to
		** receive a SIGHUP signal before we start our own process session below.
		*/

		struct sigaction act[1];

		act->sa_handler = SIG_IGN;
		sigemptyset(&act->sa_mask);
		act->sa_flags = 0;

		if (sigaction(SIGHUP, act, NULL) == -1)
			return -1;

		/*
		** Background the process.
		** Lose process session/group leadership.
		*/

		if ((pid = fork()) == -1)
			return -1;

		if (pid)
		{
#ifndef DISABLE_DAEMON_INIT_EXIT_DELAY_MSEC
			/*
			** If the user has requested an exit delay because they use
			** "exec daemon" to run KDE applications that are failing
			** immediately, then delay the parent's exit by the given
			** number of milliseconds.
			**
			** Note: We are not delaying the start of the client, just the
			** exit of the initial parent process.
			**
			** Note Also: I have no idea why this delay seems to fix
			** "exec daemon kde-app" failures.
			**
			** Also Note: This has nothing to do with daemon. The same
			** failures occur with "exec kde-app".
			**
			** Also note: Setting DAEMON_INIT_EXIT_DELAY_MSEC to at least 400
			** (i.e. 0.4 seconds) seems to be enough. Your mileage may vary.
			*/

			char *delay_var;
			long delay_msec;

			if ((delay_var = getenv("DAEMON_INIT_EXIT_DELAY_MSEC")) && (delay_msec = atol(delay_var)) > 0)
				nap(delay_msec / 1000, (delay_msec & 1000) * 1000);
#endif
			exit(EXIT_SUCCESS);
		}

		/* Become a process session leader. */
		/* This can only fail when we're already a session leader. */

		setsid();

		/*
		** Lose process session leadership to prevent gaining a controlling
		** terminal in SVR4. Always do it in case we don't know what flavour
		** of UNIX this system is.
		*/

		if ((pid = fork()) == -1)
			return -1;

		if (pid)
			exit(EXIT_SUCCESS);
	}

	/* Enter the root directory to prevent hampering umounts. */

	if (chdir(ROOT_DIR) == -1)
		return -1;

	/* Clear umask to enable explicit file modes. */

	umask(0);

	/*
	** We need to close all open file descriptors. Check how
	** many file descriptors we have (If indefinite, a usable
	** number (1024) will be returned).
	**
	** Flaw: If many files were opened and then this limit
	** was reduced to below the highest file descriptor,
	** we may not close all file descriptors.
	*/

	if ((nopen = limit_open()) == -1)
		return -1;

	/*
	** Close all open file descriptors. If started by inetd,
	** we don't close stdin, stdout and stderr.
	** Don't forget to open any future tty devices with O_NOCTTY
	** so as to prevent gaining a controlling terminal
	** (not necessary with SVR4 or modern versions of BSD).
	*/

	if (daemon_started_by_inetd())
	{
		for (fd = 0; fd < nopen; ++fd)
		{
			switch (fd)
			{
				case STDIN_FILENO:
				case STDOUT_FILENO:
				case STDERR_FILENO:
					break;
				default:
					close(fd);
			}
		}
	}
	else
	{
		for (fd = 0; fd < nopen; ++fd)
			close(fd);

		/*
		** Open stdin, stdout and stderr to /dev/null just in case some
		** code buried in a library somewhere expects them to be open.
		*/

		if ((fd = open("/dev/null", O_RDWR)) == -1)
			return -1;

		/*
		** This is only needed for very strange (hypothetical)
		** POSIX implementations where STDIN_FILENO != 0 or
		** STDOUT_FILE != 1 or STDERR_FILENO != 2 (yeah, right).
		*/

		if (fd != STDIN_FILENO)
		{
			if (dup2(fd, STDIN_FILENO) == -1)
				return -1;

			close(fd);
		}

		if (dup2(STDIN_FILENO, STDOUT_FILENO) == -1)
			return -1;

		if (dup2(STDIN_FILENO, STDERR_FILENO) == -1)
			return -1;
	}

	/* Place our process id in the file system and lock it. */

	if (name)
		return daemon_pidfile(name);

	return 0;
}

/*

=item C<int daemon_close(void)>

Unlinks the locked pid file, if any. Returns 0.

=cut

*/

int daemon_close(void)
{
	ptry(pthread_mutex_lock(&g.lock))

	if (g.pidfile)
	{
		unlink(g.pidfile);
		mem_destroy(&g.pidfile);
	}

	ptry(pthread_mutex_unlock(&g.lock))

	return 0;
}

/*

=item C<pid_t daemon_getpid(const char *name)>

Return the process id of the daemon with the given C<name>. If the daemon in
question is owned by I<root>, then this function must be invoked by I<root>.
Similarly, if the daemon in question is owned by an ordinary user, then this
function must be invoked by an ordinary user. If C<name> is the absolute
path to the pidfile (rather than just the daemon name), then any user may
call this function. On success, returns the process id of the daemon. On
error, returns C<-1> with C<errno> set appropriately.

=cut

*/

pid_t daemon_getpid(const char *name)
{
	char *pidfile = NULL;
	char buf[BUFSIZ];
	ssize_t bytes;
	int pid_fd;
	int pid = 0;

	/* Check argument */

	if (!name)
		return set_errno(EINVAL);

	/* Build the pidfile path */

	if (daemon_construct_pidfile(name, &pidfile) == -1)
		return -1;

	/* Open the pidfile */

	pid_fd = open(pidfile, O_RDONLY);
	mem_release(pidfile);

	if (pid_fd == -1)
		return -1;

	/* Read it */

	bytes = read(pid_fd, buf, BUFSIZ);
	close(pid_fd);

	if (bytes == -1)
		return -1;

	if (sscanf(buf, "%d", &pid) != 1)
		return -1;

	return (pid_t)pid;
}

/*

=item C<int daemon_is_running(const char *name)>

Checks whether or not a daemon with the given C<name> is running. If the
daemon in question is owned by I<root>, then this function must be invoked
by I<root>. Similarly, if the daemon in question is owned by an ordinary
user, then this function must be invoked by an ordinary user. However, if
C<name> is the absolute path to the pidfile (rather than just the daemon
name), then any user may call this function. On success, returns C<1> if the
daemon is running or C<0> if it is not. On error, returns C<-1> with
C<errno> set appropriately.

=cut

*/

int daemon_is_running(const char *name)
{
	char *pidfile = NULL;
	int pid_fd;

	/* Check argument */

	if (!name)
		return set_errno(EINVAL);

	/* Build the pidfile path */

	if (daemon_construct_pidfile(name, &pidfile) == -1)
		return -1;

	/* Open the pidfile to see if it exists */

	if ((pid_fd = open(pidfile, O_RDONLY)) == -1)
	{
		mem_release(pidfile);

		if (errno != ENOENT)
			return -1;

		/* The pidfile doesn't exist, so the daemon probably isn't running */

		return 0;
	}

	/* Is the pidfile write-locked? If so, the following will fail */

	if (fcntl_lock(pid_fd, F_SETLK, F_RDLCK, SEEK_SET, 0, 0) == -1)
	{
		mem_release(pidfile);
		close(pid_fd);

		if (errno != EACCES && errno != EAGAIN)
			return -1;

		return 1;
	}

	mem_release(pidfile);
	close(pid_fd);

	/* Not write-locked - daemon is not running */

	return 0;
}

/*

=item C<int daemon_stop(const char *name)>

Stop a daemon process with the given C<name> by sending it a C<SIGTERM>
signal. If the daemon in question is owned by I<root>, then this function
must be invoked by I<root>. Similarly, if the daemon in question is owned by
an ordinary user, then this function must be invoked by that user. Note that
I<root> can't use this function to stop a daemon started by another user
just by passing the name of the daemon (because the pidfiles for I<root>
daemons and user daemons are stored in different directories). In order for
I<root> to stop an ordinary user's daemon process, C<name> has to be the
absolute path to the daemon's pidfile. On success, returns C<0>. On error,
returns C<-1> with C<errno> set appropriately.

=cut

*/

int daemon_stop(const char *name)
{
	char *pidfile = NULL;
	char pidbuf[32];
	ssize_t bytes;
	int pid_fd;
	int pid = -1;

	/* Check argument */

	if (!name)
		return set_errno(EINVAL);

	/* Build the pidfile path */

	if (daemon_construct_pidfile(name, &pidfile) == -1)
		return -1;

	/* Open it and lock it (if possible) */

	if ((pid_fd = daemon_lock_pidfile(pidfile)) == -1)
	{
		/* Already locked - daemon is running */

		if (errno == EACCES || errno == EAGAIN)
		{
			/* Read the process id */

			if ((pid_fd = open(pidfile, O_RDONLY)) == -1)
			{
				mem_release(pidfile);
				return -1;
			}

			mem_release(pidfile);

			if ((bytes = read(pid_fd, pidbuf, 32)) <= 0)
			{
				close(pid_fd);
				return -1;
			}

			close(pid_fd);

			if (sscanf(pidbuf, "%d", &pid) != 1 || pid <= 0)
				return set_errno(EINVAL);

			/* Stop the daemon */

			return kill((pid_t)pid, SIGTERM);
		}

		mem_release(pidfile);

		return -1;
	}

	/* Not locked - daemon is not running */

	close(pid_fd);
	unlink(pidfile);
	mem_release(pidfile);

	return set_errno(ESRCH);
}

/*

=back

=head1 ERRORS

Additional errors may be generated and returned from the underlying system
calls. See their manual pages.

=over 4

=item C<EINVAL>

An argument was invalid (e.g. C<null>).

=item C<ENAMETOOLONG>

The C<name> passed to I<daemon_init(3)> or I<daemon_path_is_safe(3)>
resulted in a path name that is too long for the intended filesystem.

=item C<ELOOP>

I<daemon_path_is_safe(3)> recursed too deeply (16 levels).

=item C<ESRCH>

I<daemon_stop(3)> found that there was no daemon running with the given
name.

=back

=head1 MT-Level

MT-Safe

=head1 EXAMPLE

This example reads and prints C</etc/fstab> with I<daemon_parse_config(3)>,
becomes a daemon and then sends a I<syslog(3)> message and then terminates.

    #include <slack/lib.h>

    const char * const config_fname = "/etc/fstab";
    List *config = NULL;

    void fstab_parser(void *obj, const char *path, char *line, size_t lineno)
    {
        char device[64], mount[64], fstype[64], opts[64];
        int freq, passno;

        if (sscanf(line, "%63s %63s %63s %63s %d %d", device, mount, fstype, opts, &freq, &passno) != 6)
            fprintf(stderr, "Syntax Error in %s (line %d): %s\n", path, lineno, line);
        else
        {
            char *copy;

            printf("%s %s %s %s %d %d\n", device, mount, fstype, opts, freq, passno);

            if (!(copy = mem_strdup(line)))
                fprintf(stderr, "out of memory\n");
            else if (!list_append(config, copy))
                fprintf(stderr, "failed to add line %d to config\n", lineno);
        }
    }

    void hup(int signo)
    {
        list_remove_range(config, 0, -1);
        daemon_parse_config(config_fname, config, fstab_parser);
    }

    void term(int signo)
    {
        daemon_close();
        exit(EXIT_SUCCESS);
    }

    void do_stuff()
    {
        // do stuff...
        syslog(LOG_DAEMON | LOG_DEBUG, "Here we are");
        kill(getpid(), SIGTERM);
        signal_handle_all();
    }

    int main(int ac, char **av)
    {
        if (daemon_revoke_privileges() == -1 ||
            daemon_prevent_core() == -1 ||
            daemon_path_is_safe(config_fname, NULL, 0) != 1 ||
            (config = list_create(free)) == NULL ||
            daemon_parse_config(config_fname, config, fstab_parser) == NULL ||
            daemon_init(prog_basename(*av)) == -1 ||
            signal_set_handler(SIGHUP, 0, hup) == -1 ||
            signal_set_handler(SIGTERM, 0, term) == -1)
            return EXIT_FAILURE;

        do_stuff();

        return EXIT_SUCCESS; // unreached
    }

=head1 CAVEAT

Because I<root>'s pidfiles are created in a different directory
(C</var/run>) to those of ordinary users (C</tmp>), it is possible for
I<root> and another user to use the same name for a daemon client. This
shouldn't be a problem. It's probably desirable. But if it is a problem,
recompile I<libslack> and relink I<daemon> so that all pidfiles are created
in C</tmp> by defining C<ROOT_PID_DIR> and C<USER_PID_DIR> to both be
C</tmp>.

The exclusive creation and locking of the pidfile doesn't work correctly
over NFS on Linux so pidfiles must reside locally.

I<daemon_path_is_safe(3)> ignores ACLs (so does I<sendmail(8)>). It should
probably treat a path as unsafe if there are any ACLs (allowing extra
access) along the path.

The functions I<daemon_prevent_core(3)>, I<daemon_revoke_privileges(3)>,
I<daemon_become_user(3)>, I<daemon_absolute_path(3)>,
I<daemon_path_is_safe(3)> and I<daemon_parse_config(3)> should probably all
have the I<daemon_> prefix removed from their names. Their use is more
general than just in daemons.

If you use "exec daemon" to run a KDE application you may find that the KDE
application sometimes doesn't run. This problem has only been seen with I<konsole(1)>
but it may happen with other KDE applications as well. Capturing the standard
error of the KDE application might show something like:

  unnamed app(9697): KUniqueApplication: Registering failed!
  unnamed app(9697): Communication problem with  "konsole" , it probably crashed. 
  Error message was:  "org.freedesktop.DBus.Error.ServiceUnknown" : " "The name
                      org.kde.konsole was not provided by any .service files"

A workaround seems to be to delay the termination of the initial I<daemon(1)>
process by at least 0.4 seconds. To make this happen, set the environment
variable C<DAEMON_INIT_EXIT_DELAY_MSEC> to the number of milliseconds by
which to delay. For example: C<DAEMON_INIT_EXIT_DELAY_MSEC=400>.

=head1 SEE ALSO

I<libslack(3)>,
I<daemon(1)>,
I<init(8)>,
I<inetd(8)>,
I<fork(2)>,
I<umask(2)>,
I<setsid(2)>,
I<chdir(2)>,
I<setrlimit(2)>,
I<setgid(2)>,
I<setuid(2)>,
I<setgroups(2)>,
I<initgroups(3)>,
I<endpwent(3)>,
I<endgrent(3)>,
I<kill(2))>

=head1 AUTHOR

20201111 raf <raf@raf.org>

=cut

*/

#endif

#ifdef TEST

#include <syslog.h>

#include <sys/wait.h>

#include "msg.h"
#include "prog.h"
#include "sig.h"

typedef struct Pair1 Pair1;
typedef struct Data1 Data1;
typedef struct Data2 Data2;

struct Data1
{
	int test;
	int i;
	Pair1 *pair;
};

struct Pair1
{
	const char *service;
	const char *port;
};

static Pair1 pairs[] =
{
	{ "echo", "7/tcp" },
	{ "echo", "7/udp" },
	{ "ftp", "21/tcp" },
	{ "ssh", "22/tcp" },
	{ "smtp", "25/tcp" },
	{ NULL, NULL }
};

static const int final_pair = 5;

static Data1 data1[1] = {{ 0, 0, pairs }};

struct Data2
{
	int test;
	int i;
	int j;
	const char *text;
	const char *results[3][8];
};

static Data2 data2[1] =
{
	{
		0,
		0,
		0,
		"\n"
		"# This is a comment\n"
		"\n"
		"line1 = word1 word2\n"
		"line2 = word3 \\\n"
		"\tword4 word5 \\ # a comment in a funny place\n"
		"\tword6 word7\n"
		"\n"
		"line3 = \\\n"
		"\tword8\n"
		"\n",
		{
			{ "line1", "=", "word1", "word2", NULL, NULL, NULL, NULL },
			{ "line2", "=", "word3", "word4", "word5", "word6", "word7", NULL },
			{ "line3", "=", "word8", NULL, NULL, NULL, NULL, NULL }
		}
	}
};

static const int final_line = 3;
static const int final_word = 3;

static int errors = 0;

static int config_test1(int test, const char *name)
{
	FILE *out = fopen(name, "w");
	int i;

	if (!out)
	{
		++errors, printf("Test%d: failed to create file: '%s'\n", test, name);
		return 0;
	}

	for (i = 0; data1->pair[i].service; ++i)
		fprintf(out, "%s %s\n", data1->pair[i].service, data1->pair[i].port);

	fclose(out);
	return 1;
}

static void parse_test1(void *obj, const char *path, char *line, size_t lineno)
{
	Data1 *data1 = (Data1 *)obj;
	char service[1024];
	char port[1024];

	if (sscanf(line, "%1023s %1023s", service, port) != 2)
		++errors, printf("Test%d: syntax error: '%s' (file %s line %d)\n", data1->test, line, path, (int)lineno);
	else if (strcmp(service, data1->pair[data1->i].service))
		++errors, printf("Test%d: expected service '%s', received '%s' (file %s line %d)\n", data1->test, data1->pair[data1->i].service, service, path, (int)lineno);
	else if (strcmp(port, data1->pair[data1->i].port))
		++errors, printf("Test%d: expected port '%s', received '%s' (file %s line %d)\n", data1->test, data1->pair[data1->i].port, port, path, (int)lineno);
	++data1->i;
}

static int config_test2(int test, const char *name)
{
	FILE *out = fopen(name, "w");

	if (!out)
	{
		++errors, printf("Test%d: failed to create file: '%s'\n", test, name);
		return 0;
	}

	fprintf(out, "%s", data2->text);
	fclose(out);
	return 1;
}

static void parse_test2(void *obj, const char *path, char *line, size_t lineno)
{
	Data2 *data2 = (Data2 *)obj;
	char word[8][1024];
	int words;

	words = sscanf(line, "%1023s %1023s %1023s %1023s %1023s %1023s %1023s %1023s", word[0], word[1], word[2], word[3], word[4], word[5], word[6], word[7]);

	for (data2->j = 0; data2->j < words; ++data2->j)
	{
		if (!data2->results[data2->i][data2->j])
		{
			++errors, printf("Test%d: too many words: '%s' (file %s line %d)\n", data2->test, line, path, (int)lineno);
			break;
		}

		if (strcmp(word[data2->j], data2->results[data2->i][data2->j]))
		{
			++errors;
			printf("Test%d: expected '%s', received '%s' (file %s line %d)\n", data2->test, data2->results[data2->i][data2->j], word[data2->j - 1], path, (int)lineno);
			break;
		}
	}

	++data2->i;
}

void term(int signo)
{
	daemon_close();
	exit(EXIT_SUCCESS);
}

int main(int ac, char **av)
{
	const char *config_name;
	char *cwd;
	char *core = "core";
	char *core2 = "daemon.core"; /* OpenBSD, FreeBSD */
	int facility = LOG_DAEMON | LOG_ERR;
	pid_t pid;
	int rc;
	uid_t uid, euid;
	gid_t gid, egid;
	int no_privileges = 0;
	int not_safe = 0;
	int not_root = 0;

	if (ac == 2 && !strcmp(av[1], "help"))
	{
		printf("usage: %s\n", *av);
		return EXIT_SUCCESS;
	}

	printf("Testing: %s\n", "daemon");

	/* Test (a bit) daemon_started_by_init() and daemon_started_by_inetd() */

	if ((rc = daemon_started_by_init()) != 0)
		++errors, printf("Test1: daemon_started_by_init() failed (ret %d, not %d)\n", rc, 0);

	if ((rc = daemon_started_by_inetd()) != 0)
		++errors, printf("Test2: daemon_started_by_inetd() failed (ret %d, not %d)\n", rc, 0);

	/* Test daemon_prevent_core() */

	unlink(core);
	unlink(core2);

	switch (pid = fork())
	{
		case -1:
		{
			++errors, printf("Test3: Failed to run test: fork: %s\n", strerror(errno));
			break;
		}

		case 0:
		{
			if (daemon_prevent_core() == -1)
			{
				printf("Test3: daemon_prevent_core() failed: %s\n", strerror(errno));
				return 1;
			}

			dump("");
		}

		default:
		{
			struct stat statbuf[1];
			int status;

			if (waitpid(pid, &status, 0) == -1)
			{
				printf("Test3: Failed to evaluate test: waitpid: %s\n", strerror(errno));
				break;
			}

#ifndef WCOREDUMP
#define WCOREDUMP(status) 0
#endif

			if (WIFEXITED(status) && WEXITSTATUS(status) != EXIT_SUCCESS)
				++errors;
			else if (WCOREDUMP(status) && (stat(core, statbuf) == 0 || stat(core2, statbuf)) == 0)
				++errors, printf("Test3: child dumped core\n");
			unlink(core);
			unlink(core2);
		}
	}

	/* Test daemon_revoke_privileges() if possible */

	uid = getuid();
	gid = getgid();
	euid = geteuid();
	egid = getegid();

	if (euid == uid && egid == gid)
		no_privileges = 1;
	else if (daemon_revoke_privileges() == -1 || geteuid() != getuid() || getegid() != getgid())
		++errors, printf("Test4: daemon_revoke_privileges() failed: %s\n", strerror(errno));

	/* Test daemon_become_user() if possible */

	if (uid)
		not_root = 1;
	else
	{
		switch (pid = fork())
		{
			case -1:
			{
				++errors, printf("Test5: Failed to run test: fork: %s\n", strerror(errno));
				break;
			}

			case 0:
			{
				gid_t gids[10];
				errno = 0;
				if (daemon_become_user(1, 1, NULL) == -1)
				{
					printf("Test5: daemon_become_user(1, 1, NULL) failed: %s\n", strerror(errno));
					return 1;
				}

				if (geteuid() != 1 || getuid() != 1 || getegid() != 1 || getgid() != 1)
				{
					printf("Test5: daemon_become_user(1, 1, NULL) failed: euid/egid = %d/%d, uid/gid = %d/%d\n", (int)geteuid(), (int)getegid(), (int)getuid(), (int)getgid());
					return 1;
				}

				if (getgroups(0, NULL) != 0 && (getgroups(10, gids) != 1 || gids[0] != getuid()))
				{
					printf("Test5: daemon_become_user(1, 1, NULL) failed: getgroups() = %d (not 0 or 1)\n", getgroups(0, NULL));
					return 1;
				}

				return 0;
			}

			default:
			{
				int status;

				if (waitpid(pid, &status, 0) == -1)
				{
					printf("Test5: Failed to evaluate test: waitpid: %s\n", strerror(errno));
					break;
				}

				if (WIFEXITED(status) && WEXITSTATUS(status) != EXIT_SUCCESS)
					++errors;
			}
		}
	}

	/* Test daemon_absolute_path() */

#define TEST_ABSOLUTE_PATH(i, path, abs_path) \
	{ \
		char *result = daemon_absolute_path(path); \
		if (!result) \
			++errors, printf("Test%d: absolute_path(%s) failed (%s)\n", (i), (path), strerror(errno)); \
		else if (strcmp(result, (abs_path))) \
		{ \
			struct stat result_status[1], abs_status[1]; \
			++errors, printf("Test%d: absolute_path(%s) failed (was %s, not %s)\n", (i), (path), result, (abs_path)); \
			printf("\n"); \
			if (stat(result, result_status) != -1 && stat(abs_path, abs_status) != -1 && result_status->st_ino == abs_status->st_ino) \
				printf("        But they have the same inode (%d)\n", (int)abs_status->st_ino); \
				printf("        Does your pwd return canonical paths?\n\n"); \
			free(result); \
		} \
	}

	/* We must be in a safe, writable directory to test relative paths */

	if (!(cwd = mem_create(limit_path(), char)))
		++errors, printf("Test6: Failed to run test: mem_create: %s\n", strerror(errno));
	else if (!getcwd(cwd, limit_path()))
		++errors, printf("Test6: Failed to run test: getcwd: %s\n", strerror(errno));
	else if (chdir("/etc") == -1)
		++errors, printf("Test6: Failed to run test: chdir: %s\n", strerror(errno));
	else
	{
		TEST_ABSOLUTE_PATH(6, ".", "/etc")
		TEST_ABSOLUTE_PATH(7, "..", "/")
		TEST_ABSOLUTE_PATH(8, "/", "/")
		TEST_ABSOLUTE_PATH(9, "/etc/passwd", "/etc/passwd")
		TEST_ABSOLUTE_PATH(10, "/.", "/")
		TEST_ABSOLUTE_PATH(11, "/..", "/")
		TEST_ABSOLUTE_PATH(12, "/./etc", "/etc")
		TEST_ABSOLUTE_PATH(13, "/../etc", "/etc")
		TEST_ABSOLUTE_PATH(14, "/etc/.././.././../usr", "/usr")
		TEST_ABSOLUTE_PATH(15, "../../../../../etc/././../etc/./.././etc", "/etc")
		TEST_ABSOLUTE_PATH(16, "././../../../../../etc/././.", "/etc")
		TEST_ABSOLUTE_PATH(17, "/etc/./sysconfig/./network-scripts/../blog/..", "/etc/sysconfig")
		TEST_ABSOLUTE_PATH(18, "/etc/./sysconfig/./network-scripts/../blog/../..", "/etc")
		TEST_ABSOLUTE_PATH(19, "passwd", "/etc/passwd")
		TEST_ABSOLUTE_PATH(20, "passwd/", "/etc/passwd")
		TEST_ABSOLUTE_PATH(21, "passwd////", "/etc/passwd")
		TEST_ABSOLUTE_PATH(22, "///////////////", "/")
		TEST_ABSOLUTE_PATH(23, "///////etc////////", "/etc")
		TEST_ABSOLUTE_PATH(24, "//////./.././..////..//", "/")
		chdir(cwd);
	}

	/* Test daemon_path_is_safe() */

#define TEST_PATH_IS_SAFE(i, path, safe, err, explanation) \
	{ \
		int rc; \
		char buf[128]; \
		errno = 0; \
		if ((rc = daemon_path_is_safe(path, buf, 128)) != (safe)) \
		{ \
			struct stat status[1]; \
			++errors, printf("Test%d: daemon_path_is_safe(%s) failed (ret %d, not %d) %s\n", (i), (path), rc, (safe), errno ? strerror(errno) : ""); \
			if (stat(ROOT_DIR, status) != -1 && status->st_mode & (S_IWGRP | S_IWOTH)) \
				printf("\n        No Wonder! Your %s directory is %s writable!!!\n\n", ROOT_DIR, (status->st_mode & S_IWOTH) ? "world" : "group"); \
		} \
		else if (rc == -1 && errno != (err)) \
			++errors, printf("Test%d: daemon_path_is_safe(%s) failed (errno was %d, not %d)\n", (i), (path), errno, (err)); \
		else if (rc == 0 && strcmp(buf + strlen(buf) - strlen(explanation), explanation)) \
			++errors, printf("Test%d: daemon_path_is_safe(%s) failed (explanation was \"%s\", not \"%s\")\n", (i), (path), buf, (explanation)); \
	}

	TEST_PATH_IS_SAFE(25, "/etc/passwd", 1, 0, "")
	TEST_PATH_IS_SAFE(26, "/tmp", 0, 0, "/tmp is group and world writable")
	TEST_PATH_IS_SAFE(27, "/nonexistent-path", -1, ENOENT, "")

	if (daemon_path_is_safe(".", NULL, 0) != 1)
		not_safe = 1;
	else
	{
		const char *sym_link = "daemon_path_is_safe.test";
		const char *sym_linked = "/tmp/daemon_path_is_safe.test";
		mode_t mask;
		int fd;

		/* Test absolute link from safe directory to unsafe directory */

		if ((fd = open(sym_linked, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR)) == -1)
			++errors, printf("Test28: Failed to run test: open(%s) failed %s\n", sym_linked, strerror(errno));
		else
		{
			close(fd);
			TEST_PATH_IS_SAFE(28, sym_linked, 0, 0, "/tmp is group and world writable")

			if (symlink(sym_linked, sym_link) == -1)
				++errors, printf("Test28: Failed to run test: symlink(%s, %s) failed %s\n", sym_linked, sym_link, strerror(errno));
			else
			{
				TEST_PATH_IS_SAFE(28, sym_link, 0, 0, "/tmp is group and world writable")

				if (unlink(sym_link) == -1)
					++errors, printf("Test28: Failed to unlink(%s): %s\n", sym_link, strerror(errno));
			}

			if (unlink(sym_linked) == -1)
				++errors, printf("Test28: Failed to unlink(%s): %s\n", sym_linked, strerror(errno));
		}

		/* Test relative symbolic link from safe directory to safe directory */

		if (mkdir("safedir", S_IRUSR | S_IWUSR | S_IXUSR) == -1)
			++errors, printf("Test29: Failed to run test: mkdir(%s) failed: %s\n", "safedir", strerror(errno));
		else
		{
			if (symlink("..", "safedir/safelink") == -1)
				++errors, printf("Test29: symlink(.., safedir/safelink) failed: %s\n", strerror(errno));
			else
			{
				TEST_PATH_IS_SAFE(29, "safedir/safelink", 1, 0, "")

				if (unlink("safedir/safelink") == -1)
					++errors, printf("Test29: Failed to unlink(safedir/safelink): %s\n", strerror(errno));
			}

			if (rmdir("safedir") == -1)
				++errors, printf("Test29: Failed to rmdir(safedir): %s\n", strerror(errno));
		}

		/* Test relative symbolic link from safe directory to unsafe directory */

		if (mkdir("safedir", S_IRUSR | S_IWUSR | S_IXUSR) == -1)
			++errors, printf("Test30: Failed to run test: mkdir(safedir) failed: %s\n", strerror(errno));
		else
		{
			mask = umask((mode_t)0);

			if (mkdir("unsafedir", S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP) == -1)
				++errors, printf("Test30: Failed to run test: mkdir(unsafedir) failed: %s\n", strerror(errno));
			else
			{
				if (symlink("../unsafedir", "safedir/unsafelink") == -1)
					++errors, printf("Test30: symlink(../unsafedir, safedir/unsafelink) failed: %s\n", strerror(errno));
				else
				{
					TEST_PATH_IS_SAFE(30, "safedir/unsafelink", 0, 0, "/unsafedir is group writable")

					if (unlink("safedir/unsafelink") == -1)
						++errors, printf("Test30: Failed to unlink(safedir/unsafelink): %s\n", strerror(errno));
				}

				if (rmdir("unsafedir") == -1)
					++errors, printf("Test30: Failed to rmdir(unsafedir): %s\n", strerror(errno));
			}

			if (rmdir("safedir") == -1)
				++errors, printf("Test30: Failed to rmdir(safedir): %s\n", strerror(errno));

			umask(mask);
		}

		/* Test relative symbolic link from unsafe directory to safe directory */

		mask = umask((mode_t)0);

		if (mkdir("unsafedir", S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP) == -1)
			++errors, printf("Test31: Failed to run test: mkdir(unsafedir) failed: %s\n", strerror(errno));
		else
		{
			if (symlink("unsafedir", "unsafelink") == -1)
				++errors, printf("Test31: symlink(../unsafedir, unsafelink) failed: %s\n", strerror(errno));
			else
			{
				if (mkdir("unsafelink/unsafedir", S_IRUSR | S_IWUSR | S_IXUSR) == -1)
					++errors, printf("Test31: Failed to run test: mkdir(unsafelink/unsafedir) failed: %s\n", strerror(errno));
				else
				{
					TEST_PATH_IS_SAFE(31, "unsafelink/unsafedir", 0, 0, "/unsafedir is group writable")

					if (rmdir("unsafelink/unsafedir") == -1)
						++errors, printf("Test31: Failed to rmdir(unsafelink/unsafedir): %s\n", strerror(errno));
				}

				if (unlink("unsafelink") == -1)
					++errors, printf("Test31: Failed to unlink(unsafelink): %s\n", strerror(errno));
			}

			if (rmdir("unsafedir") == -1)
				++errors, printf("Test31: Failed to rmdir(unsafedir): %s\n", strerror(errno));
		}

		umask(mask);
	}

	/* Test daemon_parse_config() */

	config_name = "daemon_parse_config.testfile";

	if (config_test1(32, config_name))
	{
		int errors_save = errors;
		data1->test = 32;
		daemon_parse_config(config_name, data1, parse_test1);
		if (errors == errors_save && data1->i != final_pair)
			++errors, printf("Test32: failed to parse entire config file\n");
		unlink(config_name);
	}

	if (config_test2(33, config_name))
	{
		int errors_save = errors;
		data2->test = 33;
		daemon_parse_config(config_name, data2, parse_test2);
		if (errors == errors_save && (data2->i != final_line || data2->j != final_word))
			++errors, printf("Test33: failed to parse entire config file\n");
		unlink(config_name);
	}

	/* Test daemon_init() and daemon_close() */

	switch (pid = fork())
	{
		case -1:
		{
			++errors, printf("Test34: Failed to run test: fork: %s\n", strerror(errno));
			break;
		}

		case 0:
		{
			if (signal_set_handler(SIGTERM, 0, term) == -1)
			{
				syslog(facility, "%s: Test34: signal_set_handler(SIGTERM) failed: %s", *av, strerror(errno));
				_exit(EXIT_FAILURE);
			}

			if (daemon_init(prog_basename(*av)) == -1)
			{
				syslog(facility, "%s: Test34: daemon_init(\"%s\") failed: %s", *av, prog_basename(*av), strerror(errno));
				_exit(EXIT_FAILURE);
			}

			syslog(facility, "%s succeeded", *av);

			if (kill(getpid(), SIGTERM) == -1)
				syslog(facility, "%s: Test34: kill(%d, SIGTERM) failed: %s", *av, pid, strerror(errno));

			signal_handle_all();

			/*
			** We can only get here if the signal hasn't arrived yet.
			** If so, exit anyway.
			*/

			syslog(facility, "%s: Test34: signal_handle_all() failed", *av);
			_exit(EXIT_SUCCESS);
		}

		default:
		{
			int status;

			if (waitpid(pid, &status, 0) == -1)
			{
				printf("Test34: Failed to evaluate test: waitpid: %s\n", strerror(errno));
				break;
			}

			if (WIFEXITED(status) && WEXITSTATUS(status) != EXIT_SUCCESS)
				++errors, printf("Test34: Failed to run test: signal_set_handler() failed\n");
		}
	}

	if (errors)
		printf("%d/34 tests failed\n", errors);
	else
		printf("All tests passed\n");

	printf("\n");
	printf("    Note: Can't verify syslog daemon.err output (don't know where it goes).\n");
	printf("    Look for \"%s succeeded\" (not \"%s failed\").\n", *av, *av);

	if (no_privileges)
	{
		printf("\n");
		printf("    Note: Can't test daemon_revoke_privileges().\n");
		printf("    Rerun test suid and/or sgid as someone other than the tester.\n");
	}

	if (not_safe)
	{
		printf("\n");
		printf("    Note: Can't perform all tests on daemon_path_is_safe().\n");
		printf("    Rerun test from a safe directory (writable by the tester).\n");
	}

	if (not_root)
	{
		printf("\n");
		printf("    Note: Can't test daemon_become_user().\n");
		printf("    Audit the code and rerun the test as root.\n");
	}

	return (errors == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

#endif

/* vi:set ts=4 sw=4: */
