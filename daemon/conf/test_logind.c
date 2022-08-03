/*
*
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
*/

/*
* Test program for logind/elogind (Linux only). This program can't
* be run. It is only to be compiled so as to make sure that we have
* all of these functions and types available. On Linux systems
* with systemd, it needs to be linkd again
*
* 20210304 raf <raf@raf.org>
*/

#include <stdlib.h>
#include <systemd/sd-login.h>

int main()
{
	sd_login_monitor *lg_mon = NULL;
	int lg_mon_events = 0;
	int lg_mon_fd;
	int lg_uid = 1000;
	int ret;
	uint64_t t;

	ret = sd_login_monitor_new("uid", &lg_mon);
	lg_mon_fd = sd_login_monitor_get_fd(lg_mon);
	sd_login_monitor_get_timeout(lg_mon, &t);
	lg_mon_events = sd_login_monitor_get_events(lg_mon);
	ret = sd_uid_get_sessions(lg_uid, 0, NULL);
	sd_login_monitor_flush(lg_mon);
	sd_login_monitor_unref(lg_mon);
}

/* vi:set ts=4 sw=4: */
