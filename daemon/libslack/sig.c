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

I<libslack(sig)> - ISO C compliant signal handling module

=head1 SYNOPSIS

    #include <slack/std.h>
    #include <slack/sig.h>

    typedef void signal_handler_t(int signo);
    typedef void signal_siginfo_handler_t(int signo, siginfo_t *siginfo, void *context);

    int signal_set_handler(int signo, int flags, signal_handler_t *handler);
    int signal_set_siginfo_handler(int signo, int flags, signal_siginfo_handler_t *siginfo_handler);
    int signal_addset(int signo_handled, int signo_blocked);
    int signal_received(int signo);
    int signal_raise(int signo);
    int signal_handle(int signo);
    void signal_handle_all(void);

=head1 DESCRIPTION

This module provides functions for ISO C compliant signal handling. ISO C
compliant signal handlers may only set a single value of type
I<sig_atomic_t>. This is a very restrictive requirement. This module allows
you to specify unrestricted signal handlers while (almost) transparently
enforcing ISO C compliance.

When a handled signal arrives, an ISO C compliant signal handler is invoked
to merely record the fact that the signal was received. Then, in the main
thread of execution, when I<signal_handle(3)> or I<signal_handle_all(3)> is
invoked, the client supplied signal handlers for all signals received since
the last invocation of I<signal_handle(3)> or I<signal_handle_all(3)> are
invoked.

Since the user supplied signal handlers execute in the main thread on
execution, they are not subject to the normal restrictions on signal
handlers. Also, they will execute with the same signals blocked as the real
signal handler.

However, this indirection doesn't apply to the more dramatic signals (i.e.
C<SIGILL>, C<SIGABRT>, C<SIGFPE>, C<SIGSEGV>, C<SIGBUS> and C<SIGSYS>) whose
signal I<handler> functions are installed directly as the real signal
handlers. Signal I<siginfo> handler functions installed with
I<signal_set_siginfo_handler(3)> are installed directly as well. In general,
these I<handler> and I<siginfo_handler> functions probably won't be ISO C
compliant signal handler functions but as long as they are POSIX compliant
signal handler functions (which is far less restrictive) it will be fine.

=over 4

=cut

One of the nicest things about POSIX MT programming is how it simplifies
signal handling. A single thread can be devoted to handling signals
synchronously with I<sigwait(3)> while all other threads go about their
business, free from signals, and most importantly, free from the code
clutter of checking every blocking system call to see if it was interrupted
by a signal. To do this, block all signals in all threads except for the
signal-handling thread. Then that thread will be the only one to receive
signals, according to POSIX.

Unfortunately, if you have a Linux system, you may notice that the MT signal
handling is not POSIX compliant in this way. On the other hand, Linux
provides the I<signalfd(2)> system call which is even better. If you use
that, you won't need this module at all.

*/

#include "config.h"
#include "std.h"

#include "sig.h"
#include "err.h"

#ifdef NSIG
#define SIG_MAX NSIG
#else
#ifdef _NSIG
#define SIG_MAX _NSIG
#else
#define SIG_MAX 32
#endif
#endif

typedef struct real_signal_handler_t real_signal_handler_t;

struct real_signal_handler_t
{
	struct sigaction action[1];
	signal_handler_t *handler;
};

#ifndef TEST

static real_signal_handler_t g_handler[SIG_MAX];
static volatile sig_atomic_t g_received[SIG_MAX];

/*

C<void signal_catcher(int signo)>

This is an ISO C compliant signal handler function. It is used to catch all
signals. It records that the signal C<signo> was received.

*/

static void signal_catcher(int signo)
{
	++g_received[signo];
}

/*

=item C<int signal_set_handler(int signo, int flags, signal_handler_t *handler)>

Installs C<handler> as the signal handler function for the signal C<signo>.
C<flags> is used as the I<sa_flags> field of the C<struct sigaction *act>
argument to I<sigaction(2)>. The actual function that is set as the signal
handler is not C<handler>. It is an ISO C compliant signal handler function
that just records the fact that a signal was received. C<handler> will only
be invoked when the client invokes I<signal_handle(3)> or
I<signal_handle_all(3)> from the main thread of execution. So there are no
restrictions on C<handler>. When C<handler> is invoked, the C<signo> signal
will be blocked. Other signals can also be blocked when C<handler> is
invoked using I<signal_addset(3)> or I<sigaddset(3)>. Several signals do not
allow such treatment. Behaviour upon return from their handler function is
undefined (or defined, but not very pleasant). They are C<SIGILL>,
C<SIGABRT>, C<SIGFPE>, C<SIGBUS>, C<SIGSEGV> and C<SIGSYS>. Handler function
supplied for these signals are installed as the real signal handlers. On
success, returns C<0>. On error, returns C<-1> with C<errno> set
appropriately.

=cut

*/

int signal_set_handler(int signo, int flags, signal_handler_t *handler)
{
	real_signal_handler_t *h = &g_handler[signo];

	sigemptyset(&h->action->sa_mask);
	sigaddset(&h->action->sa_mask, signo);
	h->action->sa_flags = flags;

	if (handler == SIG_DFL || handler == SIG_IGN)
		h->action->sa_handler = handler;
	else
	{
		switch (signo)
		{
			case SIGILL:
			case SIGABRT:
			case SIGFPE:
			case SIGSEGV:
#ifdef SIGBUS
			case SIGBUS:
#endif
#ifdef SIGSYS
			case SIGSYS:
#endif
				h->action->sa_handler = handler;
				break;
			default:
				h->action->sa_handler = signal_catcher;
				break;
		}
	}

	h->handler = handler;
	g_received[signo] = 0;

	return sigaction(signo, h->action, NULL);
}

/*

=item C<int signal_set_siginfo_handler(int signo, int flags, signal_siginfo_handler_t *siginfo_handler)>

Installs C<siginfo_handler> as the signal I<siginfo> handler function for
the signal C<signo>. A I<siginfo> handler takes three arguments (C<int
signo>, C<siginfo_t *siginfo> and C<void *context>), rather than the usual
single argument (C<int signo>). C<flags> is used as the I<sa_flags> field of
the I<struct sigaction *act> argument to I<sigaction(2)> (combined with
C<SA_SIGINFO>). Unlike usual signal C<handler> functions, signal
I<siginfo_hndler> functions are installed directly. So there are no
restrictions on C<siginfo_handler>. When C<siginfo_handler> is invoked, the
C<signo> signal will be blocked. Other signals can also be blocked when
C<siginfo_handler> is invoked using I<signal_addset(3)> or I<sigaddset(3)>.
On success, returns C<0>. On error, returns C<-1> with C<errno> set
appropriately.

=cut

*/

int signal_set_siginfo_handler(int signo, int flags, signal_siginfo_handler_t *siginfo_handler)
{
#ifdef SA_SIGINFO
	real_signal_handler_t *h = &g_handler[signo];

	sigemptyset(&h->action->sa_mask);
	sigaddset(&h->action->sa_mask, signo);
	h->action->sa_flags = flags | SA_SIGINFO;
	h->action->sa_sigaction = siginfo_handler;

	h->handler = NULL;
	g_received[signo] = 0;

	return sigaction(signo, h->action, NULL);
#else
	return set_errno(ENOSYS);
#endif
}

/*

=item C<int signal_addset(int signo_handled, int signo_blocked)>

Adds C<signo_blocked> to the set of signals that will be blocked when the
handler for signal C<signo_handled> is invoked. This must not be called
before the call to I<signal_set_handler(3)> for C<signo_handled> which
initialises the signal set to include C<signo_handled>. On success, returns
C<0>. On error, returns C<-1> with C<errno> set appropriately.

=cut

*/

int signal_addset(int signo_handled, int signo_blocked)
{
	real_signal_handler_t *h = &g_handler[signo_handled];

	return sigaddset(&h->action->sa_mask, signo_blocked);
}

/*

=item C<int signal_received(int signo)>

Returns the number of times that the signal C<signo> has been received since
the last call to I<signal_handle(3)> with C<signo> as its argument
or I<signal_handle_all(3)>. On error (i.e. C<signo> is out of
range), returns C<-1> and sets C<errno> set to C<EINVAL>.

=cut

*/

int signal_received(int signo)
{
	if (signo < 0 || signo >= SIG_MAX)
		return set_errno(EINVAL);

	return g_received[signo];
}

/*

=item C<int signal_raise(int signo)>

Simulates the receipt of the signal specified by C<signo>. On success,
returns the number of unhandled C<signo> signals (including this one). On
error (i.e. if C<signo> is out of range), returns C<-1> and sets C<errno> to
C<EINVAL>.

=cut

*/

int signal_raise(int signo)
{
	if (signo < 0 || signo >= SIG_MAX)
		return set_errno(EINVAL);

	return ++g_received[signo];
}

/*

=item C<int signal_handle(int signo)>

Executes the installed signal handler for the signal C<signo>. The C<signo>
signal (and any others added with I<signal_addset(3)>) is blocked during the
execution of the signal handler. Clears the received status of the C<signo>
signal. On success, returns C<0>. On error, returns C<-1> with C<errno> set
appropriately.

=cut

*/

int signal_handle(int signo)
{
	real_signal_handler_t *h = &g_handler[signo];
	sigset_t origmask[1];

	if (!h->handler)
		return set_errno(EINVAL);

	if (sigprocmask(SIG_BLOCK, &h->action->sa_mask, origmask) == -1)
		return -1;

	h->handler(signo);
	g_received[signo] = 0;

	return sigprocmask(SIG_SETMASK, origmask, NULL);
}

/*

=item C<void signal_handle_all(void)>

Executes the installed signal handlers for all signals that have been
received since the last call to I<signal_handle(3)> or
I<signal_handle_all(3)>. During the execution of each signal handler, the
corresponding signal (and possibly others) will be blocked. Clears the
received status of all signals handled.

=cut

*/

void signal_handle_all(void)
{
	int signo;

	for (signo = 0; signo < SIG_MAX; ++signo)
		if (signal_received(signo))
			signal_handle(signo);
}

/*

=back

=head1 ERRORS

=over 4

=item C<EINVAL>

When a signal number argument is out of range.

=back

=head1 MT-Level

Unsafe

=head1 EXAMPLE

    #include <slack/std.h>
    #include <slack/sig.h>

    void hup(int signo)  { printf("SIGHUP received\n"); }
    void term(int signo) { printf("SIGTERM received\n"); exit(EXIT_SUCCESS); }

    int main(int ac, char **av)
    {
        if (signal_set_handler(SIGHUP, 0, hup) == -1)
            return EXIT_FAILURE;
        if (signal_set_handler(SIGTERM, 0, term) == -1)
            return EXIT_FAILURE;

        for (;;)
        {
            char mesg[BUFSIZ];
            ssize_t n;

            signal_handle_all();

            // Signals arriving here are lost

            while ((n = read(STDIN_FILENO, mesg, BUFSIZ)) > 0)
                fprintf(stderr, "%*.*s", n, n, mesg);

            if (n == -1 && errno == EINTR)
                continue;

            exit((n == -1) ? EXIT_FAILURE : EXIT_SUCCESS);
        }

        return EXIT_SUCCESS;
    }

=head1 SEE ALSO

I<libslack(3)>,
I<prog(3)>

=head1 AUTHOR

20201111 raf <raf@raf.org>

=cut

*/

#endif

#ifdef TEST

#include <fcntl.h>

#include <sys/wait.h>
#include <sys/stat.h>

const char * const results[3] =
{
	"Received SIGHUP\n",
	"Received SIGTERM\n",
	"Received SIGCHLD\n"
};

static void hup(int signo)
{
	printf("%s", results[0]);
}

static void term(int signo)
{
	printf("%s", results[1]);
	exit(EXIT_SUCCESS);
}

static void chld_siginfo(int signo, siginfo_t *siginfo, void *context)
{
	printf("%s from pid %d", results[2], (int)siginfo->si_pid);
}

static void child(void)
{
	char mesg[BUFSIZ];
	ssize_t n;

	/* We know signals have arrived so they will be handled immediately */

	for (;;)
	{
		signal_handle_all();

		/* Signals arriving here are lost */

		while ((n = read(STDIN_FILENO, mesg, BUFSIZ)) > 0)
			fprintf(stderr, "%*.*s", (int)n, (int)n, mesg);

		switch (n)
		{
			case -1:
			{
				if (errno != EINTR)
				{
					fprintf(stderr, "read error\n");
					exit(EXIT_FAILURE);
				}

				signal_handle_all();
				break;
			}

			default:
			{
				fprintf(stderr, "read = %d\n", (int)n);
				exit(EXIT_FAILURE);
			}
		}
	}
}

static int verify(int test, const char *name, const char *msg1, const char *msg2)
{
	char buf[BUFSIZ];
	int fd;
	ssize_t bytes;
	size_t msg1_length;

	if ((fd = open(name, O_RDONLY)) == -1)
	{
		printf("Test %d: failed to create sig file: %s (%s)\n", test, name, strerror(errno));
		return 1;
	}

	memset(buf, 0, BUFSIZ);
	bytes = read(fd, buf, BUFSIZ);
	close(fd);
	unlink(name);

	if (bytes == -1)
	{
		printf("Test %d: failed to read sig file: %s (%s)\n", test, name, strerror(errno));
		return 1;
	}

	msg1_length = strlen(msg1);

	if (strncmp(buf, msg1, msg1_length))
	{
		printf("Test %d: msg file produced incorrect input:\nshould be\n%s\nwas:\n%*.*s\n", test, msg1, (int)msg1_length, (int)msg1_length, buf);
		return 1;
	}

	if (strcmp(buf + msg1_length, msg2))
	{
		printf("Test %d: msg file produced incorrect input:\nshould be:\n%s\nwas:\n%s\n", test + 1, msg2, buf + msg1_length);
		return 1;
	}

	return 0;
}

int main(int ac, char **av)
{
	const char *out = "sig.out";
	int sync1[2];
	int sync2[2];
	pid_t pid;
	int errors = 0;

	if (ac == 2 && !strcmp(av[1], "help"))
	{
		printf("usage: %s\n", *av);
		return EXIT_SUCCESS;
	}

	printf("Testing: %s\n", "sig");

	if (signal_set_handler(SIGHUP, 0, hup) == -1)
		++errors, printf("Test1: failed to set the SIGHUP handler (%s)\n", strerror(errno));

	if (signal_set_handler(SIGTERM, 0, term) == -1)
		++errors, printf("Test2: failed to set the SIGTERM handler (%s)\n", strerror(errno));

	if (pipe(sync1) == -1)
	{
		printf("Failed to perform test - pipe() failed (%s)\n", strerror(errno));
		return EXIT_FAILURE;
	}

	if (pipe(sync2) == -1)
	{
		printf("Failed to perform test - pipe() failed (%s)\n", strerror(errno));
		return EXIT_FAILURE;
	}

	if (errors == 0)
	{
		switch (pid = fork())
		{
			case -1:
			{
				printf("Failed to perform test - fork() failed (%s)\n", strerror(errno));
				break;
			}

			case 0:
			{
				ssize_t rc;
				char ack;

				/* Send output to a file for the parent to verify */

				if (!freopen(out, "w", stdout))
				{
					fprintf(stderr, "Failed to perform test - freopen() stdout to %s failed (%s)\n", out, strerror(errno));
					exit(EXIT_FAILURE);
				}

				/* Inform the parent that we're ready to receive signals */

				close(sync1[0]);
				write(sync1[1], "1", 1);
				close(sync1[1]);

				/* Wait until the parent has sent the signals */

				close(sync2[1]);
				do { rc = read(sync2[0], &ack, 1); } while (rc == -1 && errno == EINTR);
				close(sync2[0]);

				/* Child begins, looping over signal_handle_all() and read() */

				child();
				break; /* unreached */
			}

			default:
			{
				int status;
				char ack;

				/* Wait until the child is ready to receive signals */

				close(sync1[1]);
				read(sync1[0], &ack, 1);
				close(sync1[0]);

				/* Send the signals */

				if (kill(pid, SIGHUP) == -1)
					++errors, printf("Test3: failed to perform test - kill(%d, HUP) failed (%s)\n", (int)pid, strerror(errno));

				if (kill(pid, SIGTERM) == -1)
					++errors, printf("Test4: failed to perform test - kill(%d, TERM) failed (%s)\n", (int)pid, strerror(errno));

				/* Inform the child that we've sent the signals */

				close(sync2[0]);
				write(sync2[1], &ack, 1);
				close(sync2[1]);

				/* Wait for the child to terminate */

				if (waitpid(pid, &status, 0) == -1)
				{
					fprintf(stderr, "Failed to evaluate test - waitpid(%d) failed (%s)\n", (int)pid, strerror(errno));
					break;
				}

				if (WIFSIGNALED(status) && WTERMSIG(status) != SIGABRT)
					fprintf(stderr, "Failed to evaluate test - child received signal %d\n", WTERMSIG(status));

				if (WIFEXITED(status) && WEXITSTATUS(status) != EXIT_SUCCESS)
					fprintf(stderr, "Failed to evaluate test - child exit status %d\n", WEXITSTATUS(status));

				/* Verify the output */

				errors += verify(5, out, results[0], results[1]);
				break;
			}
		}
	}

	if (signal_set_siginfo_handler(SIGCHLD, 0, chld_siginfo) == -1)
		++errors, printf("Test6: failed to set the SIGCHLD sigaction (%s)\n", strerror(errno));

	if (errors)
		printf("%d/6 tests failed\n", errors);
	else
		printf("All tests passed\n");

	return (errors == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

#endif

/* vi:set ts=4 sw=4: */
