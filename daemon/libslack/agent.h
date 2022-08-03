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

#ifndef LIBSLACK_AGENT_H
#define LIBSLACK_AGENT_H

#include <slack/hdr.h>
#include <slack/locker.h>

typedef struct Agent Agent;
typedef int agent_action_t(Agent *agent, void *arg);
typedef int agent_reaction_t(Agent *agent, int fd, int revents, void *arg);

_begin_decls
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
_end_decls

#endif

/* vi:set ts=4 sw=4: */
