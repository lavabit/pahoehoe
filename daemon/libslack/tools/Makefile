#
# libslack - http://libslack.org/
#
# Copyright (C) 1999-2002, 2004, 2010, 2020-2021 raf <raf@raf.org>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, see <https://www.gnu.org/licenses/>.
#

# 20210220 raf <raf@raf.org>

prefix := /usr/local

help:
	@echo "This makefile provides the following targets"; \
	echo; \
	echo " help                           -- shows this list of targets"; \
	echo " install-analyse-debug-locker   -- installs analyse-debug-locker"; \
	echo " install-prefix                 -- installs prefix"; \
	echo " install-prefix-h               -- installs prefix.h"; \
	echo " install-remove-prefix-h        -- installs remove_prefix.h"; \
	echo " uninstall-analyse-debug-locker -- uninstalls analyse-debug-locker"; \
	echo " uninstall-prefix               -- uninstalls prefix"; \
	echo " uninstall-prefix-h             -- uninstalls prefix.h"; \
	echo " uninstall-remove-prefix-h      -- uninstalls remove_prefix.h"; \
	echo; \
	echo "Note: There should be no reason to install prefix"; \
	echo "      except perhaps to install its manpage."; \
	echo

analyse-debug-locker.1: analyse-debug-locker
	pod2man --center='User Commands' --section=1 $< > $@

prefix.1: prefix
	pod2man --center='User Commands' --section=1 $< > $@

install-analyse-debug-locker: analyse-debug-locker.1
	install -m 755 analyse-debug-locker $(prefix)/bin
	install -m 644 analyse-debug-locker.1 $(prefix)/man/man1

install-prefix: prefix.1
	install -m 755 prefix $(prefix)/bin
	install -m 644 prefix.1 $(prefix)/man/man1

install-prefix-h:
	install -m 644 prefix.h $(prefix)/include/slack

install-remove-prefix-h:
	install -m 644 remove_prefix.h $(prefix)/include/slack

uninstall-analyse-debug-locker:
	rm -f $(prefix)/bin/analyse-debug-locker
	rm -f $(prefix)/man/man1/analyse-debug-locker.1

uninstall-prefix:
	rm -f $(prefix)/bin/prefix
	rm -f $(prefix)/man/man1/prefix.1

uninstall-prefix-h:
	rm -f $(prefix)/include/slack/prefix.h

uninstall-remove-prefix-h:
	rm -f $(prefix)/include/slack/remove_prefix.h

# vi:set ts=4 sw=4:
