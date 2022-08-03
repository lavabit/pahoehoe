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

#ifndef LIBSLACK_LOCKER_H
#define LIBSLACK_LOCKER_H

#include <pthread.h>

#include <slack/hdr.h>

typedef struct Locker Locker;
typedef int lockerf_t(void *lock);

#ifndef HAVE_PTHREAD_RWLOCK

typedef struct pthread_rwlock_t pthread_rwlock_t;
typedef struct pthread_rwlockattr_t pthread_rwlockattr_t;

struct pthread_rwlock_t
{
	pthread_mutex_t lock;   /* Lock for structure */
	pthread_cond_t readers; /* Are there readers waiting? */
	pthread_cond_t writers; /* Are there writers waiting? */
	int waiters;            /* Number of writers waiting */
	int state;              /* State: -1 -> writer, 0 -> idle, +ve -> readers */
};

struct pthread_rwlockattr_t
{
	int pshared;            /* Shared between processes or not */
};

#define PTHREAD_RWLOCK_INITIALIZER \
	{ \
		PTHREAD_MUTEX_INITIALIZER, \
		PTHREAD_COND_INITIALIZER, \
		PTHREAD_COND_INITIALIZER, \
		0, 0 \
	}

#endif

_begin_decls
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
#ifndef HAVE_PTHREAD_RWLOCK
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
#endif
_end_decls

/* Don't look below here - optimisations only */

struct Locker
{
	void *lock;
	lockerf_t *tryrdlock;
	lockerf_t *rdlock;
	lockerf_t *trywrlock;
	lockerf_t *wrlock;
	lockerf_t *unlock;
};

#define locker_tryrdlock(locker) ((locker) ? (locker)->tryrdlock((locker)->lock) : 0)
#define locker_rdlock(locker)    ((locker) ? (locker)->rdlock((locker)->lock) : 0)
#define locker_trywrlock(locker) ((locker) ? (locker)->trywrlock((locker)->lock) : 0)
#define locker_wrlock(locker)    ((locker) ? (locker)->wrlock((locker)->lock) : 0)
#define locker_unlock(locker)    ((locker) ? (locker)->unlock((locker)->lock) : 0)

#endif

/* vi:set ts=4 sw=4: */
