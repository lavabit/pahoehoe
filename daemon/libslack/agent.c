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

I<libslack(agent)> - agent module

=head1 SYNOPSIS

    #include <slack/std.h>
    #include <slack/agent.h>

    typedef struct Agent Agent;
    typedef int agent_action_t(Agent *agent, void *arg);
    typedef int agent_reaction_t(Agent *agent, int fd, int revents, void *arg);

    Agent *agent_create(void);
    Agent *agent_create_with_locker(Locker *locker);
    Agent *agent_create_measured(void);
    Agent *agent_create_measured_with_locker(Locker *locker);
    Agent *agent_create_using_select(void);
    Agent *agent_create_using_select_with_locker(Locker *locker);
    void agent_release(Agent *agent);
    void *agent_destroy(Agent **agent);
    int agent_rdlock(const Agent *agent);
    int agent_wrlock(const Agent *agent);
    int agent_unlock(const Agent *agent);
    int agent_connect(Agent *agent, int fd, int events, agent_reaction_t *reaction, void *arg);
    int agent_connect_unlocked(Agent *agent, int fd, int events, agent_reaction_t *reaction, void *arg);
    int agent_disconnect(Agent *agent, int fd);
    int agent_disconnect_unlocked(Agent *agent, int fd);
    int agent_transfer(Agent *agent, int fd, Agent *dst);
    int agent_transfer_unlocked(Agent *agent, int fd, Agent *dst);
    int agent_send(Agent *agent, int fd, int sockfd);
    int agent_send_unlocked(Agent *agent, int fd, int sockfd);
    int agent_recv(Agent *agent, int sockfd, agent_reaction_t *reaction, void *arg);
    int agent_recv_unlocked(Agent *agent, int sockfd, agent_reaction_t *reaction, void *arg);
    int agent_detail(Agent *agent, int fd);
    int agent_detail_unlocked(Agent *agent, int fd);
    const struct timeval * const agent_last(Agent *agent, int fd);
    const struct timeval * const agent_last_unlocked(Agent *agent, int fd);
    int agent_velocity(Agent *agent, int fd);
    int agent_velocity_unlocked(Agent *agent, int fd);
    int agent_acceleration(Agent *agent, int fd);
    int agent_acceleration_unlocked(Agent *agent, int fd);
    int agent_dadt(Agent *agent, int fd);
    int agent_dadt_unlocked(Agent *agent, int fd);
    void *agent_schedule(Agent *agent, long sec, long usec, agent_action_t *action, void *arg);
    void *agent_schedule_unlocked(Agent *agent, long sec, long usec, agent_action_t *action, void *arg);
    int agent_cancel(Agent *agent, void *action_id);
    int agent_cancel_unlocked(Agent *agent, void *action_id);
    int agent_start(Agent *agent);
    int agent_stop(Agent *agent);

=head1 DESCRIPTION

This module provides support for a generic agent programming model. Agents
are like event loops, except that while event loops only react to input
events, agents can also take independent actions at specific times.

Unlike event loops, which are typically GUI-specific and receive input
events by calling some concrete event retrieval function, input events for
agents take the form of data transfers across file descriptors. This means
that input events can come from any source, and have any semantics. For
example, to implement an event loop for a specific GUI using an agent, you'd
have a separate thread or process that calls the GUI's event retrieval
function, and then sends each event to the agent across a pipe or socket.

Agents multiplex input sources using I<poll(2)> (or I<select(2)> if
unavoidable) and multiplex timers for scheduled actions over I<poll(2)>'s
timeout facility using hierarchical timing wheels. If timers are not used,
agents are just an alternate interface to I<poll(2)>. If input sources are
not used, agents are just a multi-purpose timer that doesn't use any
signals.

Multiple agents can be connected to each other via pipes and sockets in
arbitrary networks (in multiple threads or multiple processes on the same
host or multiple hosts) and these connections may change over time.

It is expected that agents will generally be used to build highly scalable
internet servers because connecting and disconnecting file descriptors and
scheduling and cancelling timed actions are all O(1) operations and managing
timers has constant average time. If two or more agents cooperate (on a
system that has I<poll(2)>), responding to input events can also be highly
scalable (See the SCALABILITY section for details).

=over 4

=cut

*/

#ifndef _BSD_SOURCE
#define _BSD_SOURCE /* For timercmp() in glibc and snprintf() on OpenBSD-4.7 */
#endif

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE /* New name for _BSD_SOURCE */
#endif

#ifndef _NETBSD_SOURCE
#define _NETBSD_SOURCE /* For timercmp() on NetBSD-5.0.2 */
#endif

#include "config.h"
#include "std.h"
#include "mem.h"
#include "err.h"
#include "link.h"
#include "net.h"
#include "agent.h"

#include <sys/time.h>
#include <sys/types.h>

#ifndef TEST

#if HAVE_POLL_H
#include <poll.h>
#elif HAVE_SYS_POLL_H
#include <sys/poll.h>
#elif HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

typedef struct timewheel_t timewheel_t;
typedef struct action_t action_t;
typedef struct reaction_t reaction_t;
typedef struct activity_t activity_t;
typedef struct timeval timeval;

#define POLL_SIZE 16

enum { IDLE = 0, START = 1, STOP = 2 }; /* Agent states */
enum { POLL = 0, SELECT = 1 };          /* Agent implementations */

struct Agent
{
	int state;              /* idle, start, stop */
	ssize_t *ids;           /* map fd to array indexes */
	size_t ids_size;        /* size of ids */
	int method;             /* implementation method: poll() or select() */
	union
	{
#ifdef HAVE_POLL
		struct pollfd *pfds;                      /* for poll() */
#endif
		struct { fd_set *rfds, *xfds, *wfds; } s; /* for select() */
	} u;
	reaction_t *reactions;  /* reactions to input events */
	activity_t *tempo;      /* activity of the agent itself */
	activity_t *activity;   /* activity of each connection */
	size_t size;            /* number of elements allocated in pollfd */
	size_t length;          /* number of elements used in pollfd */
	timewheel_t *timewheel; /* hierarchical timing wheel for scheduling */
	size_t timers;          /* number of timers in the timewheel */
	Locker *locker;         /* locking strategy for this agent */
};

#ifdef HAVE_POLL
#define pollfds u.pfds
#endif
#define readfds u.s.rfds
#define writefds u.s.wfds
#define exceptfds u.s.xfds

struct action_t
{
	action_t *next;         /* link to next action */
	action_t *prev;         /* link to previous action */
	action_t **parent;      /* the list in which this action is stored */
	timeval when;           /* absolute time to act */
	agent_action_t *action; /* function to call */
	void *arg;              /* argument to pass to function */
	size_t day;             /* index into days */
	size_t hour;            /* index into hours */
	size_t minute;          /* index into minutes */
	size_t second;          /* index into seconds */
	size_t jiffy;           /* index into jiffies */
};

struct reaction_t
{
	int fd;                     /* file descriptor */
	int events;                 /* read/write/exception */
	agent_reaction_t *reaction; /* function to call */
	void *arg;                  /* argument to pass to function */
};

struct activity_t
{
	timeval since;   /* when the last event occurred */
	size_t detail;   /* how much detail do we have? */
	int dt;          /* the event rate */
	int ddt;         /* the rate of change of the event rate */
	int dddt;        /* the rate of change of the rate of change of the event rate */
};

enum
{
	DAYS = 10,
	HOURS = 24,
	MINUTES = 60,
	SECONDS = 60,
	JIFFIES = 100
};

struct timewheel_t
{
	timeval now[1];             /* current time */
	size_t day;                 /* current index into days */
	size_t hour;                /* current index into hours */
	size_t minute;              /* current index into minutes */
	size_t second;              /* current index into seconds */
	size_t jiffy;               /* current index into jiffies */
	action_t *days[DAYS];       /* timers for subsequent days */
	action_t *hours[HOURS];     /* timers for subsequent hours */
	action_t *minutes[MINUTES]; /* timers for subsequent minutes */
	action_t *seconds[SECONDS]; /* timers for subsequent seconds */
	action_t *jiffies[JIFFIES]; /* timers for this second */
};

/*

C<static timewheel_t *timewheel_create()>

Creates a I<timewheel_t> object. It is the caller's responsibility to
deallocate the timewheel object with I<timewheel_release(3)>. On error,
returns C<null> with C<errno> set appropriately.

*/

static timewheel_t *timewheel_create()
{
	timewheel_t *timewheel = mem_new(timewheel_t); /* XXX decouple */

	if (!timewheel)
		return NULL;

	memset(timewheel, 0, sizeof(timewheel_t));

	if (gettimeofday(timewheel->now, NULL) == -1)
		return NULL;

	return timewheel;
}

/*

C<static void timewheel_release(timewheel_t *timewheel)>

Releases (deallocates) C<timewheel>.

*/

static action_t *release_action(action_t *action)
{
	action_t *next = dlink_next(action);

	mem_release(action);

	return next;
}

static void release_actions(action_t *action)
{
	while (action)
		action = release_action(action);
}

static void timewheel_release(timewheel_t *timewheel)
{
	size_t i;

	if (!timewheel)
		return;

	for (i = 0; i < DAYS; ++i)
		release_actions(timewheel->days[i]);

	for (i = 0; i < HOURS; ++i)
		release_actions(timewheel->hours[i]);

	for (i = 0; i < MINUTES; ++i)
		release_actions(timewheel->minutes[i]);

	for (i = 0; i < SECONDS; ++i)
		release_actions(timewheel->seconds[i]);

	for (i = 0; i < JIFFIES; ++i)
		release_actions(timewheel->jiffies[i]);

	mem_release(timewheel);
}

/*

=item C<Agent *agent_create(void)>

Creates an I<Agent> object. On error, returns C<null> with C<errno> set
appropriately. It is the caller's responsibility to deallocate the new agent
with I<agent_release(3)> or I<agent_destroy(3)>. It is strongly recommended
to use I<agent_destroy(3)>, because it also sets the pointer variable to
C<null>.

=cut

*/

Agent *agent_create(void)
{
	return agent_create_with_locker(NULL);
}

/*

=item C<Agent *agent_create_with_locker(Locker *locker)>

Equivalent to I<agent_create(3)> except that multiple threads accessing the
new agent will be synchronised by C<locker>.

=cut

*/

Agent *agent_create_with_locker(Locker *locker)
{
	Agent *agent = mem_new(Agent); /* XXX decouple */

	if (!agent)
		return NULL;

	memset(agent, 0, sizeof(Agent));

#ifdef HAVE_POLL
	agent->method = POLL;
#else
	agent->method = SELECT;
#endif

	agent->locker = locker;

	return agent;
}

/*

=item C<Agent *agent_create_measured(void)>

Creates an I<Agent> object that measures I/O activity. Such agents can be
passed to the following functions to determine the level of I/O activity
handled by the agent: I<agent_detail(3)> which returns the level of detail
available (this determines which of the subsequent functions may be called);
I<agent_last(3)> which returns the time that the most recent I/O event
occurred; I<agent_velocity(3)> which returns the rate of I/O events;
I<agent_acceleration(3)> which returns the rate of change of the I/O event
rate; and I<agent_dadt(3)> which returns the rate of change of the rate of
change of the I/O event rate. These functions can be applied to individual
file descriptors, or to the agent as a whole. These agents can be combined
to produce a fast/slow lane structure that improves scalability of I/O with
respect to the number of connected file descriptors. See the SCALABILITY
section below for more details. On error, returns C<null> with C<errno> set
appropriately. It is the caller's responsibility to deallocate the new agent
with I<agent_release(3)> or I<agent_destroy(3)>. It is strongly recommended
to use I<agent_destroy(3)>, because it also sets the pointer variable to
C<null>. Note, if this system does not have I<poll(2)>, this function is not
very useful.

=cut

*/

Agent *agent_create_measured(void)
{
	return agent_create_measured_with_locker(NULL);
}

/*

=item C<Agent *agent_create_measured_with_locker(Locker *locker)>

Equivalent to I<agent_create_measured(3)> except that multiple threads
accessing the new agent will be synchronised by C<locker>.

=cut

*/

Agent *agent_create_measured_with_locker(Locker *locker)
{
	Agent *agent = mem_new(Agent); /* XXX decouple */

	if (!agent)
		return NULL;

	memset(agent, 0, sizeof(Agent));

#ifdef HAVE_POLL
	agent->method = POLL;
#else
	agent->method = SELECT;
#endif

	if (!(agent->tempo = mem_new(activity_t))) /* XXX decouple */
	{
		mem_release(agent);
		return NULL;
	}

	memset(agent->tempo, 0, sizeof(activity_t));
	agent->locker = locker;

	return agent;
}

/*

=item C<Agent *agent_create_using_select(void)>

Equivalent to I<agent_create(3)> except that the agent created will use
I<select(2)> instead of I<poll(2)>. This should only be used under I<Linux>
when accurate 10ms timers are required (see the BUGS section for details).
It should not be used for I/O (see the SCALABILITY section for details).

=cut

*/

Agent *agent_create_using_select(void)
{
	return agent_create_using_select_with_locker(NULL);
}

/*

=item C<Agent *agent_create_using_select_with_locker(Locker *locker)>

Equivalent to I<agent_create_using_select(3)> except that multiple threads
accessing the new agent will be synchronised by C<locker>.

=cut

*/

Agent *agent_create_using_select_with_locker(Locker *locker)
{
	Agent *agent = mem_new(Agent); /* XXX decouple */

	if (!agent)
		return NULL;

	memset(agent, 0, sizeof(Agent));
	agent->method = SELECT;
	agent->locker = locker;

	return agent;
}

/*

=item C<void agent_release(Agent *agent)>

Releases (deallocates) C<agent>.

=cut

*/

void agent_release(Agent *agent)
{
	Locker *locker;

	if (!agent)
		return;

	if (agent_wrlock(agent))
		return;

	locker = agent->locker;
	mem_release(agent->ids);

#ifdef HAVE_POLL
	if (agent->method == POLL)
		mem_release(agent->pollfds);
	else
#endif
	{
		mem_release(agent->readfds);
		mem_release(agent->writefds);
		mem_release(agent->exceptfds);
	}

	mem_release(agent->reactions);
	mem_release(agent->tempo);
	mem_release(agent->activity);
	timewheel_release(agent->timewheel);
	mem_release(agent);
	locker_unlock(locker);
}

/*

=item C<void *agent_destroy(Agent **agent)>

Destroys (deallocates and sets to C<null>) C<*agent>. Returns C<null>.
B<Note:> agents that are shared by multiple threads must not be destroyed
until after all threads have finished with it.

=cut

*/

void *agent_destroy(Agent **agent)
{
	if (agent && *agent)
	{
		agent_release(*agent);
		*agent = NULL;
	}

	return NULL;
}

/*

=item C<int agent_rdlock(const Agent *agent)>

Claims a read lock on C<agent> (if C<agent> was created with a I<Locker>).
This is needed when multiple read-only I<agent(3)> module functions need to
be called atomically. It is the caller's responsibility to call
I<agent_unlock(3)> after the atomic operation. The only functions that may
be called on C<agent> between calls to I<agent_rdlock(3)> and
I<agent_unlock(3)> are any read-only I<agent(3)> module functions whose name
ends with C<_unlocked>. On success, returns C<0>. On error, returns an error
code.

=cut

*/

#define agent_rdlock(agent) locker_rdlock((agent)->locker)
#define agent_wrlock(agent) locker_wrlock((agent)->locker)
#define agent_unlock(agent) locker_unlock((agent)->locker)

int (agent_rdlock)(const Agent *agent)
{
	return agent_rdlock(agent);
}

/*

=item C<int agent_wrlock(const Agent *agent)>

Claims a write lock on C<agent> (if C<agent> was created with a I<Locker>).
This is needed when multiple read/write I<agent(3)> module functions need to
be called atomically. It is the caller's responsibility to call
I<agent_unlock(3)> after the atomic operation. The only functions that may
be called on C<agent> between calls to I<agent_wrlock(3)> and
I<agent_unlock(3)> are any I<agent(3)> module functions whose name ends with
C<_unlocked>. On success, returns C<0>. On error, returns an error code.

=cut

*/

int (agent_wrlock)(const Agent *agent)
{
	return agent_wrlock(agent);
}

/*

=item C<int agent_unlock(const Agent *agent)>

Unlocks a read or write lock on C<agent> obtained with I<agent_rdlock(3)> or
I<agent_wrlock(3)> (if C<agent> was created with a C<locker>). On success,
returns C<0>. On error, returns an error code.

=cut

*/

int (agent_unlock)(const Agent *agent)
{
	return agent_unlock(agent);
}

/*

=item C<int agent_connect(Agent *agent, int fd, int events, agent_reaction_t *reaction, void *arg)>

Connect the file descriptor, C<fd>, to C<agent>. C<events> specifies the
input/output events of interest. It is a bitmask of the following values:
C<R_OK>, C<W_OK> and C<X_OK> indicating, respectively, readability,
writability and exceptional condition (i.e. arrival of out of band data).
When any of the specified events occur, the function, C<reaction>, will be
called with four arguments: C<agent>, C<fd>, C<revents> (the bitmask of the
events that occurred), and C<arg>. If C<fd> is already connected, the
existing C<events>, C<reaction> and C<arg> are replaced with the new values.
On success, returns C<0>. On error, returns C<-1> with C<errno> set
appropriately.

=cut

*/

int agent_connect(Agent *agent, int fd, int events, agent_reaction_t *reaction, void *arg)
{
	int ret, err;

	if (!agent)
		return set_errno(EINVAL);

	if ((err = agent_wrlock(agent)))
		return set_errno(err);

	ret = agent_connect_unlocked(agent, fd, events, reaction, arg);

	if ((err = agent_unlock(agent)))
		return set_errno(err);

	return ret;
}

/*

=item C<int agent_connect_unlocked(Agent *agent, int fd, int events, agent_reaction_t *reaction, void *arg)>

Equivalent to I<agent_connect(3)> except that C<agent> is not write-locked.

=cut

*/

int agent_connect_unlocked(Agent *agent, int fd, int events, agent_reaction_t *reaction, void *arg)
{
	/* Check the arguments */

	if (!agent || fd < 0 || !reaction || !(events & (R_OK | W_OK | X_OK)) || (events & ~(R_OK | W_OK | X_OK)))
		return set_errno(EINVAL);

	/* Allocate or extend ids if necessary */

	if (!agent->ids)
	{
		if (!(agent->ids = mem_create(fd + 1, ssize_t)))
			return -1;

		memset(agent->ids, 0xff, (fd + 1) * sizeof(ssize_t));
		agent->ids_size = fd + 1;
	}
	else if (agent->ids_size <= fd)
	{
		if (!(agent->ids = mem_resize(&agent->ids, agent->ids_size << 1)))
			return -1;

		memset(agent->ids + agent->ids_size, 0xff, agent->ids_size * sizeof(ssize_t));
		agent->ids_size <<= 1;
	}

	/* Allocate or extend pollfds, reactions and activity, if necessary */

	if (!agent->reactions)
	{
#if HAVE_POLL
		if (agent->method == POLL)
		{
			if (!(agent->pollfds = mem_create(POLL_SIZE, struct pollfd)))
				return -1;

			memset(agent->pollfds, 0, POLL_SIZE * sizeof(struct pollfd));
		}
#endif

		if (!(agent->reactions = mem_create(POLL_SIZE, reaction_t)))
			return -1;

		memset(agent->reactions, 0, POLL_SIZE * sizeof(reaction_t));

		if (agent->tempo)
		{
			if (!(agent->activity = mem_create(POLL_SIZE, activity_t)))
				return -1;

			memset(agent->activity, 0, POLL_SIZE * sizeof(activity_t));
		}

		agent->size = POLL_SIZE;
	}
	else if (agent->length == agent->size)
	{
#if HAVE_POLL
		if (agent->method == POLL)
		{
			if (!mem_resize(&agent->pollfds, agent->size << 1))
				return -1;

			memset(agent->pollfds + agent->size, 0, agent->size * sizeof(struct pollfd));
		}
#endif

		if (!mem_resize(&agent->reactions, agent->size << 1))
			return -1;

		memset(agent->reactions + agent->size, 0, agent->size * sizeof(reaction_t));

		if (agent->tempo)
		{
			if (!mem_resize(&agent->activity, agent->size << 1))
				return -1;

			memset(agent->activity + agent->size, 0, agent->size * sizeof(activity_t));
		}

		agent->size <<= 1;
	}

	/* Claim a new pollfd structure if not already connected */

	if (agent->ids[fd] == -1)
		agent->ids[fd] = agent->length++;

	/* Fill in the pollfd structure and its reaction */

#if HAVE_POLL
	if (agent->method == POLL)
	{
		agent->pollfds[agent->ids[fd]].fd = fd;
		agent->pollfds[agent->ids[fd]].events = 0;
		agent->pollfds[agent->ids[fd]].revents = 0;

		if (events & R_OK)
			agent->pollfds[agent->ids[fd]].events |= POLLIN;

		if (events & X_OK)
			agent->pollfds[agent->ids[fd]].events |= POLLPRI;

		if (events & W_OK)
			agent->pollfds[agent->ids[fd]].events |= POLLOUT;
	}
	else
#endif
	{
		if (events & R_OK && !agent->readfds)
		{
			if (!(agent->readfds = mem_new(fd_set)))
				return -1;

			FD_ZERO(agent->readfds);
		}

		if (events & X_OK && !agent->exceptfds)
		{
			if (!(agent->exceptfds = mem_new(fd_set)))
				return -1;

			FD_ZERO(agent->exceptfds);
		}

		if (events & W_OK && !agent->writefds)
		{
			if (!(agent->writefds = mem_new(fd_set)))
				return -1;

			FD_ZERO(agent->writefds);
		}

		if (events & R_OK)
			FD_SET(fd, agent->readfds);

		if (events & X_OK)
			FD_SET(fd, agent->exceptfds);

		if (events & W_OK)
			FD_SET(fd, agent->writefds);
	}

	agent->reactions[agent->ids[fd]].fd = fd;
	agent->reactions[agent->ids[fd]].events = events;
	agent->reactions[agent->ids[fd]].reaction = reaction;
	agent->reactions[agent->ids[fd]].arg = arg;

	return 0;
}

/*

=item C<int agent_disconnect(Agent *agent, int fd)>

Disconnect the file descriptor, C<fd>, from C<agent>. C<agent> will no
longer respond to input/output events that occur on C<fd>. On success,
returns C<0>. On error, returns C<-1> with C<errno> set appropriately.

=cut

*/

int agent_disconnect(Agent *agent, int fd)
{
	int ret, err;

	if (!agent)
		return set_errno(EINVAL);

	if ((err = agent_wrlock(agent)))
		return set_errno(err);

	ret = agent_disconnect_unlocked(agent, fd);

	if ((err = agent_unlock(agent)))
		return set_errno(err);

	return ret;
}

/*

=item C<int agent_disconnect_unlocked(Agent *agent, int fd)>

Equivalent to I<agent_disconnect(3)> except that C<agent> is not
write-locked.

=cut

*/

int agent_disconnect_unlocked(Agent *agent, int fd)
{
	ssize_t id, last_id;
	int last_fd;

	/* Check the arguments */

	if (!agent || fd < 0)
		return set_errno(EINVAL);

	/* Check the agent */

#if HAVE_POLL
	if (agent->method == POLL)
	{
		if (!agent->pollfds)
			return set_errno(EINVAL);
	}
	else
#endif
		if (!agent->readfds && !agent->writefds && !agent->exceptfds)
			return set_errno(EINVAL);

	if (!agent->ids || !agent->reactions || (agent->tempo && !agent->activity) || agent->ids_size <= fd)
		return set_errno(EINVAL);

	/* Remove the agent from pollfds, reactions, activity and ids */

	if ((id = agent->ids[fd]) == -1)
		return set_errno(EINVAL);

	last_fd = agent->reactions[agent->length - 1].fd;
	last_id = agent->ids[last_fd];

#if HAVE_POLL
	if (agent->method == POLL)
	{
		agent->pollfds[id] = agent->pollfds[last_id];
		memset(&agent->pollfds[last_id], 0, sizeof(struct pollfd));
	}
	else
#endif
	{
		if (agent->readfds)
			FD_CLR(fd, agent->readfds);

		if (agent->exceptfds)
			FD_CLR(fd, agent->exceptfds);

		if (agent->writefds)
			FD_CLR(fd, agent->writefds);
	}

	agent->reactions[id] = agent->reactions[last_id];

	if (agent->tempo)
		agent->activity[id] = agent->activity[last_id];

	agent->ids[last_fd] = id;
	--agent->length;

	memset(&agent->reactions[last_id], 0, sizeof(reaction_t));

	if (agent->tempo)
		memset(&agent->activity[last_id], 0, sizeof(activity_t));

	agent->ids[fd] = -1;

	return 0;
}

/*

=item C<int agent_transfer(Agent *agent, int fd, Agent *dst)>

Transfers the connected file descriptor, C<fd>, from C<agent> to C<dst>. The
activity data for C<fd> (i.e. time of last event, velocity, acceleration and
dadt) are transferred as well. Both C<agent> and C<dst> must be agents
created using I<agent_create_measured(3)>. On success, returns C<0>. On
error, returns C<-1> with C<errno> set appropriately. Note this only works
for agents in separate threads. To transfer a file descriptor to another
agent in another process on the same host, use I<agent_send(3)> and
I<agent_recv(3)>. It is not possible to transfer a file descriptor to
another agent on another host.

=cut

*/

int agent_transfer(Agent *agent, int fd, Agent *dst)
{
	int ret, err;

	if (!agent)
		return set_errno(EINVAL);

	if ((err = agent_wrlock(agent)))
		return set_errno(err);

	ret = agent_transfer_unlocked(agent, fd, dst);

	if ((err = agent_unlock(agent)))
		return set_errno(err);

	return ret;
}

/*

=item C<int agent_transfer_unlocked(Agent *agent, int fd, Agent *dst)>

Equivalent to I<agent_transfer(3)> except that C<agent> is not write-locked.
Note that C<dst> is still write-locked.

=cut

*/

int agent_transfer_unlocked(Agent *agent, int fd, Agent *dst)
{
	reaction_t reaction;
	activity_t activity;
	ssize_t id;

	/* Check the arguments */

	if (!agent || fd < 0 || !dst)
		return set_errno(EINVAL);

	/* Check agent */

	if (!agent->ids || !agent->reactions || !agent->tempo || !agent->activity || agent->ids_size <= fd)
		return set_errno(EINVAL);

	/* Check dst */

	if (!dst->tempo)
		return set_errno(EINVAL);

	/* Transfer the connection and its activity data */

	if ((id = agent->ids[fd]) == -1)
		return set_errno(EINVAL);

	reaction = agent->reactions[id];
	activity = agent->activity[id];

	if (agent_connect(dst, reaction.fd, reaction.events, reaction.reaction, reaction.arg) == -1)
		return -1;

	dst->activity[dst->ids[fd]] = activity;

	if (agent_disconnect_unlocked(agent, fd) == -1)
	{
		agent_disconnect(dst, fd); /* can't happen */
		return -1;
	}

	return 0;
}

/*

=item C<int agent_send(Agent *agent, int fd, int sockfd)>

Transfers the connected file descriptor, C<fd>, from C<agent> to a receiving
agent on the other end of the UNIX domain stream socket, C<sockfd>. Both the
sending and the receiving agent must have been created using
I<agent_create_measured(3)>. The receiving agent must call I<agent_recv(3)>
to receive the file descriptor. The activity data for C<fd> (i.e. time of
last event, velocity, acceleration and dadt) are transferred as well. The
events to react to and the reaction function and its argument are also sent
to the receiving agent but note that the reaction function and its argument
will be meaningless if the receiving agent exists in an unrelated process.
They are passed just in case the processes are related and the reaction
function's argument points to shared memory. If not, the receiving agent
must specify a new reaction function and argument in the call to
I<agent_recv(3)>. If the receiving agent exists in a separate thread,
I<agent_transfer(3)> should be used instead. It is much faster. On success,
returns C<0>. On error, returns C<-1> with C<errno> set appropriately. Note
that this function does not close C<fd>. The caller must do this. Note that
there is no provision for returning errors encountered by the receiving
process to the sending process. If this is a problem, use threads instead
and call I<agent_transfer(3)>.

=cut

*/

int agent_send(Agent *agent, int fd, int sockfd)
{
	int ret, err;

	if (!agent)
		return set_errno(EINVAL);

	if ((err = agent_wrlock(agent)))
		return set_errno(err);

	ret = agent_send_unlocked(agent, fd, sockfd);

	if ((err = agent_unlock(agent)))
		return set_errno(err);

	return ret;
}

/*

=item C<int agent_send_unlocked(Agent *agent, int fd, int sockfd)>

Equivalent to I<agent_send(3)> except that C<agent> is not write-locked.

=cut

*/

int agent_send_unlocked(Agent *agent, int fd, int sockfd)
{
	struct { reaction_t reaction; activity_t activity; } buf;
	ssize_t id;

	/* Check the arguments */

	if (!agent || fd < 0 || sockfd < 0)
		return set_errno(EINVAL);

	/* Check agent */

	if (!agent->ids || !agent->reactions || !agent->tempo || !agent->activity || agent->ids_size <= fd)
		return set_errno(EINVAL);

	/* Transfer the connection and its activity data */

	if ((id = agent->ids[fd]) == -1)
		return set_errno(EINVAL);

	buf.reaction = agent->reactions[id];
	buf.activity = agent->activity[id];

	if (sendfd(sockfd, &buf, sizeof buf, 0, fd) == -1)
		return -1;

	if (agent_disconnect_unlocked(agent, fd) == -1)
		return -1; /* can't happen */

	return 0;
}

/*

=item C<int agent_recv(Agent *agent, int sockfd, agent_reaction_t *reaction, void *arg)>

Receives a file descriptor from the UNIX domain stream socket, C<sockfd>,
and connects it to C<agent>. If C<reaction> and C<arg> are null, then the
I<reaction> function and its I<arg> argument are obtained from the sending
process along with the file descriptor. This is only possible when the
sending and receiving process share the same address space (i.e. after
I<fork(2)>). When the sending and receiving process don't share the same
address space (i.e. after I<exec(2)> or similar), then the I<reaction> and
I<arg> parameters must be supplied by the receiving process. In either case,
if the I<reaction> and I<arg> parameters are not null, they override the
reaction function and argument that are received from the sending process.
On success, returns the file descriptor received. On error, returns C<-1>
with C<errno> set appropriately.

=cut

*/

int agent_recv(Agent *agent, int sockfd, agent_reaction_t *reaction, void *arg)
{
	int ret, err;

	if (!agent || sockfd < 0 || !reaction)
		return set_errno(EINVAL);

	if ((err = agent_wrlock(agent)))
		return set_errno(err);

	ret = agent_recv_unlocked(agent, sockfd, reaction, arg);

	if ((err = agent_unlock(agent)))
		return set_errno(err);

	return ret;
}

/*

=item C<int agent_recv_unlocked(Agent *agent, int sockfd, agent_reaction_t *reaction, void *arg)>

Equivalent to I<agent_recv(3)> except that C<agent> is not write-locked.

=cut

*/

int agent_recv_unlocked(Agent *agent, int sockfd, agent_reaction_t *reaction, void *arg)
{
	struct { reaction_t reaction; activity_t activity; } buf;
	ssize_t id;
	int fd;

	/* Check the arguments */

	if (!agent || sockfd < 0 || !reaction)
		return set_errno(EINVAL);

	/* Receive the connection and its reaction data and activity data */

	if (recvfd(sockfd, &buf, sizeof buf, 0, &fd) != sizeof buf)
		return -1;

	if (!reaction)
		reaction = buf.reaction.reaction;

	if (!arg)
		arg = buf.reaction.arg;

	if (agent_connect_unlocked(agent, fd, buf.reaction.events, reaction, arg) == -1)
		return -1;

	if ((id = agent->ids[fd]) == -1)
		return set_errno(EINVAL);

	agent->activity[id] = buf.activity;

	return 0;
}

/*

=item C<int agent_detail(Agent *agent, int fd)>

Returns the level of detail in the activity data that is available for the
file descriptor C<fd> handled by C<agent>. If C<fd> is C<-1>, returns the
level of detail that is available for C<agent> itself. On error, returns
C<-1> with C<errno> set appropriately.

If C<0> is returned, there have been no I/O events for C<fd> (or agent if
C<fd> is C<-1>), so no activity data is available. If C<1> is returned,
there has been one I/O event so only I<agent_last(3)> may be called with the
same C<fd> argument. If C<2> is returned, there have been two I/O events, so
I<agent_last(3)> and I<agent_velocity(3)> may be called with the same C<fd>
argument. If C<3> is returned, there have been 3 I/O events so
I<agent_last(3)>, I<agent_velocity(3)> and I<agent_acceleration(3)> may be
called with the same C<fd> argument. If C<4> is returned, there have been at
least 4 I/O events so I<agent_last(3)>, I<agent_velocity(3)>,
I<agent_acceleration(3)> and I<agent_dadt(3)> may be called with the same
C<fd> argument.

These functions may be used to implement algorithms that determine whether
or not a given file descriptor should remain with a given agent, or be
transferred to another agent using I<agent_transfer(3)> or I<agent_send(3)>
and I<agent_recv(3)>. See the SCALABILITY section.

=cut

*/

int agent_detail(Agent *agent, int fd)
{
	int ret, err;

	if (!agent)
		return set_errno(EINVAL);

	if ((err = agent_rdlock(agent)))
		return set_errno(err);

	ret = agent_detail_unlocked(agent, fd);

	if ((err = agent_unlock(agent)))
		return set_errno(err);

	return ret;
}

/*

=item C<int agent_detail_unlocked(Agent *agent, int fd)>

Equivalent to I<agent_detail(3)> except that C<agent> is not read-locked.

=cut

*/

int agent_detail_unlocked(Agent *agent, int fd)
{
	ssize_t id;

	/* Check the arguments */

	if (!agent || fd < -1)
		return set_errno(EINVAL);

	/* Check the agent */

	if (!agent->ids || !agent->reactions || !agent->tempo || !agent->activity || agent->ids_size <= fd)
		return set_errno(EINVAL);

	/* Return the detail available for agent if fd is -1 */

	if (fd == -1)
		return agent->tempo->detail;

	/* Get the file descriptor's id and return its detail */

	if ((id = agent->ids[fd]) == -1)
		return set_errno(EINVAL);

	return agent->activity[id].detail;
}

/*

=item C<const struct timeval * const agent_last(Agent *agent, int fd)>

Returns the time of the most recent I/O event for the file descriptor C<fd>
handled by C<agent>. If C<fd> is C<-1>, returns the time of the most recent
event handled by C<agent> for any file descriptor. On error, returns C<-1>
with C<errno> set appropriately. Note: This function may only be called
after I<agent_detail(3)> has returned a value greater than C<0> for the same
C<fd> argument.

=cut

*/

const struct timeval * const agent_last(Agent *agent, int fd)
{
	const struct timeval *ret;
	int err;

	if (!agent)
		return set_errnull(EINVAL);

	if ((err = agent_rdlock(agent)))
		return set_errnull(err);

	ret = agent_last_unlocked(agent, fd);

	if ((err = agent_unlock(agent)))
		return set_errnull(err);

	return ret;
}

/*

=item C<const struct timeval * const agent_last_unlocked(Agent *agent, int fd)>

Equivalent to I<agent_last(3)> except that C<agent> is not read-locked.

=cut

*/

const struct timeval * const agent_last_unlocked(Agent *agent, int fd)
{
	ssize_t id;

	/* Check the arguments */

	if (!agent || fd < -1)
		return set_errnull(EINVAL);

	/* Check the agent */

	if (!agent->ids || !agent->reactions || !agent->tempo || !agent->activity || agent->ids_size <= fd)
		return set_errnull(EINVAL);

	/* Return the time of the last event handled by agent if fd is -1 */

	if (fd == -1)
	{
		if (agent->tempo->detail == 0)
			return set_errnull(EINVAL);

		return &agent->tempo->since;
	}

	/* Get the file descriptor's id and return the time of the last I/O event */

	if ((id = agent->ids[fd]) == -1)
		return set_errnull(EINVAL);

	if (agent->activity[id].detail == 0)
		return set_errnull(EINVAL);

	return &agent->activity[id].since;
}

/*

=item C<int agent_velocity(Agent *agent, int fd)>

Returns the number of milliseconds that elapsed between the last two I/O
events for the file descriptor C<fd> handled by C<agent>. If C<fd> is C<-1>,
returns the number of milliseconds that elapsed between the last two events
handled by C<agent> for any file descriptor. Large return values indicate
less I/O activity. On error, returns C<-1> with C<errno> set appropriately.
Note: This function may only be called after I<agent_detail(3)> has returned
a value greater than C<1> for the same C<fd> argument.

=cut

*/

int agent_velocity(Agent *agent, int fd)
{
	int ret, err;

	if (!agent)
		return set_errno(EINVAL);

	if ((err = agent_rdlock(agent)))
		return set_errno(err);

	ret = agent_velocity_unlocked(agent, fd);

	if ((err = agent_unlock(agent)))
		return set_errno(err);

	return ret;
}

/*

=item C<int agent_velocity_unlocked(Agent *agent, int fd)>

Equivalent to I<agent_velocity(3)> except that C<agent> is not read-locked.

=cut

*/

int agent_velocity_unlocked(Agent *agent, int fd)
{
	ssize_t id;

	/* Check the arguments */

	if (!agent || fd < -1)
		return set_errno(EINVAL);

	/* Check the agent */

	if (!agent->ids || !agent->reactions || !agent->tempo || !agent->activity || agent->ids_size <= fd)
		return set_errno(EINVAL);

	/* Return agent's velocity if fd is -1 */

	if (fd == -1)
	{
		if (agent->tempo->detail < 2)
			return set_errno(EINVAL);

		return agent->tempo->dt;
	}

	/* Get the file descriptor's id and return its velocity */

	if ((id = agent->ids[fd]) == -1)
		return set_errno(EINVAL);

	if (agent->activity[id].detail < 2)
		return set_errno(EINVAL);

	return agent->activity[id].dt;
}

/*

=item C<int agent_acceleration(Agent *agent, int fd)>

Returns the rate of change of the velocity of I/O events for the file
descriptor C<fd> handled by C<agent>. If C<fd> is C<-1>, returns the rate of
change of I/O events for C<agent> for any file descriptor. Negative return
values indicate acceleration. Positive return values indicate deceleration.
A zero return value indicates no acceleration. The larger the magnitude of
the return value, the greater the acceleration or deceleration. On error,
returns C<-1> with C<errno> set appropriately. Note: This function may only
be called after I<agent_detail(3)> has returned a value greater than C<2>
for the same C<fd> argument.

=cut

*/

int agent_acceleration(Agent *agent, int fd)
{
	int ret, err;

	if (!agent)
		return set_errno(EINVAL);

	if ((err = agent_rdlock(agent)))
		return set_errno(err);

	ret = agent_acceleration_unlocked(agent, fd);

	if ((err = agent_unlock(agent)))
		return set_errno(err);

	return ret;
}

/*

=item C<int agent_acceleration_unlocked(Agent *agent, int fd)>

Equivalent to I<agent_acceleration(3)> except that C<agent> is not
read-locked.

=cut

*/

int agent_acceleration_unlocked(Agent *agent, int fd)
{
	ssize_t id;

	/* Check the arguments */

	if (!agent || fd < -1)
		return set_errno(EINVAL);

	/* Check the agent */

	if (!agent->ids || !agent->reactions || !agent->tempo || !agent->activity || agent->ids_size <= fd)
		return set_errno(EINVAL);

	/* Return agent's acceleration if fd is -1 */

	if (fd == -1)
	{
		if (agent->tempo->detail < 3)
			return set_errno(EINVAL);

		return agent->tempo->ddt;
	}

	/* Get the file descriptor's id and return its acceleration */

	if ((id = agent->ids[fd]) == -1)
		return set_errno(EINVAL);

	if (agent->activity[id].detail < 3)
		return set_errno(EINVAL);

	return agent->activity[id].ddt;
}

/*

=item C<int agent_dadt(Agent *agent, int fd)>

Returns the rate of change of the rate of change of I/O events for the file
descriptor C<fd> handled by C<agent>. If C<fd> is C<-1>, returns the rate of
change of the rate of change of I/O events for C<agent> for any file
descriptor. Negative return values indicate that acceleration or
deceleration is increasing. Positive return values indicate that
acceleration or deceleration is decreasing. A zero return value indicates
that acceleration or deceleration is constant. The larger the magnitude of
the return value, the greater the increase or decrease in acceleration or
deceleration. On error, returns C<-1> with C<errno> set appropriately. Note:
This function may only be called after I<agent_detail(3)> has returned a
value greater than C<3> for the same C<fd> argument.

=cut

*/

int agent_dadt(Agent *agent, int fd)
{
	int ret, err;

	if (!agent)
		return set_errno(EINVAL);

	if ((err = agent_rdlock(agent)))
		return set_errno(err);

	ret = agent_dadt_unlocked(agent, fd);

	if ((err = agent_unlock(agent)))
		return set_errno(err);

	return ret;
}

/*

=item C<int agent_dadt_unlocked(Agent *agent, int fd)>

Equivalent to I<agent_dadt(3)> except that C<agent> is not read-locked.

=cut

*/

int agent_dadt_unlocked(Agent *agent, int fd)
{
	ssize_t id;

	/* Check the arguments */

	if (!agent || fd < -1)
		return set_errno(EINVAL);

	/* Check the agent */

	if (!agent->ids || !agent->reactions || !agent->tempo || !agent->activity || agent->ids_size <= fd)
		return set_errno(EINVAL);

	/* Return agent's dadt if fd is -1 */

	if (fd == -1)
	{
		if (agent->tempo->detail < 4)
			return set_errno(EINVAL);

		return agent->tempo->dddt;
	}

	/* Get the file descriptor's id and return its dadt */

	if ((id = agent->ids[fd]) == -1)
		return set_errno(EINVAL);

	if (agent->activity[id].detail < 4)
		return set_errno(EINVAL);

	return agent->activity[id].dddt;
}

/*

=item C<void *agent_schedule(Agent *agent, long sec, long usec, agent_action_t *action, void *arg)>

Schedule C<agent> to invoke C<action> in C<sec> seconds and C<usec>
microseconds. Note, however, that timer precision is in 10ms units. When the
timer expires, C<action> is invoked. It is passed two arguments: C<agent>
and C<arg>. On success, returns an action identifier that may be used to
cancel the action with I<agent_cancel(3)>. On error, returns <-1> with
C<errno> set appropriately.

=cut

*/

void *agent_schedule(Agent *agent, long sec, long usec, agent_action_t *action, void *arg)
{
	void *ret;
	int err;

	if (!agent)
		return set_errnull(EINVAL);

	if ((err = agent_wrlock(agent)))
		return set_errnull(err);

	ret = agent_schedule_unlocked(agent, sec, usec, action, arg);

	if ((err = agent_unlock(agent)))
		return set_errnull(err);

	return ret;
}

/*

=item C<void *agent_schedule_unlocked(Agent *agent, long sec, long usec, agent_action_t *action, void *arg)>

Equivalent to I<agent_schedule(3)> except that C<agent> is not write-locked.

=cut

*/

static void timeval_diff(timeval *start, timeval *end, timeval *diff)
{
	diff->tv_sec = end->tv_sec - start->tv_sec;

	if (end->tv_usec < start->tv_usec)
		diff->tv_usec = 1000000 + end->tv_usec - start->tv_usec, --diff->tv_sec;
	else
		diff->tv_usec = end->tv_usec - start->tv_usec;
}

static void timeval_add(timeval *absolute, timeval *relative, timeval *result)
{
	result->tv_sec = absolute->tv_sec + relative->tv_sec;
	result->tv_usec = absolute->tv_usec + relative->tv_usec;

	if (result->tv_usec >= 1000000)
		++result->tv_sec, result->tv_usec -= 1000000;
}

static void timeval_set(timeval *tv, long tv_sec, long tv_usec)
{
	tv->tv_sec = tv_sec;
	tv->tv_usec = tv_usec;
}

static void install(action_t **parent, action_t *action)
{
	*parent = dlink_insert(*parent, action);
	action->parent = parent;
}

void *agent_schedule_unlocked(Agent *agent, long sec, long usec, agent_action_t *action, void *arg)
{
	action_t *event;
	timeval now[1], delta[1], when[1];

	if (!agent || sec < 0 || usec < 0 || !action)
		return set_errnull(EINVAL);

	/* Create the timewheel if necessary */

	if (!agent->timewheel && !(agent->timewheel = timewheel_create()))
		return NULL;

	/* Get the current time and adjust if it's gone backwards */

	if (gettimeofday(now, NULL) == -1)
		return NULL;

	if (timercmp(now, agent->timewheel->now, < ))
		*agent->timewheel->now = *now;

	/* Create the action */

	if (!(event = mem_new(action_t))) /* XXX decouple */
		return NULL;

	timeval_set(delta, sec, usec);
	timeval_add(now, delta, when);

	event->when = *when;
	event->action = action;
	event->arg = arg;

	/* Schedule the action */

	timeval_diff(agent->timewheel->now, when, delta);

	event->day = delta->tv_sec / (HOURS * MINUTES * SECONDS);
	delta->tv_sec -= event->day * HOURS * MINUTES * SECONDS;

	event->hour = delta->tv_sec / (MINUTES * SECONDS);
	delta->tv_sec -= event->hour * MINUTES * SECONDS;

	event->minute = delta->tv_sec / SECONDS;
	delta->tv_sec -= event->minute * SECONDS;

	event->second = delta->tv_sec;

	event->jiffy = delta->tv_usec / 10000;

	if ((event->jiffy += agent->timewheel->jiffy) >= JIFFIES)
		event->jiffy -= JIFFIES, ++event->second;

	if ((event->second += agent->timewheel->second) >= SECONDS)
		event->second -= SECONDS, ++event->minute;

	if ((event->minute += agent->timewheel->minute) >= MINUTES)
		event->minute -= MINUTES, ++event->hour;

	if ((event->hour += agent->timewheel->hour) >= HOURS)
		event->hour -= HOURS, ++event->day;

	event->day += agent->timewheel->day;

	if (event->day != agent->timewheel->day)
		install(&agent->timewheel->days[event->day % DAYS], event);
	else if (event->hour != agent->timewheel->hour)
		install(&agent->timewheel->hours[event->hour], event);
	else if (event->minute != agent->timewheel->minute)
		install(&agent->timewheel->minutes[event->minute], event);
	else if (event->second != agent->timewheel->second)
		install(&agent->timewheel->seconds[event->second], event);
	else
		install(&agent->timewheel->jiffies[event->jiffy], event);

	++agent->timers;

	return event;
}

/*

=item C<int agent_cancel(Agent *agent, void *action_id)>

Cancel an action that was scheduled with I<agent_schedule(3)>. C<action_id>
is the value returned by I<agent_schedule(3)>. It is the caller's
responsibility to ensure that this function is not passed an C<action_id>
that corresponds to an action that has already executed (since the action
will have been deallocated). On success, returns C<0>. On error, returns
C<-1> with C<errno> set appropriately.

=cut

*/

int agent_cancel(Agent *agent, void *action_id)
{
	int ret, err;

	if (!agent)
		return set_errno(EINVAL);

	if ((err = agent_wrlock(agent)))
		return set_errno(err);

	ret = agent_cancel_unlocked(agent, action_id);

	if ((err = agent_unlock(agent)))
		return set_errno(err);

	return ret;
}

/*

=item C<int agent_cancel_unlocked(Agent *agent, void *action_id)>

Equivalent to I<agent_cancel(3)> except that C<agent> is not write-locked.

=cut

*/

int agent_cancel_unlocked(Agent *agent, void *action_id)
{
	action_t *event = action_id;
	action_t *next;

	if (!agent || !event || !agent->timewheel || !agent->timers)
		return set_errno(EINVAL);

	next = dlink_remove(event);

	if (*event->parent == event)
		*event->parent = next;

	mem_release(event);

	--agent->timers;

	return 0;
}

/*

=item C<int agent_start(Agent *agent)>

Starts C<agent>. The agent will react to events on connected file
descriptors and execute scheduled actions until there are no connected file
descriptors and no scheduled actions or until I<agent_stop(3)> is called. It
is the caller's responsibility to ensure that action and reaction functions
will not take too long to execute. If they are going to take more than a few
milliseconds, consider having them execute in their own detached thread.
Otherwise, actions scheduled for the near future (e.g. 10ms) will not
execute until they have finished. Of course, when there are no scheduled
actions, this doesn't matter. On success, returns C<0>. On error, returns
C<-1> with C<errno> set appropriately. If any action or reaction function
returns C<-1>, returns C<-1>. Note that you cannot call I<agent_start(3)> on
C<agent> inside one of its action or reaction functions.

=cut

*/

static void next_day(Agent *agent)
{
	action_t *next, *action;

	/* Install timers for the next day into hours array */

	agent->timewheel->hour = 0;

	if (++agent->timewheel->day == DAYS)
	{
		size_t i;

		agent->timewheel->day = 0;

		for (i = 0; i < DAYS; ++i)
			for (action = agent->timewheel->days[i]; action; action = dlink_next(action))
				action->day -= DAYS;
	}

	next = agent->timewheel->days[agent->timewheel->day];

	for (action = next; action; action = next)
	{
		if (action->day == agent->timewheel->day)
		{
			next = dlink_remove(action);

			if (*action->parent == action)
				*action->parent = next;

			install(&agent->timewheel->hours[action->hour], action);
		}
		else
			next = dlink_next(action);
	}
}

static void next_hour(Agent *agent)
{
	action_t *next, *action;

	/* Install timers for the next hour into minutes array */

	agent->timewheel->minute = 0;

	if (++agent->timewheel->hour == HOURS)
		next_day(agent);

	next = agent->timewheel->hours[agent->timewheel->hour];
	agent->timewheel->hours[agent->timewheel->hour] = NULL;

	for (action = next; action; action = next)
	{
		next = dlink_remove(action);
		install(&agent->timewheel->minutes[action->minute], action);
	}
}

static void next_minute(Agent *agent)
{
	action_t *next, *action;

	/* Install timers for the next minute into seconds array */

	agent->timewheel->second = 0;

	if (++agent->timewheel->minute == MINUTES)
		next_hour(agent);

	next = agent->timewheel->minutes[agent->timewheel->minute];
	agent->timewheel->minutes[agent->timewheel->minute] = NULL;

	for (action = next; action; action = next)
	{
		next = dlink_remove(action);
		install(&agent->timewheel->seconds[action->second], action);
	}
}

static void next_second(Agent *agent)
{
	action_t *next, *action;

	/* Install timers for the next second into jiffies array */

	agent->timewheel->jiffy = 0;

	if (++agent->timewheel->second == SECONDS)
		next_minute(agent);

	next = agent->timewheel->seconds[agent->timewheel->second];
	agent->timewheel->seconds[agent->timewheel->second] = NULL;

	for (action = next; action; action = next)
	{
		next = dlink_remove(action);
		install(&agent->timewheel->jiffies[action->jiffy], action);
	}
}

static int timeout(Agent *agent)
{
	int i;

	if (agent->timers == 0)
		return -1;

	/* Find the first jiffy (if any) with scheduled actions */

	for (i = agent->timewheel->jiffy; i < JIFFIES; ++i)
		if (agent->timewheel->jiffies[i])
			break;

	/* Return milliseconds until next scheduled action or second timer */

	return (i - agent->timewheel->jiffy) * 10;
}

static int act(agent_action_t *action, Agent *agent, void *arg)
{
	int err, ret;

	if ((err = agent_unlock(agent)))
		return set_errno(err);

	ret = action(agent, arg);

	if ((err = agent_wrlock(agent)))
		return set_errno(err);

	return ret;
}

static int react(agent_reaction_t *reaction, Agent *agent, int fd, int revents, void *arg)
{
	int err, ret;

	if ((err = agent_unlock(agent)))
		return set_errno(err);

	ret = reaction(agent, fd, revents, arg);

	if ((err = agent_wrlock(agent)))
		return set_errno(err);

	return ret;
}

static int expire(Agent *agent)
{
	action_t *event;

	while ((event = agent->timewheel->jiffies[agent->timewheel->jiffy]))
	{
		agent_action_t *action = event->action;
		void *arg = event->arg;

		if (agent_cancel_unlocked(agent, event) == -1)
			return -1;

		if (act(action, agent, arg) == -1)
			return -1;
	}

	return 0;
}

static int update(Agent *agent)
{
	timeval now[1], delta[1];
	int check = 1;

	while (check)
	{
		check = 0;

		if (gettimeofday(now, NULL) == -1)
			return -1;

		if (timercmp(now, agent->timewheel->now, < ))
			*agent->timewheel->now = *now;

		timeval_diff(agent->timewheel->now, now, delta);
		delta->tv_usec /= 10000; /* usec -> jiffy */
		*agent->timewheel->now = *now;

		while (delta->tv_sec || delta->tv_usec)
		{
			if (delta->tv_usec == 0)
				--delta->tv_sec, delta->tv_usec = 100;
			--delta->tv_usec;

			if (++agent->timewheel->jiffy == JIFFIES)
				next_second(agent);

			if (agent->timewheel->jiffies[agent->timewheel->jiffy])
			{
				++check;

				if (expire(agent) == -1)
					return -1;
			}
		}
	}

	return 0;
}

static void measure(Agent *agent, int fd, timeval *now)
{
	activity_t *activity;
	timeval delta[1];
	int msec, prev_dt, prev_ddt;

	activity = (fd == -1) ? agent->tempo : &agent->activity[agent->ids[fd]];

	switch (activity->detail)
	{
		case 0:
			activity->since = *now;
			activity->detail = 1;
			break;

		case 1:
			timeval_diff(&activity->since, now, delta);
			msec = delta->tv_sec * 1000 + delta->tv_usec / 1000;
			activity->dt = msec;
			activity->since = *now;
			activity->detail = 2;
			break;

		case 2:
			timeval_diff(&activity->since, now, delta);
			msec = delta->tv_sec * 1000 + delta->tv_usec / 1000;
			prev_dt = activity->dt;
			activity->dt = msec;
			activity->ddt = activity->dt - prev_dt;
			activity->since = *now;
			activity->detail = 3;
			break;

		case 3:
		case 4:
			timeval_diff(&activity->since, now, delta);
			msec = delta->tv_sec * 1000 + delta->tv_usec / 1000;
			prev_dt = activity->dt;
			activity->dt = msec;
			prev_ddt = activity->ddt;
			activity->ddt = activity->dt - prev_dt;
			activity->dddt = activity->ddt - prev_ddt;
			activity->since = *now;
			activity->detail = 4;
			break;
	}
}

static int translate(int revents)
{
	int ret = 0;

	if (revents & POLLIN)
		ret |= R_OK;

	if (revents & POLLPRI)
		ret |= X_OK;

	if (revents & POLLOUT)
		ret |= W_OK;

	return ret;
}

#ifdef HAVE_LINUX_POLL_BUG
#define tune(timo) (((timo) > 10) ? (timo) - 10 : (timo))
#else
#define tune(timo) (timo)
#endif

static int agent_start_unlocked(Agent *agent)
{
	if (!agent || agent->state != IDLE)
		return set_errno(EINVAL);

	/* Run until no connections and no timers or until stopped */

	agent->state = START;

	/* Update agent's notion of the time, executing any missed actions */

	if (agent->timers && update(agent) == -1)
		return -1;

	while ((agent->length || agent->timers) && agent->state != STOP)
	{
		int nfds, timo;
		size_t i;

		/* Sleep until there's something to do or until this second is up */

		timo = timeout(agent);

#if HAVE_POLL
		if (agent->method == POLL)
		{
#ifdef HAVE_POLL_THAT_ABORTS_WHEN_POLLFDS_IS_NULL
			struct pollfd dummy;

			if ((nfds = poll(agent->pollfds ? agent->pollfds : &dummy, agent->length, tune(timo))) == -1)
#else
			if ((nfds = poll(agent->pollfds, agent->length, tune(timo))) == -1)
#endif
			{
				if (errno == EINTR)
					agent->state = IDLE;

				return -1;
			}

			if (nfds) /* React to I/O events */
			{
				timeval now[1];

				if (gettimeofday(now, NULL) == -1)
					return -1;

				if (agent->tempo)
					measure(agent, -1, now);

				for (i = 0; nfds && i < agent->length; ++i)
				{
					if (agent->pollfds[i].revents)
					{
						agent_reaction_t *reaction = agent->reactions[i].reaction;
						int fd = agent->reactions[i].fd;
						int revents = translate(agent->pollfds[i].revents);
						void *arg = agent->reactions[i].arg;

						agent->pollfds[i].revents = 0;

						if (agent->tempo)
							measure(agent, fd, now);

						if (react(reaction, agent, fd, revents, arg) == -1)
							return -1;

						--nfds;
					}
				}
			}
			else /* Perform scheduled actions */
			{
				timeval delta[1], result[1];

				timeval_set(delta, 0, timo * 1000);
				timeval_add(agent->timewheel->now, delta, result);
				*agent->timewheel->now = *result;

				if ((agent->timewheel->jiffy += timo / 10) == JIFFIES)
					next_second(agent);

				if (expire(agent) == -1)
					return -1;
			}
		}
		else
#endif
		{
			timeval tv[1], *to;
			fd_set readset[1], *rfds = NULL;
			fd_set writeset[1], *wfds = NULL;
			fd_set exceptset[1], *xfds = NULL;

			if (agent->readfds)
				rfds = readset, *rfds = *agent->readfds;

			if (agent->writefds)
				wfds = writeset, *wfds = *agent->writefds;

			if (agent->exceptfds)
				xfds = exceptset, *xfds = *agent->exceptfds;

			if (timo == -1)
				to = NULL;
			else
			{
				tv->tv_sec = timo / 1000;
				tv->tv_usec = (timo % 1000) * 1000;
				to = tv;
			}

			if ((nfds = select(agent->ids_size, rfds, wfds, xfds, to)) == -1)
				return -1;

			if (nfds) /* React to I/O events */
			{
				timeval now[1];

				if (gettimeofday(now, NULL) == -1)
					return -1;

				if (agent->tempo)
					measure(agent, -1, now);

				for (i = 0; nfds && i < agent->length; ++i)
				{
					int fd = agent->reactions[i].fd;
					int revents = 0;

					if (rfds && FD_ISSET(fd, rfds))
						revents |= R_OK;

					if (wfds && FD_ISSET(fd, wfds))
						revents |= W_OK;

					if (xfds && FD_ISSET(fd, xfds))
						revents |= X_OK;

					if (revents)
					{
						agent_reaction_t *reaction = agent->reactions[i].reaction;
						void *arg = agent->reactions[i].arg;

						if (agent->tempo)
							measure(agent, fd, now);

						if (react(reaction, agent, fd, revents, arg) == -1)
							return -1;

						--nfds;
					}
				}
			}
			else /* Perform scheduled actions */
			{
				timeval delta[1], result[1];

				timeval_set(delta, 0, timo * 1000);
				timeval_add(agent->timewheel->now, delta, result);
				*agent->timewheel->now = *result;

				if ((agent->timewheel->jiffy += timo / 10) == JIFFIES)
					next_second(agent);

				if (expire(agent) == -1)
					return -1;
			}
		}

		/* Update agent's notion of the time, executing any missed actions */

		if (agent->timers && update(agent) == -1)
			return -1;
	}

	agent->state = IDLE;

	return 0;
}

#undef tune

int agent_start(Agent *agent)
{
	int ret, err;

	if (!agent)
		return set_errno(EINVAL);

	if ((err = agent_wrlock(agent)))
		return set_errno(err);

	ret = agent_start_unlocked(agent);

	if ((err = agent_unlock(agent)))
		return set_errno(err);

	return ret;
}

/*

=item C<int agent_stop(Agent *agent)>

Stops C<agent>. All connected file descriptors and scheduled actions remain
intact and C<agent> can be started again with I<agent_start(3)>. Note that
any actions scheduled to occur while C<agent> is stopped will be executed
when I<agent_start(3)> is next called. On success, returns C<0>. On error,
returns C<-1> with C<errno> set appropriately.

=cut

*/

static int agent_stop_unlocked(Agent *agent)
{
	if (!agent || agent->state != START)
		return set_errno(EINVAL);

	agent->state = STOP;

	return 0;
}

int agent_stop(Agent *agent)
{
	int ret, err;

	if (!agent)
		return set_errno(EINVAL);

	if ((err = agent_wrlock(agent)))
		return set_errno(err);

	ret = agent_stop_unlocked(agent);

	if ((err = agent_unlock(agent)))
		return set_errno(err);

	return ret;
}

/*

=back

=head1 ERRORS

On error, C<errno> is set either by an underlying function, or as follows:

=over 4

=item C<EINVAL>

When arguments to any of the functions is invalid.

When I<agent_start(3)> is called on an agent that isn't idle.

When I<agent_stop(3)> is called on an agent that isn't started.

=back

=head1 MT-Level

I<MT-Disciplined>

=head1 SCALABILITY

There are two aspects to the scalability of agents: scalability with respect
to the number of scheduled actions, and scalability with respect to the
number of connected file descriptors.

The timers for scheduled actions are multiplexed over the timeout facility
provided by I<poll(2)> using a state of the art data structure for timing
facilities (hierarchical timing wheels) which guarantees constant time to
start and stop timers and constant average time to maintain timers so that
thousands of timers may be outstanding without performance penalty.

Adding and removing connected file descriptors take constant time but
maintaining them is I<O(n)> where I<n> is the number of connected file
descriptors. That wouldn't be a problem if all of the file descriptors were
active since work would have to be done reacting to all the events anyway,
but if only a few file descriptors are active, both the kernel and the
application waste significant effort examining the elements of the C<pollfd>
array that correspond to the inactive file descriptors. Over a WAN such as
the internet, inactive file descriptors typically far outnumber active file
descriptors since many connections can be waiting for lost packets to be
retransmitted.

To implement a portable internet service that scales well with respect to
the number of inactive file descriptors, use two agents, each running in its
own thread. The first only deals with active file descriptors. The second
only deals with inactive file descriptors. These agents swap file
descriptors between themselves as their activity changes. Agents can measure
the activity of each file descriptor to facilitate this. The result is one
thread being woken up frequently but only dealing with a small number of
active file descriptors each time, and another thread being woken up
infrequently and dealing with a large number of file descriptors each time.
The second thread still wastes effort but it does so less often. Credit goes
to Richard Gooch for this I<"fast/slow lane"> approach. To reduce overhead
further, more agents could be created to deal with the inactive file
descriptors (multiple slow lanes) but it's unlikely to be worthwhile on
hosts with a single processor. Note that one process can pass an open file
descriptor to another process, so these agents could exist in separate
processes but it's not as fast.

The simpler, traditional approach is to just have multiple pre-forked
servers, each I<accept(2)>ing connections. The set of connections will then
be split between the servers. Experiments indicate that the connections are
split evenly between the servers, but if the active connections are split
between multiple servers, then the context switching overhead of multiple
threads waking up could outweigh the savings gained by splitting up the
connections into smaller sets. In the worst case, all of the threads might
be woken up at the same time, resulting in the entire set of connections
being processed. This is precisely the problem we are trying to avoid, but
we've added context switching overhead as well. Another thing to note is
that since this method is usually implemented with I<select(3)>, rather than
I<poll(2)>, the effort wasted is far greater. Consider 1000 connections
split between 10 pre-forked servers using I<select(3)>. Assume for
simplicity that the first 100 connections are handled by the first server,
the next 100 connections are handled by the second server, and so on. Due to
the fact that I<select(2)> uses bitsets to record the file descriptors of
interest, and has to check every bit up to the one corresponding to the
highest numbered file descriptor, the total number of bits checked would be
1000 + 900 + 800 + 700 + 600 + 500 + 400 + 300 + 200 + 100 = 5500. In the
worst case it would be 1000 + 999 + 998 + 997 + 996 + 995 + 994 + 993 + 992
+ 991 = 9955. That's an order of magnitude more work than the obvious
single-threaded approach.

=cut

XXX Add throughput/timing results for: single thread select/poll, multiple
thread select/poll, fast/slow lane

XXX The EXAMPLES section below contains a skeleton implementation of an
internet service that is scalable with respect to the number of inactive
connections.

=head1 EXAMPLES

Trivial example: Read from stdin and timeout after 5 seconds with no input

    #include <slack/std.h>
    #include <slack/agent.h>

    void *timeout;

    int action(Agent *agent, void *arg)
    {
        return agent_stop(agent);
    }

    int reaction(Agent *agent, int fd, int revents, void *arg)
    {
        char buf[BUFSIZ];
        ssize_t bytes;

        // Reschedule timeout for 5 seconds into the future
        // Note: action hasn't executed or we wouldn't be here

        if (agent_cancel(agent, timeout) == -1)
            return -1;

        if (!(timeout = agent_schedule(agent, 5, 0, action, NULL)))
            return -1;

        // Read from fd and write to stdout

        if ((bytes = read(fd, buf, BUFSIZ)) == -1)
            return -1;

        if (bytes && write(STDOUT_FILENO, buf, bytes) == -1)
            return -1;

        // Disconnect fd upon EOF

        if (bytes == 0 && agent_disconnect(agent, fd) == -1)
            return -1;

        return 0;
    }

    int main(int ac, char **av)
    {
        Agent *agent;
        int rc;

        // Create an agent

        if (!(agent = agent_create()))
            return EXIT_FAILURE;

        // Schedule an action

        if (!(timeout = agent_schedule(agent, 5, 0, action, NULL)))
            return EXIT_FAILURE;

        // Connect standard input

        if (agent_connect(agent, STDIN_FILENO, R_OK, reaction, NULL) == -1)
            return EXIT_FAILURE;

        // Start the agent

        while ((rc = agent_start(agent)) == -1 && errno == EINTR)
        {}

        return (rc == -1) ? EXIT_FAILURE : EXIT_SUCCESS;
    }

=cut

XXX Show example of twin fast/slow lane agents swapping fds for scalability

=head1 BUGS

I<Linux> (at least 2.2.x and 2.4.x) has a "bug" in I<poll(2)> that can wreak
havoc with timers. If you specify a timeout of between 10n-9 and 10n ms
(where n >= 1) under I<Linux>, I<poll(2)> will timeout after 10(n+1) ms
instead of 10n ms like I<select(2)>. This means that if you ask I<poll(2)>
for a 10ms timeout, you get a 20ms timeout. If you ask for 20ms, you get
30ms and so on. As a workaround, the agent module subtracts 10ms from
timeouts greater than 10ms under I<Linux>. This means that (under I<Linux>)
you can't have a 10ms timer but you can have 20ms, 30ms and so on. It also
means that if two actions are scheduled to occur 10ms apart, the second
action will execute 20ms after the first. Note that this isn't really a bug
in I<poll(2)> which is allowed to behave this way according to I<POSIX>.
It's just really unfortunate. If you need accurate 10ms timers under
I<Linux>, use I<agent_create_using_select(3)> instead of I<agent_create(3)>.
This will create an agent that uses I<select(2)> instead of I<poll(2)>.
Note, however, that I<select(2)> is unscalable with respect to the number of
connections and hence can't be used in a fast/slow lane server (See the
SCALABILITY section for details). If accurate 10ms timers and scalable I/O
are both required under I<Linux>, use I<agent_create(3)> for all agents that
will handle I/O and use I<agent_create_using_select(3)> for a separate agent
that will handle actions. Note that on systems whose I<poll(2)> does not
have this bug (e.g. I<Solaris>), this isn't necessary. Also note that on
systems that don't have I<poll(2)> (e.g. I<Mac OS X>), agents will always
use I<select(2)> and hence can't be used in a fast/slow lane server.

It is an error to call I<agent_cancel(3)> for an action that has already
happened (because the memory associated with the action is deallocated when
it is executed). Unfortunately, there is no guaranteed atomic way to tell if
an action has already occurred. If it is necessary to be able to safely
cancel scheduled actions, the client must provide the necessary safeguards
itself. This could prove difficult. The simplest safe way to cancel is to do
so from another action that was scheduled at least 10ms before the action
being cancelled. Alternatively, you could disable, rather than cancel, an
action by modifying a global variable that it checks before doing anything.

If an action or reaction take a long time to run, and an action scheduled
for the near future misses its schedule, the agent will catch up, executing
any missed actions (better late than never). Unfortunately, there is no way
to distinguish between an action or reaction taking a long time to run and
the system's clock being set forward. So, if the system's clock is set
forward, the agent will execute all actions scheduled for the missing time.
The solution is to run an NTP daemon on the system to maintain accurate
system time. Then, there would never be a large enough change to the system
time to cause problems.

=head1 SEE ALSO

I<libslack(3)>,
I<poll(2)>,
I<select(2)>

=head1 AUTHOR

20210220 raf <raf@raf.org>

=cut

*/

#endif

#ifdef TEST

#include <slack/net.h>
#include <slack/fio.h>

#include <sys/socket.h>
#include <sys/wait.h>

typedef struct timeval timeval;

int errors = 0;

static int reader(Agent *agent, int fd, int revents, void *arg)
{
	int *count = arg;
	char buf[2];
	int rc;

	if ((rc = read(fd, buf, 1)) == -1)
	{
		++errors, printf("Test1: read() failed (%s)\n", strerror(errno));
		return -1;
	}

	if (rc)
	{
		++*count;
	}
	else /* eof */
	{
		if (agent_disconnect(agent, fd) == -1)
			++errors, printf("Test1: agent_disconnect(RD) failed (%s)\n", strerror(errno));
		if (close(fd) == -1)
			++errors, printf("Test1: close(RD) failed (%s)\n", strerror(errno));
	}

	return 0;
}

static int writer(Agent *agent, int fd, int revents, void *arg)
{
	int *count = arg;
	char buf[2];

	snprintf(buf, 2, "%d", *count);

	if (write(fd, buf, 1) == -1)
	{
		++errors, printf("Test1: write() failed (%s)\n", strerror(errno));
		return -1;
	}

	if (++*count == 10)
	{
		if (agent_disconnect(agent, fd) == -1)
			++errors, printf("Test1: agent_disconnect(WR) failed (%s)\n", strerror(errno));
		if (close(fd) == -1)
			++errors, printf("Test1: close(RD) failed (%s)\n", strerror(errno));
	}

	return 0;
}

static int actor(Agent *agent, void *arg)
{
	int *count = arg;

	++*count;

	return 0;
}

static int invalid1(Agent *agent, void *arg)
{
	return agent_start(agent);
}

static int invalid2(Agent *agent, void *arg)
{
	if (agent_stop(agent) == -1)
		return -1;

	if (agent_start(agent) == -1)
		return -1;

	return 0;
}

static int every_jiffy(Agent *agent, void *arg)
{
	int *count = arg;
	timeval now[1];

	if (--*count && !agent_schedule(agent, 0, 10000, every_jiffy, arg))
		return -1;

	if (gettimeofday(now, NULL) == -1)
		return -1;

	printf("every_jiffy(%24.24s %ld usec) count = %d\n", ctime((const time_t *)&now->tv_sec), (long)now->tv_usec, *count);

	return 0;
}

static int every_second(Agent *agent, void *arg)
{
	int *count = arg;
	timeval now[1];

	if (--*count && !agent_schedule(agent, 1, 0, every_second, arg))
		return -1;

	if (gettimeofday(now, NULL) == -1)
		return -1;

	printf("every_second(%24.24s %ld usec) count = %d\n", ctime((const time_t *)&now->tv_sec), (long)now->tv_usec, *count);

	return 0;
}

static int every_minute(Agent *agent, void *arg)
{
	int *count = arg;
	timeval now[1];

	if (--*count && !agent_schedule(agent, 60, 0, every_minute, arg))
		return -1;

	if (gettimeofday(now, NULL) == -1)
		return -1;

	printf("every_minute(%24.24s %ld usec) count = %d\n", ctime((const time_t *)&now->tv_sec), (long)now->tv_usec, *count);

	return 0;
}

static int every_hour(Agent *agent, void *arg)
{
	int *count = arg;
	timeval now[1];

	if (--*count && !agent_schedule(agent, 60 * 60, 0, every_hour, arg))
		return -1;

	if (gettimeofday(now, NULL) == -1)
		return -1;

	printf("every_hour(%24.24s %ld usec) count = %d\n", ctime((const time_t *)&now->tv_sec), (long)now->tv_usec, *count);

	return 0;
}

static int every_day(Agent *agent, void *arg)
{
	int *count = arg;
	timeval now[1];

	if (--*count && !agent_schedule(agent, 24 * 60 * 60, 0, every_day, arg))
		return -1;

	if (gettimeofday(now, NULL) == -1)
		return -1;

	printf("every_day(%24.24s %ld usec) count = %d\n", ctime((const time_t *)&now->tv_sec), (long)now->tv_usec, *count);

	return 0;
}

static int every_second2(Agent *agent, void *arg)
{
	timeval now[1];

	if (gettimeofday(now, NULL) == -1)
		return -1;

	printf("every_second(%24.24s %ld usec)\n", ctime((const time_t *)&now->tv_sec), (long)now->tv_usec);

	return 0;
}

static int every_minute2(Agent *agent, void *arg)
{
	timeval now[1];

	if (gettimeofday(now, NULL) == -1)
		return -1;

	printf("every_minute(%24.24s %ld usec)\n", ctime((const time_t *)&now->tv_sec), (long)now->tv_usec);

	return 0;
}

static int every_hour2(Agent *agent, void *arg)
{
	timeval now[1];

	if (gettimeofday(now, NULL) == -1)
		return -1;

	printf("every_hour(%24.24s %ld usec)\n", ctime((const time_t *)&now->tv_sec), (long)now->tv_usec);

	return 0;
}

static int every_day2(Agent *agent, void *arg)
{
	timeval now[1];

	if (gettimeofday(now, NULL) == -1)
		return -1;

	printf("every_day(%24.24s %ld usec)\n", ctime((const time_t *)&now->tv_sec), (long)now->tv_usec);

	return 0;
}

static int receiver(Agent *agent, int fd, int revents, void *arg)
{
	char buf[BUFSIZ];
	int bytes;

	if ((bytes = read(fd, buf, BUFSIZ)) == -1)
	{
		++errors, printf("Test1: read() failed (%s)\n", strerror(errno));
		return -1;
	}

	if (bytes)
	{
		switch (agent_detail(agent, fd))
		{
			case 0: /* nothing - can't happen */
				printf("no activity - can't happen\n");
				break;

			case 1: /* time of last event */
			{
				const timeval * const tv = agent_last(agent, fd);
				printf("last %22.22s %6ld usec\n", ctime((const time_t *)&tv->tv_sec), (long)tv->tv_usec);
				break;
			}

			case 2: /* velocity */
			{
				const timeval * const tv = agent_last(agent, fd);
				printf("last %22.22s %6ld usec, vel %5d\n", ctime((const time_t *)&tv->tv_sec), (long)tv->tv_usec, agent_velocity(agent, fd));
				break;
			}

			case 3: /* acceleration */
			{
				const timeval * const tv = agent_last(agent, fd);
				printf("last %22.22s %6ld usec, vel %5d, accel %5d\n", ctime((const time_t *)&tv->tv_sec), (long)tv->tv_usec, agent_velocity(agent, fd), agent_acceleration(agent, fd));
				break;
			}

			case 4: /* dadt */
			{
				const timeval * const tv = agent_last(agent, fd);
				printf("last %22.22s %6ld usec, vel %5d, accel %5d, dadt %5d\n", ctime((const time_t *)&tv->tv_sec), (long)tv->tv_usec, agent_velocity(agent, fd), agent_acceleration(agent, fd), agent_dadt(agent, fd));
				break;
			}
		}
	}
	else /* eof */
	{
		if (agent_disconnect(agent, fd) == -1)
			++errors, printf("Test1: agent_disconnect(RD) failed (%s)\n", strerror(errno));
		if (close(fd) == -1)
			++errors, printf("Test1: close(RD) failed (%s)\n", strerror(errno));
	}

	return 0;
}

static int sender(Agent *agent, void *arg)
{
	static int state = 0;
	static int progress = 0;
	int *fd = arg;
	long sec, usec;

	if (write(*fd, "unimportant", 11) == -1)
		return -1;

	if (state & 1)
		sec = 0, usec = 10000;
	else
		sec = 0, usec = 250000;

	if (++progress == 10)
		++state, progress = 0;

	if (state == 10)
	{
		if (close(*fd) == -1)
			++errors, printf("Test1: failed to close(WR)\n");

		return 0;
	}

	if (!agent_schedule(agent, sec, usec, sender, arg))
		return -1;

	return 0;
}

static int oobrecv(Agent *agent, int fd, int revents, void *arg)
{
	char oob;

	if (revents == X_OK)
		printf("Test1: Out Of Band data arrives with revents == X_OK\n");
	else if (revents == R_OK)
		printf("Test1: Out Of Band data arrives with revents == R_OK\n");
	else if (revents == (R_OK | X_OK))
		printf("Test1: Out Of Band data arrives with revents == R_OK | X_OK\n");
	else
		printf("Test1: Out Of Band data arrives with revents == %d\n", revents);

	if (recv(fd, &oob, 1, MSG_OOB) == -1)
	{
		++errors, printf("Test1: recv(MSG_OOB) failed (%s)\n", strerror(errno));
		return -1;
	}

	if (agent_disconnect(agent, fd) == -1)
		++errors, printf("Test1: agent_disconnect(RD) failed (%s)\n", strerror(errno));
	if (close(fd) == -1)
		++errors, printf("Test1: close(RD) failed (%s)\n", strerror(errno));

	return 0;
}

static int slow(Agent *agent, void *arg)
{
	long *usec = arg;
	timeval now[1];

	if (gettimeofday(now, NULL) == -1)
		return -1;

	printf("s l o w(scheduled for %5ld usec) %24.24s %ld usec\n", *usec, ctime((const time_t *)&now->tv_sec), (long)now->tv_usec);

	sleep(1);

	return 0;
}

static int delayed(Agent *agent, void *arg)
{
	long *usec = arg;
	timeval now[1];

	if (gettimeofday(now, NULL) == -1)
		return -1;

	printf("delayed(scheduled for %5ld usec) %24.24s %ld usec\n", *usec, ctime((const time_t *)&now->tv_sec), (long)now->tv_usec);

	return 0;
}

int main(int ac, char **av)
{
	Agent *agent;
	void *ptr;
	int num;

	if (ac == 2 && !strcmp(av[1], "help"))
	{
		printf("usage: %s [activity|oob|accuracy(1|2|3) [#]|delay]\n", *av);
		return EXIT_SUCCESS;
	}

	printf("Testing: %s\n", "agent");

	/* Test empty agent */

	if (!(agent = agent_create()))
		++errors, printf("Test1: agent_create() failed (%s)\n", strerror(errno));
	else
	{
		if (agent_start(agent) == -1)
			++errors, printf("Test2: agent_start() failed (%s)\n", strerror(errno));

		agent_destroy(&agent);
		if (agent)
			++errors, printf("Test3: agent_destroy() failed (%s)\n", strerror(errno));
	}

	/* Test empty agent (using select) */

	if (!(agent = agent_create_using_select()))
		++errors, printf("Test4: agent_create_using_select() failed (%s)\n", strerror(errno));
	else
	{
		if (agent_start(agent) == -1)
			++errors, printf("Test5: agent_start() failed (%s)\n", strerror(errno));

		agent_destroy(&agent);
		if (agent)
			++errors, printf("Test6: agent_destroy() failed (%s)\n", strerror(errno));
	}

	/* Test reactions */

	if (!(agent = agent_create()))
		++errors, printf("Test7: agent_create() failed (%s)\n", strerror(errno));
	else
	{
		int pipefds[2];
		int rdcount = 0;
		int wrcount = 0;

		if (pipe(pipefds) == -1)
			++errors, printf("Test8: failed to perform test: pipe() failed (%s)\n", strerror(errno));
		else
		{
			if (agent_connect(agent, pipefds[0], R_OK, reader, &rdcount) == -1)
				++errors, printf("Test9: agent_connect(pipefds[RD]) failed (%s)\n", strerror(errno));
			else if (agent_connect(agent, pipefds[1], W_OK, writer, &wrcount) == -1)
				++errors, printf("Test10: agent_connect(pipefds[WR]) failed (%s)\n", strerror(errno));
			else if (agent_start(agent) == -1)
				++errors, printf("Test11: agent_start() failed (%s)\n", strerror(errno));
			else if (rdcount != 10)
				++errors, printf("Test12: rdcount = %d, not %d\n", rdcount, 10);
			else if (wrcount != 10)
				++errors, printf("Test13: wrcount = %d, not %d\n", wrcount, 10);

			close(pipefds[0]);
			close(pipefds[1]);
		}

		agent_destroy(&agent);
		if (agent)
			++errors, printf("Test14: agent_destroy() failed (%s)\n", strerror(errno));
	}

	/* Test reactions (using select) */

	if (!(agent = agent_create_using_select()))
		++errors, printf("Test15: agent_create_using_select() failed (%s)\n", strerror(errno));
	else
	{
		int pipefds[2];
		int rdcount = 0;
		int wrcount = 0;

		if (pipe(pipefds) == -1)
			++errors, printf("Test16: failed to perform test: pipe() failed (%s)\n", strerror(errno));
		else
		{
			if (agent_connect(agent, pipefds[0], R_OK, reader, &rdcount) == -1)
				++errors, printf("Test17: agent_connect(pipefds[RD]) failed (%s)\n", strerror(errno));
			else if (agent_connect(agent, pipefds[1], W_OK, writer, &wrcount) == -1)
				++errors, printf("Test18: agent_connect(pipefds[WR]) failed (%s)\n", strerror(errno));
			else if (agent_start(agent) == -1)
				++errors, printf("Test19: agent_start() failed (%s)\n", strerror(errno));
			else if (rdcount != 10)
				++errors, printf("Test20: rdcount = %d, not %d\n", rdcount, 10);
			else if (wrcount != 10)
				++errors, printf("Test21: wrcount = %d, not %d\n", wrcount, 10);

			close(pipefds[0]);
			close(pipefds[1]);
		}

		agent_destroy(&agent);
		if (agent)
			++errors, printf("Test22: agent_destroy() failed (%s)\n", strerror(errno));
	}

	/* Test actions */

	if (!(agent = agent_create()))
		++errors, printf("Test23: agent_create() failed (%s)\n", strerror(errno));
	else
	{
		int count = 0;

		if (!agent_schedule(agent, 0, 20000, actor, &count))
			++errors, printf("Test24: agent_schedule(actor) failed (%s)\n", strerror(errno));
		else if (!agent_schedule(agent, 0, 30000, actor, &count))
			++errors, printf("Test25: agent_schedule(actor) failed (%s)\n", strerror(errno));
		else if (!agent_schedule(agent, 0, 40000, actor, &count))
			++errors, printf("Test26: agent_schedule(actor) failed (%s)\n", strerror(errno));
		else if (agent_start(agent) == -1)
			++errors, printf("Test27: agent_start() failed (%s)\n", strerror(errno));
		else if (count != 3)
			++errors, printf("Test28: count = %d, not %d\n", count, 3);

		agent_destroy(&agent);
		if (agent)
			++errors, printf("Test29: agent_destroy() failed (%s)\n", strerror(errno));
	}

	/* Test actions using select */

	if (!(agent = agent_create_using_select()))
		++errors, printf("Test30: agent_create_using_select() failed (%s)\n", strerror(errno));
	else
	{
		int count = 0;

		if (!agent_schedule(agent, 0, 20000, actor, &count))
			++errors, printf("Test31: agent_schedule(actor) failed (%s)\n", strerror(errno));
		else if (!agent_schedule(agent, 0, 30000, actor, &count))
			++errors, printf("Test32: agent_schedule(actor) failed (%s)\n", strerror(errno));
		else if (!agent_schedule(agent, 0, 40000, actor, &count))
			++errors, printf("Test33: agent_schedule(actor) failed (%s)\n", strerror(errno));
		else if (agent_start(agent) == -1)
			++errors, printf("Test34: agent_start() failed (%s)\n", strerror(errno));
		else if (count != 3)
			++errors, printf("Test35: count = %d, not %d\n", count, 3);

		agent_destroy(&agent);
		if (agent)
			++errors, printf("Test36: agent_destroy() failed (%s)\n", strerror(errno));
	}

	/* Test actions and reactions */

	if (!(agent = agent_create()))
		++errors, printf("Test37: agent_create() failed (%s)\n", strerror(errno));
	else
	{
		int pipefds[2];
		int rdcount = 0;
		int wrcount = 0;
		int count = 0;

		if (pipe(pipefds) == -1)
			++errors, printf("Test38: failed to perform test: pipe() failed (%s)\n", strerror(errno));
		else
		{
			if (agent_connect(agent, pipefds[0], R_OK, reader, &rdcount) == -1)
				++errors, printf("Test39: agent_connect(pipefds[RD]) failed (%s)\n", strerror(errno));
			else if (agent_connect(agent, pipefds[1], W_OK, writer, &wrcount) == -1)
				++errors, printf("Test40: agent_connect(pipefds[WR]) failed (%s)\n", strerror(errno));
			else if (!agent_schedule(agent, 0, 20000, actor, &count))
				++errors, printf("Test41: agent_schedule(actor) failed (%s)\n", strerror(errno));
			else if (!agent_schedule(agent, 0, 30000, actor, &count))
				++errors, printf("Test42: agent_schedule(actor) failed (%s)\n", strerror(errno));
			else if (!agent_schedule(agent, 0, 40000, actor, &count))
				++errors, printf("Test43: agent_schedule(actor) failed (%s)\n", strerror(errno));
			else if (agent_start(agent) == -1)
				++errors, printf("Test44: agent_start() failed (%s)\n", strerror(errno));
			else if (rdcount != 10)
				++errors, printf("Test45: rdcount = %d, not %d\n", rdcount, 10);
			else if (wrcount != 10)
				++errors, printf("Test46: wrcount = %d, not %d\n", wrcount, 10);
			else if (count != 3)
				++errors, printf("Test47: count = %d, not %d\n", count, 3);

			close(pipefds[0]);
			close(pipefds[1]);
		}

		agent_destroy(&agent);
		if (agent)
			++errors, printf("Test48: agent_destroy() failed (%s)\n", strerror(errno));
	}

	/* Test actions and reactions (using select) */

	if (!(agent = agent_create_using_select()))
		++errors, printf("Test49: agent_create_using_select() failed (%s)\n", strerror(errno));
	else
	{
		int pipefds[2];
		int rdcount = 0;
		int wrcount = 0;
		int count = 0;

		if (pipe(pipefds) == -1)
			++errors, printf("Test50: failed to perform test: pipe() failed (%s)\n", strerror(errno));
		else
		{
			if (agent_connect(agent, pipefds[0], R_OK, reader, &rdcount) == -1)
				++errors, printf("Test51: agent_connect(pipefds[RD]) failed (%s)\n", strerror(errno));
			else if (agent_connect(agent, pipefds[1], W_OK, writer, &wrcount) == -1)
				++errors, printf("Test52: agent_connect(pipefds[WR]) failed (%s)\n", strerror(errno));
			else if (!agent_schedule(agent, 0, 20000, actor, &count))
				++errors, printf("Test53: agent_schedule(actor) failed (%s)\n", strerror(errno));
			else if (!agent_schedule(agent, 0, 30000, actor, &count))
				++errors, printf("Test54: agent_schedule(actor) failed (%s)\n", strerror(errno));
			else if (!agent_schedule(agent, 0, 40000, actor, &count))
				++errors, printf("Test55: agent_schedule(actor) failed (%s)\n", strerror(errno));
			else if (agent_start(agent) == -1)
				++errors, printf("Test56: agent_start() failed (%s)\n", strerror(errno));
			else if (rdcount != 10)
				++errors, printf("Test57: rdcount = %d, not %d\n", rdcount, 10);
			else if (wrcount != 10)
				++errors, printf("Test58: wrcount = %d, not %d\n", wrcount, 10);
			else if (count != 3)
				++errors, printf("Test59: count = %d, not %d\n", count, 3);

			close(pipefds[0]);
			close(pipefds[1]);
		}

		agent_destroy(&agent);
		if (agent)
			++errors, printf("Test60: agent_destroy() failed (%s)\n", strerror(errno));
	}

	/* Test agent state protocol errors */

	if (!(agent = agent_create()))
		++errors, printf("Test61: agent_create() failed (%s)\n", strerror(errno));
	else
	{
		/* Test that agent_stop() on an idle agent fails */

		if (agent_stop(agent) != -1)
			++errors, printf("Test62: agent_stop() on idle agent failed (returned 0, not -1)\n");
		else if (errno != EINVAL)
			++errors, printf("Test63: agent_stop() on idle agent failed (errno = %s, not %s)\n", strerror(errno), strerror(EINVAL));

		/* Test that agent_start() inside a scheduled event fails */

		if (!agent_schedule(agent, 0, 10000, invalid1, NULL))
			++errors, printf("Test64: agent_schedule(invalid1) failed (%s)\n", strerror(errno));
		else if (agent_start(agent) != -1)
			++errors, printf("Test65: agent_start(invalid1) failed (returned 0, not -1)\n");
		else if (errno != EINVAL)
			++errors, printf("Test66: agent_start(invalid1) failed (errno = %s, not %s)\n", strerror(errno), strerror(EINVAL));

		/* Test that agent_stop() followed by agent_start() inside a scheduled event fails */

		if (!agent_schedule(agent, 0, 10000, invalid2, NULL))
			++errors, printf("Test67: agent_schedule(invalid2) failed (%s)\n", strerror(errno));
		else if (agent_start(agent) != -1)
			++errors, printf("Test68: agent_start(invalid2) failed (returned 0, not -1)\n");
		else if (errno != EINVAL)
			++errors, printf("Test69: agent_start(invalid2) failed (errno = %s, not %s)\n", strerror(errno), strerror(EINVAL));

		agent_destroy(&agent);
		if (agent)
			++errors, printf("Test70: agent_destroy() failed (%s)\n", strerror(errno));
	}

	/* Test agent state protocol errors (using select) */

	if (!(agent = agent_create_using_select()))
		++errors, printf("Test71: agent_create_using_select() failed (%s)\n", strerror(errno));
	else
	{
		/* Test that agent_stop() on an idle agent fails */

		if (agent_stop(agent) != -1)
			++errors, printf("Test72: agent_stop() on idle agent failed (returned 0, not -1)\n");
		else if (errno != EINVAL)
			++errors, printf("Test73: agent_stop() on idle agent failed (errno = %s, not %s)\n", strerror(errno), strerror(EINVAL));

		/* Test that agent_start() inside a scheduled event fails */

		if (!agent_schedule(agent, 0, 10000, invalid1, NULL))
			++errors, printf("Test74: agent_schedule(invalid1) failed (%s)\n", strerror(errno));
		else if (agent_start(agent) != -1)
			++errors, printf("Test75: agent_start(invalid1) failed (returned 0, not -1)\n");
		else if (errno != EINVAL)
			++errors, printf("Test76: agent_start(invalid1) failed (errno = %s, not %s)\n", strerror(errno), strerror(EINVAL));

		/* Test that agent_stop() followed by agent_start() inside a scheduled event fails */

		if (!agent_schedule(agent, 0, 10000, invalid2, NULL))
			++errors, printf("Test77: agent_schedule(invalid2) failed (%s)\n", strerror(errno));
		else if (agent_start(agent) != -1)
			++errors, printf("Test78: agent_start(invalid2) failed (returned 0, not -1)\n");
		else if (errno != EINVAL)
			++errors, printf("Test79: agent_start(invalid2) failed (errno = %s, not %s)\n", strerror(errno), strerror(EINVAL));

		agent_destroy(&agent);
		if (agent)
			++errors, printf("Test80: agent_destroy() failed (%s)\n", strerror(errno));
	}

	/* Check connection activity measurement */

	if (ac == 2 && !strcmp(av[1], "activity"))
	{
		printf("Checking the measurement of connection activity\n");

		if (!(agent = agent_create_measured()))
			++errors, printf("Test81: agent_create_measured() failed (%s)\n", strerror(errno));
		else
		{
			int pipefds[2];

			if (pipe(pipefds) == -1)
				++errors, printf("Test82: failed to run test: pipe() failed (%s)\n", strerror(errno));
			else
			{
				if (agent_connect(agent, pipefds[0], R_OK | X_OK, receiver, NULL) == -1)
					++errors, printf("Test83: agent_connect(receiver) failed (%s)\n", strerror(errno));
				else if (!agent_schedule(agent, 0, 0, sender, &pipefds[1]))
					++errors, printf("Test84: agent_schedule(sender) failed (%s)\n", strerror(errno));
				else if (agent_start(agent) == -1)
					++errors, printf("Test85: agent_start(sender/receiver) failed (%s)\n", strerror(errno));

				close(pipefds[0]);
				close(pipefds[1]);
			}

			agent_destroy(&agent);
			if (agent)
				++errors, printf("Test86: agent_destroy() failed (%s)\n", strerror(errno));
		}
	}

	/* Check Out Of Band data recognition */

	/*
	** Linux's poll(2) manpage states that POLLPRI is used for Urgent data.
	** On other system's however, TCP Urgent data is treated as normal
	** because it is not delivered ahead of previously sent data.
	*/

	if (ac == 2 && !strcmp(av[1], "oob"))
	{
		printf("Checking whether out of band data is treated as normal and/or high priority\n");

		if (!(agent = agent_create()))
			++errors, printf("Test87: agent_create() failed (%s)\n", strerror(errno));
		else
		{
			int server = net_server("localhost", NULL, 30000, 0, 0, NULL, NULL);
			if (server == -1)
				++errors, printf("Test88: net_server(\"localhost\", 30000) failed: %s\n", strerror(errno));
			else
			{
				pid_t pid;

				switch (pid = fork())
				{
					case -1:
					{
						printf("Failed to fork (%s)\n", strerror(errno));
						return 1;
					}

					default:
					{
						int s;
						sockaddr_any_t addr;
#ifdef HAVE_NETADDR_SIZE_IS_INT
						int addrsize = sizeof(sockaddr_any_t);
#else
						size_t addrsize = sizeof(sockaddr_any_t);
#endif
						int status;

						if (read_timeout(server, 5, 0) == -1 || (s = accept(server, (sockaddr_t *)&addr, (void *)&addrsize)) == -1)
							++errors, printf("Test89: accept() failed (%s)\n", strerror(errno));
						else
						{
							if (agent_connect(agent, s, R_OK | X_OK, oobrecv, NULL) == -1)
								++errors, printf("Test90: agent_connect(oobrecv) failed (%s)\n", strerror(errno));
							else if (agent_start(agent) == -1)
								++errors, printf("Test91: agent_start(oob) failed (%s)\n", strerror(errno));
						}

						if (waitpid(pid, &status, 0) == -1)
							++errors, printf("Test92: failed to waitpid(%d) (%s)\n", (int)pid, strerror(errno));

						if (WIFSIGNALED(status))
							++errors, printf("Test93: child failed: received signal %d\n", WTERMSIG(status));

						if (WIFEXITED(status) && WEXITSTATUS(status) != EXIT_SUCCESS)
							++errors, printf("Test94: child failed: return code = %d\n", WEXITSTATUS(status));

						break;
					}

					case 0:
					{
						int i, c = 0;
						errors = 0;

						for (i = 0; i < 5; ++i)
						{
							c = net_client(NULL, NULL, 30000, 5, 0, 0, NULL, NULL);
							if (c != -1)
								break;
							sleep(1);
						}

						if (c == -1)
							++errors, printf("Test95: net_client(\"localhost\", 30000) failed (%s)\n", strerror(errno));
						else if (send(c, "!", 1, MSG_OOB) == -1)
							++errors, printf("Test96: send(MSG_OOB) failed (%s)\n", strerror(errno));

						return errors;
					}
				}

				if (close(server) == -1)
					++errors, printf("Test97: close(server) failed (%s)\n", strerror(errno));
			}

			agent_destroy(&agent);
			if (agent)
				++errors, printf("Test98: agent_destroy() failed (%s)\n", strerror(errno));
		}
	}

	/* Check timer accuracy */

	if (ac >= 2 && !strcmp(av[1], "accuracy1"))
	{
		int seconds = (av[2]) ? atoi(av[2]) : 10;

		if (seconds == 0)
			seconds = 10;

		printf("Checking timer accuracy\n");

		/* Check an action every jiffy for 10 seconds */

		printf("Check timer every 10ms for %ds\n", seconds);

		if (!(agent = agent_create()))
			++errors, printf("Test99: agent_create() failed (%s)\n", strerror(errno));
		else
		{
			int jiffies = seconds * 100;

			if (!agent_schedule(agent, 0, 0, every_jiffy, &jiffies))
				++errors, printf("Test100: agent_schedule(every_jiffy) failed (%s)\n", strerror(errno));
			else if (agent_start(agent) == -1)
				++errors, printf("Test101: agent_start(every_jiffy) failed (%s)\n", strerror(errno));
			else if (jiffies)
				++errors, printf("Test102: agent_start(every_jiffy) failed (jiffies = %d, not %d)\n", jiffies, 0);

			agent_destroy(&agent);
			if (agent)
				++errors, printf("Test103: agent_destroy() failed (%s)\n", strerror(errno));
		}

		printf("Check timer every 10ms for %ds (using select)\n", seconds);

		if (!(agent = agent_create_using_select()))
			++errors, printf("Test104: agent_create_using_select() failed (%s)\n", strerror(errno));
		else
		{
			int jiffies = seconds * 100;

			/* Test an action every jiffy for 10 seconds (using select) */

			if (!agent_schedule(agent, 0, 0, every_jiffy, &jiffies))
				++errors, printf("Test105: agent_schedule(every_jiffy) failed (%s)\n", strerror(errno));
			else if (agent_start(agent) == -1)
				++errors, printf("Test106: agent_start(every_jiffy) failed (%s)\n", strerror(errno));
			else if (jiffies)
				++errors, printf("Test107: agent_start(every_jiffy) failed (jiffies = %d, not %d)\n", jiffies, 0);

			agent_destroy(&agent);
			if (agent)
				++errors, printf("Test108: agent_destroy() failed (%s)\n", strerror(errno));
		}
	}

	if (ac >= 2 && !strcmp(av[1], "accuracy2"))
	{
		int days = (av[2]) ? atoi(av[2]) : 21;

		if (days == 0)
			days = 21;

		printf("Checking timer accuracy\n");

		/* Check actions every second, minute, hour and day for three days */

		printf("Check timer every second, minute, hour and day for three days\n");
		printf("Actions scheduled lazily\n");

		/*
		** Note: this test needs to be performed over > DAYS * 2 days to
		** ensure that the timewheel rotates correctly but that takes at
		** least 21 days. You can supply a second argument specifying the
		** number of days if you want to run this test for fewer days.
		**
		** I've tested it with DAYS == 2 for 5 "days" where a "day" is 2
		** "hours" and an "hour" is 2 "minutes" and a minute is 2 seconds so
		** the test only took 40 seconds to run.
		*/

		if (!(agent = agent_create_using_select()))
			++errors, printf("Test109: agent_create_using_select() failed (%s)\n", strerror(errno));
		else
		{
			int hours = days * 24;
			int minutes = hours * 60;
			int seconds = minutes * 60;

			if (!agent_schedule(agent, 0, 0, every_second, &seconds))
				++errors, printf("Test110: agent_schedule(every_second) failed (%s)\n", strerror(errno));
			if (!agent_schedule(agent, 0, 0, every_minute, &minutes))
				++errors, printf("Test111: agent_schedule(every_minute) failed (%s)\n", strerror(errno));
			if (!agent_schedule(agent, 0, 0, every_hour, &hours))
				++errors, printf("Test112: agent_schedule(every_hour) failed (%s)\n", strerror(errno));
			if (!agent_schedule(agent, 0, 0, every_day, &days))
				++errors, printf("Test113: agent_schedule(every_day) failed (%s)\n", strerror(errno));
			else if (agent_start(agent) == -1)
				++errors, printf("Test114: agent_start(accuracy) failed (%s)\n", strerror(errno));
			else if (days)
				++errors, printf("Test115: agent_start(accuracy) failed (days = %d, not %d)\n", days, 0);
			else if (hours)
				++errors, printf("Test116: agent_start(accuracy) failed (hours = %d, not %d)\n", hours, 0);
			else if (minutes)
				++errors, printf("Test117: agent_start(accuracy) failed (minutes = %d, not %d)\n", minutes, 0);
			else if (seconds)
				++errors, printf("Test118: agent_start(accuracy) failed (seconds = %d, not %d)\n", seconds, 0);

			agent_destroy(&agent);
			if (agent)
				++errors, printf("Test119: agent_destroy() failed (%s)\n", strerror(errno));
		}
	}

	if (ac >= 2 && !strcmp(av[1], "accuracy3"))
	{
		int days = (av[2]) ? atoi(av[2]) : 21;

		if (days == 0)
			days = 21;

		printf("Checking timer accuracy\n");

		/* Check actions every second, minute, hour and day for three days */

		printf("Check timer every second, minute, hour and day for three days\n");
		printf("Actions scheduled in advance\n");

		/*
		** Note: this test needs to be performed over > DAYS * 2 days to
		** ensure that the timewheel rotates correctly but that takes at
		** least 21 days. You can supply a second argument specifying the
		** number of days if you want to run this test for fewer days.
		**
		** I've tested it with DAYS == 2 for 5 "days" where a "day" is 2
		** "hours" and an "hour" is 2 "minutes" and a minute is 2 seconds so
		** the test only took 40 seconds to run.
		*/

		if (!(agent = agent_create_using_select()))
			++errors, printf("Test120: agent_create_using_select() failed (%s)\n", strerror(errno));
		else
		{
			int hours = days * 24;
			int minutes = hours * 60;
			int seconds = minutes * 60;
			int day, hour, minute, second;

			for (day = 0; day < days; ++day)
				if (!agent_schedule(agent, day * 24 * 60 * 60, 0, every_day2, NULL))
					++errors, printf("Test121: agent_schedule(every_day2) failed (%s)\n", strerror(errno));

			for (hour = 0; hour < hours; ++hour)
				if (!agent_schedule(agent, hour * 60 * 60, 0, every_hour2, NULL))
					++errors, printf("Test122: agent_schedule(every_hour2) failed (%s)\n", strerror(errno));

			for (minute = 0; minute < minutes; ++minute)
				if (!agent_schedule(agent, minute * 60, 0, every_minute2, NULL))
					++errors, printf("Test123: agent_schedule(every_minute2) failed (%s)\n", strerror(errno));

			for (second = 0; second < seconds; ++second)
				if (!agent_schedule(agent, second, 0, every_second2, NULL))
					++errors, printf("Test124: agent_schedule(every_second2) failed (%s)\n", strerror(errno));

			if (agent_start(agent) == -1)
				++errors, printf("Test125: agent_start(accuracy) failed (%s)\n", strerror(errno));

			agent_destroy(&agent);
			if (agent)
				++errors, printf("Test126: agent_destroy() failed (%s)\n", strerror(errno));
		}
	}

	/* Check long running actions/reactions and delayed actions */

	if (ac == 2 && !strcmp(av[1], "delay"))
	{
		printf("Checking delays caused by long running actions/reactions\n");

		if (!(agent = agent_create()))
			++errors, printf("Test127: agent_create() failed (%s)\n", strerror(errno));
		else
		{
			long usec0 = 0;
			long usec1 = 10000;
			long usec2 = 20000;

			if (!agent_schedule(agent, 0, usec0, slow, &usec0))
				++errors, printf("Test128: agent_schedule(slow) failed (%s)\n", strerror(errno));
			else if (!agent_schedule(agent, 0, usec1, delayed, &usec1))
				++errors, printf("Test129: agent_schedule(delayed) failed (%s)\n", strerror(errno));
			else if (!agent_schedule(agent, 0, usec2, delayed, &usec2))
				++errors, printf("Test130: agent_schedule(delayed) failed (%s)\n", strerror(errno));
			else if (agent_start(agent) == -1)
				++errors, printf("Test131: agent_start() failed (%s)\n", strerror(errno));

			agent_destroy(&agent);
			if (agent)
				++errors, printf("Test132: agent_destroy() failed (%s)\n", strerror(errno));
		}
	}

	/* XXX Test MT */

	/* XXX Test fast/slow lane */

	/* Test errors */

	if (!(agent = agent_create()))
		++errors, printf("Test133: agent_create() failed (%s)\n", strerror(errno));
	else
	{
		if (agent_connect(NULL, 0, R_OK, reader, NULL) != -1)
			++errors, printf("Test134: agent_connect(agent == NULL) failed\n");
		else if (errno != EINVAL)
			++errors, printf("Test135: agent_connect(agent == NULL) failed (errno = %s, not %s)\n", strerror(errno), strerror(EINVAL));

		if (agent_connect(agent, -1, R_OK, reader, NULL) != -1)
			++errors, printf("Test136: agent_connect(fd == -1) failed\n");
		else if (errno != EINVAL)
			++errors, printf("Test137: agent_connect(fd == -1) failed (errno = %s, not %s)\n", strerror(errno), strerror(EINVAL));

		if (agent_connect(agent, 0, 0, reader, NULL) != -1)
			++errors, printf("Test138: agent_connect(events == 0) failed\n");
		else if (errno != EINVAL)
			++errors, printf("Test139: agent_connect(events == 0) failed (errno = %s, not %s)\n", strerror(errno), strerror(EINVAL));

		if (agent_connect(agent, 0, -1, reader, NULL) != -1)
			++errors, printf("Test140: agent_connect(events == -1) failed\n");
		else if (errno != EINVAL)
			++errors, printf("Test141: agent_connect(events == -1) failed (errno = %s, not %s)\n", strerror(errno), strerror(EINVAL));

		if (agent_connect(agent, 0, R_OK, NULL, NULL) != -1)
			++errors, printf("Test142: agent_connect(reaction == NULL) failed\n");
		else if (errno != EINVAL)
			++errors, printf("Test143: agent_connect(reaction == NULL) failed (errno = %s, not %s)\n", strerror(errno), strerror(EINVAL));

		if (agent_disconnect(NULL, 0) != -1)
			++errors, printf("Test144: agent_disconnect(agent == NULL) failed\n");
		else if (errno != EINVAL)
			++errors, printf("Test145: agent_disconnect(agent == NULL) failed (errno = %s, not %s)\n", strerror(errno), strerror(EINVAL));

		if (agent_disconnect(agent, -1) != -1)
			++errors, printf("Test146: agent_disconnect(fd == -1) failed\n");
		else if (errno != EINVAL)
			++errors, printf("Test147: agent_disconnect(fd == -1) failed (errno = %s, not %s)\n", strerror(errno), strerror(EINVAL));

		if (agent_disconnect(agent, 0) != -1)
			++errors, printf("Test148: agent_disconnect(unconnected fd) failed\n");
		else if (errno != EINVAL)
			++errors, printf("Test149: agent_disconnect(unconnected fd) failed (errno = %s, not %s)\n", strerror(errno), strerror(EINVAL));

		if (agent_velocity(NULL, 0) != -1)
			++errors, printf("Test150: agent_velocity(agent == NULL) failed\n");
		else if (errno != EINVAL)
			++errors, printf("Test151: agent_velocity(agent == NULL) failed (errno = %s, not %s)\n", strerror(errno), strerror(EINVAL));

		if (agent_velocity(agent, -1) != -1)
			++errors, printf("Test152: agent_velocity(fd == -1) failed\n");
		else if (errno != EINVAL)
			++errors, printf("Test153: agent_velocity(fd == -1) failed (errno = %s, not %s)\n", strerror(errno), strerror(EINVAL));

		if (agent_velocity(agent, 0) != -1)
			++errors, printf("Test154: agent_velocity(unconnected fd) failed\n");
		else if (errno != EINVAL)
			++errors, printf("Test155: agent_velocity(unconnected fd) failed (errno = %s, not %s)\n", strerror(errno), strerror(EINVAL));

		if (agent_schedule(NULL, 0, 0, actor, NULL) != NULL)
			++errors, printf("Test156: agent_schedule(agent == NULL) failed\n");
		else if (errno != EINVAL)
			++errors, printf("Test157: agent_schedule(agent == NULL) failed (errno = %s, not %s)\n", strerror(errno), strerror(EINVAL));

		if (agent_schedule(agent, -1, 0, actor, NULL) != NULL)
			++errors, printf("Test158: agent_schedule(sec == -1) failed\n");
		else if (errno != EINVAL)
			++errors, printf("Test159: agent_schedule(sec == -1) failed (errno = %s, not %s)\n", strerror(errno), strerror(EINVAL));

		if (agent_schedule(agent, 0, -1, actor, NULL) != NULL)
			++errors, printf("Test160: agent_schedule(usec == -1) failed\n");
		else if (errno != EINVAL)
			++errors, printf("Test161: agent_schedule(usec == -1) failed (errno = %s, not %s)\n", strerror(errno), strerror(EINVAL));

		if (agent_schedule(agent, 0, 0, NULL, NULL) != NULL)
			++errors, printf("Test162: agent_schedule(action == NULL) failed\n");
		else if (errno != EINVAL)
			++errors, printf("Test163: agent_schedule(action == NULL) failed (errno = %s, not %s)\n", strerror(errno), strerror(EINVAL));

		if (agent_cancel(NULL, (void *)1) != -1)
			++errors, printf("Test164: agent_cancel(agent == NULL) failed\n");
		else if (errno != EINVAL)
			++errors, printf("Test165: agent_cancel(agent == NULL) failed (errno = %s, not %s)\n", strerror(errno), strerror(EINVAL));

		if (agent_cancel(agent, NULL) != -1)
			++errors, printf("Test166: agent_cancel(action_id == NULL) failed\n");
		else if (errno != EINVAL)
			++errors, printf("Test167: agent_cancel(action_id == NULL) failed (errno = %s, not %s)\n", strerror(errno), strerror(EINVAL));

		if (agent_cancel(agent, (void *)1) != -1)
			++errors, printf("Test168: agent_cancel(never scheduled an action) failed\n");
		else if (errno != EINVAL)
			++errors, printf("Test169: agent_cancel(never scheduled an action) failed (errno = %s, not %s)\n", strerror(errno), strerror(EINVAL));

		if (agent_start(NULL) != -1)
			++errors, printf("Test170: agent_start(agent == NULL) failed\n");
		else if (errno != EINVAL)
			++errors, printf("Test171: agent_start(agent == NULL) failed (errno = %s, not %s)\n", strerror(errno), strerror(EINVAL));

		if (agent_stop(NULL) != -1)
			++errors, printf("Test172: agent_stop(agent == NULL) failed\n");
		else if (errno != EINVAL)
			++errors, printf("Test173: agent_stop(agent == NULL) failed (errno = %s, not %s)\n", strerror(errno), strerror(EINVAL));

		agent_destroy(&agent);
		if (agent)
			++errors, printf("Test174: agent_destroy() failed (%s)\n", strerror(errno));
	}

	/* Test assumption: memset(&ptr, 0, sizeof(void *)) same as NULL */

	memset(&ptr, 0, sizeof(void *));
	if (ptr != NULL)
		++errors, printf("Test175: assumption failed: memset(&ptr, 0, sizeof(void *)) not == NULL\n");

	/* Test assumption: memset(&num, 0, sizeof(int)) same as 0 */

	memset(&num, 0, sizeof(int));
	if (num != 0)
		++errors, printf("Test176: assumption failed: memset(&num, 0, sizeof(int)) not == 0\n");

	/* Test assumption: memset(&num, 0xff, sizeof(int)) same as -1 */

	memset(&num, 0xff, sizeof(int));
	if (num != -1)
		++errors, printf("Test177: assumption failed: memset(&num, 0xff, sizeof(int)) not == -1\n");

	if (errors)
		printf("%d/177 tests failed\n", errors);
	else
		printf("All tests passed\n");

	return (errors == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

#endif

/* vi:set ts=4 sw=4: */
