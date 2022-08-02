#
# libslack - http://libslack.org/
#
# Copyright (C) 1999-2002, 2004, 2010, 2020 raf <raf@raf.org>
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
# 20201111 raf <raf@raf.org>

# Uncomment these to override the defines in daemon.h and prog.h
#
# SLACK_DEFINES += -DPATH_SEP=\'/\'
# SLACK_DEFINES += -DROOT_DIR=\"/\"
# SLACK_DEFINES += -DETC_DIR=\"/etc\"
# SLACK_DEFINES += -DROOT_PID_DIR=\"/var/run\"
# SLACK_DEFINES += -DUSER_PID_DIR=\"/tmp\"

# Uncomment this if your system doesn't have GNU getopt_long()
#
# GETOPT := getopt

# Uncomment this if your system does have getopt_long()
#
SLACK_CLIENT_CFLAGS += -DHAVE_GETOPT_LONG=1

# Uncomment this if your system doesn't have a good snprintf()
#
# SNPRINTF := snprintf

# Uncomment this if your system does have a good snprintf()
#
SLACK_CLIENT_CFLAGS += -DHAVE_SNPRINTF=1

# Uncomment this if your system doesn't have vsscanf()
#
# VSSCANF := vsscanf

# Uncomment this if your system does have vsscanf()
#
SLACK_CLIENT_CFLAGS += -DHAVE_VSSCANF=1

# Uncomment these if your system has POSIX threads reader/writer locks.
#
SLACK_CLIENT_CFLAGS += -DHAVE_PTHREAD_RWLOCK=1

# Uncomment this to prevent an extra fork on SVR4 which prevents
# the process from ever gaining a controlling terminal. If this is
# uncommented, the O_NOCTTY flag should be passed to all calls to
# open(2) made by the process (as is required on BSD anyway)
#
# SLACK_DEFINES += -DNO_EXTRA_SVR4_FORK=1

# Uncomment this to override the default value of 8192 bytes
# as the size for message buffers
#
# SLACK_DEFINES += -DMSG_SIZE=8192

# Uncomment this to override the default value of 32 as the maximum
# number of dimensions of an allocated space
#
# SLACK_DEFINES += -DMEM_MAX_DIM=32

# Uncomment these if necessary (Needed on MacOSX 10.4)
#
# SLACK_DEFINES += -DNO_POSIX_C_SOURCE=1
# SLACK_TEST_DEFINES += -DNO_POSIX_C_SOURCE=1

# Uncomment these on MacOSX to create universal binaries
#
# SLACK_CCFLAGS += -arch x86_64 -arch i386 -arch ppc # -arch ppc64
# SLACK_TEST_LDFLAGS += -arch x86_64 -arch i386 -arch ppc # -arch ppc64

# Uncomment these on 64-bit OpenSolaris or Solaris to compile for 64-bit
#
# SLACK_CCFLAGS += -m64
# SLACK_TEST_CCFLAGS += -m64
# SLACK_TEST_LDFLAGS += -m64
# SLACK_CLIENT_LDFLAGS += -m64

# Uncomment these if your system has the "long long int" type.
#
SLACK_CCFLAGS += -Wno-long-long
SLACK_TEST_CCFLAGS += -Wno-long-long

# Uncomment this if your gcc supports it
#
SLACK_TEST_CCFLAGS += -Wno-overlength-strings

# Uncomment these if your gcc supports them. Don't panic.
# These are to test that snprintf handles bad input.
#
SLACK_TEST_CCFLAGS += -Wno-address
SLACK_TEST_CCFLAGS += -Wno-nonnull
SLACK_TEST_CCFLAGS += -Wno-format
SLACK_TEST_CCFLAGS += -Wno-restrict

# Uncomment these if you need to (i.e. on macOS).
# Note that the deprecated warning is about tmpnam()
# which is used safely here (believe it or check the code).
# It's used to create a path for bind() for UNIX domain
# datagram sockets on some systems. The reason that it is
# safe is because bind() will fail if the path already exists.
# That's the equivalent of using open with O_EXCL that is
# recommended when using tmpnam(). Some compilers will warn
# about using tmpnam() saying that it is decprecated and to
# use mkstemp() instead but that function creates the file as
# well to eliminate the race condition but we don't want a
# file to be created here. We want a UNIX domain datagram
# socket which can only be created by bind(). Correct me if
# I'm wrong.

# SLACK_CCFLAGS += -Wno-deprecated-declarations
# SLACK_CCFLAGS += -Wno-gnu-folding-constant

# Uncomment this to exclude compilation of the debug locker functions.
# These functions shamefully assume that pthread_self() can be cast into
# an unsigned long. If this is not true on on your system, uncomment this
# define or change the code.
#
# SLACK_DEFINES += -DNO_DEBUG_LOCKERS=1

SLACK_NAME := slack
SLACK_VERSION := 0.7
SLACK_URL := http://libslack.org/
SLACK_ID := lib$(SLACK_NAME)-$(SLACK_VERSION)
SLACK_DIST := $(SLACK_ID).tar.gz
SLACK_HTML_ID := $(SLACK_ID)-html
SLACK_HTML_DIST := $(SLACK_HTML_ID).tar.gz

SLACK_TARGET := $(SLACK_SRCDIR)/lib$(SLACK_NAME).a
SLACK_INSTALL := $(SLACK_ID).a
SLACK_INSTALL_LINK := lib$(SLACK_NAME).a
SLACK_CONFIG := $(SLACK_SRCDIR)/lib$(SLACK_NAME)-config
SLACK_MODULES := agent coproc daemon err fio $(GETOPT) hsort lim link list locker map mem msg net prog prop pseudo sig $(SNPRINTF) str $(VSSCANF)
SLACK_HEADERS := std lib hdr socks
SLACK_LIB_PODS := libslack
SLACK_APP_PODS := libslack-config

SLACK_HTMLDIR := $(DATA_INSDIR)/lib$(SLACK_NAME)/doc

SLACK_CFILES := $(patsubst %, $(SLACK_SRCDIR)/%.c, $(SLACK_MODULES))
SLACK_OFILES := $(patsubst %, $(SLACK_SRCDIR)/%.o, $(SLACK_MODULES))
SLACK_HFILES := $(patsubst %, $(SLACK_SRCDIR)/%.h, $(SLACK_MODULES) $(SLACK_HEADERS))

SLACK_LIB_PODNAMES := $(patsubst %.pod, %, $(SLACK_LIB_PODS)) $(SLACK_MODULES)
SLACK_LIB_MANFILES := $(patsubst %, $(SLACK_SRCDIR)/%.$(LIB_MANSECT), $(SLACK_LIB_PODNAMES))
SLACK_LIB_HTMLFILES := $(sort $(patsubst %, $(SLACK_SRCDIR)/%.$(LIB_MANSECT).html, $(SLACK_LIB_PODNAMES) getopt snprintf vsscanf))

SLACK_APP_PODNAMES := $(patsubst %.pod, %, $(SLACK_APP_PODS))
SLACK_APP_MANFILES := $(patsubst %, $(SLACK_SRCDIR)/%.$(APP_MANSECT), $(SLACK_APP_PODNAMES))
SLACK_APP_HTMLFILES := $(patsubst %, $(SLACK_SRCDIR)/%.$(APP_MANSECT).html, $(SLACK_APP_PODNAMES))

ifeq ($(MAN_GZIP), 1)
SLACK_LIB_MANFILES := $(patsubst %, %.gz, $(SLACK_LIB_MANFILES))
SLACK_APP_MANFILES := $(patsubst %, %.gz, $(SLACK_APP_MANFILES))
MAN_SUFFIX := .gz
endif

SLACK_SWIGFILE := $(SLACK_SRCDIR)/slack.swig

SLACK_TESTDIR := $(SLACK_SRCDIR)/test
SLACK_TESTS := $(patsubst %, $(SLACK_TESTDIR)/%, $(SLACK_MODULES))

SLACK_INCLINK := $(SLACK_SRCDIR)/$(SLACK_NAME)

ifeq ($(FINAL_PREFIX),)
FINAL_PREFIX := $(PREFIX)
endif

TAG_FILES += $(SLACK_HFILES) $(SLACK_CFILES)
DEPEND_HFILES += $(SLACK_HFILES)
DEPEND_CFILES += $(SLACK_CFILES)

ifeq ($(SLACK_SRCDIR), .)
SLACK_MAIN := 1
endif

ALL_TARGETS += slack
READY_TARGETS += ready-slack
TEST_TARGETS += test-slack
MAN_TARGETS += man-slack
HTML_TARGETS += html-slack
ifeq ($(SLACK_MAIN), 1)
INSTALL_TARGETS += install-slack
UNINSTALL_TARGETS += uninstall-slack
endif
DIST_TARGETS += dist-slack
RPM_TARGETS += rpm-slack
DEB_TARGETS +=
SOL_TARGETS += sol-slack
OBSD_TARGETS += obsd-slack
FBSD_TARGETS += fbsd-slack
NBSD_TARGETS += nbsd-slack
OSX_TARGETS += osx-slack

CLEAN_FILES += $(SLACK_OFILES) $(SLACK_CONFIG) $(SLACK_LIB_MANFILES) $(SLACK_APP_MANFILES) $(SLACK_LIB_HTMLFILES) $(SLACK_APP_HTMLFILES) $(SLACK_SRCDIR)/pod2html-* $(SLACK_SWIGFILE)
CLOBBER_FILES += $(SLACK_TARGET) $(SLACK_SRCDIR)/tags $(SLACK_TESTDIR) $(SLACK_INCLINK)

SLACK_RPM_FILES += $(LIB_INSDIR)/$(SLACK_INSTALL_LINK)
SLACK_RPM_FILES += $(LIB_INSDIR)/$(SLACK_INSTALL)
SLACK_RPM_FILES += $(APP_INSDIR)/$(notdir $(SLACK_CONFIG))
SLACK_RPM_FILES += $(patsubst %, $(HDR_INSDIR)/$(SLACK_NAME)/%, $(notdir $(SLACK_HFILES)))
SLACK_RPM_DOCFILES += $(patsubst %, $(APP_MANDIR)/%, $(notdir $(SLACK_APP_MANFILES)))
SLACK_RPM_DOCFILES += $(patsubst %, $(LIB_MANDIR)/%, $(notdir $(SLACK_LIB_MANFILES)))
SLACK_RPM_DOCFILES += $(foreach MODULE, $(SLACK_MODULES), $(patsubst %, $(LIB_MANDIR)/%.$(LIB_MANSECT)$(MAN_SUFFIX), $(shell perl -n -e 'print $$1, "\n" if /^=item C<(?:const )?\w+[\s*]*(\w+)\(.*\)>$$/ or /^=item C< \#define (\w+)\(.*\)>$$/' "$(SLACK_SRCDIR)/$(MODULE).c")))
SLACK_SOL := RAFOslk

SLACK_CPPFLAGS += $(SLACK_DEFINES) $(patsubst %, -I%, $(SLACK_INCDIRS))
SLACK_CCFLAGS += $(CCFLAGS)
SLACK_CFLAGS += $(SLACK_CPPFLAGS) $(SLACK_CCFLAGS)

SLACK_TEST_CPPFLAGS += $(SLACK_TEST_DEFINES) $(patsubst %, -I%, $(SLACK_INCDIRS))
SLACK_TEST_CCFLAGS += -Wall -pedantic
SLACK_TEST_CFLAGS += $(SLACK_TEST_CPPFLAGS) $(SLACK_TEST_CCFLAGS)

# SLACK_TEST_LDFLAGS += -pthread
SLACK_TEST_LIBS += $(SLACK_NAME)
SLACK_TEST_LIBS += pthread
SLACK_TEST_LIBS += util

# SLACK_CLIENT_LDFLAGS += -pthread
SLACK_CLIENT_LIBS += $(SLACK_NAME)
SLACK_CLIENT_LIBS += pthread
SLACK_CLIENT_LIBS += util

# Uncomment these on Solaris for sockets (used by the daemon and net modules)
#
# SLACK_TEST_LIBS += xnet
# SLACK_TEST_LIBS += socket
# SLACK_TEST_LIBS += nsl
# SLACK_CLIENT_LIBS += xnet
# SLACK_CLIENT_LIBS += socket
# SLACK_CLIENT_LIBS += nsl

# Uncomment these on Solaris if you are using Sun's C compiler
#
# SLACK_CLIENT_LIBS += m
# SLACK_TEST_LIBS += m

SLACK_TEST_LDFLAGS += $(patsubst %, -L%, $(SLACK_LIBDIRS)) $(patsubst %, -l%, $(SLACK_TEST_LIBS))
SLACK_CLIENT_LDFLAGS += $(patsubst %, -l%, $(SLACK_CLIENT_LIBS))

SLACK_SUBTARGETS :=
SLACK_SUBDIRS :=

