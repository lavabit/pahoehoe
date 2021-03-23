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

/* Define if defining _POSIX_SOURCE breaks things */
/* #undef NO_POSIX_SOURCE */

/* Define if defining _XOPEN_SOURCE breaks things */
/* #undef NO_XOPEN_SOURCE */

/* Define if we have <stdarg.h> */
#define HAVE_STDARG_H 1

/* Define if we have <stdint.h> */
/* #undef HAVE_STDINT_H */

/* Define if we have <poll.h> */
#define HAVE_POLL_H 1

/* Define if we have <sys/poll.h> */
#define HAVE_SYS_POLL_H 1

/* Define if we have <sys/select.h> */
#define HAVE_SYS_SELECT_H 1

/* Define if we have <regex.h> (POSIX regular expressions) */
#define HAVE_REGEX_H 1

/* Define if we have long double */
#define HAVE_LONG_DOUBLE 1

/* Define if we have long long */
#define HAVE_LONG_LONG 1

/* Define if we have a good snprintf() */
#define HAVE_SNPRINTF 1

/* Define if we have a good vsnprintf() */
#define HAVE_VSNPRINTF 1

/* Define if we have getopt_long() */
#define HAVE_GETOPT_LONG 1

/* Define if we have vsscanf() */
#define HAVE_VSSCANF 1

/* Define if we have strcasecmp() */
#define HAVE_STRCASECMP 1

/* Define if we have strncasecmp() */
#define HAVE_STRNCASECMP 1

/* Define if we have strlcpy() */
/* #undef HAVE_STRLCPY */

/* Define if we have strlcat() */
/* #undef HAVE_STRLCAT */

/* Define if we have asprintf() */
/* #undef HAVE_ASPRINTF */

/* Define if we have vasprintf() */
/* #undef HAVE_VASPRINTF */

/* Define if we have flockfile(), funlockfile() and getc_unlocked() */
#define HAVE_FLOCKFILE 1

/* Define if fnctl() can lock fifos */
#define HAVE_FCNTL_THAT_CAN_LOCK_FIFOS 1

/* Define if we have an ISO C compliant realloc() */
#define HAVE_ISOC_REALLOC 1

/* Define if we have poll() */
#define HAVE_POLL 1

/* Define if we have the Linux poll() bug (always times out 10ms too late) */
#define HAVE_LINUX_POLL_BUG 1

/* Define if we have mlock() */
#define HAVE_MLOCK 1

/* Define if we have gethostbyname_r() with six arguments */
#define HAVE_FUNC_GETHOSTBYNAME_R_6 1

/* Define if we have gethostbyname_r() with five arguments */
/* #undef HAVE_FUNC_GETHOSTBYNAME_R_5 */

/* Define if we have gethostbyname_r() with three arguments */
/* #undef HAVE_FUNC_GETHOSTBYNAME_R_3 */

/* Define if we have getservbyname_r() with six arguments */
#define HAVE_FUNC_GETSERVBYNAME_R_6 1

/* Define if we have getservbyname_r() with five arguments */
/* #undef HAVE_FUNC_GETSERVBYNAME_R_5 */

/* Define if we have getservbyname_r() with four arguments */
/* #undef HAVE_FUNC_GETSERVBYNAME_R_4 */

/* Define if we have PTHREAD_PROCESS_PRIVATE */
#define HAVE_PTHREAD_PROCESS_PRIVATE 1

/* Define if we have PTHREAD_PROCESS_SHARED */
#define HAVE_PTHREAD_PROCESS_SHARED 1

/* Define if we have pthread_condattr_init() */
#define HAVE_PTHREAD_CONDATTR_INIT 1

/* Define if we have pthread_condattr_setpshared() */
#define HAVE_PTHREAD_CONDATTR_SETPSHARED 1

/* Define if we have pthread_mutexattr_setpshared() */
#define HAVE_PTHREAD_MUTEXATTR_SETPSHARED 1

/* Define if we have POSIX readers/writer locks */
#define HAVE_PTHREAD_RWLOCK 1

/* Define if struct msghdr has msg_control and msg_controllen */
#define HAVE_MSGHDR_MSG_CONTROL 1

/* Define if struct ifreq has ifr_ifindex */
#define HAVE_IFREQ_IFR_IFINDEX 1

/* Define if struct ifreq has ifr_mtu */
#define HAVE_IFREQ_IFR_MTU 1

/* Define if struct sockaddr_t has sa_len */
/* #undef HAVE_SOCKADDR_SA_LEN */

/* Define if we have if_indextoname() */
#define HAVE_IF_INDEXTONAME 1

/* Define if we have if_nametoindex() */
#define HAVE_IF_NAMETOINDEX 1

/* Define if mlock() requires the first argument to be on a page boundary */
/* #undef MLOCK_REQUIRES_PAGE_BOUNDARY */

/* Define if %p produces hex with a 0x prefix */
#define HAVE_PRINTF_PTR_FMT_ALTERNATE 1

/* Define if %p is treated almost like a signed conversion */
#define HAVE_PRINTF_PTR_FMT_SIGNED 1

/* Define if %p prints NULL as "(nil)" */
#define HAVE_PRINTF_PTR_FMT_NIL 1

/* Define if %s prints NULL as "(null)" */
#define HAVE_PRINTF_STR_FMT_NULL 1

/* Define if %g has a default precision of 1 (as specified by ISO C) */
/* #undef HAVE_PRINTF_FLT_FMT_G_STD */

/* Define if %p never produces a 0x prefix */
/* #undef HAVE_PRINTF_PTR_FMT_NOALT */

/* Define if *printf() has non standard Solaris negative width behaviour */
/* #undef HAVE_PRINTF_WITH_SOLARIS_NEGATIVE_WIDTH_BEHAVIOUR */

/* Define if *printf() has non standard Solaris8 zero precision alternate octal behaviour */
/* #undef HAVE_PRINTF_WITH_SOLARIS8_ZERO_PRECISION_ALT_OCTAL_BEHAVIOUR */

/* Define if we have the UNIX domain wildcard (i.e. can bind to "") */
#define HAVE_UNIX_DOMAIN_WILDCARD 1

/* Define on SVR4 */
#define SVR4 1

/* Define if we have socks */
/* #undef SOCKS */

/* Define on CYGWIN */
/* #undef HAVE_CYGWIN */

/* Define if we have /dev/ptmx */
#define HAVE_DEV_PTMX 1

/* Define if we have /dev/pts and /dev/ptc */
/* #undef HAVE_DEV_PTS_AND_PTC */

/* Define if we have openpty() - Linux has it in libutil */
#define HAVE_OPENPTY 1

/* Define if we have <pty.h> */
#define HAVE_PTY_H 1

/* Define if we have <util.h> */
/* #undef HAVE_UTIL_H */

/* Define if we have <libutil.h> */
/* #undef HAVE_LIBUTIL_H */

/* Define if we have <sys/stropts.h> */
#define HAVE_SYS_STROPTS_H 1

/* Define if we have vhangup() */
#define HAVE_VHANGUP 1

/* Define if we have _getpty() */
/* #undef HAVE__GETPTY */

/* Define if we have ttyname_r() */
#define HAVE_TTYNAME_R 1

/* Define if we have ptsname_r() */
#define HAVE_PTSNAME_R 1

/* Define if we have ptsname() */
#define HAVE_PTSNAME 1

/* Define if we have an evil poll() that aborts when pollfds is null */
/* #undef HAVE_POLL_THAT_ABORTS_WHEN_POLLFDS_IS_NULL */
