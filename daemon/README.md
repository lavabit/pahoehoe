README
======
*daemon* - turns other processes into daemons

DESCRIPTION
===========
*Daemon* turns other processes into daemons. There are many tasks that need to
be performed to correctly set up a daemon process. This can be tedious.
*Daemon* performs these tasks for other processes. This is useful for writing
daemons in languages other than *C*, *C++* or *Perl* (e.g. `/bin/sh`, *Java*).

If you want to write daemons in languages that can link against *C* functions
(e.g. *C*, *C++*), see *libslack* which contains the core functionality of *daemon*.

DETAILS
=======
*Daemon* turns other processes into daemons. There are many tasks that need to
be performed to correctly set up a daemon process. This can be tedious.
*Daemon* performs these tasks for other processes.

The preparatory tasks that *daemon* performs for other processes are:

First revoke any setuid or setgid privileges that *daemon* may have been
installed with (by system administrators who laugh in the face of danger).

Process command line options.

Change the root directory if the `--chroot` option was supplied.

Change the process `uid` and `gid` if the `--user` option was supplied. Only
*root* can use this option. Note that the `uid` of *daemon* itself is changed,
rather than just changing the `uid` of the client process.

Read the system configuration file(s) (`/etc/daemon.conf` and
`/etc/daemon.conf.d/*` by default, or specified by the `--config` option)
unless the `--noconfig` option was supplied. Then read the user's personal
configuration file(s) (`~/.daemonrc` and `~/.daemonrc.d/*`), if any. Note:
The root directory and the user must be set before access to the
configuration file(s) can be attempted so neither `--chroot` nor `--user`
options may appear in the configuration file.

On *BSD* systems (except *macOS*), the system configuration file(s) are
`/usr/local/etc/daemon.conf` and `/usr/local/etc/daemon.conf.d/*` by
default.

On *macOS*, when installed via *macports*, the system configuration file(s)
are `/opt/local/etc/daemon.conf` and `/opt/local/etc/daemon.conf.d/*`.

Disable core file generation to prevent leaking potentially sensitive
information in daemons that are run by *root* (unless the `--core` option
was supplied).

Become a daemon process:

  * If *daemon* was not invoked by *init(8)* (i.e. pid 1) or *inetd(8)*:

    * Ignore `SIGHUP` signals in case the current process session leader
      terminates while attached to a controlling terminal, causing us to
      receive a `SIGHUP` signal before we start our own process session below.
      This can happen when *daemon* was invoked interactively via the shell
      builtin `exec`. When this initial process terminates below, the terminal
      emulator that invoked the shell also terminates, so *daemon* needs to
	  protect itself from that.

    * Background the process to lose process group leadership.

    * Start a new process session.

	* Background the process again to lose process session leadership. Under
	  *SVR4* this prevents the process from ever gaining a controlling
	  terminal. This is only necessary under *SVR4*, but is always done for
	  simplicity. Note that ignoring `SIGHUP` signals earlier means that
	  when the newly created process session leader terminates, then even if
	  it has a controlling terminal open, the newly backgrounded process
	  won't receive the corresponding `SIGHUP` signal that is sent to all
	  processes in the process session's foreground process group, because
	  it inherited signal dispositions from the initial process.

  * Change the current directory to the root directory so as not to hamper
    umounts.

  * Clear the *umask* to enable explicit file creation modes.

  * Close all open file descriptors. If *daemon* was invoked by *inetd(8)*,
	`stdin`, `stdout` and `stderr` are left open, because they are open to a
	socket.

  * Open `stdin`, `stdout` and `stderr` to `/dev/null`, in case something
    requires them to be open. Of course, this is not done if *daemon* was
    invoked by *inetd(8)*.

  * If the `--name` option was supplied, create and lock a file containing the
	process id of the *daemon* process. The presence of this locked file
	prevents two instances of a daemon with the same name from running at
	the same time. The default location of the pidfile is `/var/run` for
	*root* (`/etc` on *Solaris*, `/opt/local/var/run` on *macOS* when
	installed via *macports*), and `/tmp` for normal users. If the
	`--pidfiles` option was supplied, its argument specifies the directory
	in which the pidfile will be placed. If the `--pidfile` option was
	supplied, its argument specifies the name of the pidfile and the
	directory in which it will be placed.

If the `--umask` option was supplied, set the *umask* to its argument, which
must be a valid three-digit octal mode. Otherwise, set the *umask* to `022`,
to prevent accidentally creating group- or world-writable files.

Set the current directory if the `--chdir` option was supplied.

Spawn the client command and wait for it to terminate. The client command
can be specified as command line arguments, or as the argument of the
`--command` option. If both the `--command` option and command line
arguments are present, the client command is the result of appending the
command line arguments to the argument of the `--command` option.

If the `--output`, `--stdout` and/or `--stderr` option were supplied, the client's
standard output and/or standard error are captured by *daemon*, and sent to the
respective *syslog* destinations.

When the client terminates, *daemon* respawns it if the `--respawn` option
was supplied. If the client ran for less than `300` seconds (or the value of
the `--acceptable` option), then *daemon* sees this as a failure. It will
attempt to restart the client up to five times (or the value of the
`--attempts` option), before waiting for `300` seconds (or the value of the
`--delay` option). This gives the system administrator the chance to correct
whatever is preventing the client from running successfully without
overloading system resources. If the `--limit` option was supplied, *daemon*
terminates after the specified number of respawn attempt bursts. The default
is zero, which means never give up, never surrender.

When the client terminates, and the `--respawn` option wasn't supplied,
*daemon* terminates as well.

If the `--foreground` option was supplied, the client process is run as a
foreground process, and is not turned into a daemon at all. If *daemon* is
connected to a terminal, then the client process will also be connected to
it. If *daemon* is not connected to a terminal, but the client needs to be
connected to a terminal, use the `--pty` option.

If the `--bind` option was supplied, on systems with *systemd-logind* or
*elogind*, the client process will be terminated when the user logs out.

The `--stop` option sends a `SIGTERM` signal to a currently running named
*daemon*, which causes it to terminate its client process (with a `SIGTERM`
signal), and to then terminate itself.

The `--restart` option sends a `SIGUSR1` signal to a currently running named
*daemon*, which causes it to terminate its client process (with a `SIGTERM`
signal). If the named *daemon* was started with the `--respawn` option, it
will then restart the client. Otherwise, it will terminate itself.

The `--signal` option sends a user-specified signal directly to a currently
running named *daemon*'s client process.

The `--running` option reports whether or not a given named *daemon* process
is currently running.

The `--list` option reports all the currently running named *daemon* processes.

--------------------------------------------------------------------------------

    URL: http://libslack.org/daemon
    URL: http://raf.org/daemon
    GIT: https://github.com/raforg/daemon
    Date: 20210304
    Author: raf <raf@raf.org>

