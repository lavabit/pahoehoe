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

I<libslack(locker)> - abstract locking, rwlocks

=head1 SYNOPSIS

    #include <slack/std.h>
    #include <slack/locker.h>

    typedef struct Locker Locker;
    typedef int lockerf_t(void *lock);

    Locker *locker_create_mutex(pthread_mutex_t *mutex);
    Locker *locker_create_rwlock(pthread_rwlock_t *rwlock);
    Locker *locker_create_debug_mutex(pthread_mutex_t *mutex);
    Locker *locker_create_debug_rwlock(pthread_rwlock_t *rwlock);
    Locker *locker_create(void *lock, lockerf_t *tryrdlock, lockerf_t *rdlock, lockerf_t *trywrlock, lockerf_t *wrlock, lockerf_t *unlock);
    void locker_release(Locker *locker);
    void *locker_destroy(Locker **locker);
    int locker_tryrdlock(Locker *locker);
    int locker_rdlock(Locker *locker);
    int locker_trywrlock(Locker *locker);
    int locker_wrlock(Locker *locker);
    int locker_unlock(Locker *locker);

    int pthread_rwlock_init(pthread_rwlock_t *rwlock, const pthread_rwlockattr_t *attr);
    int pthread_rwlock_destroy(pthread_rwlock_t *rwlock);
    int pthread_rwlock_rdlock(pthread_rwlock_t *rwlock);
    int pthread_rwlock_tryrdlock(pthread_rwlock_t *rwlock);
    int pthread_rwlock_wrlock(pthread_rwlock_t *rwlock);
    int pthread_rwlock_trywrlock(pthread_rwlock_t *rwlock);
    int pthread_rwlock_unlock(pthread_rwlock_t *rwlock);
    int pthread_rwlockattr_init(pthread_rwlockattr_t *attr);
    int pthread_rwlockattr_destroy(pthread_rwlockattr_t *attr);
    int pthread_rwlockattr_getpshared(const pthread_rwlockattr_t *attr, int *pshared);
    int pthread_rwlockattr_setpshared(pthread_rwlockattr_t *attr, int pshared);

=head1 DESCRIPTION

This module provides an abstraction of thread synchronisation that
facilitates the implementation of I<MT-Disciplined> libraries. I'll explain
what this means.

Libraries need to be I<MT-Safe> when used in a multi threaded program.
However, most programs are single threaded and synchronisation doesn't come
for free so libraries should be I<Unsafe> when used in a single threaded
program. Even in multi threaded programs, some functions or objects may only
be accessed by a single thread and so they should not incur the expense of
synchronisation.

When an object is shared between multiple threads which need to be
synchronised, the method of synchronisation must be carefully selected by
the client code. There are tradeoffs between concurrency and overhead. The
greater the concurrency, the greater the overhead. More locks give greater
concurrency but have greater overhead. Readers/Writer locks can give greater
concurrency than Mutex locks but have greater overhead. One lock for each
object may be required, or one lock for all (or a set of) objects may be
more appropriate.

Generally, the best synchronisation strategy for a given application can
only be determined by testing/benchmarking the written application. It is
important to be able to experiment with the synchronisation strategy at this
stage of development without pain.

The solution, of course, is to decouple the synchronisation strategy from
the library code. To facilitate this, the I<Locker> type and associated
functions can be incorporated into library code to provide the necessary
flexibility.

The I<Locker> type specifies a lock and a set of functions for manipulating
the lock. Arbitrary objects can include a pointer to a I<Locker> object to
use for thread synchronisation. Such objects may each have their own lock by
having separate I<Locker> objects or they may share the same lock by sharing
the same I<Locker> object. Only the application developer can determine what
is appropriate for each application on a case by case basis.

I<MT-Disciplined> means that the application developer has a mechanism for
specifying the synchronisation requirements to be applied to library code.
For more details, see I<MT-Disciplined - decoupling thread safety>
(C<http://raf.org/papers/mt-disciplined.html>).

This module also provides an implementation of readers/writer locks which
are defined in recent standards but may not be on your system yet. The
readers/writer lock implementation originally came from code by Bil Lewis
and was then completed and made robust.

=over 4

=cut

*/

#include "config.h"
#include "std.h"

#include "locker.h"
#include "mem.h"
#include "err.h"

#ifndef HAVE_PTHREAD_PROCESS_PRIVATE
#define PTHREAD_PROCESS_PRIVATE 0
#endif

#ifndef HAVE_PTHREAD_PROCESS_SHARED
#define PTHREAD_PROCESS_SHARED 0
#endif

#ifndef HAVE_PTHREAD_CONDATTR_INIT
#define pthread_condattr_init(condattr) 0
#endif

#ifndef HAVE_PTHREAD_CONDATTR_SETPSHARED
#define pthread_condattr_setpshared(condattr, pshared) 0
#endif

#ifndef HAVE_PTHREAD_MUTEXATTR_SETPSHARED
#define pthread_mutexattr_setpshared(mutexattr, pshared) 0
#endif

#if 0
struct Locker
{
	void *lock;
	lockerf_t *tryrdlock;
	lockerf_t *rdlock;
	lockerf_t *trywrlock;
	lockerf_t *wrlock;
	lockerf_t *unlock;
};
#endif

#ifndef TEST

#define try(action) { int rc = (action); if (rc != 0) return rc; }
#define try_catch(action, catch) { int rc = (action); if (rc != 0) return (catch), rc; }

/*

=item C<Locker *locker_create_mutex(pthread_mutex_t *mutex)>

Creates a I<Locker> object that will operate on the mutex lock, C<mutex>.
I<locker_tryrdlock(3)> and I<locker_trywrlock(3)> will call
I<pthread_mutex_trylock(3)>. I<locker_rdlock(3)> and I<locker_wrlock(3)>
will call I<pthread_mutex_lock(3)>. I<locker_unlock(3)> will call
I<pthread_mutex_unlock(3)>. It is the caller's responsibility to initialise
C<mutex> if necessary before use and to destroy C<mutex> if necessary after
use. On success, returns the new I<Locker> object. On error, returns C<null>
with C<errno> set appropriately.

=cut

*/

Locker *locker_create_mutex(pthread_mutex_t *mutex)
{
	return locker_create
	(
		mutex,
		(lockerf_t *)pthread_mutex_trylock,
		(lockerf_t *)pthread_mutex_lock,
		(lockerf_t *)pthread_mutex_trylock,
		(lockerf_t *)pthread_mutex_lock,
		(lockerf_t *)pthread_mutex_unlock
	);
}

/*

=item C<Locker *locker_create_rwlock(pthread_rwlock_t *rwlock)>

Creates a I<Locker> object that will operate on the readers/writer lock,
C<rwlock>. I<locker_tryrdlock(3)> will call I<pthread_rwlock_tryrdlock(3)>.
I<locker_rdlock(3)> will call I<pthread_rwlock_rdlock(3)>.
I<locker_trywrlock(3)> will call I<pthread_rwlock_trywrlock(3)>.
I<locker_wrlock(3)> will call I<pthread_rwlock_wrlock(3)>.
I<locker_unlock(3)> will call I<pthread_rwlock_unlock(3)>. It is the
caller's responsibility to initialise C<rwlock> if necessary before use and
to destroy C<rwlock> if necessary after use. On success, returns the new
I<Locker> object. On error, returns C<null> with C<errno> set appropriately.

=cut

*/

Locker *locker_create_rwlock(pthread_rwlock_t *rwlock)
{
	return locker_create
	(
		rwlock,
		(lockerf_t *)pthread_rwlock_tryrdlock,
		(lockerf_t *)pthread_rwlock_rdlock,
		(lockerf_t *)pthread_rwlock_trywrlock,
		(lockerf_t *)pthread_rwlock_wrlock,
		(lockerf_t *)pthread_rwlock_unlock
	);
}

#ifndef NO_DEBUG_LOCKERS

/*

=item C<Locker *locker_create_debug_mutex(pthread_mutex_t *mutex)>

Equivalent to I<locker_create_mutex(3)> except that debug messages are
printed to standard output before and after each locking function is called
to help locate deadlocks. The debug messages look like:

    [thread id] funcname(mutex address) ...
    [thread id] funcname(mutex address) done

On success, returns the new I<Locker>. On error, returns C<null> with
C<errno> set appropriately.

=cut

*/

static int debug_invoke(const char *name, lockerf_t *action, void *lock)
{
	int err;

	printf("[%lu] %s(%p) ...\n", (unsigned long)pthread_self(), name, lock);
	err = action(lock);
	if (err)
		printf("[%lu] %s(%p) done (%s)\n", (unsigned long)pthread_self(), name, lock, strerror(err));
	else
		printf("[%lu] %s(%p) done\n", (unsigned long)pthread_self(), name, lock);
	return err;
}

static int debug_pthread_mutex_trylock(pthread_mutex_t *mutex)
{
	return debug_invoke("pthread_mutex_trylock", (lockerf_t *)pthread_mutex_trylock, mutex);
}

static int debug_pthread_mutex_lock(pthread_mutex_t *mutex)
{
	return debug_invoke("pthread_mutex_lock", (lockerf_t *)pthread_mutex_lock, mutex);
}

static int debug_pthread_mutex_unlock(pthread_mutex_t *mutex)
{
	return debug_invoke("pthread_mutex_unlock", (lockerf_t *)pthread_mutex_unlock, mutex);
}

Locker *locker_create_debug_mutex(pthread_mutex_t *mutex)
{
	return locker_create
	(
		mutex,
		(lockerf_t *)debug_pthread_mutex_trylock,
		(lockerf_t *)debug_pthread_mutex_lock,
		(lockerf_t *)debug_pthread_mutex_trylock,
		(lockerf_t *)debug_pthread_mutex_lock,
		(lockerf_t *)debug_pthread_mutex_unlock
	);
}

/*

=item C<Locker *locker_create_debug_rwlock(pthread_rwlock_t *rwlock)>

Equivalent to I<locker_create_rwlock(3)> except that debug messages are
printed to standard output before and after each locking function is called
to help locate deadlocks. The debug messages look like:

    [thread id] funcname(rwlock address) ...
    [thread id] funcname(rwlock address) done

On success, returns the new I<Locker>. On error, returns C<null> with
C<errno> set appropriately.

=cut

*/

static int debug_pthread_rwlock_tryrdlock(pthread_rwlock_t *rwlock)
{
	return debug_invoke("pthread_rwlock_tryrdlock", (lockerf_t *)pthread_rwlock_tryrdlock, rwlock);
}

static int debug_pthread_rwlock_rdlock(pthread_rwlock_t *rwlock)
{
	return debug_invoke("pthread_rwlock_rdlock", (lockerf_t *)pthread_rwlock_rdlock, rwlock);
}

static int debug_pthread_rwlock_trywrlock(pthread_rwlock_t *rwlock)
{
	return debug_invoke("pthread_rwlock_trywrlock", (lockerf_t *)pthread_rwlock_trywrlock, rwlock);
}

static int debug_pthread_rwlock_wrlock(pthread_rwlock_t *rwlock)
{
	return debug_invoke("pthread_rwlock_wrlock", (lockerf_t *)pthread_rwlock_wrlock, rwlock);
}

static int debug_pthread_rwlock_unlock(pthread_rwlock_t *rwlock)
{
	return debug_invoke("pthread_rwlock_unlock", (lockerf_t *)pthread_rwlock_unlock, rwlock);
}

Locker *locker_create_debug_rwlock(pthread_rwlock_t *rwlock)
{
	return locker_create
	(
		rwlock,
		(lockerf_t *)debug_pthread_rwlock_tryrdlock,
		(lockerf_t *)debug_pthread_rwlock_rdlock,
		(lockerf_t *)debug_pthread_rwlock_trywrlock,
		(lockerf_t *)debug_pthread_rwlock_wrlock,
		(lockerf_t *)debug_pthread_rwlock_unlock
	);
}

#endif

/*

=item C<Locker *locker_create(void *lock, lockerf_t *tryrdlock, lockerf_t *rdlock, lockerf_t *trywrlock, lockerf_t *wrlock, lockerf_t *unlock)>

Creates a I<Locker> object that will operate on the synchronisation
variable, C<lock>. I<locker_tryrdlock(3)> will call C<tryrdlock>.
I<locker_rdlock(3)> will call C<rdlock>. I<locker_trywrlock(3)> will call
C<trywrlock>. I<locker_wrlock(3)> will call C<wrlock>. I<locker_unlock(3)>
will call C<unlock>. It is the caller's responsibility to initialise C<lock>
if necessary before use and to destroy C<lock> if necessary after use. None
of the arguments may be C<null>. On success, returns the new I<Locker>
object. On error, returns C<null> with C<errno> set appropriately.

=cut

*/

Locker *locker_create(void *lock, lockerf_t *tryrdlock, lockerf_t *rdlock, lockerf_t *trywrlock, lockerf_t *wrlock, lockerf_t *unlock)
{
	Locker *locker;

	if (!lock || !tryrdlock || !rdlock || !trywrlock || !wrlock || !unlock)
		return set_errnull(EINVAL);

	if (!(locker = mem_new(Locker))) /* XXX decouple */
		return NULL;

	locker->lock = lock;
	locker->tryrdlock = tryrdlock;
	locker->rdlock = rdlock;
	locker->trywrlock = trywrlock;
	locker->wrlock = wrlock;
	locker->unlock = unlock;

	return locker;
}

/*

=item C<void locker_release(Locker *locker)>

Releases (deallocates) C<locker>. It is the caller's responsibility to
destroy the synchronisation variable used by C<locker> if necessary.

=cut

*/

void locker_release(Locker *locker)
{
	if (!locker)
		return;

	mem_release(locker);
}

/*

=item C<void *locker_destroy(Locker **locker)>

Destroys (deallocates and sets to C<null>) C<*locker>. Returns C<null>. It
is the caller's responsibility to destroy the synchronisation variable used
by C<locker> if necessary.

=cut

*/

void *locker_destroy(Locker **locker)
{
	if (locker && *locker)
	{
		locker_release(*locker);
		*locker = NULL;
	}

	return NULL;
}

/*

=item C<int locker_tryrdlock(Locker *locker)>

Tries to claim a read lock on the synchronisation variable managed by
C<locker>. See I<pthread_mutex_trylock(3)> and
I<pthread_rwlock_tryrdlock(3)> for details. On success, returns C<0>. On
error, returns the error code from the underlying I<pthread> library
function.

=cut

*/

int (locker_tryrdlock)(Locker *locker)
{
	return locker ? locker->tryrdlock(locker->lock) : 0;
}

/*

=item C<int locker_rdlock(Locker *locker)>

Claims a read lock on the synchronisation variable managed by C<locker>. See
I<pthread_mutex_lock(3)> and I<pthread_rwlock_rdlock(3)> for details. On
success, returns C<0>. On error, returns the error code from the underlying
I<pthread> library function.

=cut

*/

int (locker_rdlock)(Locker *locker)
{
	return locker ? locker->rdlock(locker->lock) : 0;
}

/*

=item C<int locker_trywrlock(Locker *locker)>

Tries to claim a write lock on the synchronisation variable managed by
C<locker>. See I<pthread_mutex_trylock(3)> and
I<pthread_rwlock_trywrlock(3)> for details. On success, returns C<0>. On
error, returns the error code from the underlying I<pthread> library
function.

=cut

*/

int (locker_trywrlock)(Locker *locker)
{
	return locker ? locker->trywrlock(locker->lock) : 0;
}

/*

=item C<int locker_wrlock(Locker *locker)>

Claims a write lock on the synchronisation variable managed by C<locker>.
See I<pthread_mutex_lock(3)> and I<pthread_rwlock_wrlock(3)> for details. On
success, returns C<0>. On error, returns the error code from the underlying
I<pthread> library function.

=cut

*/

int (locker_wrlock)(Locker *locker)
{
	return locker ? locker->wrlock(locker->lock) : 0;
}

/*

=item C<int locker_unlock(Locker *locker)>

Unlocks the synchronisation variable managed by C<locker>. See
I<pthread_mutex_unlock(3)> and I<pthread_rwlock_unlock(3)> for details. On
success, returns C<0>. On error, returns the error code from the underlying
I<pthread> library function.

=cut

*/

int (locker_unlock)(Locker *locker)
{
	return locker ? locker->unlock(locker->lock) : 0;
}

#ifndef HAVE_PTHREAD_RWLOCK

/*

=item I<int pthread_rwlock_init(pthread_rwlock_t *rwlock, const pthread_rwlockattr_t *attr)>

Initialises the readers/writer lock, C<rwlock>, with the attributes in
C<attr>. If C<attr> is C<null>, C<rwlock> is initialised as a process
private lock. Note that this is the only option under Linux. On success,
returns C<0>. On error, returns an error code.

=cut

*/

int pthread_rwlock_init(pthread_rwlock_t *rwlock, const pthread_rwlockattr_t *attr)
{
#define DMA pthread_mutexattr_destroy(&mutexattr)
#define DCA pthread_condattr_destroy(&condattr)
#define DL pthread_mutex_destroy(&rwlock->lock)
#define DR pthread_cond_destroy(&rwlock->readers)
#define DW pthread_cond_destroy(&rwlock->writers)

	pthread_mutexattr_t mutexattr;
	pthread_condattr_t condattr;
	int pshared;

	if (!rwlock)
		return EINVAL;

	try(pthread_mutexattr_init(&mutexattr))
	try_catch(pthread_condattr_init(&condattr), DMA)

	if (attr)
		try_catch(pthread_rwlockattr_getpshared(attr, &pshared), (DCA, DMA))
	else
		pshared = PTHREAD_PROCESS_PRIVATE;

	try_catch(pthread_mutexattr_setpshared(&mutexattr, pshared), (DCA, DMA))
	try_catch(pthread_condattr_setpshared(&condattr, pshared), (DCA, DMA))

	try_catch(pthread_mutex_init(&rwlock->lock, &mutexattr), (DCA, DMA))
	try_catch(pthread_cond_init(&rwlock->readers, &condattr), (DL, DCA, DMA))
	try_catch(pthread_cond_init(&rwlock->writers, &condattr), (DR, DL, DCA, DMA))

	rwlock->waiters = 0;
	rwlock->state = 0;

	return 0;

#undef DMA
#undef DCA
#undef DL
#undef DR
#undef DW
}

/*

=item I<int pthread_rwlock_destroy(pthread_rwlock_t *rwlock)>

Destroys the readers/writer lock, C<rwlock>, that was initialised by
I<pthread_rwlock_init(3)>. It is the caller's responsibility to deallocate
the memory pointed to by C<rwlock> if necessary. On success, returns C<0>.
On error, returns an error code.

=cut

*/

int pthread_rwlock_destroy(pthread_rwlock_t *rwlock)
{
	if (!rwlock)
		return EINVAL;

	pthread_mutex_destroy(&rwlock->lock);
	pthread_cond_destroy(&rwlock->readers);
	pthread_cond_destroy(&rwlock->writers);

	return 0;
}

/*

=item I<int pthread_rwlock_rdlock(pthread_rwlock_t *rwlock)>

Claims a read lock on C<rwlock>. Multiple threads may hold a read lock at
the same time. On success, returns C<0>. On error, returns an error code.

=cut

*/

static void rdlock_cleanup(void *arg)
{
	pthread_rwlock_t *rwlock = (pthread_rwlock_t *)arg;

	pthread_mutex_unlock(&rwlock->lock);
}

int pthread_rwlock_rdlock(pthread_rwlock_t *rwlock)
{
	if (!rwlock)
		return EINVAL;

	try(pthread_mutex_lock(&rwlock->lock))
	pthread_cleanup_push(rdlock_cleanup, rwlock);

	/* Wait until there are no active or queued writers */

	while (rwlock->state == -1 || rwlock->waiters)
		try(pthread_cond_wait(&rwlock->readers, &rwlock->lock))

	++rwlock->state;
	pthread_cleanup_pop(1);

	return 0;
}

/*

=item I<int pthread_rwlock_tryrdlock(pthread_rwlock_t *rwlock)>

Attempts to claim a read lock on C<rwlock>. On success, returns C<0>. On
error, returns an error code. If C<rwlock> is already locked, returns
C<EBUSY>.

=cut

*/

int pthread_rwlock_tryrdlock(pthread_rwlock_t *rwlock)
{
	if (!rwlock)
		return EINVAL;

	/* Are there no active or waiting writers? */

	try(pthread_mutex_lock(&rwlock->lock))

	if (rwlock->state != -1 && !rwlock->waiters)
	{
		++rwlock->state;
		try(pthread_mutex_unlock(&rwlock->lock))

		return 0;
	}

	try(pthread_mutex_unlock(&rwlock->lock))

	return EBUSY;
}

/*

=item I<int pthread_rwlock_wrlock(pthread_rwlock_t *rwlock)>

Claims a write lock on C<rwlock>. Only a single thread may hold a write lock
at any point in time. On success, returns C<0>. On error, returns an error
code.

=cut

*/

static void wrlock_cleanup(void *arg)
{
	pthread_rwlock_t *rwlock = (pthread_rwlock_t *)arg;

	/*
	** Was the only queued writer and lock is available for readers.
	** Called through cancellation clean-up so lock is held at entry.
	*/

	if (--rwlock->waiters == 0 && rwlock->state != -1)
		pthread_cond_broadcast(&rwlock->readers);

	pthread_mutex_unlock(&rwlock->lock);
}

int pthread_rwlock_wrlock(pthread_rwlock_t *rwlock)
{
	if (!rwlock)
		return EINVAL;

	try(pthread_mutex_lock(&rwlock->lock))

	/* Queue this writer */

	++rwlock->waiters;
	pthread_cleanup_push(wrlock_cleanup, rwlock);

	/* Wait until readers have finished */

	while (rwlock->state != 0)
		try(pthread_cond_wait(&rwlock->writers, &rwlock->lock))

	rwlock->state = -1;
	pthread_cleanup_pop(1);

	return 0;
}

/*

=item I<int pthread_rwlock_trywrlock(pthread_rwlock_t *rwlock)>

Attempts to claim a write lock on C<rwlock>. On success, returns C<0>. On
error, returns an error code. If C<rwlock> is already locked, returns
C<EBUSY>.

=cut

*/

int pthread_rwlock_trywrlock(pthread_rwlock_t *rwlock)
{
	if (!rwlock)
		return EINVAL;

	try(pthread_mutex_lock(&rwlock->lock))

	/* Are there no readers and no active or queued writers? */

	if (rwlock->state == 0 && rwlock->waiters == 0)
	{
		rwlock->state = -1;
		try(pthread_mutex_unlock(&rwlock->lock))

		return 0;
	}

	try(pthread_mutex_unlock(&rwlock->lock))

	return EBUSY;
}

/*

=item I<int pthread_rwlock_unlock(pthread_rwlock_t *rwlock)>

Unlocks C<rwlock>. On success, returns C<0>. On error, returns an error
code.

=cut

*/

int pthread_rwlock_unlock(pthread_rwlock_t *rwlock)
{
	if (!rwlock)
		return EINVAL;

	/* Writer releasing lock */

	if (rwlock->state == -1)
	{
		/* Mark as available */

		rwlock->state = 0;

		/* Signal queued writers if any, or broadcast to readers */

		if (rwlock->waiters)
			try(pthread_cond_signal(&rwlock->writers))
		else
			try(pthread_cond_broadcast(&rwlock->readers))
	}
	else
	{
		/* Reader releasing lock */

		if (--rwlock->state == 0)
			try(pthread_cond_signal(&rwlock->writers))
	}

	try(pthread_mutex_unlock(&rwlock->lock))

	return 0;
}

/*

=item I<int pthread_rwlockattr_init(pthread_rwlockattr_t *attr)>

Initialises the readers/writer lock attribute object, C<attr>. On success,
returns C<0>. On error, returns an error code.

=cut

*/

int pthread_rwlockattr_init(pthread_rwlockattr_t *attr)
{
	if (!attr)
		return EINVAL;

	attr->pshared = PTHREAD_PROCESS_PRIVATE;

	return 0;
}

/*

=item I<int pthread_rwlockattr_destroy(pthread_rwlockattr_t *attr)>

Destroys the readers/writer lock attribute object, C<rwlock>, that was
initialised by I<pthread_rwlockattr_init(3)>. Note that the memory pointed
to by C<attr> may also need to be deallocated separately. On success,
returns C<0>. On error, returns an error code.

=cut

*/

int pthread_rwlockattr_destroy(pthread_rwlockattr_t *attr)
{
	if (!attr)
		return EINVAL;

	return 0;
}

/*

=item I<int pthread_rwlockattr_getpshared(const pthread_rwlockattr_t *attr, int *pshared)>

Stores the C<pshared> attribute of C<attr> into C<*pshared>. On success,
returns C<0>. On error, returns an error code.

=cut

*/

int pthread_rwlockattr_getpshared(const pthread_rwlockattr_t *attr, int *pshared)
{
	if (!attr || !pshared)
		return EINVAL;

	*pshared = attr->pshared;

	return 0;
}

/*

=item I<int pthread_rwlockattr_setpshared(pthread_rwlockattr_t *attr, int pshared)>

Sets the C<pshared> attribute of C<attr> to C<pshared>. On success, returns
C<0>. On error, returns an error code. Note that under Linux, C<pshared>
must be C<PTHREAD_PROCESS_PRIVATE> or I<pthread_rwlock_init(3)> will
subsequently fail.

=cut

*/

int pthread_rwlockattr_setpshared(pthread_rwlockattr_t *attr, int pshared)
{
	if (!attr)
		return EINVAL;

	if (pshared != PTHREAD_PROCESS_PRIVATE && pshared != PTHREAD_PROCESS_SHARED)
		return EINVAL;

	attr->pshared = pshared;

	return 0;
}

#endif

/*

=back

=head1 ERRORS

On error, C<errno> is set either by an underlying function, or as follows:

=over 4

=item C<EINVAL>

Arguments are C<null> or invalid.

=back

=head1 MT-Level

MT-Safe

=head1 EXAMPLES

A mutex example:

    #include <slack/std.h>
    #include <slack/locker.h>

    int main(int ac, char **av)
    {
        pthread_mutex_t mutex[1];
        Locker *locker;

        errno = pthread_mutex_init(mutex, NULL);
        locker = locker_create_mutex(mutex);

        errno = locker_rdlock(locker);
        errno = locker_unlock(locker);

        errno = locker_wrlock(locker);
        errno = locker_unlock(locker);

        locker_destroy(&locker);
        pthread_mutex_destroy(mutex);

        return EXIT_SUCCESS;
    }

A rwlock example:

    #include <slack/std.h>
    #include <slack/locker.h>

    int main(int ac, char **av)
    {
        pthread_rwlock_t rwlock[1];
        Locker *locker;

        errno = pthread_rwlock_init(rwlock, NULL);
        locker = locker_create_rwlock(rwlock);

        errno = locker_rdlock(locker);
        errno = locker_unlock(locker);

        errno = locker_wrlock(locker);
        errno = locker_unlock(locker);

        locker_destroy(&locker);
        pthread_rwlock_destroy(rwlock);

        return EXIT_SUCCESS;
    }

A debug mutex example:

    #include <slack/std.h>
    #include <slack/locker.h>

    int main(int ac, char **av)
    {
        pthread_mutex_t mutex[1];
        Locker *locker;

        errno = pthread_mutex_init(mutex, NULL);
        locker = locker_create_debug_mutex(mutex);

        errno = locker_rdlock(locker);
        errno = locker_unlock(locker);

        errno = locker_wrlock(locker);
        errno = locker_unlock(locker);

        locker_destroy(&locker);
        pthread_mutex_destroy(mutex);

        return EXIT_SUCCESS;
    }

A debug rwlock example:

    #include <slack/std.h>
    #include <slack/locker.h>

    int main(int ac, char **av)
    {
        pthread_rwlock_t rwlock[1];
        Locker *locker;

        errno = pthread_rwlock_init(rwlock, NULL);
        locker = locker_create_debug_rwlock(rwlock);

        errno = locker_rdlock(locker);
        errno = locker_unlock(locker);

        errno = locker_wrlock(locker);
        errno = locker_unlock(locker);

        locker_destroy(&locker);
        pthread_rwlock_destroy(rwlock);

        return EXIT_SUCCESS;
    }

A non-locking example:

    #include <slack/std.h>
    #include <slack/locker.h>

    int main(int ac, char **av)
    {
        Locker *locker = NULL;

        errno = locker_rdlock(locker);
        errno = locker_unlock(locker);

        errno = locker_wrlock(locker);
        errno = locker_unlock(locker);

        return EXIT_SUCCESS;
    }

=head1 SEE ALSO

I<libslack(3)>,
C<http://raf.org/papers/mt-disciplined.html>

=head1 AUTHOR

20201111 raf <raf@raf.org>

=cut

*/

#endif

#ifdef TEST

#include <time.h>

#include <slack/list.h>

struct List                  /* identical to list.c */
{
	size_t size;             /* number of item slots allocated */
	size_t length;           /* number of items used */
	void **list;             /* vector of items (void *) */
	list_release_t *destroy; /* item destructor, if any */
	Lister *lister;          /* built-in iterator */
	Locker *locker;          /* locking strategy for this object */
};

/* Unsafe */

#define list_length_test_nolock_macro(list) ((list) ? (list)->length : -1)

ssize_t list_length_test_nolock_func(const List *list)
{
	if (!list)
		return -1;

	return list->length;
}

/* MT-Safe */

ssize_t list_length_test_direct_mutex(const List *list, pthread_mutex_t *mutex)
{
	size_t length;

	if (!list)
		return -1;

	if (pthread_mutex_lock(mutex))
		return -1;

	length = list->length;

	if (pthread_mutex_unlock(mutex))
		return -1;

	return length;
}

ssize_t list_length_test_direct_rwlock(const List *list, pthread_rwlock_t *rwlock)
{
	size_t length;

	if (!list)
		return -1;

	if (pthread_rwlock_rdlock(rwlock))
		return -1;

	length = list->length;

	if (pthread_rwlock_unlock(rwlock))
		return -1;

	return length;
}

/* MT-Disciplined */

ssize_t list_length_test_lock_funcptr(const List *list, lockerf_t *lockf, lockerf_t *unlockf, void *lock)
{
	size_t length;

	if (!list)
		return -1;

	if (lock && lockf(lock))
		return -1;

	length = list->length;

	if (lock && unlockf(lock))
		return -1;

	return length;
}

ssize_t list_length_test_lock_funcptr_1test(const List *list, lockerf_t *lockf, lockerf_t *unlockf, void *lock)
{
	size_t length;

	if (!list)
		return -1;

	if (!lock)
		return list->length;

	if (lockf(lock))
		return -1;

	length = list->length;

	if (unlockf(lock))
		return -1;

	return length;
}

ssize_t list_length_test_separate_locker(const List *list, Locker *locker)
{
	size_t length;

	if (!list)
		return -1;

	if (locker_rdlock(locker))
		return -1;

	length = list->length;

	if (locker_unlock(locker))
		return -1;

	return length;
}

ssize_t list_length_test_separate_locker_1test(const List *list, Locker *locker)
{
	size_t length;

	if (!list)
		return -1;

	if (!locker)
		return list->length;

	if (locker->rdlock(locker->lock))
		return -1;

	length = list->length;

	if (locker->unlock(locker->lock))
		return -1;

	return length;
}

ssize_t list_length_test_builtin_locker(const List *list)
{
	size_t length;

	if (!list)
		return -1;

	if (locker_rdlock(list->locker))
		return -1;

	length = list->length;

	if (locker_unlock(list->locker))
		return -1;

	return length;
}

ssize_t list_length_test_builtin_locker_cacheaddr(const List *list)
{
	size_t length;
	Locker *locker;

	if (!list)
		return -1;

	locker = list->locker;

	if (locker_rdlock(locker))
		return -1;

	length = list->length;

	if (locker_unlock(locker))
		return -1;

	return length;
}

ssize_t list_length_test_builtin_locker_1test(const List *list)
{
	size_t length;

	if (!list)
		return -1;

	if (!list->locker)
		return list->length;

	if (list->locker->rdlock(list->locker->lock))
		return -1;

	length = list->length;

	if (list->locker->unlock(list->locker->lock))
		return -1;

	return length;
}

ssize_t list_length_test_builtin_locker_1test_cacheaddr(const List *list)
{
	size_t length;
	Locker *locker;
	lockerf_t *rdlock;
	lockerf_t *unlock;
	void *lock;

	if (!list)
		return -1;

	if (!list->locker)
		return list->length;

	locker = list->locker;
	rdlock = locker->rdlock;
	unlock = locker->unlock;
	lock = locker->lock;

	if (rdlock(lock))
		return -1;

	length = list->length;

	if (unlock(lock))
		return -1;

	return length;
}

int main(int ac, char **av)
{
	int errors = 0;
	int debug;
	Locker *locker;
	pthread_mutex_t mutex[1];
	pthread_rwlock_t rwlock[1];

	if (ac == 2 && !strcmp(av[1], "help"))
	{
		printf("usage: %s [debug]\n", *av);
		return EXIT_SUCCESS;
	}

	printf("Testing: %s\n", "locker");

	/*
	** Note: Debug lockers are not tested by default. Invoke the test with
	** av[1] == "debug" to use debug lockers instead of ordinary lockers.
	** Local implementation of rwlocks are not tested unless your system
	** requires them.
	*/

	debug = (av[1] && !strcmp(av[1], "debug"));

	if (debug)
		setbuf(stdout, NULL);

	/* Test mutex lockers */

	if ((errno = pthread_mutex_init(mutex, NULL)))
		++errors, printf("Test1: failed to perform test: pthread_mutex_init() failed: err = %d\n", errno);
	else if (!(locker = (debug ? locker_create_debug_mutex : locker_create_mutex)(mutex)))
		++errors, printf("Test1: %s() failed (%s)\n", debug ? "locker_create_debug_mutex" : "locker_create_mutex", strerror(errno));
	else
	{
		if ((errno = locker_tryrdlock(locker)))
			++errors, printf("Test2: locker_tryrdlock(mutex_locker) failed (%s)\n", strerror(errno));
		else if ((errno = locker_unlock(locker)))
			++errors, printf("Test3: locker_unlock(mutex_locker) failed (%s)\n", strerror(errno));

		if ((errno = locker_rdlock(locker)))
			++errors, printf("Test4: locker_rdlock(mutex_locker) failed (%s)\n", strerror(errno));
		else if ((errno = locker_unlock(locker)))
			++errors, printf("Test5: locker_unlock(mutex_locker) failed (%s)\n", strerror(errno));

		if ((errno = locker_trywrlock(locker)))
			++errors, printf("Test6: locker_trywrlock(mutex_locker) failed (%s)\n", strerror(errno));
		else if ((errno = locker_unlock(locker)))
			++errors, printf("Test7: locker_unlock(mutex_locker) failed (%s)\n", strerror(errno));

		if ((errno = locker_wrlock(locker)))
			++errors, printf("Test8: locker_wrlock(mutex_locker) failed (%s)\n", strerror(errno));
		else if ((errno = locker_unlock(locker)))
			++errors, printf("Test9: locker_unlock(mutex_locker) failed (%s)\n", strerror(errno));

		locker_destroy(&locker);
		if (locker)
			++errors, printf("Test10: locker_destroy(mutex_locker) failed (%s)\n", strerror(errno));
	}

	pthread_mutex_destroy(mutex);

	/* Test rwlock lockers */

	if ((errno = pthread_rwlock_init(rwlock, NULL)))
		++errors, printf("Test11: failed to perform test: pthread_rwlock_init() failed: err = %d\n", errno);
	else if (!(locker = (debug ? locker_create_debug_rwlock : locker_create_rwlock)(rwlock)))
		++errors, printf("Test11: %s() failed (%s)\n", debug ? "locker_create_debug_rwlock" : "locker_create_rwlock", strerror(errno));
	else
	{
		if ((errno = locker_tryrdlock(locker)))
			++errors, printf("Test12: locker_tryrdlock(rwlock_locker) failed (%s)\n", strerror(errno));
		else if ((errno = locker_unlock(locker)))
			++errors, printf("Test13: locker_unlock(rwlock_locker) failed (%s)\n", strerror(errno));

		if ((errno = locker_rdlock(locker)))
			++errors, printf("Test14: locker_rdlock(rwlock_locker) failed (%s)\n", strerror(errno));
		else if ((errno = locker_unlock(locker)))
			++errors, printf("Test15: locker_unlock(rwlock_locker) failed (%s)\n", strerror(errno));

		if ((errno = locker_trywrlock(locker)))
			++errors, printf("Test16: locker_trywrlock(rwlock_locker) failed (%s)\n", strerror(errno));
		else if ((errno = locker_unlock(locker)))
			++errors, printf("Test17: locker_unlock(rwlock_locker) failed (%s)\n", strerror(errno));

		if ((errno = locker_wrlock(locker)))
			++errors, printf("Test18: locker_wrlock(rwlock_locker) failed (%s)\n", strerror(errno));
		else if ((errno = locker_unlock(locker)))
			++errors, printf("Test19: locker_unlock(rwlock_locker) failed (%s)\n", strerror(errno));

		locker_destroy(&locker);
		if (locker)
			++errors, printf("Test20: locker_destroy(rwlock_locker) failed (%s)\n", strerror(errno));
	}

	pthread_rwlock_destroy(rwlock);

	/* Timing tests */

	if (av[1] && !strcmp(av[1], "time"))
	{
		List *list, *mutex_list, *rwlock_list;
		clock_t start, end;
		size_t i, length;
		pthread_mutex_t mutex;
		Locker *mutex_locker;
		pthread_rwlock_t rwlock;
		Locker *rwlock_locker;
		double nm, nf;
		double dm, dr;
		double np, mp, rp, np1, mp1, rp1;
		double nsl, msl, rsl, nsl1, msl1, rsl1;
		double nbl, mbl, rbl, nblc, mblc, rblc;
		double nbl1, mbl1, rbl1, nbl1c, mbl1c, rbl1c;

		printf("Timing: struct attribute accesses with differing MT safety\n");

		list = list_make(NULL, "...", NULL);

		pthread_mutex_init(&mutex, NULL);
		mutex_locker = locker_create_mutex(&mutex);
		mutex_list = list_create_with_locker(mutex_locker, NULL);

		pthread_rwlock_init(&rwlock, NULL);
		rwlock_locker = locker_create_rwlock(&rwlock);
		rwlock_list = list_create_with_locker(rwlock_locker, NULL);

#define ITERATIONS 10000000
#define TIME_TEST(label, base, basetime, timevar, action) \
	start = clock(); \
	for (i = 0; i < ITERATIONS; ++i) \
		action; \
	end = clock(); \
	timevar = ((double)(end - start) / (double)CLOCKS_PER_SEC) / ITERATIONS * 1000000000; \
	printf("  %-39s %g ns", (label), (timevar)); \
	if (!base) \
	{ \
		printf(" (overhead = %g ns", (timevar) - (basetime)); \
		if ((basetime) != 0.0) \
			printf(" = %g%%", (((timevar) / (basetime)) - 1.0) * 100); \
		printf(")"); \
	} \
	printf("\n");

		printf(" Unsafe:\n");

		TIME_TEST("nolock/macro", 1, 0.0, nm, length = list_length_test_nolock_macro(list))
		TIME_TEST("nolock/func", 1, 0.0, nf, length = list_length_test_nolock_func(list))
		printf("\n");

		printf(" MT-Safe:\n");

		TIME_TEST("direct mutex", 1, 0.0, dm, length = list_length_test_direct_mutex(list, &mutex))
		TIME_TEST("direct rwlock", 1, 0.0, dr, length = list_length_test_direct_rwlock(list, &rwlock))
		printf("\n");

		printf(" MT-Disciplined:\n");

		TIME_TEST("null pointers", 0, nf, np, length = list_length_test_lock_funcptr(list, NULL, NULL, NULL))
		TIME_TEST("null pointers (1test)", 0, nf, np1, length = list_length_test_lock_funcptr_1test(list, NULL, NULL, NULL))
		TIME_TEST("null separate locker", 0, nf, nsl, length = list_length_test_separate_locker(list, NULL))
		TIME_TEST("null separate locker (1test)", 0, nf, nsl1, length = list_length_test_separate_locker_1test(list, NULL))
		TIME_TEST("null builtin locker", 0, nf, nbl, length = list_length_test_builtin_locker(list))
		TIME_TEST("null builtin locker (1test)", 0, nf, nbl1, length = list_length_test_builtin_locker_1test(list))
		TIME_TEST("null builtin locker (cacheaddr)", 0, nf, nblc, length = list_length_test_builtin_locker_cacheaddr(list))
		TIME_TEST("null builtin locker (1test/cacheaddr)", 0, nf, nbl1c, length = list_length_test_builtin_locker_1test_cacheaddr(list))
		printf("\n");

		TIME_TEST("mutex pointers", 0, dm, mp, length = list_length_test_lock_funcptr(mutex_list, (lockerf_t *)pthread_mutex_lock, (lockerf_t *)pthread_mutex_unlock, &mutex))
		TIME_TEST("mutex pointers (1test)", 0, dm, mp1, length = list_length_test_lock_funcptr_1test(mutex_list, (lockerf_t *)pthread_mutex_lock, (lockerf_t *)pthread_mutex_unlock, &mutex))
		TIME_TEST("mutex separate locker", 0, dm, msl, length = list_length_test_separate_locker(mutex_list, mutex_locker))
		TIME_TEST("mutex separate locker (1test)", 0, dm, msl1, length = list_length_test_separate_locker_1test(mutex_list, mutex_locker))
		TIME_TEST("mutex builtin locker", 0, dm, mbl, length = list_length_test_builtin_locker(mutex_list))
		TIME_TEST("mutex builtin locker (1test)", 0, dm, mbl1, length = list_length_test_builtin_locker_1test(mutex_list))
		TIME_TEST("mutex builtin locker (cacheaddr)", 0, dm, mblc, length = list_length_test_builtin_locker_cacheaddr(mutex_list))
		TIME_TEST("mutex builtin locker (1test/cacheaddr)", 0, dm, mbl1c, length = list_length_test_builtin_locker_1test_cacheaddr(mutex_list))
		printf("\n");

		TIME_TEST("rwlock pointers", 0, dr, rp, length = list_length_test_lock_funcptr(rwlock_list, (lockerf_t *)pthread_rwlock_rdlock, (lockerf_t *)pthread_rwlock_unlock, &rwlock))
		TIME_TEST("rwlock pointers (1test)", 0, dr, rp1, length = list_length_test_lock_funcptr_1test(rwlock_list, (lockerf_t *)pthread_rwlock_rdlock, (lockerf_t *)pthread_rwlock_unlock, &rwlock))
		TIME_TEST("rwlock separate locker", 0, dr, rsl, length = list_length_test_separate_locker(rwlock_list, rwlock_locker))
		TIME_TEST("rwlock separate locker (1test)", 0, dr, rsl1, length = list_length_test_separate_locker_1test(rwlock_list, rwlock_locker))
		TIME_TEST("rwlock builtin locker", 0, dr, rbl, length = list_length_test_builtin_locker(rwlock_list))
		TIME_TEST("rwlock builtin locker (1test)", 0, dr, rbl1, length = list_length_test_builtin_locker_1test(rwlock_list))
		TIME_TEST("rwlock builtin locker (cacheaddr)", 0, dr, rblc, length = list_length_test_builtin_locker_cacheaddr(rwlock_list))
		TIME_TEST("rwlock builtin locker (1test/cacheaddr)", 0, dr, rbl1c, length = list_length_test_builtin_locker_1test_cacheaddr(rwlock_list))
		printf("\n");

		/* Suppress compiler warning */
		if (length)
			length = 0;

		list_release(list);
		list_release(mutex_list);
		locker_release(mutex_locker);
		pthread_mutex_destroy(&mutex);
		list_release(rwlock_list);
		locker_release(rwlock_locker);
		pthread_rwlock_destroy(&rwlock);
	}

	if (errors)
		printf("%d/20 tests failed\n", errors);
	else
		printf("All tests passed\n");

	return (errors == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

#endif

/* vi:set ts=4 sw=4: */
