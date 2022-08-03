/*
* daemon - http://libslack.org/daemon/
*
* Copyright (C) 1999-2004, 2010, 2020-2021 raf <raf@raf.org>
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
* 20210304 raf <raf@raf.org>
*/

/*

=head1 NAME

I<daemon> - turns other processes into daemons

=head1 SYNOPSIS

 usage: daemon [options] [--] [cmd arg...]
 options:

 -h, --help                - Print a help message then exit
 -V, --version             - Print a version message then exit
 -v, --verbose[=level]     - Set the verbosity level
 -d, --debug[=level]       - Set the debugging level

 -C, --config=path         - Specify the system configuration file
 -N, --noconfig            - Bypass the system configuration file
 -n, --name=name           - Guarantee a single named instance
 -X, --command="cmd"       - Specify the client command as an option
 -P, --pidfiles=/dir       - Override standard pidfile location
 -F, --pidfile=/path       - Override standard pidfile name and location

 -u, --user=user[:[group]] - Run the client as user[:group]
 -R, --chroot=path         - Run the client with path as root
 -D, --chdir=path          - Run the client in directory path
 -m, --umask=umask         - Run the client with the given umask
 -e, --env="var=val"       - Set a client environment variable
 -i, --inherit             - Inherit environment variables
 -U, --unsafe              - Allow execution of unsafe executable
 -S, --safe                - Disallow execution of unsafe executable
 -c, --core                - Allow core file generation
     --nocore              - Disallow core file generation (default)

 -r, --respawn             - Respawn the client when it terminates
 -a, --acceptable=#        - Minimum acceptable client duration (seconds)
 -A, --attempts=#          - Respawn # times on error before delay
 -L, --delay=#             - Delay between respawn attempt bursts (seconds)
 -M, --limit=#             - Maximum number of respawn attempt bursts
     --idiot               - Idiot mode (trust root with the above)

 -f, --foreground          - Run the client in the foreground
 -p, --pty[=noecho]        - Allocate a pseudo terminal for the client

 -B, --bind                - Stop when the user's last logind session ends

 -l, --errlog=spec         - Send daemon's error output to syslog or file
 -b, --dbglog=spec         - Send daemon's debug output to syslog or file
 -o, --output=spec         - Send client's output to syslog or file
 -O, --stdout=spec         - Send client's stdout to syslog or file
 -E, --stderr=spec         - Send client's stderr to syslog or file

     --ignore-eof          - After SIGCHLD ignore any client output
     --read-eof            - After SIGCHLD read any client output (default)

     --running             - Check if a named daemon is running
     --restart             - Restart a named daemon client
     --stop                - Terminate a named daemon process
     --signal=signame      - Send a signal to a named daemon
     --list                - Print a list of named daemons

=head1 DESCRIPTION

I<daemon(1)> turns other processes into daemons. There are many tasks that
need to be performed to correctly set up a daemon process. This can be
tedious. I<daemon> performs these tasks for other processes.

The preparatory tasks that I<daemon> performs for other processes are:

=over 4

=item *

First, revoke any setuid or setgid privileges that I<daemon> may have been
installed with (by system administrators who laugh in the face of danger).

=item *

Process command line options.

=item *

Change the root directory if the C<--chroot> option was supplied.

=item *

Change the process uid and gid if the C<--user> option was supplied. Only
I<root> can use this option. Note that the uid of I<daemon> itself is
changed, rather than just changing the uid of the client process.

=item *

Read the system configuration file(s) (C</etc/daemon.conf> and
C</etc/daemon.conf.d/*> by default, or specified by the C<--config> option),
unless the C<--noconfig> option was supplied. Then read the user's personal
configuration file(s) (C<~/.daemonrc> and C<~/.daemonrc.d/*>), if any.
Generic options that apply to all daemons are processed first, then options
that are specific to the daemon with the given name. B<Note: The root
directory and the user must be set before access to the configuration
file(s) can be attempted, so neither the C<--chroot> nor C<--user> options
may appear in the configuration file.>

On I<BSD> systems (except I<macOS>), the system configuration file(s) are
C</usr/local/etc/daemon.conf> and C</usr/local/etc/daemon.conf.d/*> by
default.

On I<macOS>, when installed via I<macports>, the system configuration
file(s) are C</opt/local/etc/daemon.conf> and
C</opt/local/etc/daemon.conf.d/*>.

=item *

Disable core file generation to prevent leaking potentially sensitive
information in daemons that are run by I<root> (unless the C<--core> option
was supplied).

=item *

Become a daemon process:

=over 4

=item *

If I<daemon> was not invoked by I<init(8)> (i.e. parent process id 1) or
I<inetd(8)> (i.e. C<stdin> is a socket):

=over 4

=item *

Ignore C<SIGHUP> signals in case the current process session leader
terminates while attached to a controlling terminal, causing us to
receive a C<SIGHUP> signal before we start our own process session below.

This can happen when I<daemon> was invoked interactively via the shell
builtin C<exec>. When this initial process terminates below, the terminal
emulator that invoked the shell also terminates, so I<daemon> need to
protect itself from that.

=item *

Background the process to lose process group leadership.

=item *

Start a new process session.

=item *

Background the process again to lose process session leadership. Under
I<SVR4>, this prevents the process from ever gaining a controlling terminal.
This is only necessary under I<SVR4>, but is always done for simplicity.
Note that ignoring C<SIGHUP> signals earlier means that when the newly
created process session leader terminates, then even if it has a controlling
terminal open, the newly backgrounded process won't receive the
corresponding C<SIGHUP> signal that is sent to all processes in the process
session's foreground process group, because it inherited signal dispositions
from the initial process.

=back

=item *

Change the current directory to the root directory so as not to hamper
umounts.

=item *

Clear the I<umask> to enable explicit file creation modes.

=item *

Close all open file descriptors. If I<daemon> was invoked by I<inetd(8)>,
C<stdin>, C<stdout> and C<stderr> are left open, because they are open to a
socket.

=item *

Open C<stdin>, C<stdout> and C<stderr> to C</dev/null>, in case something
requires them to be open. Of course, this is not done if I<daemon> was
invoked by I<inetd(8)>.

=item *

If the C<--name> option was supplied, create and lock a file containing the
process id of the I<daemon> process. The presence of this locked file
prevents two instances of a daemon with the same name from running at the
same time. The default location of the pidfile is C</var/run> for I<root>
(C</etc> on I<Solaris>, C</opt/local/var/run> on I<macOS> when installed via
I<macports>), and C</tmp> for normal users. If the C<--pidfiles> option was
supplied, its argument specifies the directory in which the pidfile will be
placed. If the C<--pidfile> option was supplied, its argument specifies the
name of the pidfile and the directory in which it will be placed.

=back

=item *

If the C<--umask> option was supplied, set the I<umask> to its argument,
which must be a valid three-digit octal mode. Otherwise, set the umask to
C<022>, to prevent clients from accidentally creating group- or
world-writable files.

=item *

Set the current directory if the C<--chdir> option was supplied.

=item *

Spawn the client command and wait for it to terminate. The client command
can be specified as command line arguments, or as the argument of the
C<--command> option. If both the C<--command> option and command line
arguments are present, the client command is the result of appending the
command line arguments to the argument of the C<--command> option.

=item *

If the C<--output>, C<--stdout> and/or C<--stderr> options were supplied,
the client's standard output and/or standard error are captured by
I<daemon>, and are sent to the respective I<syslog> destinations.

=item *

When the client terminates, I<daemon> respawns it if the C<--respawn> option
was supplied. If the client ran for less than C<300> seconds (or the value
of the C<--acceptable> option), then I<daemon> sees this as a failure. It
will attempt to restart the client up to five times (or the value of the
C<--attempts> option), before waiting for C<300> seconds (or the value of
the C<--delay> option). This gives the system administrator the chance to
correct whatever is preventing the client from running successfully without
overloading system resources. If the C<--limit> option was supplied,
I<daemon> terminates after the specified number of respawn attempt bursts.
The default is zero, which means never give up, never surrender.

When the client terminates, and the C<--respawn> option wasn't supplied,
I<daemon> terminates as well.

=item *

If I<daemon> receives a C<SIGTERM> signal (e.g. from a separate invocation
of I<daemon> with the C<--stop> option), it propagates the signal to the
client and then terminates.

=item *

If I<daemon> receives a C<SIGUSR1> signal (from a separate invocation of
I<daemon> with the C<--restart> option), it sends a C<SIGTERM> signal to the
client. If it was started with the C<--respawn> option, the client process
will be restarted after it is terminated by the C<SIGTERM> signal.

=item *

If the C<--foreground> option was supplied, the client process is run as a
foreground process, and is not turned into a daemon at all. If I<daemon> is
connected to a terminal, then the client process will also be connected to
it. If I<daemon> is not connected to a terminal, but the client needs to be
connected to a terminal, use the C<--pty> option.

=back

=head1 OPTIONS

=over 4

=item C<-h>, C<--help>

Display a help message and exit.

=item C<-V>, C<--version>

Display a version message and exit.

=item C<-v>I<[level]>, C<--verbose>I<[=level]>

Set the message verbosity level to I<level> (or 1 if I<level> is not
supplied). This only effects the C<--running> and C<--list> options.

=item C<-d>I<[level]>, C<--debug>I<[=level]>

Set the debug message level to I<level> (or 1 if I<level> is not supplied).
Level 1 traces high-level function calls. Level 2 traces lower-level
function calls and shows configuration information. Level 3 adds environment
variables. Level 9 adds every return value from I<select(2)>. Debug messages
are sent to the destination specified by the C<--dbglog> option (by default,
the I<syslog(3)> facility, C<daemon.debug>).

=item C<-C> I<path>, C<--config=>I<path>

Specify the system configuration file to use. By default,
C</etc/daemon.conf> is the system configuration file, if it exists and is
not group- or world-writable, and does not exist in a group- or
world-writable directory. The configuration file lets you predefine options
that apply to all clients, and to specifically named clients.

As well as the system configuration file, additional configuration files
will be read from the directory whose path matches the system configuration
file with C<".d"> appended to it (e.g. C</etc/daemon.conf.d>). Any file in
that directory whose name starts with a dot character (C<".">) is ignored.
The same checks as described above apply to these files as well.

On I<BSD> systems (except I<macOS>), the system configuration file(s) are
C</usr/local/etc/daemon.conf> and C</usr/local/etc/daemon.conf.d/*> by
default.

On I<macOS>, when installed via I<macports>, the system configuration
file(s) are C</opt/local/etc/daemon.conf> and
C</opt/local/etc/daemon.conf.d/*>.

=item C<-N>, C<--noconfig>

Bypass the system configuration files, C</etc/daemon.conf> and
C</etc/daemon.conf.d/*>. Only the user's C<~/.daemonrc> and
C<~/.daemonrc.d/*> configuration files will be read (if they exist).

=item C<-n> I<name>, C<--name=>I<name>

Create and lock a pidfile (I<name>C<.pid>), ensuring that only one daemon
with the given I<name> is active at the same time. The standard location of
the pidfile is C</var/run> for root (C</etc> on I<Solaris>,
C</opt/local/var/run> on I<macOS> when installed via I<macports>), and
C</tmp> for normal users. This location can be overridden with the
I<--pidfiles> option.

The name may only consist of the following characters:

    -._abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789

While a named daemon's client process is running, there will also be a
separate pidfile to record the process id of the client process. Its
filename will be the same as the I<daemon> pidfile's, except that the
filename extension will be C<.clientpid> rather than C<.pid>. The only
reason that there should be a C<.pid> file, with no C<.clientpid> file, is
during the delay between respawn attempts bursts.

=item C<-X> I<"cmd">, C<--command=>I<"cmd">

Specify the client command as an option. If a command is specified along
with its name in the configuration file, then a daemon can be invoked merely
by mentioning its name:

    daemon --name ftumch

B<Note:> If the client command is specified with the C<--command> option,
either in the configuration file, or on the command line, then any
additional command line arguments on the I<daemon> command line are appended
to the client command that is specified with the C<--command> option.

=item C<-P> I</dir>, C<--pidfiles=>I</dir>

Override the standard pidfile location. The standard pidfile location is
C</var/run> for root (C</etc> on I<Solaris>, C</opt/local/var/run> on
I<macOS> when installed via I<macports>), and C</tmp> for normal users.

This option only affects the C<--name> and C<--list> options. Use this
option if these standard locations are unacceptable, but make sure that you
don't forget where you put your pidfiles. This option is best used in
configuration files, or in shell scripts, rather than on an interactive
command line.

The pidfile location will be created automatically only if it is within the
user's home directory.

=item C<-F> I</path>, C<--pidfile=>I</path>

Override the standard pidfile name and location. The standard pidfile
location is described immediately above. The standard pidfile name is the
argument of the C<--name> option followed by C<.pid>. Use this option if the
standard pidfile name and location are unacceptable, but make sure that you
don't forget where you put your pidfile. This option is best used in
configuration files, or in shell scripts, rather than on an interactive
command line.

The pidfile location will be created automatically only if it is within the
user's home directory.

=item C<-u> I<user[:[group]]>, C<--user=>I<user[:[group]]>

Run the client as a different user (and group). This only works for I<root>.
If the argument includes a I<:group> specifier, I<daemon> will assume the
specified group and no other. Otherwise, I<daemon> will assume all groups
that the specified user is in. For backwards compatibility, C<"."> may be
used instead of C<":"> to separate the user and group but since C<"."> can
appear in user and group names, ambiguities can arise such as using
C<--user=>I<u.g> when users I<u> and I<u.g> and group I<g> all exist. With
such an ambiguity, I<daemon> will assume the user I<u> and group I<g>. Use
C<--user=>I<u.g:> instead for the other interpretation.

=item C<-R> I<path>, C<--chroot=>I<path>

Change the root directory to I<path> before running the client. On some
systems, only I<root> can do this. Note that the path to the client program
and to the configuration file (if any) must be relative to the new root
path.

=item C<-D> I<path>, C<--chdir=>I<path>

Change the current directory to I<path> before running the client. The
default current directory is the root directory (possibly after I<chroot>).

=item C<-m> I<umask>, C<--umask=>I<umask>

Change the umask to I<umask> before running the client. I<umask> must
be a valid octal mode. The default umask is C<022>.

=item C<-e> I<"var=val">, C<--env=>I<"var=val">

Set an environment variable for the client process. This option can be used
any number of times. If it is used, only the supplied environment variables
are passed to the client process. Otherwise, the client process inherits the
current set of environment variables.

=item C<-i>, C<--inherit>

Explicitly inherit environment variables. This is only needed when the
C<--env> option is used. When this option is used, the C<--env> option adds
to the inherited environment, rather than replacing it.

=item C<-U>, C<--unsafe>

Allow reading an unsafe configuration file, and allow the execution of an
unsafe executable. A configuration file or executable is considered to be
unsafe if it is group- or world-writable or is in a directory that is group-
or world-writable (following symbolic links). If an executable is a script
that is interpreted by another executable, then it is considered to be
unsafe if the interpreter is unsafe. If the interpreter is C</usr/bin/env>
(with an argument that is a command name to be searched for in C<$PATH>),
then that command must be safe. By default, I<daemon(1)> will refuse to read
an unsafe configuration file or to execute an unsafe executable when run by
I<root>. This option overrides that behaviour and hence should never be
used.

=item C<-S>, C<--safe>

Disallow reading an unsafe configuration file, and disallow the execution of
an unsafe executable. By default, I<daemon(1)> will allow reading an unsafe
configuration file, and allow the execution of an unsafe executable, when
run by normal users. This option overrides that behaviour.

=item C<-c>, C<--core>

Allow the client to create a core file. This should only be used for
debugging, as it could lead to security-related information disclosures by
daemons run by I<root>.

=item C<--nocore>

By default, clients are prevented from creating a core file. If the
C<--core> option has been used in a configuration file to apply to all named
daemons, then this option can be used to restore the default behaviour for
specific named daemons.

=item C<-r>, C<--respawn>

Respawn the client when it terminates. Without this option, the termination
of a client process causes I<daemon> itself to terminate as well.

=item C<-a> I<#>, C<--acceptable=>I<#>

Specify the minimum acceptable duration of a client process, in seconds.
This option can only be used with the C<--respawn> option. If a client
process terminates before this threshold is reached, then it is considered
to have failed. The default value is C<300> seconds. It cannot be set to
less than C<10> seconds, except by I<root> when used in conjunction with the
C<--idiot> option.

=item C<-A> I<#>, C<--attempts=>I<#>

Specify the number of attempts to respawn before delaying. This option can
only be used with the C<--respawn> option. The default value is C<5>. It
cannot be set to more than C<100> attempts, except by I<root> when used in
conjunction with the C<--idiot> option.

=item C<-L> I<#>, C<--delay=>I<#>

Specify the delay in seconds between each burst of respawn attempts. This
option can only be used with the C<--respawn> option. The default value is
C<300> seconds. It cannot be set to less than C<10> seconds except by
I<root> when used in conjunction with the C<--idiot> option.

=item C<-M> I<#>, C<--limit=>I<#>

Specify a limit to the number of respawn attempt bursts. This option can
only be used with the C<--respawn> option. The default value is C<0>, which
means no limit.

=item C<--idiot>

Turn on idiot mode in which I<daemon> will not enforce the minimum or
maximum values normally imposed on the C<--acceptable>, C<--attempts> and
C<--delay> options. The C<--idiot> option must appear before any of these
options. Only the I<root> user may use this option, because it can turn a
slight misconfiguration into a lot of wasted CPU energy and log messages,
somewhat akin to a self-inflicted denial of service.

Idiot mode also allows the I<root> user to expand environment variable
notation (e.g. C<$VAR> and C<${VAR}>) in command line option arguments, and
in configuration files. By default, internal environment variable expansion
is only performed for normal users. Note that this doesn't apply to any such
expansion performed earlier by the shell that invokes I<daemon(1)>. See the
C<EXPANSION> section below for more details.

=item C<-f>, C<--foreground>

Run the client in the foreground. The client is not turned into a daemon.

=item C<-p>I<[noecho]>, C<--pty>I<[=noecho]>

Connect the client to a pseudo terminal. This option can only be used with
the C<--foreground> option. This is the default when the C<--foreground>
option is supplied and I<daemon>'s standard input is connected to a
terminal. This option is only necessary when the client process must be
connected to a controlling terminal, but I<daemon> itself has been run
without a controlling terminal (e.g. from I<cron(8)> or a pipeline).

If the C<noecho> argument is supplied with this option, the client's side of
the pseudo terminal will be set to C<noecho> mode. Use this only if there
really is a terminal involved and input is being echoed twice.

=item C<-B>, C<--bind>

Automatically terminate the client process (and I<daemon(1)> itself) as soon
as the user has no I<systemd-logind(8)> (or I<elogind(8)>) user sessions. In
other words, automatically terminate when the user logs out. If the user has
no sessions to start with, the client process will be terminated
immediately.

This option is only available on I<Linux> systems that have either
I<systemd(1)> (e.g. I<Debian>) or I<elogind(8)> (e.g. I<Slackware>). On
systems with I<systemd(1)>, you could instead use a I<systemd> user service,
particularly if your user account is not allowed to have user services that
I<linger>.

=item C<-l> I<spec>, C<--errlog=>I<spec>

Send I<daemon>'s standard output and standard error to the syslog
destination or file that is specified by I<spec>. If I<spec> is a syslog
destination of the form C<"facility.priority">, then output is sent to
I<syslog(3)>. Otherwise, output is appended to the file whose path is given
in I<spec>. By default, output is sent to the syslog destination,
C<daemon.err>. See the C<MESSAGING> section below for more details.

=item C<-b> I<spec>, C<--dbglog=>I<spec>

Send I<daemon>'s debug output to the syslog destination or file that is
specified by I<spec>. If I<spec> is a syslog destination of the form
C<"facility.priority">, then output is sent to I<syslog(3)>. Otherwise,
output is appended to the file whose path is given in I<spec>. By default,
output is sent to the syslog destination C<daemon.debug>. See the
C<MESSAGING> section below for more details.

=item C<-o> I<spec>, C<--output=>I<spec>

Capture the client's standard output and standard error, and send it to the
syslog destination or file that is specified by I<spec>. If I<spec> is a
syslog destination of the form C<"facility.priority">, then output is sent
to I<syslog(3)>. Otherwise, output is appended to the file whose path is
given in I<spec>. By default, output is discarded unless the C<--foreground>
option is present, in which case, the client's stdout and stderr are
propagated to I<daemon>'s stdout and stderr, respectively. See the
C<MESSAGING> section below for more details.

=item C<-O> I<spec>, C<--stdout=>I<spec>

Capture the client's standard output, and send it to the syslog destination
or file that is specified by I<spec>. If I<spec> is a syslog destination of
the form C<"facility.priority">, then output is sent to I<syslog(3)>.
Otherwise, stdout is appended to the file whose path is given in I<spec>. By
default, stdout is discarded unless the C<--foreground> option is present,
in which case, the client's stdout is propagated to I<daemon>'s stdout. See
the C<MESSAGING> section below for more details.

=item C<-E> I<spec>, C<--stderr=>I<spec>

Capture the client's standard error, and send it to the syslog destination
or file that is specified by I<spec>. If I<spec> is a syslog destination of
the form C<"facility.priority">, then stderr is sent to I<syslog(3)>.
Otherwise, stderr is appended to the file whose path is given in I<spec>. By
default, stderr is discarded unless the C<--foreground> option is present,
in which case, the client's stderr is propagated to I<daemon>'s stderr. See
the C<MESSAGING> section below for more details.

=item C<--ignore-eof>

After receiving a C<SIGCHLD> signal due to a stopped or restarted client
process, don't bother reading the client's output until the end-of-file is
reached before reaping the client process's termination status with
I<wait(2)>. Normally, there will be little or no output after the C<SIGCHLD>
signal, because the client process has just terminated. However, the client
process might have its own child processes keeping its output open long
after its own termination. When this happens, by default, the client process
remains as a zombie process until its child processes terminate and close
the output. Waiting for the client's child processes to terminate before
considering the client stopped, and before restarting a new invocation,
might be desirable. If not, this option can be used to consider the client
process as being terminated as soon as the C<SIGCHLD> signal has been
received, and reaping its termination status with I<wait(2)> immediately.

=item C<--read-eof>

After receiving a C<SIGCHLD> signal due to a stopped or restarted client
process, continue reading the client's output until the end-of-file is
reached before reaping the client process's termination status with
I<wait(2)>. This is the default behaviour. Normally, there will be little or
no output after the C<SIGCHLD> signal, because the client process has just
terminated. However, the client process might have its own child processes
keeping its output open long after its own termination. When this happens,
the client process remains as a zombie process until its child processes
terminate and close the output. Waiting for the client's child processes to
terminate before considering the client stopped, and before restarting a new
invocation, might be desirable. If so, but the C<--ignore-eof> option has
been used in a configuration file to apply to all named daemons, then this
option can be used to restore the default behaviour for specific named
daemons.

=item C<--running>

Check whether or not a named daemon is running, then I<exit(3)> with
C<EXIT_SUCCESS> if the named daemon is running or C<EXIT_FAILURE> if it
isn't.

If the C<--verbose> option is supplied, print a message before exiting. If
both the named daemon and its client process are running, the output will
look like this, showing both process IDs:

    daemon:  name is running (pid 7455) (clientpid 7457)

If the named daemon is running but its client process is not (there might be
a delay between respawn attempt bursts), the output will look like this,
showing only the daemon process's ID:

    daemon:  name is running (pid 7455) (client is not running)

If the named daemon is not running at all, the output will look
like this:

    daemon:  name is not running

This option can only be used with the C<--name> option. Note that the
C<--chroot>, C<--user>, C<--name>, C<--pidfiles> and C<--pidfile> (and
possibly C<--config>) options must be the same as for the target daemon.

=item C<--restart>

Instruct a named daemon to terminate and restart its client process, by
sending it a C<SIGUSR1> signal. This will cause the named daemon to send its
client process a C<SIGTERM> signal to stop it. If the named daemon had been
started with the C<--restart> option, the named daemon will then restart its
client process. Otherwise, this has the same effect as the C<--stop> option,
and the named daemon's client process is not restarted.

This option can only be used with the C<--name> option. Note that the
C<--chroot>, C<--user>, C<--name>, C<--pidfiles> and C<--pidfile> (and
possibly C<--config>) options must be the same as for the target daemon.

=item C<--stop>

Stop a named daemon by sending it a C<SIGTERM> signal. This will cause the
named daemon to send its client process a C<SIGTERM> option and then exit.

This option can only be used with the C<--name> option. Note that the
C<--chroot>, C<--user>, C<--name>, C<--pidfiles> and C<--pidfile> (and
possibly C<--config>) options must be the same as for the target daemon.

=item C<--signal=>I<signame>

Send the given signal to a named daemon's client process. The signal can be
specified either by number or by name (with or without the "sig" prefix).
Any signal may be sent. However, the named daemon's client process might be
ignoring some signals. For example, C<SIGHUP> will be ignored by default
unless the client process has installed a signal handler for it.

The known list of signals are: C<hup>, C<int>, C<quit>, C<ill>, C<trap>,
C<abrt>, C<iot>, C<bus>, C<fpe>, C<kill>, C<usr1>, C<segv>, C<usr2>,
C<pipe>, C<alrm>, C<term>, C<stkflt>, C<cld>, C<chld>, C<cont>, C<stop>,
C<tstp>, C<ttin>, C<ttou>, C<urg>, C<xcpu>, C<xfsz>, C<vtalrm>, C<prof>,
C<winch>, C<poll>, C<io>, C<pwr>, C<sys>, C<emt> and C<info>. Not all of
them are available on all platforms.

=item C<--list>

Print a list of the currently running named daemons whose pidfiles are in
the applicable pidfile directory which will either be the default (i.e.
C</var/run> for I<root> (C</etc> on I<Solaris>, C</opt/local/var/run> on
I<macOS> when installed via I<macports>), and C</tmp> for normal users), or
it will be specified by the C<--pidfiles> option. Then exit.

Without the C<--verbose> option, this will only list the names of daemons
whose pidfiles are locked, as this implies that the corresponding daemon
must still be running. Note that pidfiles for daemons that were not started
by I<daemon(1)> might not be locked. An unlocked pidfile might indicate that
I<daemon(1)> has died unexpectedly, or it might just be a pidfile for a
daemon that was not started by I<daemon(1)>. If this might lead to
confusion, you might want to consider using a dedicated pidfiles directory
for named daemons started by I<daemon(1)>, and leave the default pidfiles
directories for other daemons that were started independently of
I<daemon(1)>.

With the C<--verbose> option, the items in the list will look like the
output of the C<--running> option with C<--verbose>, but with more detail.

If there are no pidfiles at all, the output will look like this:

    No named daemons are running

If a pidfile is locked, and there is a corresponding client pidfile, that
indicates that the named daemon and its client are both running, and the
output will look like this, showing both process IDs:

    name is running (pid ####) (client pid ####)

If a pidfile is locked, but there is no client pidfile, that indicates that
the named daemon is running, but its client is not (e.g. during a delay
between respawn attempt bursts when the client is failing to start
successfully), and the output will look like one of the following three
options:

When we can tell that the pidfile is for a process whose executable name is
I<daemon>:

    name is running (pid ####) (client is not running)

When we can tell that the pidfile is for a process whose executable name is
something other than I<daemon> (i.e. is independent of I<daemon(1)>):

    name is running (pid ####) (independent)

When it's not possible to determine the name of the executable associated
with the I<pidfile> (i.e. On systems other than I<Linux> without a C</proc>
file system):

    name is running (pid ####) (client is not running or is independent)

If a pidfile is not locked, and the applicable pidfiles directory is the
default, that indicates either that the daemon has unexpectedly terminated,
or just that the pidfile is for a daemon that was not started by
I<daemon(1)>, and the output will look like this:

    name is not running (or is independent)

If a pidfile is not locked, and the applicable pidfiles directory is not the
default, then it is assumed that all pidfiles are for daemons that were
started by I<daemon(1)>, and the output will look like this:

    name is not running

=back

As with all other programs, a C<--> argument signifies the end of options.
Any options that appear on the command line after C<--> are part of the
client command.

=head1 EXPANSION

Some simple shell-like expansion is performed internally on the arguments of
the command line options with a text argument (but not the options with a
numeric argument).

Environment variable notation, such as C<$VAR> or C<${VAR}>, is expanded.
Then user home directory notation, such as C<~> or C<~user>, is expanded.
File name expansion (i.e. globbing) is NOT performed internally. Neither are
any of your login shell's other wonderful expansions. This is very basic.

This might not be of much use on the command line, since I<daemon> is
normally invoked via a shell, which will first perform all of its usual
expansions. It might even be undesirable to perform expansion internally
after the shell has already done so (e.g. if you refer to any directory
names that actually contain the C<'$'> character, or if you use any
environment variables whose values contain the C<'$'> character, which is
unlikely).

But it can be useful in configuration files. See the C<FILES> section below
for more details. It can also be useful when I<daemon> is invoked directly
by another program without the use of a shell.

By default, environment variable expansion is not performed for the I<root>
user, even if the environment variable was defined in the configuration
files. The C<--idiot> option can be used to change this behaviour, and allow
the expansion of environment variables for the I<root> user. Home directory
notation expansion is performed for all users.

=head1 FILES

C</etc/daemon.conf>, C</etc/daemon.conf.d/*> - system-wide default options

C</usr/local/etc/daemon.conf>, C</usr/local/etc/daemon.conf.d/*> -
system-wide default options on I<BSD> systems (except I<macOS>).

C</opt/local/etc/daemon.conf>, C</opt/local/etc/daemon.conf.d/*> -
system-wide default options on I<macOS> when installed via I<macports>.

C<~/.daemonrc>, C<~/.daemonrc.d/*> - user-specific default options

Each line of the configuration file is either an environment variable
definition, or a configuration directive.

Environment variable definitions consist of the variable name, followed
immediately by C<'='> and the value of the variable. They look like they do
in shell, except that there is no quoting or other shell syntax. Environment
variable values can include simple environment variable notation (e.g.
C<$VAR> or C<${VAR}>), and user home directory notation (e.g. C<~> or
C<~user>). These will be expanded internally by I<daemon>. See the
C<EXPANSION> section above for more details.

Note that any environment variables that are defined in the configuration
file, which are subsequently used explicitly in another environment variable
definition or in an option argument, will have these expansions performed
multiple times. Avoid environment variables whose values can change again if
expansion is performed multiple times.

Example:

    PATH=/usr/bin:/usr/sbin:$HOME/bin:~app/bin
    PIDFILES=~/.run

Configuration directives consist of a client name (for options that apply to
a single client), or C<'*'> (for generic options that apply to all clients),
followed by spaces and/or tabs, followed by a comma-separated list of
options. Any option arguments must not contain any commas. The commas that
separate options can have spaces and tabs before and after them. Option
arguments that are text (but not numbers) can include simple environment
variable notation (e.g. C<$VAR> or C<${VAR}>), and user home directory
notation (e.g. C<~> or C<~user>). These will be expanded internally by
I<daemon>. See the C<EXPANSION> section above for more details.

Blank lines and comments (C<'#'> to end of the line) are ignored. Lines can
be continued with a C<'\'> character at the end of the line.

Example:

    *       errlog=daemon.err,output=local0.err,core
    test1   syslog=local0.debug,debug=9,verbose=9,respawn
    test2   syslog=local0.debug,debug=9, \
            verbose=9,respawn, \
            pidfiles=$PIDFILES

The command line options are processed first, to look for a C<--config>
option. If no C<--config> option was supplied, the default configuration
files, C</etc/daemon.conf> and C</etc/daemon.conf.d/*>, are used. On I<BSD>
systems (except I<macOS>), the default configuration files are
C</usr/local/etc/daemon.conf> and C</usr/local/etc/daemon.conf.d/*>. On
I<macOS> when installed via I<macports>, the default configuration files are
C</opt/local/etc/daemon.conf> and C</opt/local/etc/daemon.conf.d/*>.
If the user has their own configuration files, C<~/.daemonrc> and
C<~/.daemonrc.d/*>, they are also used.

If the configuration files contain any generic (C<'*'>) entries, their
options are applied in order of appearance. If the C<--name> option was
supplied, and the configuration files contain any entries for the given
name, those options are then applied in order of appearance.

Finally, the command line options are applied again. This ensures that any
generic options apply to all clients by default. Client-specific options
override generic options. User options override system-wide options. Command
line options override everything else.

Note that the configuration files are not opened and read until after any
C<--chroot> and/or C<--user> command line options are processed. This means
that the configuration file paths and the client's file path must be
relative to the C<--chroot> argument. It also means that the configuration
files and the client executable must be readable/executable by the user
specified by the C<--user> argument. It also means that the C<--chroot> and
C<--user> options must not appear in the configuration file. Also note that
the C<--name> option must not appear on the right hand side in the
configuration file either.

=head1 MESSAGING

The C<--errlog>, C<--dbglog>, C<--output>, C<--stdout> and C<--stderr>
options all take an argument that can be either a syslog destination of the
form C<"facility.priority"> or the path to a file. Here are the lists of
syslog facilities and priorities:

  Facilities:
  kern, user, mail, daemon, auth, syslog, lpr, news, uucp, cron,
  local0, local1, local2, local3, local4, local5, local6, local7.

  Priorities:
  emerg, alert, crit, err, warning, notice (on some systems), info, debug.

If the path to a file is supplied instead, bear in mind the fact that
I<daemon(1)> changes to the root directory by default, and so the file path
should be an absolute path (or relative to the C<--chroot> and/or C<--chdir>
option argument). Otherwise, I<daemon(1)> will attempt to create the file
relative to its current directory. You might not have permissions to do
that, or want to even if you do.

=head1 CAVEAT

Clients can only be restarted if they were started with the C<--respawn>
option. Using C<--restart> on a non-respawning daemon client is equivalent
to using C<--stop>. If you try to restart a named daemon, and it stops
instead, then it probably wasn't started with the C<--respawn> option.

Clients that are run in the foreground with a pseudo terminal don't respond
to job control (i.e. suspending with Control-Z doesn't work). This is
because the client belongs to an orphaned process group (it starts in its
own process session), so the kernel won't send it C<SIGSTOP> signals.
However, if the client is a shell that supports job control, then its
subprocesses can be suspended.

In KDE, if you use C<"exec daemon"> (or just C<"exec"> without C<daemon>) in
a shell, to run a KDE application, you might find that the KDE application
sometimes doesn't run. This problem has only been seen with I<konsole(1)>,
but it might happen with other KDE applications as well. Capturing the
standard error of the KDE application might show something like:

  unnamed app(9697): KUniqueApplication: Registering failed!
  unnamed app(9697): Communication problem with  "konsole" , it probably crashed.
  Error message was:  "org.freedesktop.DBus.Error.ServiceUnknown" : " "The name
                      org.kde.konsole was not provided by any .service files"

A workaround seems to be to delay the termination of the initial
I<daemon(1)> process by at least 0.4 seconds. To make this happen, set the
environment variable C<DAEMON_INIT_EXIT_DELAY_MSEC> to the number of
milliseconds by which to delay. For example:
C<DAEMON_INIT_EXIT_DELAY_MSEC=400>. Or you could just avoid using C<exec>
when starting I<KDE> applications.

On I<Linux> systems that have I<systemd(1)> or I<elogind(8)>, you might find
that your I<daemon> processes and their client processes are terminated when
you logout, even though they are in a different process session, and so
should be unaffected. This is because I<systemd> has the ability to
terminate all of your processes when you logout. Luckily, this feature is
turned off by default in some I<Linux> distributions. However, if it is on,
you can turn it off by adding the following line to
C</etc/systemd/logind.conf> (or C</etc/elogind/logind.conf>):

  KillUserProcesses=no

=head1 SEE ALSO

I<libslack(3)>,
I<daemon(3)>,
I<coproc(3)>,
I<pseudo(3)>,
I<init(8)>,
I<inetd(8)>,
I<fork(2)>,
I<umask(2)>,
I<setsid(2)>,
I<chdir(2)>,
I<chroot(2)>,
I<setrlimit(2)>,
I<setgid(2)>,
I<setuid(2)>,
I<setgroups(2)>,
I<initgroups(3)>,
I<syslog(3)>,
I<kill(2)>,
I<wait(2)>,
I<systemd-logind(8)>,
I<elogind(8)>

=head1 AUTHOR

20210304 raf <raf@raf.org>

=cut

*/

#ifndef _BSD_SOURCE
#define _BSD_SOURCE /* For SIGWINCH and CEOF on OpenBSD-4.7 */
#endif

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE /* New name for _BSD_SOURCE */
#endif

#ifndef __BSD_VISIBLE
#define __BSD_VISIBLE 1 /* For SIGWINCH on FreeBSD-8.0 */
#endif

#ifndef _NETBSD_SOURCE
#define _NETBSD_SOURCE /* For CEOF, chroot() on NetBSD-5.0.2 */
#endif

#include <slack/std.h>

#include <pwd.h>
#include <grp.h>
#include <fcntl.h>
#include <syslog.h>
#ifdef _POSIX_SOURCE
#undef _POSIX_SOURCE /* For CEOF on FreeBSD-8.0 */
#define _RESTORE_POSIX_SOURCE
#endif
#include <termios.h>
#ifdef _RESTORE_POSIX_SOURCE
#define _POSIX_SOURCE
#endif
#ifdef HAVE_SYS_TTYDEFAULTS_H /* For CEOF in musl libc (Linux only) */
#include <sys/ttydefaults.h>
#endif
#include <dirent.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/select.h>

#include <slack/prog.h>
#include <slack/daemon.h>
#include <slack/coproc.h>
#include <slack/sig.h>
#include <slack/err.h>
#include <slack/lim.h>
#include <slack/mem.h>
#include <slack/msg.h>
#include <slack/list.h>
#include <slack/str.h>
#include <slack/fio.h>

#include "config.h"

#ifdef HAVE_LOGIND
#include <systemd/sd-login.h>
#endif

/* Configuration file entries */

typedef struct Config Config;

struct Config
{
	char *name;
	List *options;
};

#ifndef RESPAWN_ACCEPTABLE
#define RESPAWN_ACCEPTABLE 300
#endif

#ifndef RESPAWN_ACCEPTABLE_MIN
#define RESPAWN_ACCEPTABLE_MIN 10
#endif

#ifndef RESPAWN_ATTEMPTS
#define RESPAWN_ATTEMPTS 5
#endif

#ifndef RESPAWN_ATTEMPTS_MIN
#define RESPAWN_ATTEMPTS_MIN 0
#endif

#ifndef RESPAWN_ATTEMPTS_MAX
#define RESPAWN_ATTEMPTS_MAX 100
#endif

#ifndef RESPAWN_DELAY
#define RESPAWN_DELAY 300
#endif

#ifndef RESPAWN_DELAY_MIN
#define RESPAWN_DELAY_MIN 10
#endif

#ifndef RESPAWN_LIMIT
#define RESPAWN_LIMIT 0
#endif

#ifndef RESPAWN_LIMIT_MIN
#define RESPAWN_LIMIT_MIN 0
#endif

#ifndef PTY_DEVICE_NAME_SIZE
#define PTY_DEVICE_NAME_SIZE 64
#endif

#ifndef CONFIG_PATH
#define CONFIG_PATH "/etc/daemon.conf"
#endif

#ifndef CONFIG_DIR_PATH_SUFFIX
#define CONFIG_DIR_PATH_SUFFIX ".d"
#endif

#ifndef CONFIG_PATH_USER
#define CONFIG_PATH_USER ".daemonrc"
#endif

#ifndef STATUS_BUFLEN
#define STATUS_BUFLEN 64
#endif

#ifndef DEFAULT_ROOT_PATH
#define DEFAULT_ROOT_PATH "/bin:/usr/bin"
#endif

#ifndef DEFAULT_USER_PATH
#define DEFAULT_USER_PATH ":/bin:/usr/bin"
#endif

/* Global variables */

extern char **environ;

static struct
{
	int ac;            /* number of command line arguments */
	char **av;         /* the command line arguments */
	char **cmd;        /* command vector to execute (prefixed by name) */
	char *cmdpath;     /* executable command path (execve filename argument) */
	char *name;        /* the daemon's name to use for the locked pidfile */
	char *daemon_init_name; /* the name argument for daemon_init() */
	char *pidfiles;    /* location of the pidfile */
	char *pidfile;     /* absolute path for the pidfile */
	char *user;        /* name of user to run as */
	char *group;       /* name of group to run as */
	char userbuf[BUFSIZ];  /* buffer to store the user name */
	char groupbuf[BUFSIZ]; /* buffer to store the group name */
	char *chroot;      /* name of root directory to run under */
	char *chdir;       /* name of directory to change to */
	char *command;     /* the client command as a string */
	mode_t umask;      /* set umask to this */
	int init_groups;   /* initgroups(3) if group not specified */
	uid_t initial_uid; /* the uid when the program started */
	uid_t uid;         /* run the client as this user */
	gid_t gid;         /* run the client as this group */
	List *env;         /* client environment variables */
    char **environ;    /* client environment */
	int inherit;       /* inherit environment variables? */
	int respawn;       /* respawn the client process when it terminates? */
	int acceptable;    /* minimum acceptable client duration in seconds */
	int attempts;      /* number of times to attempt respawning before delay */
	int delay;         /* delay in seconds between respawn attempt bursts */
	int limit;         /* number of respawn attempt bursts */
	int idiot;         /* idiot mode */
	int attempt;       /* spawn attempt counter */
	int burst;         /* respawn attempt burst counter */
	int foreground;    /* run the client in the foreground? */
	int pty;           /* allocate a pseudo terminal for the client? */
	int noecho;        /* set client pty to noecho mode? */
	int bind;          /* bind the daemon to the user's logind session? */
#ifdef HAVE_LOGIND
	sd_login_monitor *logind_monitor; /* The systemd-logind/elogind monitor object */
	int logind_monitor_fd;            /* The systemd-logind/elogind monitor file descriptor */
#endif
	int core;          /* do we allow core file generation? */
	int unsafe;        /* executable unsafe executables as root? */
	int safe;          /* do not execute unsafe executables? */
	char *client_out;  /* syslog/file spec for client stdout */
	char *client_err;  /* syslog/file spec for client stderr */
	char *daemon_err;  /* syslog/file spec for daemon output */
	char *daemon_dbg;  /* syslog/file spec for daemon debug output */
	int client_outlog; /* syslog facility for client stdout */
	int client_errlog; /* syslog facility for client stderr */
	int daemon_errlog; /* syslog facility for daemon output */
	int daemon_dbglog; /* syslog facility for daemon debug output */
	int client_outfd;  /* file descriptor for client stdout */
	int client_errfd;  /* file descriptor for client stderr */
	char *config;      /* name of the config file to use - /etc/daemon.conf */
	int noconfig;      /* bypass the system configuration file? */
	int read_eof;      /* read_eof mode (after SIGCHLD) */
	pid_t pid;         /* the pid of the client process to run as a daemon */
	int in;            /* file descriptor for client stdin */
	int out;           /* file descriptor for client stdout */
	int err;           /* file descriptor for client stderr */
	int pty_user_fd;   /* user side of the pseudo terminal */
	char pty_device_name[PTY_DEVICE_NAME_SIZE]; /* pseudo terminal device name */
	size_t pty_device_name_size;                /* size of g.pty_device_name */
	int stop;          /* stop a named daemon? */
	int running;       /* check whether or not a named daemon is running? */
	int restart;       /* restart a named daemon? */
	time_t spawn_time; /* when did we last spawn the client? */
	int done_name;     /* have we already set the name? */
	int done_chroot;   /* have we already set the root directory? */
	int done_user;     /* have we already set the user id? */
	int done_config;   /* have we already processed the configuration file? */
	struct termios stdin_termios; /* stdin's terminal attributes */
	struct winsize stdin_winsize; /* stdin's terminal window size */
	int stdin_isatty;             /* is stdin a terminal? */
	int stdin_eof;                /* has stdin received eof? */
	int terminated;               /* have we received a term signal? */
	int received_sigchld;         /* have we received a chld signal? */
	char *signame;                /* name of the signal to send */
	int signo;                    /* number of the signal to send */
	int list;                     /* are we listing all currently running daemons? */
}
g =
{
	0,                      /* ac */
	null,                   /* av */
	null,                   /* cmd */
	null,                   /* cmdpath */
	null,                   /* name */
	null,                   /* daemon_init_name */
	null,                   /* pidfiles */
	null,                   /* pidfile */
	null,                   /* user */
	null,                   /* group */
	{ 0 },                  /* userbuf */
	{ 0 },                  /* groupbuf */
	null,                   /* chroot */
	null,                   /* chdir */
	null,                   /* command */
	S_IWGRP | S_IWOTH,      /* umask */
	0,                      /* init_groups */
	0,                      /* initial_uid */
	0,                      /* uid */
	0,                      /* gid */
	null,                   /* env */
	null,                   /* environ */
	0,                      /* inherit */
	0,                      /* respawn */
	RESPAWN_ACCEPTABLE,     /* acceptable */
	RESPAWN_ATTEMPTS,       /* attempts */
	RESPAWN_DELAY,          /* delay */
	RESPAWN_LIMIT,          /* limit */
	0,                      /* idiot */
	0,                      /* attempt */
	0,                      /* burst */
	0,                      /* foreground */
	0,                      /* pty */
	0,                      /* noecho */
	0,                      /* bind */
#ifdef HAVE_LOGIND
	null,                   /* logind_monitor */
	-1,                     /* logind_monitor_fd */
#endif
	0,                      /* core */
	0,                      /* unsafe */
	0,                      /* safe */
	null,                   /* client_out */
	null,                   /* client_err */
	null,                   /* daemon_err */
	null,                   /* daemon_dbg */
	0,                      /* client_outlog */
	0,                      /* client_errlog */
	LOG_DAEMON | LOG_ERR,   /* daemon_errlog */
	LOG_DAEMON | LOG_DEBUG, /* daemon_dbglog */
	-1,                     /* client_outfd */
	-1,                     /* client_errfd */
	null,                   /* config */
	0,                      /* noconfig */
	1,                      /* read_eof */
	(pid_t)0,               /* pid */
	-1,                     /* in */
	-1,                     /* out */
	-1,                     /* err */
	-1,                     /* pty_user_fd */
	{ 0 },                  /* pty_device_name */
	PTY_DEVICE_NAME_SIZE,   /* pty_device_name_size */
	0,                      /* stop */
	0,                      /* running */
	0,                      /* restart */
	(time_t)0,              /* spawn_time */
	0,                      /* done_name */
	0,                      /* done_chroot */
	0,                      /* done_user */
	0,                      /* done_config */
	{ 0 },                  /* stdin_termios */
	{ 0 },                  /* stdin_winsize */
	0,                      /* stdin_isatty */
	0,                      /* stdin_eof */
	0,                      /* terminated */
	0,                      /* received_sigchld */
	null,                   /* signame */
	0,                      /* signo */
	0                       /* list */
};

#define is_space(c) isspace((int)(unsigned char)(c))

/*

C<char *expand(const char *input)>

Returns a dynamically allocated string containing the contents of the
C<input> string with simple environment variables replaced with their values
(e.g. C<$VARNAME> or C<${VARNAME}>) and shell-style user home directory
notation (e.g. C<~> and C<~username>) replaced with the path to the home
directory. It is the caller's responsibility to deallocate it with
I<free(3)> or I<mem_release(3)> or I<mem_destroy(3)>. It is strongly
recommended to use I<mem_destroy(3)>, because it also sets the pointer
variable to C<null>.

*/

static char *expand(const char *input)
{
	char *expanding = null;
	char *expanded = null;
	int i, len;

	/* Check arguments */

	if (!input)
		return set_errnull(EINVAL);

	/* Replace simple environment variables: $VARNAME or ${VARNAME} */

	expanding = mem_strdup(input);
	len = strlen(expanding);

	/* But not for root, unless --idiot */

	if (g.idiot || (getuid() && geteuid()))
	{
		for (i = 0; i < len; ++i)
		{
			if (expanding[i] == '$')
			{
				int braces;
				size_t varnamelen;
				char *varname = null;
				char *varvalue = null;
				size_t valuelen;

				braces = (expanding[i + 1] == '{') ? 1 : 0;
				varnamelen = strspn(expanding + i + 1 + braces, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_");

				if (varnamelen == 0)
					continue;

				if (asprintf(&varname, "%.*s", (int)varnamelen, expanding + i + 1 + braces) == -1)
					fatalsys("failed to expand");

				varvalue = getenv(varname);

				debug((2, "getenv %s=%s", varname, varvalue))

				mem_destroy(&varname);

				if (!varvalue)
					varvalue = "";

				if (asprintf(&expanded, "%.*s%s%s", i, expanding, varvalue, expanding + i + 1 + braces + varnamelen + braces) == -1)
					fatalsys("failed to expand");

				free(expanding);
				expanding = expanded;
				expanded = null;

				valuelen = strlen(varvalue);
				len -= 1 + braces + varnamelen + braces;
				len += valuelen;
				i += valuelen - 1;
			}
		}
	}

	/* Replace home directory notation: ~ or ~username */

	for (i = 0; i < len; ++i)
	{
		if (expanding[i] == '~' && (i == 0 || is_space(expanding[i - 1]) || expanding[i - 1] == ':' || expanding[i - 1] == '='))
		{
			size_t usernamelen = strcspn(expanding + i + 1, ":/ \t");
			struct passwd *pwd;
			
			if (usernamelen)
			{
				char *username = null;

				if (asprintf(&username, "%.*s", (int)usernamelen, expanding + i + 1) == -1)
					fatalsys("failed to expand");

				pwd = getpwnam(username);
				free(username);
			}
			else
			{
				uid_t uid = g.uid ? g.uid : getuid();

				pwd = getpwuid(uid);
			}

			if (pwd)
			{
				size_t homedirlen;

				if (asprintf(&expanded, "%.*s%s%s", i, expanding, pwd->pw_dir, expanding + i + 1 + usernamelen) == -1)
					fatalsys("failed to expand");

				free(expanding);
				expanding = expanded;
				expanded = null;

				homedirlen = strlen(pwd->pw_dir);
				len -= 1 + usernamelen;
				len += homedirlen;
				i += homedirlen - 1;
			}
		}
	}

	return expanding;
}

/*

C<void prepare_pidfiles(const char *path)>

If C<path> refers to a directory within the user's home directory that
doesn't exist yet, create it.

*/

static void prepare_pidfiles(const char *path)
{
	struct stat status[1];
	struct passwd *pwd;
	const char *homedir;
	size_t homelen;
	char *dir = null;
	const char *start, *end;

	debug((1, "prepare_pidfiles(%s)", path))

	/* Does path already exist? */

	if (stat(path, status) == 0)
		return;

	/* Get the user's home directory */

	if (!(pwd = getpwuid(g.uid ? g.uid : getuid())))
		return;

	homedir = pwd->pw_dir;
	homelen = strlen(homedir);

	/* Is path within it? */

	if (!(strncmp(path, homedir, homelen) == 0 && path[homelen] == '/'))
		return;

	/* Create it */

	for (start = path + homelen + 1;; start = end + 1)
	{
		if ((end = strchr(start, PATH_SEP)))
		{
			if (asprintf(&dir, "%.*s", (int)(end - path), path) == -1)	
				fatalsys("failed to prepare pidfiles location");
		}
		else
		{
			if (!(dir = mem_strdup(path)))
				fatalsys("failed to prepare pidfiles location");
		}

		debug((2, "dir %s", dir))

		if (stat(dir, status) == -1)
		{
			debug((2, "mkdir %s", dir))

			if (mkdir(dir, S_IRUSR | S_IWUSR | S_IXUSR) == -1)
				fatalsys("failed to create directory %s", dir);
		}

		free(dir);

		if (!end)
			break;
	}
}

/*

C<void handle_config_option(const char *spec)>

Store the C<--config> option argument, C<spec>.

*/

static void handle_config_option(const char *spec)
{
	debug((1, "handle_config_option(spec = %s)", spec))

	g.config = expand(spec);

	debug((2, "config = %s", g.config))
}

/*

C<void handle_name_option(const char *spec)>

Store the C<--name> option argument, C<spec>.

*/

#ifndef ACCEPT_NAME
#define ACCEPT_NAME "-._abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
#endif

#ifndef ACCEPT_PATH
#define ACCEPT_PATH ACCEPT_NAME "/"
#endif

static void handle_name_option(const char *spec)
{
	debug((1, "handle_name_option(spec = %s)", spec))

	if (g.done_config)
		return;

	if (g.done_name)
		prog_usage_msg("Misplaced option: --name=%s in config file (Must be on the command line)", spec);

	spec = expand(spec);

	if (strspn(spec, ACCEPT_NAME) != strlen(spec))
		prog_usage_msg("Invalid --name argument: '%s' (Must consist entirely of [-._a-zA-Z0-9])", spec);

	g.name = (char *)spec;

	debug((2, "name = %s", g.name))
}

/*

C<void handle_command_option(const char *spec)>

Store the C<--command> option argument, C<spec>.

*/

static void handle_command_option(const char *spec)
{
	debug((1, "handle_command_option(spec = %s)", spec))

	g.command = expand(spec);

	debug((2, "command = %s", g.command))
}

/*

C<void handle_pidfiles_option(const char *spec)>

Store the C<--pidfiles> option argument, C<spec>.

*/

static void handle_pidfiles_option(const char *spec)
{
	struct stat status[1];

	debug((1, "handle_pidfiles_option(spec = %s)", spec))

	spec = expand(spec);

	if (strspn(spec, ACCEPT_PATH) != strlen(spec))
		prog_usage_msg("Invalid --pidfiles argument: '%s' (Must consist entirely of [-._a-zA-Z0-9/])", spec);

	if (*spec != PATH_SEP)
		prog_usage_msg("Invalid --pidfiles argument: '%s' (Must be an absolute directory path)", spec);

	prepare_pidfiles(spec);

	if (stat(spec, status) == -1 || !S_ISDIR(status->st_mode))
		prog_usage_msg("Invalid --pidfiles argument: '%s' (Directory does not exist)", spec);

	/* Check directory writability later in sanity_check() after all options have been seen */

	g.pidfiles = (char *)spec;

	debug((2, "pidfiles = %s", g.pidfiles))
}

/*

C<void handle_pidfile_option(const char *spec)>

Store the C<--pidfile> option argument, C<spec>.

*/

static void handle_pidfile_option(const char *spec)
{
	struct stat status[1];
	char *buf;
	size_t size;

	debug((1, "handle_pidfile_option(spec = %s)", spec))

	spec = expand(spec);

	if (strspn(spec, ACCEPT_PATH) != strlen(spec))
		prog_usage_msg("Invalid --pidfile argument: '%s' (Must consist entirely of [-._a-zA-Z0-9/])", spec);

	if (*spec != PATH_SEP || (stat(spec, status) == 0 && S_ISDIR(status->st_mode)))
		prog_usage_msg("Invalid --pidfile argument: '%s' (Must be an absolute file path)", spec);

	if ((size = strrchr(spec, PATH_SEP) - spec + 1) == 1)
		++size;

	if (!(buf = mem_create(size, char)))
		fatalsys("out of memory");

	snprintf(buf, size, "%.*s", (int)size - 1, spec);
	prepare_pidfiles(buf);

	if (stat(buf, status) == -1 || !S_ISDIR(status->st_mode))
		prog_usage_msg("Invalid --pidfile argument: '%s' (Parent directory does not exist)", spec);

	mem_release(buf);

	/* Check parent directory writability later in sanity_check() after all options have been seen */

	g.pidfile = (char *)spec;

	debug((2, "pidfile = %s", g.pidfile))
}

/*

C<void handle_user_option(const char *spec)>

Parse and store the client C<--user[:group]> option argument, C<spec>.

*/

static void handle_user_option(const char *spec)
{
	struct passwd *pwd;
	struct group *grp;
	char **member;
	char *pos;

	debug((1, "handle_user_option(spec = %s)", spec))

	if (g.done_config)
		return;

	if (g.done_user)
		prog_usage_msg("Misplaced option: --user=%s in config file (Must be on the command line)", spec);

	if (getuid() || geteuid())
		prog_usage_msg("Invalid option: --user (Only works for root)");

	spec = expand(spec);

	debug((2, "user = %s", spec))

	if ((pos = strchr(spec, ':')) || (pos = strchr(spec, '.')))
	{
		if (pos > spec)
			snprintf(g.user = g.userbuf, BUFSIZ, "%*.*s", (int)(pos - spec), (int)(pos - spec), spec);

		if (*++pos)
			snprintf(g.group = g.groupbuf, BUFSIZ, "%s", pos);
	}
	else
	{
		snprintf(g.user = g.userbuf, BUFSIZ, "%s", spec);
	}

	g.init_groups = (g.group == null);

	if (!g.user)
		prog_usage_msg("Invalid --user argument: '%s' (No user name)", spec);

	if (!(pwd = getpwnam(g.user)))
		prog_usage_msg("Invalid --user argument: '%s' (Unknown user %s)", spec, g.user);

	g.uid = pwd->pw_uid;
	g.gid = pwd->pw_gid;

	if (g.group)
	{
		if (!(grp = getgrnam(g.group)))
			prog_usage_msg("Invalid --user argument: '%s' (Unknown group %s)", spec, g.group);

		if (grp->gr_gid != pwd->pw_gid)
		{
			for (member = grp->gr_mem; *member; ++member)
				if (!strcmp(*member, g.user))
					break;

			if (!*member)
				prog_usage_msg("Invalid --user argument: '%s' (User %s is not in group %s)", spec, g.user, g.group);
		}

		g.gid = grp->gr_gid;
	}
}

/*

C<void handle_chroot_option(const char *spec)>

Store the C<--chroot> option argument, C<spec>.

*/

static void handle_chroot_option(const char *spec)
{
	debug((1, "handle_chroot_option(spec = %s)", spec))

	if (g.done_config)
		return;

	if (g.done_chroot)
		prog_usage_msg("Misplaced option: --chroot=%s in config file (Must be on the command line)", spec);

	g.chroot = expand(spec);

	debug((2, "chroot = %s", g.chroot))
}

/*

C<void handle_chdir_option(const char *spec)>

Store the C<--chdir> option argument, C<spec>.

*/

static void handle_chdir_option(const char *spec)
{
	debug((1, "handle_chdir_option(spec = %s)", spec))

	g.chdir = expand(spec);

	debug((2, "chdir = %s", g.chdir))
}

/*

C<void handle_umask_option(const char *spec)>

Parse and store the C<--umask> option argument, C<spec>.

*/

static void handle_umask_option(const char *spec)
{
	char *end;
	long val;

	debug((1, "handle_umask_option(spec = %s)", spec))

	spec = expand(spec);
	val = strtol(spec, &end, 8);

	if (end == spec || *end || val < 0 || val > 0777)
		prog_usage_msg("Invalid --umask argument: '%s' (Must be a valid octal mode)", spec);

	g.umask = val;

	debug((2, "umask = %03o", g.umask))
}

/*

C<void handle_env_option(const char *var)>

Store the C<--env> option argument, C<var>.

*/

static void handle_env_option(const char *var)
{
	debug((1, "handle_env_option(spec = %s)", var))

	if (g.env == null && !(g.env = list_create(null)))
		fatalsys("failed to create environment list");

	var = expand(var);

	if (!list_append(g.env, (void *)var))
		fatalsys("failed to add '%s' to environment list", var);

	debug((2, "env += %s", var))
}

/*

C<void handle_inherit_option(void)>

Process the C<--inherit> option. Add the contents of C<environ> to the list
of environment variables to be used by the client.

*/

static void handle_inherit_option(void)
{
	char **env;

	debug((1, "handle_inherit_option()"))

	if (g.env == null && !(g.env = list_create(null)))
		fatalsys("failed to create environment list");

	for (env = environ; *env; ++env)
		if (!list_append(g.env, *env))
			fatalsys("failed to add '%s' to environment list", env);

	g.inherit = 1;
}

/*

C<void handle_core_option(void)>

Allow core file generation.

*/

static void handle_core_option(void)
{
	debug((1, "handle_core_option()"))

	g.core = 1;
}

/*

C<void handle_nocore_option(void)>

Disallow core file generation (default).

*/

static void handle_nocore_option(void)
{
	debug((1, "handle_nocore_option()"))

	g.core = 0;
}

/*

C<void handle_acceptable_option(int acceptable)>

Store the C<--acceptable> option argument, C<acceptable>.

*/

static void handle_acceptable_option(int acceptable)
{
	debug((1, "handle_acceptable_option(acceptable = %d)", acceptable))

	if (!g.idiot && acceptable < RESPAWN_ACCEPTABLE_MIN)
		prog_usage_msg("Invalid --acceptable argument: %d (Less than %d)\n", acceptable, RESPAWN_ACCEPTABLE_MIN);

	g.acceptable = acceptable;
}

/*

C<void handle_attempts_option(int attempts)>

Store the C<--attempts> option argument, C<attempts>.

*/

static void handle_attempts_option(int attempts)
{
	debug((1, "handle_attempts_option(attempts = %d)", attempts))

	if (!g.idiot && (attempts < RESPAWN_ATTEMPTS_MIN || attempts > RESPAWN_ATTEMPTS_MAX))
		prog_usage_msg("Invalid --attempts argument: %d (Not between %d and %d)", attempts, RESPAWN_ATTEMPTS_MIN, RESPAWN_ATTEMPTS_MAX);

	g.attempts = attempts;
}

/*

C<void handle_delay_option(int delay)>

Store the C<--delay> option argument, C<delay>.

*/

static void handle_delay_option(int delay)
{
	debug((1, "handle_delay_option(delay = %d)", delay))

	if (!g.idiot && delay < RESPAWN_DELAY_MIN)
		prog_usage_msg("Invalid --delay argument: %d (Less than %d)\n", delay, RESPAWN_DELAY_MIN);

	g.delay = delay;
}

/*

C<void handle_limit_option(int limit)>

Store the C<--limit> option argument, C<limit>.

*/

static void handle_limit_option(int limit)
{
	debug((1, "handle_limit_option(limit = %d)", limit))

	if (limit < RESPAWN_LIMIT_MIN)
		prog_usage_msg("Invalid --limit argument: %d (Less than %d)\n", limit, RESPAWN_LIMIT_MIN);

	g.limit = limit;
}

/*

C<void handle_idiot_option(void)>

Store the C<--idiot> option argument if allowed.

*/

static void handle_idiot_option(void)
{
	debug((1, "handle_idiot_option()"))

	if (g.initial_uid)
		prog_usage_msg("Invalid option: --idiot (Only for root)");

	g.idiot = 1;
}

/*

C<void handle_pty_option(const char *arg)>

Store the C<--pty> option argument, C<arg>.

*/

static void handle_pty_option(const char *arg)
{
	debug((1, "handle_pty_option(arg = %s)", (arg) ? arg : ""))

	g.pty = 1;

	if (arg)
	{
		arg = expand(arg);

		debug((2, "pty %s", arg))

		if (strcmp(arg, "noecho"))
			prog_usage_msg("Invalid --pty argument: '%s' (Only 'noecho' is supported)", arg);

		g.noecho = 1;
	}
}

/*

C<void store_syslog(const char *spec, char **str, int *var)>

Parse the syslog target, C<spec>. Store C<spec> in C<*str> and store
the parsed facility and priority in C<*var>.

*/

static void store_syslog(const char *option, const char *spec, char **str, int *var)
{
	int facility;
	int priority;

	debug((1, "store_syslog(spec = %s)", spec))

	if (syslog_parse(spec, &facility, &priority) == -1)
	{
		*str = (char *)spec; /* Must be a file */
		*var = 0;            /* Erase default syslog */
		return;
	}

	*str = (char *)spec;
	*var = facility | priority;
}

/*

C<void handle_errlog_option(const char *spec)>

Parse and store the C<--errlog> option argument, C<spec>.

*/

static void handle_errlog_option(const char *spec)
{
	debug((1, "handle_errlog_option(spec = %s)", spec))

	spec = expand(spec);
	store_syslog("errlog", spec, &g.daemon_err, &g.daemon_errlog);

	debug((2, "errlog %s", spec))
}

/*

C<void handle_dbglog_option(const char *spec)>

Parse and store the C<--dbglog> option argument, C<spec>.

*/

static void handle_dbglog_option(const char *spec)
{
	debug((1, "handle_dbglog_option(spec = %s)", spec))

	spec = expand(spec);
	store_syslog("dbglog", spec, &g.daemon_dbg, &g.daemon_dbglog);

	debug((2, "dbglog %s", spec))
}

/*

C<void handle_output_option(const char *spec)>

Parse and store the C<--output> option argument, C<spec>.

*/

static void handle_output_option(const char *spec)
{
	debug((1, "handle_output_option(spec = %s)", spec))

	spec = expand(spec);
	store_syslog("output", spec, &g.client_out, &g.client_outlog);
	store_syslog("output", spec, &g.client_err, &g.client_errlog);

	debug((2, "output %s", spec))
}

/*

C<void handle_stdout_option(const char *spec)>

Parse and store the C<--stdout> option argument, C<spec>.

*/

static void handle_stdout_option(const char *spec)
{
	debug((1, "handle_stdout_option(spec = %s)", spec))

	spec = expand(spec);
	store_syslog("stdout", spec, &g.client_out, &g.client_outlog);

	debug((2, "stdout %s", spec))
}

/*

C<void handle_stderr_option(const char *spec)>

Parse and store the C<--stderr> option argument, C<spec>.

*/

static void handle_stderr_option(const char *spec)
{
	debug((1, "handle_stderr_option(spec = %s)", spec))

	spec = expand(spec);
	store_syslog("stderr", spec, &g.client_err, &g.client_errlog);

	debug((2, "stderr %s", spec))
}

/*

C<void handle_ignore_eof_option(void)>

Turn off I<read_eof> mode.

*/

static void handle_ignore_eof_option(void)
{
	debug((1, "handle_ignore_eof_option()"))

	g.read_eof = 0;
}

/*

C<void handle_read_eof_option(void)>

Restore I<read_eof> mode.

*/

static void handle_read_eof_option(void)
{
	debug((1, "handle_read_eof_option()"))

	g.read_eof = 1;
}

/*

C<void handle_signal_option(const char *signame)>

Store the C<--signal> option argument, C<signame>.

*/

#ifdef NSIG
#define SIG_MAX NSIG
#else
#ifdef _NSIG
#define SIG_MAX _NSIG
#else
#define SIG_MAX 32
#endif
#endif

typedef struct sigmap_t sigmap_t;
struct sigmap_t
{
	char *signame;
	int signo;
};

static sigmap_t signames[] =
{
#ifdef SIGHUP
	{ "hup", SIGHUP },
#endif
#ifdef SIGINT
	{ "int", SIGINT },
#endif
#ifdef SIGQUIT
	{ "quit", SIGQUIT },
#endif
#ifdef SIGILL
	{ "ill", SIGILL },
#endif
#ifdef SIGTRAP
	{ "trap", SIGTRAP },
#endif
#ifdef SIGABRT
	{ "abrt", SIGABRT },
#endif
#ifdef SIGIOT
	{ "iot", SIGIOT },
#endif
#ifdef SIGBUS
	{ "bus", SIGBUS },
#endif
#ifdef SIGFPE
	{ "fpe", SIGFPE },
#endif
#ifdef SIGKILL
	{ "kill", SIGKILL },
#endif
#ifdef SIGUSR1
	{ "usr1", SIGUSR1 },
#endif
#ifdef SIGSEGV
	{ "segv", SIGSEGV },
#endif
#ifdef SIGUSR2
	{ "usr2", SIGUSR2 },
#endif
#ifdef SIGPIPE
	{ "pipe", SIGPIPE },
#endif
#ifdef SIGALRM
	{ "alrm", SIGALRM },
#endif
#ifdef SIGTERM
	{ "term", SIGTERM },
#endif
#ifdef SIGSTKFLT
	{ "stkflt", SIGSTKFLT },
#endif
#ifdef SIGCLD
	{ "cld", SIGCLD },
#endif
#ifdef SIGCHLD
	{ "chld", SIGCHLD },
#endif
#ifdef SIGCONT
	{ "cont", SIGCONT },
#endif
#ifdef SIGSTOP
	{ "stop", SIGSTOP },
#endif
#ifdef SIGTSTP
	{ "tstp", SIGTSTP },
#endif
#ifdef SIGTTIN
	{ "ttin", SIGTTIN },
#endif
#ifdef SIGTTOU
	{ "ttou", SIGTTOU },
#endif
#ifdef SIGURG
	{ "urg", SIGURG },
#endif
#ifdef SIGXCPU
	{ "xcpu", SIGXCPU },
#endif
#ifdef SIGXFSZ
	{ "xfsz", SIGXFSZ },
#endif
#ifdef SIGVTALRM
	{ "vtalrm", SIGVTALRM },
#endif
#ifdef SIGPROF
	{ "prof", SIGPROF },
#endif
#ifdef SIGWINCH
	{ "winch", SIGWINCH },
#endif
#ifdef SIGPOLL
	{ "poll", SIGPOLL },
#endif
#ifdef SIGIO
	{ "io", SIGIO },
#endif
#ifdef SIGPWR
	{ "pwr", SIGPWR },
#endif
#ifdef SIGSYS
	{ "sys", SIGSYS },
#endif
#ifdef SIGEMT
	{ "emt", SIGEMT },
#endif
#ifdef SIGINFO
	{ "info", SIGINFO },
#endif
	{ null, 0 }
};

static void handle_signal_option(const char *signame)
{
	long int signo;
	char *endptr = null;
	int i;

	debug((1, "handle_signal_option(signame = %s)", signame))

	errno = 0;
	signame = expand(signame);
	signo = strtol(signame, &endptr, 10);

	debug((2, "signal %s", signame))

	if (errno == 0 && *signame && !*endptr && signo > 0 && signo < SIG_MAX)
	{
		g.signame = (char *)signame;
		g.signo = (int)signo;

		for (i = 0; signames[i].signame; ++i)
			if (signames[i].signo == g.signo)
				break;

		if (signames[i].signame)
			g.signame = signames[i].signame;
	}
	else
	{
		const char *start = signame;

		if (strncasecmp(start, "sig", 3) == 0)
			start += 3;

		for (i = 0; signames[i].signame; ++i)
			if (strcasecmp(start, signames[i].signame) == 0)
				break;

		if (!signames[i].signame)
			prog_usage_msg("Invalid --signal argument: '%s' (Must be a signal name or number)", signame);

		g.signame = (char *)signame;
		g.signo = signames[i].signo;
	}
}

/*

C<Option daemon_optab[];>

Application-specific command line options.

*/

static Option daemon_optab[] =
{
	{
		"config", 'C', "path", "Specify the configuration file",
		required_argument, OPT_STRING, OPT_FUNCTION, null, (func_t *)handle_config_option
	},
	{
		"noconfig", 'N', null, "Bypass the system configuration file",
		no_argument, OPT_NONE, OPT_VARIABLE, &g.noconfig, null
	},
	{
		"name", 'n', "name", "Guarantee a single named instance",
		required_argument, OPT_STRING, OPT_FUNCTION, null, (func_t *)handle_name_option
	},
	{
		"command", 'X', "\"cmd\"", "Specify the client command as an option",
		required_argument, OPT_STRING, OPT_FUNCTION, null, (func_t *)handle_command_option
	},
	{
		"pidfiles", 'P', "/dir", "Override standard pidfile location",
		required_argument, OPT_STRING, OPT_FUNCTION, null, (func_t *)handle_pidfiles_option
	},
	{
		"pidfile", 'F', "/path", "Override standard pidfile name and location\n",
		required_argument, OPT_STRING, OPT_FUNCTION, null, (func_t *)handle_pidfile_option
	},
	{
		"user", 'u', "user[:group]", "Run the client as user[:group]",
		required_argument, OPT_STRING, OPT_FUNCTION, null, (func_t *)handle_user_option
	},
	{
		"chroot", 'R', "path", "Run the client with path as root",
		required_argument, OPT_STRING, OPT_FUNCTION, null, (func_t *)handle_chroot_option
	},
	{
		"chdir", 'D', "path", "Run the client in directory path",
		required_argument, OPT_STRING, OPT_FUNCTION, null, (func_t *)handle_chdir_option
	},
	{
		"umask", 'm', "umask", "Run the client with the given umask",
		required_argument, OPT_STRING, OPT_FUNCTION, null, (func_t *)handle_umask_option
	},
	{
		"env", 'e', "\"var=val\"", "Set a client environment variable",
		required_argument, OPT_STRING, OPT_FUNCTION, null, (func_t *)handle_env_option
	},
	{
		"inherit", 'i', null, "Inherit environment variables",
		no_argument, OPT_NONE, OPT_FUNCTION, null, (func_t *)handle_inherit_option
	},
	{
		"unsafe", 'U', null, "Allow execution of unsafe executable",
		no_argument, OPT_NONE, OPT_VARIABLE, &g.unsafe, null
	},
	{
		"safe", 'S', null, "Disallow execution of unsafe executable",
		no_argument, OPT_NONE, OPT_VARIABLE, &g.safe, null
	},
	{
		"core", 'c', null, "Allow core file generation",
		no_argument, OPT_NONE, OPT_FUNCTION, null, (func_t *)handle_core_option
	},
	{
		"nocore", nul, null, "Disallow core file generation (default)\n",
		no_argument, OPT_NONE, OPT_FUNCTION, null, (func_t *)handle_nocore_option
	},
	{
		"respawn", 'r', null, "Respawn the client when it terminates",
		no_argument, OPT_NONE, OPT_VARIABLE, &g.respawn, null
	},
	{
		"acceptable", 'a', "#", "Minimum acceptable client duration (seconds)",
		required_argument, OPT_INTEGER, OPT_FUNCTION, null, (func_t *)handle_acceptable_option
	},
	{
		"attempts", 'A', "#", "Respawn # times on error before delay",
		required_argument, OPT_INTEGER, OPT_FUNCTION, null, (func_t *)handle_attempts_option
	},
	{
		"delay", 'L', "#", "Delay between respawn attempt bursts (seconds)",
		required_argument, OPT_INTEGER, OPT_FUNCTION, null, (func_t *)handle_delay_option
	},
	{
		"limit", 'M', "#", "Maximum number of respawn attempt bursts",
		required_argument, OPT_INTEGER, OPT_FUNCTION, null, (func_t *)handle_limit_option
	},
	{
		"idiot", nul, null, "Idiot mode (trust root with the above)\n",
		no_argument, OPT_NONE, OPT_FUNCTION, null, (func_t *)handle_idiot_option
	},
	{
		"foreground", 'f', null, "Run the client in the foreground",
		no_argument, OPT_NONE, OPT_VARIABLE, &g.foreground, null
	},
	{
		"pty", 'p', "noecho", "Allocate a pseudo terminal for the client\n",
		optional_argument, OPT_STRING, OPT_FUNCTION, null, (func_t *)handle_pty_option
	},
#ifdef HAVE_LOGIND
	{
		"bind", 'B', null, "Stop when the user's last logind session ends\n",
		no_argument, OPT_NONE, OPT_VARIABLE, &g.bind, null
	},
#endif
	{
		"errlog", 'l', "spec", "Send daemon's error output to syslog or file",
		required_argument, OPT_STRING, OPT_FUNCTION, null, (func_t *)handle_errlog_option
	},
	{
		"dbglog", 'b', "spec", "Send daemon's debug output to syslog or file",
		required_argument, OPT_STRING, OPT_FUNCTION, null, (func_t *)handle_dbglog_option
	},
	{
		"output", 'o', "spec", "Send client's output to syslog or file",
		required_argument, OPT_STRING, OPT_FUNCTION, null, (func_t *)handle_output_option
	},
	{
		"stdout", 'O', "spec", "Send client's stdout to syslog or file",
		required_argument, OPT_STRING, OPT_FUNCTION, null, (func_t *)handle_stdout_option
	},
	{
		"stderr", 'E', "spec", "Send client's stderr to syslog or file\n",
		required_argument, OPT_STRING, OPT_FUNCTION, null, (func_t *)handle_stderr_option
	},
	{
		"ignore-eof", nul, null, "After SIGCHLD ignore any client output",
		no_argument, OPT_NONE, OPT_FUNCTION, null, (func_t *)handle_ignore_eof_option
	},
	{
		"read-eof", nul, null, "After SIGCHLD read any client output (default)\n",
		no_argument, OPT_NONE, OPT_FUNCTION, null, (func_t *)handle_read_eof_option
	},
	{
		"running", nul, null, "Check if a named daemon is running",
		no_argument, OPT_NONE, OPT_VARIABLE, &g.running, null
	},
	{
		"restart", nul, null, "Restart a named daemon client",
		no_argument, OPT_NONE, OPT_VARIABLE, &g.restart, null
	},
	{
		"stop", nul, null, "Terminate a named daemon process",
		no_argument, OPT_NONE, OPT_VARIABLE, &g.stop, null
	},
	{
		"signal", nul, "signame", "Send a signal to a named daemon",
		required_argument, OPT_STRING, OPT_FUNCTION, null, (func_t *)handle_signal_option
	},
	{
		"list", nul, null, "Print a list of named daemons",
		no_argument, OPT_NONE, OPT_VARIABLE, &g.list, null
	},
	{
		null, '\0', null, null, 0, 0, 0, null, null
	}
};

static Options options[1] = {{ prog_options_table, daemon_optab }};

/*

C<Config *config_create(char *name, char *options)>

Create a I<Config> object from a name and a comma-separated list of C<options>.
Space characters before or after the commas are skipped.

*/

static Config *config_create(char *name, char *options)
{
	Config *config;
	List *tokens;
	int i;

	debug((1, "config_create(name = \"%s\", options = \"%s\")", name, options))

	if (!(config = mem_new(Config)))
		return null;

	if (!(config->name = mem_strdup(name)))
	{
		mem_release(config);
		return null;
	}

	if (!(config->options = list_create(free)))
	{
		mem_release(config->name);
		mem_release(config);
		return null;
	}

	if (!(tokens = split(options, ",")))
	{
		list_release(config->options);
		mem_release(config->name);
		mem_release(config);
		return null;
	}

	for (i = 0; i < list_length(tokens); ++i)
	{
		char *tok = cstr((String *)list_item(tokens, i));
		size_t size, len;
		char *option;

		/* Strip leading and trailing space */

		while (*tok && is_space(*tok))
			++tok;

		len = strlen(tok);

		while (len && is_space(tok[len - 1]))
			tok[--len] = nul;

		/* Construct and append the command line option */

		size = len + 3;

		if (!(option = mem_create(size, char)))
		{
			list_release(tokens);
			list_release(config->options);
			mem_release(config->name);
			mem_release(config);
			return null;
		}

		strlcpy(option, "--", size);
		strlcat(option, tok, size);

		if (!list_append(config->options, option))
		{
			mem_release(option);
			list_release(tokens);
			list_release(config->options);
			mem_release(config->name);
			mem_release(config);
			return null;
		}
	}

	return config;
}

/*

C<void config_release(Config *config)>

Release all memory associated with C<config>.

*/

static void config_release(Config *config)
{
	mem_release(config->name);
	list_release(config->options);
	mem_release(config);
}

/*

C<void config_parse(void *obj, const char *path, char *line, size_t lineno)>

Parse a C<line> of the configuration file, storing results in C<obj> which
is a list of lists of strings. C<lineno> is the current line number within
the configuration file.

*/

static void config_parse(void *obj, const char *path, char *line, size_t lineno)
{
	List *list = (List *)obj;
	Config *config;
	char name[512], *n = name;
	char options[4096], *o = options;
	char *s = line;

	debug((1, "config_parse(obj = %p, path = %s, line = \"%s\", lineno = %d)", obj, path, line, lineno))

	/* Skip any leading space */

	while (*s && is_space(*s))
		++s;

	/* Extract the daemon name (or "*" for generic options, or the environment variable name) to a buffer */

	while ((n - name) < sizeof(name) - 1 && *s && !is_space(*s) && *s != '=')
	{
		if (*s == '\\')
			++s;
		*n++ = *s++;
	}

	*n = '\0';

	/* Check that there is a daemon name (or "*" or an environment variable name) */

	if (!*name)
		fatal("syntax error in %s, line %d, expected * or a daemon name:\n%s", path, lineno, line);

	/* Define environment variables */

	if (*s == '=')
	{
		char *expanded;
		char *definition = null;

		/* Check that it's a valid identifier */

		if (strspn(name, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_") != (n - name) || strspn(name, "0123456789") == 1)
			fatal("syntax error in %s, line %d, invalid environment variable name:\n%s", path, lineno, line);

		/* Expand the value */

		expanded = expand(s + 1);

		if (asprintf(&definition, "%s=%s", name, expanded) == -1)
			fatalsys("failed to parse %s", path);

		free(expanded);

		debug((2, "putenv %s", definition))

		putenv(definition); /* Leak to environ */

		return;
	}

	/* Check that the daemon name isn't too long for the buffer */

	if (!is_space(*s))
		fatal("syntax error in %s, line %d, name too long:\n%s", path, lineno, line);

	/* Skip space between the daemon name and options */

	while (*s && is_space(*s))
		++s;

	/* Extract the options to a buffer */

	while ((o - options) < sizeof(options) - 1 && *s)
	{
		if (*s == '\\')
			++s;
		*o++ = *s++;
	}

	*o = '\0';

	/* Check that there are options */

	if (!*options)
		fatal("syntax error in %s, line %d, expected options:\n%s", path, lineno, line);

	/* Check that the options aren't too long for the buffer */

	if ((o - options) == sizeof(options) - 1 && *s)
		fatal("syntax error in %s, line %d, options too long:\n%s", path, lineno, line);

	/* Append this configuration item to the list */

	if (!(config = config_create(name, options)) || !list_append(list, config))
		fatalsys("out of memory");
}

/*

C<void config_process(List *conf, char *target)>

Searches for C<target> in C<conf> and processes the all configuration lines
that match C<target>.

*/

static void config_process(List *conf, char *target)
{
	int ac;
	char **av;
	Config *config;
	int j;

	debug((1, "config_process(target = %s)", target))

	while (list_has_next(conf) == 1)
	{
		config = (Config *)list_next(conf);

		if (!strcmp(config->name, target))
		{
			if (!(av = mem_create(list_length(config->options) + 2, char *)))
				fatalsys("out of memory");

			av[0] = (char *)prog_name();

			for (j = 1; list_has_next(config->options) == 1; ++j)
				if (!(av[j] = mem_strdup(list_next(config->options))))
					fatalsys("out of memory");

			av[ac = j] = null;
			optind = 0;
			prog_opt_process(ac, av);
			mem_release(av); /* Leak av elements since g might refer to them now */
		}
	}
}

/*

C<void config_load(List **conf, const char *configfile)>

Loads the contents of C<configfile> into the list pointed to by C<*conf> if
it is safe to do so. The list C<*conf> is created if necessary. It is the
caller's responsibility to deallocate it with I<list_release(3)> or
I<list_destroy(3)>. It is strongly recommended to use I<list_destroy(3)>,
because it also sets the pointer variable to C<null>.

*/

static void config_load(List **conf, const char *configfile)
{
	char explanation[256];
	char *configdir;
	List *entries;
	struct dirent *entry;
	DIR *dir;
	char *configdirfile = null;
	int is_ok;

	debug((1, "config_load(configfile = %s)", configfile))

	/* Check that the config file is safe. If it is, parse it. */

	is_ok = 1;

	if (g.safe || (getuid() == 0 && !g.unsafe))
	{
		switch (daemon_path_is_safe(configfile, explanation, 256))
		{
			case -1:
				/* Don't emit an error message if the file doesn't exist */
				if (errno != ENOENT)
					errorsys("ignoring %s (failed to check if it is safe) %d", configfile, errno);
				is_ok = 0;
				break;
			case 0:
				error("ignoring unsafe %s (%s)", configfile, explanation);
				is_ok = 0;
				break;
			case 1:
				break;
		}
	}

	if (is_ok)
		daemon_parse_config(configfile, *conf, config_parse);

	/* Parse files in the corresponding configuration directory, if any */

	if (!(entries = list_create(free)))
	{
		errorsys("failed to load %s%s/*", configfile, CONFIG_DIR_PATH_SUFFIX);
		return;
	}

	if (asprintf(&configdir, "%s%s", configfile, CONFIG_DIR_PATH_SUFFIX) == -1)
	{
		errorsys("failed to load %s%s/*", configfile, CONFIG_DIR_PATH_SUFFIX);
		list_release(entries);
		return;
	}

	if ((dir = opendir(configdir)))
	{
		while ((entry = readdir(dir)))
		{
			/* Skip files whose names start with the dot character */

			if (entry->d_name && entry->d_name[0] == '.')
				continue;

			if (asprintf(&configdirfile, "%s/%s", configdir, entry->d_name) == -1)
			{
				errorsys("failed to load %s/%s", configdir, entry->d_name);
				break;
			}

			/* Check that the config file is safe. If it is, parse it. */

			is_ok = 1;

			if (g.safe || (getuid() == 0 && !g.unsafe))
			{
				switch (daemon_path_is_safe(configdirfile, explanation, 256))
				{
					case -1:
						errorsys("ignoring %s (failed to check if it is safe)", configdirfile);
						is_ok = 0;
						break;
					case 0:
						error("ignoring unsafe %s (%s)", configdirfile, explanation);
						is_ok = 0;
						break;
					case 1:
						break;
				}
			}

			if (is_ok)
				daemon_parse_config(configdirfile, *conf, config_parse);

			mem_destroy(&configdirfile);
		}

		closedir(dir);
	}

	mem_destroy(&configdir);
	list_release(entries);
}

/*

C<void config(void)>

Parse the configuration file, if any, and process the contents as command
line options. Generic options are applied to all clients. Options specific
to a particular named client override the generic options. Command line options
override both specific and generic options.

*/

static void config(void)
{
	List *conf = null;
	struct passwd *pwd;
	char *config_user;
	size_t size;

	debug((1, "config()"))

	/* Create the config list */

	if (!(conf = list_create((list_release_t *)config_release)))
		fatalsys("out of memory");

	/* Load the system configuration file(s) */

	if (!g.noconfig)
		config_load(&conf, g.config ? g.config : CONFIG_PATH);

	/* Load the user configuration file(s) */

	if ((pwd = getpwuid(g.uid ? g.uid : getuid())))
	{
		size = strlen(pwd->pw_dir) + 1 + sizeof(CONFIG_PATH_USER) + 1;
		if (!(config_user = mem_create(size, char)))
			fatalsys("out of memory");
		snprintf(config_user, size, "%s%c%s", pwd->pw_dir, PATH_SEP, CONFIG_PATH_USER);
		config_load(&conf, config_user);
		mem_destroy(&config_user);
	}

	/* Apply generic options */

	config_process(conf, "*");

	/* Override with specific options */

	if (g.name)
		config_process(conf, g.name);

	/* Override with command line options */

	optind = 0;
	g.done_config = 1;
	prog_opt_process(g.ac, g.av);

	/* Release the config list */

	list_release(conf);
}

/*

C<void term(int signo)>

This function is registered as the C<SIGTERM> handler. It propagates the
C<SIGTERM> signal to the client process and records the fact that the client
has been stopped. That will cause I<exit(3)> to be called later.
I<daemon_close(3)> will be called by I<atexit(3)> to unlink the locked pid
file (if any).

*/

static void term(int signo)
{
	debug((1, "term(signo = %d)", signo))

	if (g.pid != 0 && g.pid != -1 && g.pid != getpid())
	{
		debug((2, "kill(term) process %d", (int)g.pid))

		if (kill(g.pid, SIGTERM) == -1)
			errorsys("failed to terminate client (%d)", (int)g.pid);

		debug((2, "stopped"))
	}

	g.terminated = 1;
}

/*

C<void chld(int signo)>

Registered as the C<SIGCHLD> handler. Records the fact that we received the
signal so that run() knows not to enter select() when not in read_eof mode.

*/

static void chld(int signo)
{
	debug((1, "chld(signo = %d) g.pid = %d", signo, (int)g.pid))
	g.received_sigchld = 1;
}

/*

C<void usr1(int signo)>

This function is registered as the C<SIGUSR1> handler. When another daemon
process has been used to restart this daemon's client, it will send this
daemon a C<SIGUSR1> signal causing this daemon to send a C<SIGTERM> signal
to the client. This is like receiving a C<SIGTERM> signal except that this
daemon process doesn't I<exit()> (if started with the C<--respawn> option).

*/

static void usr1(int signo)
{
	debug((1, "usr1(signo = %d)", signo))

	if (g.pid != 0 && g.pid != -1 && g.pid != getpid())
	{
		debug((2, "kill(term) process %d", (int)g.pid))

		g.spawn_time = (time_t)0;
		g.attempt = 0;
		g.burst = 0;

		if (kill(g.pid, SIGTERM) == -1)
			errorsys("failed to terminate client (%d)", (int)g.pid);

		debug((2, "stopped"))
	}
}

/*

C<void winch(int signo)>

Registered as the C<SIGWINCH> handler when connected to the client via a
pseudo terminal. Propagates the window size change to the client.

*/

static void winch(int signo)
{
	struct winsize win;

	debug((1, "winch(signo = %d)", signo))

	if (g.pty_user_fd == -1)
		return;

	debug((2, "ioctl(stdin, TIOCGWINSZ)"))

	if (ioctl(STDIN_FILENO, TIOCGWINSZ, &win) == -1)
	{
		errorsys("failed to get stdin's window size");
		return;
	}

	debug((2, "ioctl(pty_user_fd = %d, TIOCSWINSZ, row = %d, col = %d, xpixel = %d, ypixel = %d)", g.pty_user_fd, win.ws_row, win.ws_col, win.ws_xpixel, win.ws_ypixel))

	if (ioctl(g.pty_user_fd, TIOCSWINSZ, &win) == -1)
		errorsys("failed to set pty's window size");
}

/*

C<void prepare_environment(void)>

Convert the environment variables specified on the command line into a form
suitable for passing to I<execve(2)>.

*/

static void prepare_environment(void)
{
	int i;

	debug((1, "prepare_environment()"))

	if (!g.env)
		return;

	if (!(g.environ = mem_create(list_length(g.env) + 1, char *)))
		fatalsys("out of memory");

	for (i = 0; list_has_next(g.env) == 1; ++i)
	{
		char *env = list_next(g.env);

		if (!(g.environ[i] = mem_strdup(env)))
			fatalsys("out of memory");
	}

	g.environ[i] = null;
}

/*

C<int tty_raw(int fd)>

Sets the terminal descriptor, C<fd>, to raw mode. On success, returns C<0>.
On error, returns C<-1> with C<errno> set appropriately.

*/

static int tty_raw(int fd)
{
	struct termios attr[1];

	debug((1, "tty_raw(fd = %d)", fd))

	debug((2, "tcgetattr(fd = %d)", fd))

	if (tcgetattr(fd, attr) == -1)
		return -1;

	attr->c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	attr->c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	attr->c_cflag &= ~(CSIZE | PARENB);
	attr->c_cflag |= (CS8);
	attr->c_oflag &= ~(OPOST);
	attr->c_cc[VMIN] = 1;
	attr->c_cc[VTIME] = 0;

	debug((2, "tcsetattr(fd = %d, TCSANOW, raw)", fd))

	return tcsetattr(fd, TCSANOW, attr);
}

/*

C<int tty_noecho(int fd)>

Sets the terminal descriptor, C<fd>, to noecho mode. On success, returns
C<0>. On error, returns C<-1> with C<errno> set appropriately.

*/

static int tty_noecho(int fd)
{
	struct termios attr[1];

	debug((1, "tty_noecho(fd = %d)", fd))

	debug((2, "tcgetattr(fd = %d)", fd))

	if (tcgetattr(fd, attr) == -1)
		return errorsys("failed to get terminal attributes for the process side of the pty");

	attr->c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL);
#ifdef ONLCR
	attr->c_oflag &= ~ONLCR;
#endif

	debug((2, "tcsetattr(fd = %d, TCSANOW, noecho)", fd))

	return tcsetattr(fd, TCSANOW, attr);
}

/*

C<void restore_stdin(void)>

Restore stdin's saved terminal attributes and initialise the terminal on exit.

*/

static void restore_stdin(void)
{
	debug((1, "restore_stdin()"))

	debug((2, "tcsetattr(stdin, TCSANOW, orig)"))

	if (tcsetattr(STDIN_FILENO, TCSANOW, &g.stdin_termios) == -1)
		errorsys("failed to restore stdin terminal attributes");
}

#ifdef HAVE_LOGIND
/*

C<void unbind(void)>

Unbind the client from the systemd-logind/elogind session. Close the logind
monitor file descriptor, and release the logind monitor object.

*/

static void unbind(void)
{
	debug((1, "unbind"))

	if (g.logind_monitor_fd != -1)
	{
		debug((2, "close g.logind_monitor_fd = %d", g.logind_monitor_fd))

		close(g.logind_monitor_fd);
		g.logind_monitor_fd = -1;
	}

	if (g.logind_monitor != null)
	{
		debug((2, "release g.logind_monitor"))

		sd_login_monitor_unref(g.logind_monitor);
		g.logind_monitor = null;
	}

	debug((2, "reset g.bind = 0"))

	g.bind = 0;
}
#endif

/*

C<void prepare_parent(void)>

Before forking, set the term, chld and usr1 signal handlers.
If --foreground and stdin isatty, prepare for the pseudo terminal.
If --bind, setup the systemd-logind/elogind monitor.

*/

static void prepare_parent(void)
{
	debug((1, "prepare_parent()"))

	/* Before forking, set the term, chld and usr1 signal handlers */

	debug((2, "setting sigterm action"))

	if (signal_set_handler(SIGTERM, 0, term) == -1)
		fatalsys("failed to set sigterm action");

	debug((2, "setting sigchld action"))

	if (signal_set_handler(SIGCHLD, 0, chld) == -1)
		fatalsys("failed to set sigchld handler");

	debug((2, "setting sigusr1 action"))

	if (signal_set_handler(SIGUSR1, 0, usr1) == -1)
		fatalsys("failed to set sigusr1 action");

	/* If --foreground and stdin isatty, prepare for the pseudo terminal */

	if (g.foreground && isatty(STDIN_FILENO))
	{
		debug((2, "saving stdin's terminal attributes"))

		/* Get stdin's terminal attributes and window size */

		debug((2, "tcgetattr(stdin)"))

		if (tcgetattr(STDIN_FILENO, &g.stdin_termios) == -1)
			errorsys("failed to get terminal attributes for stdin");

		debug((2, "ioctl(stdin, TIOCGWINSZ)"))

		if (ioctl(STDIN_FILENO, TIOCGWINSZ, &g.stdin_winsize) == -1)
			errorsys("failed to get terminal window size for stdin");

		/* Set stdin to raw mode (let the process side of the pty do everything) */

		if (tty_raw(STDIN_FILENO) == -1)
			errorsys("failed to set stdin to raw mode");

		/* Restore stdin's terminal settings on exit */

		debug((2, "atexit(restore_stdin)"))

		if (atexit((void (*)(void))restore_stdin) == -1)
			errorsys("failed to atexit(restore_stdin)");

		g.stdin_isatty = 1;
	}

#ifdef HAVE_LOGIND
	/*
	** If --bind, setup a systemd-logind/elogind monitor object and file descriptor,
	** so we can bind the client to the duration of the user's logind session.
	**
	** If it fails, continue unbound. This seems more resilient than
	** treating it as a fatal error, and refusing to continue. But it does
	** mean that the client will continue to run after the user logs out.
	** That might be a problem in some cases. Naming the daemon to ensure a
	** singleton client might help in such cases.
	**
	** Is another option needed to make such a failure fatal?
	** Are these calls likely to ever fail? Only if out of memory.
	** If that happens, we'll probably fail to start the client anyway.
	*/

	if (g.bind)
	{
		int ret;

		debug((2, "sd_login_monitor_new(\"uid\")"))

		if ((ret = sd_login_monitor_new("uid", &g.logind_monitor)) < 0)
		{
			errno = -ret;
			errorsys("failed to bind to the logind session (continuing unbound): sd_login_monitor_new");
			unbind();
		}
	}

	if (g.bind)
	{
		int ret;

		debug((2, "sd_login_monitor_get_fd"))

		if ((ret = sd_login_monitor_get_fd(g.logind_monitor)) < 0)
		{
			errno = -ret;
			errorsys("failed to bind to the logind session (continuing unbound): sd_login_monitor_get_fd");
			unbind();
		}
		else
		{
			g.logind_monitor_fd = ret;
		}
	}
#endif
}

/*

C<void prepare_child(void *data)>

Reset the default signal handlers for C<SIGTERM> and C<SIGCHLD>. Called by
I<coproc_open(3)> or I<coproc_pty_open(3)> in the child process.

*/

static void prepare_child(void *data)
{
	debug((1, "prepare_child()"))

	debug((2, "child pid = %d", getpid()))

	debug((2, "child restoring sigterm action"))

	if (signal_set_handler(SIGTERM, 0, SIG_DFL) == -1)
		fatalsys("failed to restore sigterm action, exiting");

	debug((2, "child restoring sigchld action"))

	if (signal_set_handler(SIGCHLD, 0, SIG_DFL) == -1)
		fatalsys("failed to restore sigchld action, exiting");

	if (g.stdin_isatty)
	{
		debug((2, "child restoring sigwinch action"))

		if (signal_set_handler(SIGWINCH, 0, SIG_DFL) == -1)
			fatalsys("failed to restore sigwinch action, exiting");
	}

	if (g.noecho)
	{
		debug((2, "child setting the process side of the pty to noecho mode"))

		if (tty_noecho(STDIN_FILENO) == -1)
			fatalsys("failed to set noecho on the process side of the pty");
	}
}

/*

C<int construct_clientpidfile(const char *name, char **clientpidfile)>

Constructs the clientpidfile for the given C<name> in C<clientpidfile>. If
C<name> is already an absolute path, it is interpreted as the path of a
pidfile and its C<.pid> suffix is just changed to C<.clientpid> in the new
buffer. On success, returns C<0>, and the resulting buffer in
C<clientpidfile> must be deallocated by the caller. On error, returns C<-1>
with C<errno> set appropriately.

*/

static int construct_clientpidfile(const char *name, char **clientpidfile)
{
	long path_len;
	const char *pid_dir;
	char *suffix = ".pid";
	size_t size;
	char *pidfile = null;
	size_t len;

	/* Construct the pidfile */

	path_len = limit_path();
	pid_dir = (getuid()) ? USER_PID_DIR : ROOT_PID_DIR;
	size = ((*name == PATH_SEP) ? strlen(name) : sizeof(pid_dir) + 1 + strlen(name) + strlen(suffix)) + 1;

	if (size > path_len)
		return set_errno(ENAMETOOLONG);

	if (!(pidfile = mem_create(path_len, char)))
		return -1;

	if (*name == PATH_SEP)
		snprintf(pidfile, path_len, "%s", name);
	else
		snprintf(pidfile, path_len, "%s%c%s%s", pid_dir, PATH_SEP, name, suffix);

	/* Replace .pid suffix with .clientpid */

	len = strlen(pidfile);

	if (asprintf(clientpidfile, "%.*s.clientpid", (int)len - 4, pidfile) == -1)
	{
		mem_destroy(&pidfile);
		return -1;
	}

	mem_destroy(&pidfile);

	return 0;
}

/*

C<int create_clientpidfile(void)>

Like I<daemon_pidfile()> except that this creates a pidfile containing the
client process's pid rather than the current process's pid and the filename
suffix is C<.clientpid> rather than C<.pid>.

*/

static int create_clientpidfile(void)
{
	char *clientpidfile = null;
	int clientpid_fd;
	char clientpid[32];

	/* Build the clientpidfile path */

	if (construct_clientpidfile(g.daemon_init_name, &clientpidfile) == -1)
		return -1;

	debug((2,"create_clientpidfile %s", clientpidfile))

	/* Open it */

	if ((clientpid_fd = open(clientpidfile, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) == -1)
	{
		mem_destroy(&clientpidfile);
		return -1;
	}

	/* Store our clientpid */

	snprintf(clientpid, 32, "%d\n", (int)g.pid);

	if (write(clientpid_fd, clientpid, strlen(clientpid)) != strlen(clientpid))
	{
		unlink(clientpidfile);
		close(clientpid_fd);
		mem_destroy(&clientpidfile);
		return -1;
	}

	close(clientpid_fd);
	mem_destroy(&clientpidfile);

	return 0;
}

/*

C<int unlink_clientpidfile(void)>

Unlinks the clientpidfile created by I<create_clientpidfile()>.

*/

static int unlink_clientpidfile(void)
{
	char *clientpidfile = null;

	/* Build the clientpidfile path */

	if (construct_clientpidfile(g.daemon_init_name, &clientpidfile) == -1)
		return -1;

	debug((2,"unlink_clientpidfile %s", clientpidfile))

	/* Unlink it */

	debug((2, "unlinking clientpidfile %s", clientpidfile))
	unlink(clientpidfile);
	mem_destroy(&clientpidfile);

	return 0;
}

/*

C<pid_t getclientpid(const char *name)>

Return the process id of the client process. The I<name> argument is the
same as the name passed to I<daemon_init()>. On success, returns the process
id of the daemon's client process. On error, returns C<-1> with C<errno> set
appropriately.

*/

static pid_t getclientpid(const char *name)
{
	char *clientpidfile = null;
	char buf[BUFSIZ];
	ssize_t bytes;
	int clientpid_fd;
	int clientpid = 0;

	/* Build the clientpidfile path */

	if (construct_clientpidfile(name, &clientpidfile) == -1)
		return -1;

	/* Open the clientpidfile */

	clientpid_fd = open(clientpidfile, O_RDONLY);
	mem_release(clientpidfile);

	if (clientpid_fd == -1)
		return -1;

	/* Read it */

	bytes = read(clientpid_fd, buf, BUFSIZ);
	close(clientpid_fd);

	if (bytes == -1)
		return -1;

	if (sscanf(buf, "%d", &clientpid) != 1)
		return -1;

	return (pid_t)clientpid;
}

/*

C<void spawn_child(void)>

Spawn the client process. If this is not the first time the client has been
spawned and the previous instance lasted less than C<--delay> seconds,
respawn immediately if there have been fewer that C<--attempts> attempts in
the current burst. Otherwise, first wait for C<--delay> seconds unless we
have reached C<--limit> bursts (in which case we terminate). If the clock
has gone backwards, the spawn time of the previous instance is first reset
to the current time.

When spawning the client process in the foreground and either C<stdin> is a
terminal or the C<--pty> option was supplied, use I<coproc_pty_open(3)>,
otherwise use I<coproc_open(3)>. The child process restores default signal
actions for C<SIGTERM> and C<SIGCHLD>.

*/

static void spawn_child(void)
{
	time_t spawn_time;

	debug((1, "spawn_child()"))
	g.received_sigchld = 0;
	debug((2, "g.received_sigchld=%d", g.received_sigchld))

	if ((spawn_time = time(0)) == -1)
		fatalsys("failed to get the time");

	if (g.spawn_time)
	{
		debug((2, "preparing to respawn"))

		/* Assume zero duration if clock has gone backwards */

		if (spawn_time < g.spawn_time)
		{
			debug((2, "clock has gone backwards, resetting previous spawn time to now"))

			g.spawn_time = spawn_time;
		}

		/* Handle failed respawn attempts - burst, wait, burst, wait, ... */

		if (spawn_time - g.spawn_time < g.acceptable)
		{
			debug((2, "previous instance only lasted %d second%s", spawn_time - g.spawn_time, ((spawn_time - g.spawn_time) == 1) ? "" : "s"))

			if (++g.attempt >= g.attempts)
			{
				if (g.limit && ++g.burst >= g.limit)
					fatal("reached respawn attempt burst limit (%d), exiting", g.limit);

				error("terminating too quickly, waiting %d second%s", g.delay, (g.delay == 1) ? "" : "s");

				while (nap(g.delay, 0) == -1 && errno == EINTR)
				{
					signal_handle_all();

					if (g.terminated)
						fatal("terminated");
				}

				error("end of %d second respawn attempt burst delay", g.delay);

				if ((spawn_time = time(0)) == -1)
					fatalsys("failed to get the time");

				g.attempt = 0;
			}
		}
	}

	g.spawn_time = spawn_time;

	debug((2, "starting client"))

	if (g.foreground && (g.stdin_isatty || g.pty))
	{
		struct termios *pty_device_termios = null;
		struct winsize *pty_device_winsize = null;

		debug((2, "foreground with pty: coproc_pty_open()"))

		if (g.stdin_isatty)
		{
			pty_device_termios = &g.stdin_termios;
			pty_device_winsize = &g.stdin_winsize;

			debug((2, "setting sigwinch handler"))

			if (signal_set_handler(SIGWINCH, 0, winch) == -1)
				errorsys("failed to set sigwinch action");
		}

		if ((g.pid = coproc_pty_open(&g.pty_user_fd, g.pty_device_name, g.pty_device_name_size, pty_device_termios, pty_device_winsize, g.cmdpath, g.cmd, (g.env) ? g.environ : environ, prepare_child, null)) == -1)
			fatalsys("failed to start: %s", g.cmdpath);
	}
	else
	{
		debug((2, "no pty: coproc_open()"))

		if ((g.pid = coproc_open(&g.in, &g.out, &g.err, g.cmdpath, g.cmd, (g.env) ? g.environ : environ, prepare_child, null)) == -1)
			fatalsys("failed to start: %s", g.cmdpath);
	}

	debug((2, "parent pid = %d, child pid = %d", (int)getpid(), (int)g.pid))

	/* Create client pidfile */

	if (g.daemon_init_name)
	{
		debug((2, "creating client pidfile"))

		if (create_clientpidfile() == -1)
			errorsys("failed to create client pidfile");
	}
}

/*

C<void examine_child(void)>

Wait for the child process specified by C<g.pid>. Calls I<coproc_close(3)>
or I<coproc_pty_close(3)> depending on how the child process was started. If
we need to respawn the client, do so. Otherwise, we exit. So, if this
function returns at all, a new child will have been spawned.

*/

static void examine_child(void)
{
	int status;

	debug((1, "examine_child(pid = %d)", (int)g.pid))

	if (g.pty_user_fd != -1)
	{
		debug((2, "coproc_pty_close(pid = %d, pty_user_fd = %d, pty_device_name = %s)", (int)g.pid, g.pty_user_fd, g.pty_device_name))

		while ((status = coproc_pty_close(g.pid, &g.pty_user_fd, g.pty_device_name)) == -1 && errno == EINTR)
			signal_handle_all();

		if (status == -1)
			errorsys("coproc_pty_close(pid = %d) failed", (int)g.pid);
	}
	else
	{
		debug((2, "coproc_close(pid = %d, in = %d, out = %d, err = %d)", (int)g.pid, g.in, g.out, g.err))

		while ((status = coproc_close(g.pid, &g.in, &g.out, &g.err)) == -1 && errno == EINTR)
			signal_handle_all();

		if (status == -1)
			errorsys("coproc_close(pid = %d) failed", (int)g.pid);
	}

	if (status != -1)
	{
		debug((2, "pid %d received sigchld for pid %d", getpid(), (int)g.pid))

		if (WIFEXITED(status))
		{
			debug((2, "child terminated with status %d", WEXITSTATUS(status)))

			if (WEXITSTATUS(status) != EXIT_SUCCESS)
			{
				if (g.terminated)
					error("client (pid %d) exited with %d status, stopping", (int)g.pid, WEXITSTATUS(status));
				else if (g.respawn)
					error("client (pid %d) exited with %d status, respawning", (int)g.pid, WEXITSTATUS(status));
				else
					error("client (pid %d) exited with %d status, exiting", (int)g.pid, WEXITSTATUS(status));
			}
		}
		else if (WIFSIGNALED(status))
		{
			if (g.terminated)
				error("client (pid %d) killed by signal %d, stopping", (int)g.pid, WTERMSIG(status));
			else if (g.respawn)
				error("client (pid %d) killed by signal %d, respawning", (int)g.pid, WTERMSIG(status));
			else
				error("client (pid %d) killed by signal %d, exiting", (int)g.pid, WTERMSIG(status));
		}
		else if (WIFSTOPPED(status)) /* can't happen - we didn't set WUNTRACED */
		{
			error("client (pid %d) stopped by signal %d, exiting", (int)g.pid, WSTOPSIG(status));
		}
		else /* can't happen - there are no other options */
		{
			error("client (pid %d) died under mysterious circumstances, exiting", (int)g.pid);
		}
	}

	g.pid = (pid_t)0;

	if (g.daemon_init_name)
	{
		debug((2, "about to unlink clientpidfile"))

		if (unlink_clientpidfile() == -1)
			errorsys("failed to unlink client pidfile");
	}

	if (g.respawn && !g.terminated)
	{
		debug((2, "about to respawn"))
		spawn_child();
	}
	else
	{
		debug((2, "%schild terminated, exiting", (g.terminated) ? "daemon and " : ""))

#ifdef HAVE_LOGIND
		if (g.bind)
			unbind();
#endif

		exit(EXIT_SUCCESS);
	}
}

/*

C<void run(void)>

The main run loop. Calls I<prepare_parent()> and I<spawn_child()>. Send the
client's stdout/stderr to syslog or a file (or to the user) if necessary.
Send any input to the client if necessary. Handle any signals that arrive in
the meantime. When there is no more to read from the client (either the
client has died or it has closed stdout and stderr), just wait for the
client to terminate.

*/

static void run(void)
{
	debug((1, "run()"))

	prepare_parent();
	spawn_child();

	for (;;)
	{
		debug((2, "run loop - outer loop"))

		for (;;)
		{
			char buf[BUFSIZ + 1];
			fd_set readfds[1];
			int maxfd = -1;
			int n;

			debug((2, "run loop - handle any signals"))

			signal_handle_all();

			/* Signals arriving between here and select are lost */

			if (!g.read_eof && g.received_sigchld)
			{
				debug((2, "received sigchld, skipping any final output (to avoid zombies)"))
				break;
			}

			if (g.pty_user_fd == -1 && g.out == -1 && g.err == -1)
			{
				debug((2, "all outputs closed, skipping select"))
				break;
			}

			debug((2, "select(%s) preparation", (g.pty_user_fd != -1) ? "pty" : "pipes"))

			FD_ZERO(readfds);

			if (g.foreground)
			{
				if (!g.stdin_eof)
				{
					debug((9, "select() preparation readfds += stdin = fd %d", STDIN_FILENO))

					FD_SET(STDIN_FILENO, readfds);
					if (STDIN_FILENO > maxfd)
						maxfd = STDIN_FILENO;
				}
			}
			else
			{
				if (g.in != -1)
				{
					debug((9, "select() preparation close g.in = fd %d", g.in))

					if (close(g.in) == -1)
						errorsys("failed to close(in = %d)", g.in);

					g.in = -1;
				}
			}

			if (g.pty_user_fd != -1)
			{
				debug((9, "select() preparation readfds += g.pty_user_fd = fd %d", g.pty_user_fd))

				FD_SET(g.pty_user_fd, readfds);
				if (g.pty_user_fd > maxfd)
					maxfd = g.pty_user_fd;
			}
			else
			{
				if (g.out != -1)
				{
					debug((9, "select() preparation readfds += g.out = fd %d", g.out))

					FD_SET(g.out, readfds);
					if (g.out > maxfd)
						maxfd = g.out;
				}

				if (g.err != -1)
				{
					debug((9, "select() preparation readfds += g.err = fd %d", g.err))

					FD_SET(g.err, readfds);
					if (g.err > maxfd)
						maxfd = g.err;
				}
			}

#ifdef HAVE_LOGIND
			if (g.bind)
			{
				debug((9, "select() preparation readfds += g.logind_monitor_fd = fd %d", g.logind_monitor_fd))

				FD_SET(g.logind_monitor_fd, readfds);
				if (g.logind_monitor_fd > maxfd)
					maxfd = g.logind_monitor_fd;
			}
#endif

			debug((2, "select(%s)", (g.pty_user_fd != -1) ? "pty" : "pipes"))

			if ((n = select(maxfd + 1, readfds, null, null, null)) == -1 && errno != EINTR)
			{
				errorsys("failed to select(2): refusing to handle client %soutput anymore", g.foreground ? "input/" : "");
				break;
			}

			if (n == -1 && errno == EINTR)
			{
				debug((9, "select() was interrupted by a signal"))
				continue;
			}

			debug((9, "select(%s) returned %d", (g.pty_user_fd != -1) ? "pty" : "pipes", n))

			if (g.out != -1 && FD_ISSET(g.out, readfds))
			{
				if ((n = read(g.out, buf, BUFSIZ)) > 0)
				{
					char *p, *q;

					debug((2, "read(out) returned %d", n))
					buf[n] = '\0';

					if (g.foreground)
						if (write(STDOUT_FILENO, buf, n) == -1)
							errorsys("failed to write(fd stdout, buf %*.*s)", n, n, buf);

					if (g.client_outfd != -1)
					{
						debug((2, "writing client stdout (fd %d, %d bytes)", g.client_outfd, n))

						if (write(g.client_outfd, buf, n) == -1)
							errorsys("failed to write(client_outfd = %d)", g.client_outfd);
					}

					if (g.client_outlog)
					{
						for (p = buf; (q = strchr(p, '\n')); p = q + 1)
						{
							debug((2, "stdout syslog(%s, %*.*s)", g.client_out, (int)(q - p), (int)(q - p), p))
							syslog(g.client_outlog, "%*.*s", (int)(q - p), (int)(q - p), p);
						}

						if (*p && (*p != '\n' || p[1] != '\0'))
						{
							debug((2, "stdout syslog(%s, %s)", g.client_out, p))
							syslog(g.client_outlog, "%s", p);
						}
					}
				}
				else if (n == -1 && errno == EINTR)
				{
					debug((2, "read(out) was interrupted by a signal\n"))
					continue;
				}
				else if (n == -1)
				{
					errorsys("read(out) failed, refusing to handle client stdout anymore");

					if (close(g.out) == -1)
						errorsys("failed to close(out = %d)", g.out);

					g.out = -1;
				}
				else /* eof */
				{
					debug((2, "read(out) returned %d, closing out", n))

					if (close(g.out) == -1)
						errorsys("failed to close(out = %d)", g.out);

					g.out = -1;
				}
			}

			if (g.err != -1 && FD_ISSET(g.err, readfds))
			{
				if ((n = read(g.err, buf, BUFSIZ)) > 0)
				{
					char *p, *q;

					debug((2, "read(err) returned %d", n))
					buf[n] = '\0';

					if (g.foreground)
						if (write(STDERR_FILENO, buf, n) == -1)
							errorsys("failed to write(fd stderr, buf %*.*s)", n, n, buf);

					if (g.client_errfd != -1)
					{
						debug((2, "writing client stderr (fd %d, %d bytes)", g.client_errfd, n))

						if (write(g.client_errfd, buf, n) == -1)
							errorsys("failed to write(client_errfd = %d)", g.client_errfd);
					}

					if (g.client_errlog)
					{
						for (p = buf; (q = strchr(p, '\n')); p = q + 1)
						{
							debug((2, "stderr syslog(%s, %*.*s)", g.client_err, (int)(q - p), (int)(q - p), p))
							syslog(g.client_errlog, "%*.*s", (int)(q - p), (int)(q - p), p);
						}

						if (*p && (*p != '\n' || p[1] != '\0'))
						{
							debug((2, "stderr syslog(%s, %s)", g.client_err, p))
							syslog(g.client_errlog, "%s", p);
						}
					}
				}
				else if (n == -1 && errno == EINTR)
				{
					debug((2, "read(err) was interrupted by a signal\n"))
					continue;
				}
				else if (n == -1)
				{
					errorsys("read(err) failed, refusing to handle client stderr anymore");

					if (close(g.err) == -1)
						errorsys("failed to close(err = %d)", g.err);

					g.err = -1;
				}
				else /* eof */
				{
					debug((2, "read(err) returned %d, closing err", n))

					if (close(g.err) == -1)
						errorsys("failed to close(err = %d)", g.err);

					g.err = -1;
				}
			}

			if (g.pty_user_fd != -1 && FD_ISSET(g.pty_user_fd, readfds))
			{
				if ((n = read(g.pty_user_fd, buf, BUFSIZ)) > 0)
				{
					char *p, *q;

					debug((2, "read(pty_user_fd) returned %d", n))
					buf[n] = '\0';

					if (g.foreground)
						if (write(STDOUT_FILENO, buf, n) == -1)
							errorsys("failed to write(fd stdout, buf %*.*s)", n, n, buf);

					if (g.client_outfd != -1)
					{
						debug((2, "writing client stdout/stderr (fd %d, %d bytes)", g.client_outfd, n))

						if (write(g.client_outfd, buf, n) == -1)
							errorsys("failed to write(client_outfd = %d)", g.client_outfd);
					}

					if (g.client_outlog)
					{
						for (p = buf; (q = strchr(p, '\n')); p = q + 1)
						{
							debug((2, "stdout syslog(%s, %*.*s)", g.client_out, (int)(q - p), (int)(q - p), p))
							syslog(g.client_outlog, "%*.*s", (int)(q - p), (int)(q - p), p);
						}

						if (*p && (*p != '\n' || p[1] != '\0'))
						{
							debug((2, "stdout syslog(%s, %s)", g.client_out, p))
							syslog(g.client_outlog, "%s", p);
						}
					}
				}
				else if (n == -1 && errno == EINTR)
				{
					debug((2, "read(pty_user_fd) was interrupted by a signal\n"))
					continue;
				}
				else if (n == -1)
				{
					if (errno != EIO)
						errorsys("read(pty_user_fd) failed, refusing to handle client output anymore");

					break;
				}
				else /* eof */
				{
					debug((2, "read(pty_user_fd) returned %d, closing pty_user_fd", n))
					break;
				}
			}

			if (g.foreground && FD_ISSET(STDIN_FILENO, readfds))
			{
				if ((n = read(STDIN_FILENO, buf, BUFSIZ)) > 0)
				{
					debug((2, "read(stdin) returned %d", n))
					buf[n] = '\0';

					if (g.pty_user_fd != -1)
					{
						if (write(g.pty_user_fd, buf, n) != n)
						{
							errorsys("failed to write(pty_user_fd = %d)", g.pty_user_fd);
							break;
						}
					}
					else if (g.in != -1)
					{
						if (write(g.in, buf, n) != n)
						{
							errorsys("failed to write(in = %d), closing in", g.in);

							if (close(g.in) == -1)
								errorsys("failed to close(in = %d)", g.in);

							g.in = -1;
						}
					}
				}
				else if (n == -1 && errno == EINTR)
				{
					debug((2, "read(stdin) was interrupted by a signal\n"))
					continue;
				}
				else /* error or eof */
				{
					if (g.pty_user_fd != -1)
					{
						struct termios attr[1];
						char eof = CEOF;

						if (tcgetattr(g.pty_user_fd, attr) == -1)
							errorsys("failed to get terminal attributes for pty_user_fd = %d", g.pty_user_fd);
						else
							eof = attr->c_cc[VEOF];

						debugsys((2, "read(stdin) returned %d, sending eof(%d) to pty_user_fd", n, (int)eof))

						if (write(g.pty_user_fd, &eof, 1) == -1)
						{
							errorsys("failed to write(pty_user_fd = %d) when sending eof (%d)", g.pty_user_fd, (int)eof);
							break;
						}
					}
					else if (g.in != -1)
					{
						debugsys((2, "read(stdin) returned %d, closing in", n))

						if (close(g.in) == -1)
							errorsys("failed to close(in = %d)", g.in);

						g.in = -1;
					}

					g.stdin_eof = 1;
				}
			}

#ifdef HAVE_LOGIND
			if (g.bind && !g.terminated)
			{
				int ret;

				if ((ret = sd_login_monitor_flush(g.logind_monitor)) < 0)
				{
					errno = -ret;
					errorsys("failed to reset logind monitor fd (continuing unbound): sd_login_monitor_flush");
					unbind();
				}
				else
				{
					uid_t uid = g.uid ? g.uid : getuid();
					int num_sessions = sd_uid_get_sessions(uid, 0, null);

					if (num_sessions < 0)
					{
						errno = -num_sessions;
						errorsys("failed to count logind sessions (continuing unbound): sd_uid_get_sessions(%d)", uid);
						unbind();
					}

					/*
					** Stop the client if the user has logged out.
					** Note: If there is no user session when we start,
					** this will terminate the client immediately.
					** Should we wait until there has been a logind session
					** before looking for the absense of a logind session?
					*/

					if (num_sessions == 0)
					{
						debug((2, "bound to logind session that no longer exists, automatically terminating"))

						unbind();
						term(SIGTERM);
					}
				}
			}
#endif
		}

		debug((2, "no more output, just wait for child to terminate"))

		examine_child();
	}
}

/*

C<void show(void)>

Emit a debug message that shows the current configuration.

*/

static void show(void)
{
	int i;

	debug((1, "show()"))

	debug((2, "options:"))

	debug((2, " config %s, noconfig %d, name %s, command \"%s\", pidfiles %s, pidfile %s, uid %d, gid %d, init_groups %d, chroot %s, chdir %s, umask %03o, inherit %s, respawn %s, acceptable %d, attempts %d, delay %d, limit %d, idiot %d, foreground %s, pty %s, noecho %s, bind %s, stdout %s%s%s%s, stderr %s%s%s%s, errlog %s%s%s%s, dbglog %s%s%s%s, core %s, unsafe %s, safe %s, read_eof %s, stop %s, running %s, restart %s, signame %s, signo %d, list %s, verbose %d, debug %d",
		g.config ? g.config : "<none>",
		g.noconfig,
		g.name ? g.name : "<none>",
		g.command ? g.command : "<none>",
		g.pidfiles ? g.pidfiles : "<none>",
		g.pidfile ? g.pidfile : "<none>",
		g.uid,
		g.gid,
		g.init_groups,
		g.chroot ? g.chroot : "<none>",
		g.chdir ? g.chdir : "<none>",
		g.umask,
		g.inherit ? "yes" : "no",
		g.respawn ? "yes" : "no",
		g.acceptable,
		g.attempts,
		g.delay,
		g.limit,
		g.idiot,
		g.foreground ? "yes" : "no",
		g.pty ? "yes" : "no",
		g.noecho ? "yes" : "no",
		g.bind ? "yes" : "no",
		g.client_outlog ? syslog_facility_str(g.client_outlog) : "",
		g.client_outlog ? "." : "",
		g.client_outlog ? syslog_priority_str(g.client_outlog) : "",
		g.client_outlog ? "" : g.client_out ? g.client_out : "<none>",
		g.client_errlog ? syslog_facility_str(g.client_errlog) : "",
		g.client_errlog ? "." : "",
		g.client_errlog ? syslog_priority_str(g.client_errlog) : "",
		g.client_errlog ? "" : g.client_err ? g.client_out : "<none>",
		g.daemon_errlog ? syslog_facility_str(g.daemon_errlog) : "",
		g.daemon_errlog ? "." : "",
		g.daemon_errlog ? syslog_priority_str(g.daemon_errlog) : "",
		g.daemon_errlog ? "" : g.daemon_err ? g.client_out : "<none>",
		g.daemon_dbglog ? syslog_facility_str(g.daemon_dbglog) : "",
		g.daemon_dbglog ? "." : "",
		g.daemon_dbglog ? syslog_priority_str(g.daemon_dbglog) : "",
		g.daemon_dbglog ? "" : g.daemon_dbg ? g.client_out : "<none>",
		g.core ? "yes" : "no",
		g.unsafe ? "yes" : "no",
		g.safe ? "yes" : "no",
		g.read_eof ? "yes" : "no",
		g.stop ? "yes" : "no",
		g.running ? "yes" : "no",
		g.restart ? "yes" : "no",
		g.signame ? g.signame : "<none>",
		g.signo,
		g.list ? "yes" : "no",
		prog_verbosity_level(),
		prog_debug_level()
	))

	debug((2, "command line:"))

	if (g.cmd)
	{
		for (i = 0; g.cmd[i]; ++i)
			debug((2, " argv[%d] = \"%s\"", i, g.cmd[i]))
	}

	if (g.cmdpath)
	{
		debug((2, " cmdpath = \"%s\"", g.cmdpath))
	}

	debug((3, "environment:"))

	for (i = 0; (g.environ ? g.environ : environ)[i]; ++i)
	{
		debug((3, " %s", (g.environ ? g.environ : environ)[i]))
	}
}

/*

C<int safety_check_script(const char *cmd)>

If C<cmd> refers to a script, checks that the interpreter is safe. On
success, returns C<1> if the interpreter is safe, or C<0> if it isn't. On
error, returns C<-1> with C<errno> set appropriately.

*/

static int safety_check(const char *cmd, char *explanation, size_t explanation_size);

static int safety_check_script(const char *cmd, char *explanation, size_t explanation_size)
{
	char intbuf[256];
	ssize_t bytes;
	size_t end;
	int ret;
	int fd;

	/*
	** Note: Shell scripts without #! will still work because the
	** coprocess functions will try /bin/sh if execve(2) fails.
	** Since we can't tell whether or not this will happen without
	** calling execve(2), we assume that /bin/sh is not group- or
	** world-writable.
	*/

	if ((fd = open(cmd, O_RDONLY)) != -1)
	{
		bytes = read(fd, intbuf, 256);
		intbuf[255] = nul;
		close(fd);

		if (bytes > 0 && intbuf[0] == '#' && intbuf[1] == '!')
		{
			if ((end = strcspn(intbuf + 2, " \n")))
			{
				char oldch = intbuf[2 + end];

				intbuf[2 + end] = nul;

				debug((2, "checking #! interpreter: %s", intbuf + 2))

				if ((ret = daemon_path_is_safe(intbuf + 2, explanation, explanation_size)) != 1)
					return ret;

				/* If it's "#!/usr/bin/env cmd", check the cmd */

				if (!strncmp(intbuf + 2, "/usr/bin/env", 12) && is_space(oldch))
				{
					if ((end = strcspn(intbuf + 15, " \n")))
						intbuf[15 + end] = nul;

					debug((2, "checking interpreter (via env): %s", intbuf + 15))

					if ((ret = safety_check(intbuf + 15, explanation, explanation_size)) != 1)
						return ret;
				}
			}
		}
	}

	return 1;
}

/*

C<int safety_check(const char *cmd)>

Determine whether or not C<cmd> is safe (searching for it in C<$PATH> if
necessary). If C<cmd> refers to an executable script with a C<#!> line, then
extract the path of the interpreter and determine whether or not it is safe.
If C<cmd> refers to an executable script with a C<#!/usr/bin/env> line, then
search for the real interpreter and determine whether or not it is safe. On
success, returns C<1> if the interpreter is safe, or C<0> if it isn't. On
error, returns C<-1> with C<errno> set appropriately.

*/

static int safety_check(const char *cmd, char *explanation, size_t explanation_size)
{
	struct stat status[1];
	char cmdbuf[512];
	char *path, *s, *f;
	int ret;

	debug((1, "safety_check(\"%s\")", cmd))

	/* Path is absolute (starts with "/") or relative (contains "/") */

	if (cmd[0] == PATH_SEP || strchr(cmd, PATH_SEP))
	{
		if (!(path = daemon_absolute_path(cmd)))
			return -1;

		debug((2, "checking \"%s\"", path))

		if ((ret = daemon_path_is_safe(path, explanation, explanation_size)) != 1)
		{
			mem_release(path);
			return ret;
		}

		ret = safety_check_script(path, explanation, explanation_size);
		mem_release(path);

		return ret;
	}

	/* Search $PATH */

	if (!(path = getenv("PATH")))
		path = geteuid() ? DEFAULT_USER_PATH : DEFAULT_ROOT_PATH;

	debug((2, "PATH = %s", path))

	for (s = path; s; s = (*f) ? f + 1 : null)
	{
		if (!(f = strchr(s, PATH_LIST_SEP)))
			f = s + strlen(s);

		if (snprintf(cmdbuf, 512, "%.*s%s%s", (int)(f - s), s, (f - s) ? PATH_SEP_STR : "", cmd) >= 512)
			continue;

		/* Check if it exists and is executable */

		if (stat(cmdbuf, status) == -1)
		{
			if (errno != ENOENT)
				errorsys("failed to stat(\"%s\")", cmdbuf);

			continue;
		}

		if (status->st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))
		{
			debug((2, "checking \"%s\"", cmdbuf))

			if ((ret = daemon_path_is_safe(cmdbuf, explanation, explanation_size)) != 1)
				return ret;

			return safety_check_script(cmdbuf, explanation, explanation_size);
		}
	}

	return set_errno(ENOENT);
}

/*

C<void sanity_check(void)>

Checks that there are no consistency errors in the options supplied.

*/

static void sanity_check(void)
{
	struct stat status[1];

	debug((1, "sanity_check()"))

	if (g.acceptable != RESPAWN_ACCEPTABLE && !g.respawn)
		prog_usage_msg("Missing option: --respawn (Required for --acceptable)");

	if (g.attempts != RESPAWN_ATTEMPTS && !g.respawn)
		prog_usage_msg("Missing option: --respawn (Required for --attempts)");

	if (g.delay != RESPAWN_DELAY && !g.respawn)
		prog_usage_msg("Missing option: --respawn (Required for --delay)");

	if (g.limit != RESPAWN_LIMIT && !g.respawn)
		prog_usage_msg("Missing option: --respawn (Required for --limit)");

	if (g.pty && !g.foreground)
		prog_usage_msg("Missing option: --foreground (Required for --pty)");

	if (g.stop && !g.name)
		prog_usage_msg("Missing option: --name (Required for --stop)");

	if (g.running && !g.name)
		prog_usage_msg("Missing option: --name (Required for --running)");

	if (g.restart && !g.name)
		prog_usage_msg("Missing option: --name (Required for --restart)");

	if (g.signame && !g.name)
		prog_usage_msg("Missing option: --name (Required for --signal)");

	if (g.list && g.name)
		prog_usage_msg("Incompatible options: --list and --name");

	if (g.running && g.restart)
		prog_usage_msg("Incompatible options: --running and --restart");

	if (g.running && g.stop)
		prog_usage_msg("Incompatible options: --running and --stop");

	if (g.running && g.signame)
		prog_usage_msg("Incompatible options: --running and --signal");

	if (g.running && g.list)
		prog_usage_msg("Incompatible options: --running and --list");

	if (g.restart && g.stop)
		prog_usage_msg("Incompatible options: --restart and --stop");

	if (g.restart && g.signame)
		prog_usage_msg("Incompatible options: --restart and --signal");

	if (g.restart && g.list)
		prog_usage_msg("Incompatible options: --restart and --list");

	if (g.stop && g.signame)
		prog_usage_msg("Incompatible options: --stop and --signal");

	if (g.stop && g.list)
		prog_usage_msg("Incompatible options: --stop and --list");

	if (g.signame && g.list)
		prog_usage_msg("Incompatible options: --signal and --list");

	if (g.safe && g.unsafe)
		prog_usage_msg("Incompatible options: --safe and --unsafe");

	if (g.config && g.noconfig)
		prog_usage_msg("Incompatible options: --config and --noconfig");

	if (g.config && stat(g.config, status) == -1)
		prog_usage_msg("Invalid --config option argument %s: %s", g.config, strerror(errno));

	if (g.pidfiles && !g.running && !g.list && access(g.pidfiles, W_OK) == -1)
		prog_usage_msg("Invalid --pidfiles argument: '%s' (Directory is not writable)", g.pidfiles);

	if (g.pidfile && !g.running && !g.list)
	{
		char *buf;
		size_t size;

		if ((size = strrchr(g.pidfile, PATH_SEP) - g.pidfile + 1) == 1)
			++size;

		if (!(buf = mem_create(size, char)))
			fatalsys("out of memory");

		snprintf(buf, size, "%.*s", (int)size - 1, g.pidfile);

		if (access(buf, W_OK) == -1)
			prog_usage_msg("Invalid --pidfile argument: '%s' (Parent directory is not writable)", g.pidfile);

		mem_release(buf);
	}
}

/*

C<int strsmartcmp(const char *a, const char *b)>

Compares two strings, C<a> and C<b>, ignoring the case of the characters,
and comparing numeric substrings numerically. It returns an integer less
than, equal to, or greater than zero if C<a> is found to be less than,
equal to, or greater than C<b>, respectively.

*/

#define to_lower(c)  tolower((int)(unsigned char)(c))

static int strsmartcmp(const char **ah, const char **bh)
{
	const char *a = *ah;
	const char *b = *bh;
	size_t la, lb, ia, ib, ja, jb;
	char numbuf[BUFSIZ];

	debug((1, "strsmartcmp(%s, %s)", a, b))

	if (!a && b)
		return -1;

	if (a && !b)
		return 1;

	if (!a && !b)
		return 0;

	/* Locate the end of common prefix */

	la = strlen(a);
	lb = strlen(b);

	debug((2, "la %d, lb %d", la, lb))

	ia = ib = 0;

	while (ia < la && ib < lb)
	{
		debug((2, " 1 ia %d(%s), ib %d(%s)", ia, a + ia, ib, b + ib))

		/* Skip any identical non-numeric prefix */

		while (ia < la && ib < lb && to_lower(a[ia]) == to_lower(b[ib]) && !isdigit(a[ia]))
		{
			ia += 1;
			ib += 1;
		}

		debug((2, " 2 ia %d(%s), ib %d(%s)", ia, a + ia, ib, b + ib))

		/* Handle digits after the common prefix numerically */

		if (ia < la && isdigit(a[ia]) && ib < lb && isdigit(b[ib]))
		{
			unsigned int anum, bnum;

			ja = ia;

			while (ja < la && isdigit(a[ja]))
				ja += 1;

			snprintf(numbuf, BUFSIZ, "%.*s", (int)(ja - ia), a + ia);
			anum = (unsigned int)atoi(numbuf);

			jb = ib;

			while (jb < lb && isdigit(b[jb]))
				jb += 1;

			snprintf(numbuf, BUFSIZ, "%.*s", (int)(jb - ib), b + ib);
			bnum = (unsigned int)atoi(numbuf);

			debug((2, " X %u cmp %u = %d", anum, bnum, (int)(anum - bnum)))

			/* Return now if numerically different */

			if (anum - bnum != 0)
				return (int)(anum - bnum);

			/* Otherwise continue on past the numbers */

			ia = ja;
			ib = jb;

			continue;
		}

		/* Otherwise revert to normal */

		debug((2, " 3 ia %d(%s), ib %d(%s)", ia, a + ia, ib, b + ib))

		return strcasecmp(a + ia, b + ib);
	}

	debug((2, " 4 ia %d(%s), ib %d(%s)", ia, a + ia, ib, b + ib))

	return strcasecmp(a + ia, b + ib);
}

/*

C<int is_daemon(pid_t pid)>

Tries to check if the C</proc/pid/comm> virtual file corresponding to the
given process ID (I<pid>) contains the name C<"daemon\n">. Return C<1> if it
does. Returns C<0> if it doesn't. Returns C<-1> if unable to open the
virtual file. This only works on systems that support this C</proc>
filesystem virtual file (i.e. I<Linux>). Note: This doesn't guarantee that
the process is an instance of this daemon program. It could conceivably be
another executable named I<daemon>.

*/

static int is_daemon(pid_t pid)
{
	char fname[64], buf[BUFSIZ];
	ssize_t bytes;
	int fd;

	if (snprintf(fname, 64, "/proc/%d/comm", (int)pid) >= 64)
		return -1;

	if ((fd = open(fname, O_RDONLY)) == -1)
		return -1;

	bytes = read(fd, buf, BUFSIZ);
	close(fd);
	if (bytes == -1)
		return -1;

	if (buf[bytes - 1] == '\n')
		buf[bytes - 1] = '\0';

	return (strcmp(buf, DAEMON_NAME) == 0);
}

/*

C<int list(void)>

Prints the list of currently running daemons whose pidfiles are in the
specified or default pidfile location. On success, returns C<0>. On error,
returns C<-1> with I<errno> set appropriately.

*/

static int list(void)
{
	const char *default_pid_dir = (getuid()) ? USER_PID_DIR : ROOT_PID_DIR;
	const char *pid_dir = (g.pidfiles) ? g.pidfiles : default_pid_dir;
	int is_default_pid_dir = (strcmp(pid_dir, USER_PID_DIR) == 0 || strcmp(pid_dir, ROOT_PID_DIR) == 0);
	List *entries;
	struct dirent *entry;
	DIR *dir;

	debug((1, "list"))

	if (!(entries = list_create(free)))
		return -1;

	if (!(dir = opendir(pid_dir)))
	{
		list_release(entries);
		return -1;
	}

	while ((entry = readdir(dir)))
	{
		size_t len = strlen(entry->d_name);
		char *name = null;

		if (len <= 4 || strcmp(entry->d_name + len - 4, ".pid") != 0)
			continue;

		if (asprintf(&name, "%.*s", (int)len - 4, entry->d_name) == -1)
		{
			closedir(dir);
			list_release(entries);
			return -1;
		}

		if (!(list_append(entries, name)))
		{
			closedir(dir);
			list_release(entries);
			return -1;
		}
	}

	closedir(dir);

	if (!list_length(entries))
	{
		if (prog_verbosity_level())
			printf("No named daemons are running\n");

		list_release(entries);
		return 0;
	}

	if (!list_sort(entries, (list_cmp_t *)strsmartcmp))
	{
		list_release(entries);
		return -1;
	}

	while (list_has_next(entries) == 1)
	{
		char *name = list_next(entries);
		char *pidfile = null;

		if (asprintf(&pidfile, "%s/%s.pid", pid_dir, name) == -1)
		{
			list_release(entries);
			return -1;
		}

		switch (daemon_is_running(pidfile))
		{
			case 0:
			{
				if (prog_verbosity_level())
					printf("%s is not running%s\n", name, (is_default_pid_dir) ? " (or is independent)" : "");

				break;
			}

			case 1:
			{
				if (prog_verbosity_level())
				{
					pid_t clientpid = getclientpid(pidfile);

					if (clientpid == -1)
					{
						pid_t pid = daemon_getpid(pidfile);
						int pid_is_daemon = is_daemon(pid);

						printf("%s is running (pid %d)%s\n", name, (int)pid,
							(pid_is_daemon == 1) ? " (client is not running)" :
							(pid_is_daemon == 0) ? " (independent)" :
							(pid_is_daemon == -1) ? "" : " (client is not running or is independent)");
					}
					else
						printf("%s is running (pid %d) (client pid %d)\n", name, (int)daemon_getpid(pidfile), (int)clientpid);
				}
				else
					printf("%s\n", name);

				break;
			}

			default:
				errorsys("failed to tell if the %s daemon is running", name);
				break;
		}

		mem_release(pidfile);
	}

	list_release(entries);
	return 0;
}

/*

C<int msg_filter_pid_prefix(void **mesgp, const void *mesg, size_t mesglen)>

Message prefix filter function that inserts the current process
id into debug messages.

*/

static int msg_filter_pid_prefix(void **mesgp, const void *mesg, size_t mesglen)
{
	return asprintf((char **)mesgp, "[pid %d] %*.*s", (int)getpid(), (int)mesglen, (int)mesglen, (char *)mesg);
}

/*

C<void init(int ac, char **av)>

Initialises the program. Revokes any setuid/setgid privileges. Processes
command line options. Processes the configuration file(s). Calls
I<daemon_prevent_core()> unless the C<--core> option was supplied. Calls
I<daemon_init()> with the C<--name> option's argument, if any. Arranges to
have C<SIGTERM> signals propagated to the client process. And stores the
remaining command line arguments to be I<execvp()>d later.

*/

static void init(int ac, char **av)
{
	mode_t mode;
	int flags;
	List *cmd = null;
	int i = 0;
	int a;

	prog_dbg_stdout();
	debug((1, "init()"))

	/* Initialise locale and libslack */

	setlocale(LC_ALL, "");
	prog_init();

	/* Identify self */

	prog_set_name(DAEMON_NAME);
	prog_set_version(DAEMON_VERSION);
	prog_set_date(DAEMON_DATE);
	prog_set_syntax("[options] [--] [cmd arg...]");
	prog_set_options(options);
	prog_set_author("raf <raf@raf.org>");
	prog_set_contact(prog_author());
	prog_set_url(DAEMON_URL);

	prog_set_legal
	(
		"Copyright (C) 1999-2004, 2010, 2020-2021 raf <raf@raf.org>\n"
		"\n"
		"This is free software released under the terms of the GPLv2+:\n"
		"\n"
		"    https://www.gnu.org/licenses/\n"
		"\n"
		"There is no warranty; not even for merchantability or fitness\n"
		"for a particular purpose.\n"
#ifndef HAVE_GETOPT_LONG
		"\n"
		"Includes the GNU getopt functions:\n"
		"    Copyright (C) 1997, 1998 Free Software Foundation, Inc.\n"
#endif
	);

	prog_set_desc
	(
		"Daemon turns other processes into daemons.\n"
		"See the daemon(1) manpage for more information.\n"
	);

	/* Drop any special setuid/setgid privileges */

	debug((2, "revoking privileges"))

	if (daemon_revoke_privileges() == -1)
		fatalsys("failed to revoke uid/gid privileges: uid/gid = %d/%d euid/egid = %d/%d", getuid(), getgid(), geteuid(), getegid());

	/* Parse command line options */

	debug((2, "processing command line options"))

	g.initial_uid = getuid();
	a = prog_opt_process(g.ac = ac, g.av = av);
	g.done_name = 1;

	/* Set file system root */

	if (g.chroot)
	{
		debug((2, "chroot %s", g.chroot))

		if (chdir(g.chroot) == -1)
			fatalsys("failed to change directory to new root directory %s", g.chroot);

		if (chroot(g.chroot) == -1)
			fatalsys("failed to change root directory to %s", g.chroot);

		if (chdir("/") == -1)
			fatalsys("failed to change directory to new root directory after chroot %s", g.chroot);
	}

	g.done_chroot = 1;

	/* Set user and groups */

	if (g.uid)
	{
		debug ((2, "changing to user %s/%d", g.user, g.uid))

		if (daemon_become_user(g.uid, g.gid, (g.init_groups) ? g.user : null) == -1)
		{
			struct group *grp = getgrgid(g.gid);
			struct passwd *pwd = getpwuid(g.uid);
			fatalsys("failed to set user/group to %s/%s (%d/%d): uid/gid = %d/%d euid/egid = %d/%d", (pwd) ? pwd->pw_name : "<noname>", (grp) ? grp->gr_name : "<noname>", (int)g.uid, (int)g.pid, (int)getuid(), (int)getgid(), (int)geteuid(), (int)getegid());
		}
	}

	g.done_user = 1;

	/* Parse configuration files (reparses command line options last) */

	config();
	show();

	/* Check sanity of command line options */

	sanity_check();

	/* Prevent core file generation */

	if (!g.core)
	{
		debug((2, "preventing core files"))

		if (daemon_prevent_core() == -1)
			fatalsys("failed to prevent core file generation");
	}

	/* Build absolute pidfile path if necessary */

	debug((2, "constructing pidfile path"))

	if (g.pidfile)
	{
		if (!(g.daemon_init_name = mem_strdup(g.pidfile)))
			fatalsys("out of memory");
	}
	else if (g.pidfiles && g.name)
	{
		const char *suffix = ".pid";
		size_t size = strlen(g.pidfiles) + 1 + strlen(g.name) + strlen(suffix) + 1;

		if (!(g.daemon_init_name = mem_create(size, char)))
			fatalsys("out of memory");

		snprintf(g.daemon_init_name, size, "%s%c%s%s", g.pidfiles, PATH_SEP, g.name, suffix);
	}
	else if (g.name)
	{
		if (!(g.daemon_init_name = mem_strdup(g.name)))
			fatalsys("out of memory");
	}

	/* Stop a named daemon */

	if (g.stop)
	{
		show();

		debug((2, "stopping daemon %s", g.daemon_init_name))

		if (daemon_stop(g.daemon_init_name) == -1)
			fatalsys("failed to stop the %s daemon: pidfile %s", g.name, g.daemon_init_name);

		exit(EXIT_SUCCESS);
	}

	/* Test whether or not a named daemon is running */

	if (g.running)
	{
		show();

		debug((2, "checking if daemon %s is running: pidfile %s", g.name, g.daemon_init_name))

		switch (daemon_is_running(g.daemon_init_name))
		{
			case 0:
				verbose(1, "%s is not running", g.name);
				exit(EXIT_FAILURE);

			case 1:
			{
				pid_t clientpid = getclientpid(g.daemon_init_name);

				if (clientpid == -1)
					verbose(1, "%s is running (pid %d) (client is not running)", g.name, (int)daemon_getpid(g.daemon_init_name));
				else
					verbose(1, "%s is running (pid %d) (clientpid %d)", g.name, (int)daemon_getpid(g.daemon_init_name), (int)clientpid);

				exit(EXIT_SUCCESS);
			}

			default:
				fatalsys("failed to tell if the %s daemon is running", g.name);
		}
	}

	/* Print a list of currently running daemons */

	if (g.list)
	{
		show();

		debug((2, "printing a list of currently running daemons: pidfiles %s", g.pidfiles ? g.pidfiles : "default"))

		if (list() == -1)
			fatalsys("failed to list currently running daemons");

		exit(EXIT_SUCCESS);
	}

	/* Restart a named daemon */

	if (g.restart)
	{
		show();

		debug((2, "restarting daemon %s: pidfile %s", g.name, g.daemon_init_name))

		if ((g.pid = daemon_getpid(g.daemon_init_name)) == -1)
			fatalsys("failed to find pid for %s", g.name ? g.name : g.daemon_init_name);

		if (kill(g.pid, SIGUSR1) == -1)
			fatalsys("failed to send sigusr1 to %s daemon", g.name ? g.name : g.daemon_init_name);

		exit(EXIT_SUCCESS);
	}

	/* Send a signal to a named daemon's client process */

	if (g.signo)
	{
		show();

		debug((2, "sending signal %s=%d to daemon %s client" , g.signame, g.signo, g.name))

		if ((g.pid = getclientpid(g.daemon_init_name)) == -1)
			fatalsys("failed to find client pid for %s", g.name ? g.name : g.daemon_init_name);

		if (kill(g.pid, g.signo) == -1)
			fatalsys("failed to send %s signal to %s daemon client", g.signame, g.name ? g.name : g.daemon_init_name);

		exit(EXIT_SUCCESS);
	}

	/* Build a command line argument vector for the client */

	debug((2, "constructing command line arguments for the client"))

	if (g.command && !(cmd = split(g.command, " ")))
		fatalsys("out of memory");

	if (!(g.cmd = mem_create((cmd ? list_length(cmd) : 0) + (ac - a) + 1, char *)))
		fatalsys("out of memory");

	for (i = 0; i < list_length(cmd); ++i)
		if (!(g.cmd[i] = mem_strdup(cstr((String *)list_item(cmd, i)))))
			fatalsys("out of memory");

	list_release(cmd);

	if (a != ac)
		memmove(g.cmd + i, av + a, (ac - a) * sizeof(char *));

	g.cmd[i + ac - a] = null;

	/* Check that we have a command to run */

	debug((2, "checking the client command"))

	if (g.cmd[0] == null)
		prog_usage_msg("Invalid arguments: no command supplied");

	/* Prepare coproc_open() cmd argument */

	if (g.name)
	{
		g.cmdpath = g.cmd[0];
		g.cmd[0] = null;

		if (asprintf(&g.cmd[0], "%s: %s", g.name, g.cmdpath) == -1)
			fatalsys("out of memory");
	}
	else
	{
		if (!(g.cmdpath = mem_strdup(g.cmd[0])))
			fatalsys("out of memory");
	}

	/* Check that the client executable is safe */

	if (g.safe || (getuid() == 0 && !g.unsafe))
	{
		char explanation[256];

		switch (safety_check(g.cmdpath, explanation, 256))
		{
			case 1: break;
			case 0: fatal("refusing to execute unsafe program: %s (%s)", g.cmdpath, explanation);
			default: fatalsys("failed to tell if %s is safe", g.cmdpath);
		}
	}

	/* Set message prefix to the --name argument, if any */

	if (g.name)
		prog_set_name(g.name);

	/* Enter daemon space, or just name the client, or neither */

	if (g.foreground)
	{
		debug((2, "locking pidfile only (foreground)"))

		if (g.daemon_init_name && daemon_pidfile(g.daemon_init_name) == -1)
			fatalsys("failed to create pidfile for %s", g.name ? g.name : g.daemon_init_name);
	}
	else
	{
		int rc;

		debug((2, "becoming a daemon and locking pidfile"))

		rc = daemon_init(g.daemon_init_name);
		prog_err_syslog(prog_name(), 0, LOG_DAEMON, LOG_ERR);
		prog_dbg_syslog(prog_name(), 0, LOG_DAEMON, LOG_DEBUG);

		if (rc == -1)
			fatalsys("failed to become a daemon");
	}

	if (g.daemon_init_name)
	{
		debug((2, "atexit(daemon_close)"))

		if (atexit((void (*)(void))daemon_close) == -1)
		{
			daemon_close();
			fatalsys("%s: failed to atexit(daemon_close)", g.daemon_init_name);
		}

		debug((2, "atexit(unlink_clientpidfile)"))

		if (atexit((void (*)(void))unlink_clientpidfile) == -1)
		{
			unlink_clientpidfile();
			fatalsys("%s: failed to atexit(unlink_clientpidfile)", g.daemon_init_name);
		}
	}

	/* Set umask */

	debug((2, "setting umask to %03o", g.umask))

	umask(g.umask);

	/* Set directory */

	if (g.chdir)
	{
		debug((2, "chdir %s", g.chdir))

		if (chdir(g.chdir) == -1)
			fatalsys("failed to change directory to %s", g.chdir);
	}

	/* Set daemon's error message destination (syslog or file) */

	if (g.daemon_errlog)
	{
		debug((2, "starting error delivery to syslog %s.%s", syslog_facility_str(g.daemon_errlog), syslog_priority_str(g.daemon_errlog)))

		if (prog_err_syslog(prog_name(), 0, g.daemon_errlog & LOG_FACMASK, g.daemon_errlog & LOG_PRIMASK) == -1)
			fatalsys("failed to start error delivery to %s.%s", syslog_facility_str(g.daemon_errlog), syslog_priority_str(g.daemon_errlog));
	}
	else if (g.daemon_err)
	{
		debug((2, "starting error delivery to file %s", g.daemon_err))

		if (prog_err_file(g.daemon_err) == -1)
			fatalsys("failed to start error delivery to %s", g.daemon_err);
	}

	/* Set daemon's debug message destination (syslog or file) */

	if (g.daemon_dbglog)
	{
		debug((2, "starting debug delivery to syslog %s.%s", syslog_facility_str(g.daemon_dbglog), syslog_priority_str(g.daemon_dbglog)))

		if (prog_dbg_syslog(prog_name(), 0, g.daemon_dbglog & LOG_FACMASK, g.daemon_dbglog & LOG_PRIMASK) == -1)
			fatalsys("failed to start debug delivery to %s.%s", syslog_facility_str(g.daemon_dbglog), syslog_priority_str(g.daemon_dbglog));

		if (prog_dbg_push_filter(msg_filter_pid_prefix) == -1)
			errorsys("failed to push pid-prefixing message filter to the debug message destination");
	}
	else if (g.daemon_dbg)
	{
		debug((2, "starting debug delivery to file %s", g.daemon_dbg))

		if (prog_dbg_file(g.daemon_dbg) == -1)
			fatalsys("failed to start debug delivery to %s", g.daemon_dbg);

		if (prog_dbg_push_filter(msg_filter_pid_prefix) == -1)
			errorsys("failed to push pid-prefixing message filter to the debug message destination");
	}

	/* Set client's stdout and stderr destinations (syslog or file) */

	flags = O_CREAT | O_WRONLY | O_APPEND;
	mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;

	if (g.client_out && !g.client_outlog)
	{
		debug((2, "opening client output file %s", g.client_out))

		if ((g.client_outfd = open(g.client_out, flags, mode)) == -1)
			errorsys("failed to open %s to log client stdout", g.client_out);
	}

	if (g.client_err && !g.client_errlog)
	{
		debug((2, "opening client error file %s", g.client_err))

		if ((g.client_errfd = open(g.client_err, flags, mode)) == -1)
			errorsys("failed to open %s to log client stderr", g.client_err);
	}

	/* Build an environment variable vector for the client */

	prepare_environment();

	/* Show configuration when debugging */

	show();
}

/*

C<int main(int ac, char **av)>

Initialise the program with the command line arguments specified in
C<ac> and C<av> then run it.

*/

int main(int ac, char **av)
{
	init(ac, av);
	run();

	return EXIT_SUCCESS; /* unreached */
}

/* vi:set ts=4 sw=4: */
