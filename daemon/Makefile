#
# daemon - http://libslack.org/daemon/
#
# Copyright (C) 1999-2004, 2010, 2020-2021 raf <raf@raf.org>
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

# 20210304 raf <raf@raf.org>

CC := gcc
# CC := cc
# CC := other
AR := ar
RANLIB := ranlib
POD2MAN := pod2man
POD2HTML := pod2html
GZIP := gzip -f -9

DESTDIR :=
PREFIX := /usr/local
# PREFIX := /usr/pkg
APP_INSDIR := $(PREFIX)/bin
LIB_INSDIR := $(PREFIX)/lib
MAN_SYSDIR := $(PREFIX)/share/man
MAN_LOCDIR := $(PREFIX)/share/man
ifeq ($(PREFIX),/usr)
MAN_INSDIR := $(MAN_SYSDIR)
else
MAN_INSDIR := $(MAN_LOCDIR)
endif
HDR_INSDIR := $(PREFIX)/include
DATA_INSDIR := $(PREFIX)/share
CONF_INSDIR := /etc
APP_MANSECT := 1
LIB_MANSECT := 3
FMT_MANSECT := 5
APP_MANDIR := $(MAN_INSDIR)/man$(APP_MANSECT)
LIB_MANDIR := $(MAN_INSDIR)/man$(LIB_MANSECT)
FMT_MANDIR := $(MAN_INSDIR)/man$(FMT_MANSECT)
APP_MANSECTNAME := User Commands
LIB_MANSECTNAME := C Library Functions - libslack
FMT_MANSECTNAME := File Formats
MAN_GZIP := 1

CCFLAGS += -O3
CCFLAGS += -Wall -pedantic

# CCFLAGS += -xO4

CLEAN_FILES += tags core Makefile.bak .makefile.bak pod2htm*

DAEMON_SRCDIR := .
DAEMON_INCDIRS := libslack
# Uncomment this for Linux systems without systemd but with elogind (e.g. Slackware)
# DAEMON_INCDIRS += /usr/include/elogind
DAEMON_LIBDIRS := libslack
include $(DAEMON_SRCDIR)/macros.mk

.PHONY: all ready test check man html install uninstall dist rpm deb sol obsd fbsd nbsd osx

all: ready $(ALL_TARGETS)
ready: $(READY_TARGETS)
check test: all $(TEST_TARGETS)
man: $(MAN_TARGETS)
html: $(HTML_TARGETS)
install: all $(INSTALL_TARGETS)
uninstall: $(UNINSTALL_TARGETS)
dist: $(DIST_TARGETS)
rpm: $(RPM_TARGETS)
deb: $(DEB_TARGETS)
sol: $(SOL_TARGETS)
obsd: $(OBSD_TARGETS)
fbsd: $(FBSD_TARGETS)
nbsd: $(NBSD_TARGETS)
osx: $(OSX_TARGETS)

.PHONY: help help-macros depend clean clobber distclean

help::
	@echo "This makefile provides the following targets."; \
	echo; \
	echo " help                 -- shows this list of targets"; \
	echo " help-macros          -- shows the values of all make macros"; \
	echo " all                  -- makes $(DAEMON_TARGET) and $(DAEMON_SUBTARGETS) (default)"; \
	echo " ready                -- prepares the source directory for make"; \
	echo " test                 -- makes and runs library unit tests"; \
	echo " check                -- same as test"; \
	echo " man                  -- generates all manpages"; \
	echo " html                 -- generates all manpages in html"; \
	echo " install              -- installs everything under $(PREFIX)"; \
	echo " uninstall            -- uninstalls everything from $(PREFIX)"; \
	echo " depend               -- generates source dependencies using makedepend"; \
	echo " tags                 -- generates a tags file using ctags"; \
	echo " clean                -- removes object files, tags, core and Makefile.bak"; \
	echo " clobber              -- clean + rm $(DAEMON_TARGET), $(DAEMON_SUBTARGETS) and tests"; \
	echo " distclean            -- clobber + rm source dependencies"; \
	echo " dist                 -- makes the distribution: ../$(DAEMON_DIST)"; \
	echo " rpm                  -- makes source and binary rpm packages"; \
	echo " deb                  -- makes binary debian package"; \
	echo " sol                  -- makes binary solaris package"; \
	echo " obsd                 -- makes binary openbsd package"; \
	echo " fbsd                 -- makes binary freebsd package"; \
	echo " nbsd                 -- makes binary netbsd package"; \
	echo " osx                  -- makes binary macosx package"; \
	echo

help-macros::
	@echo "CC = $(CC)"; \
	echo "PREFIX = $(PREFIX)"; \
	echo "APP_INSDIR = $(APP_INSDIR)"; \
	echo "LIB_INSDIR = $(LIB_INSDIR)"; \
	echo "MAN_INSDIR = $(MAN_INSDIR)"; \
	echo "HDR_INSDIR = $(HDR_INSDIR)"; \
	echo "DATA_INSDIR = $(DATA_INSDIR)"; \
	echo "CONF_INSDIR = $(CONF_INSDIR)"; \
	echo "APP_MANSECT = $(APP_MANSECT)"; \
	echo "LIB_MANSECT = $(LIB_MANSECT)"; \
	echo "FMT_MANSECT = $(FMT_MANSECT)"; \
	echo "APP_MANDIR = $(APP_MANDIR)"; \
	echo "LIB_MANDIR = $(LIB_MANDIR)"; \
	echo "FMT_MANDIR = $(FMT_MANDIR)"; \
	echo "TAG_FILES = $(TAG_FILES)"; \
	echo "DEPEND_CFILES = $(DEPEND_CFILES)"; \
	echo "DEPEND_HFILES = $(DEPEND_HFILES)"; \
	echo "CCFLAGS = $(CCFLAGS)"; \
	echo "READY_TARGETS = $(READY_TARGETS)"; \
	echo "ALL_TARGETS = $(ALL_TARGETS)"; \
	echo "TEST_TARGETS = $(TEST_TARGETS)"; \
	echo "MAN_TARGETS = $(MAN_TARGETS)"; \
	echo "HTML_TARGETS = $(HTML_TARGETS)"; \
	echo "INSTALL_TARGETS = $(INSTALL_TARGETS)"; \
	echo "UNINSTALL_TARGETS = $(UNINSTALL_TARGETS)"; \
	echo "CLEAN_FILES = $(CLEAN_FILES)"; \
	echo "CLOBBER_FILES = $(CLOBBER_FILES)"; \
	echo "DEBIAN_CLOBBER_FILES = $(DEBIAN_CLOBBER_FILES)"; \
	echo "DIST_TARGETS = $(DIST_TARGETS)"; \
	echo "RPM_TARGETS = $(RPM_TARGETS)"; \
	echo "DEB_TARGETS = $(DEB_TARGETS)"; \
	echo "SOL_TARGETS = $(SOL_TARGETS)"; \
	echo "OBSD_TARGETS = $(OBSD_TARGETS)"; \
	echo "FBSD_TARGETS = $(FBSD_TARGETS)"; \
	echo "NBSD_TARGETS = $(NBSD_TARGETS)"; \
	echo "OSX_TARGETS = $(OSX_TARGETS)"; \
	echo

tags: $(TAG_FILES)
	@ctags $(TAG_FILES)

depend: ready $(DEPEND_CFILES) $(DEPEND_HFILES)
	@makedepend $(DAEMON_CPPFLAGS) $(DEPEND_CFILES)

clean::
	@rm -rf $(CLEAN_FILES)

ifeq ($(shell uname -a 2>/dev/null | grep -i Debian),)
clobber::
	@rm -rf $(CLEAN_FILES) $(CLOBBER_FILES)
else
clobber:: debian-clobber
endif

distclean:: clobber
	@perl -pi -e 'last if /[D]O NOT DELETE/;' $(patsubst %, %/Makefile, $(DAEMON_SRCDIR) $(DAEMON_SUBDIRS))

include $(DAEMON_SRCDIR)/rules.mk

