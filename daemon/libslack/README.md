README
======
*libslack* - A *UNIX/C* library of general utilities for programmers with Slack

    Slack(n.): The state in which you need nothing,
               because you already have it.
                     -- J. R. "Bob" Dobbs, 1956

DESCRIPTION
===========
*Libslack* is a library of general utilities designed to make *UNIX/C* programming
a bit easier on the eye. It is a seemingly random collection of modules and
functions that I find commonly useful.

It's a small library with lots of functionality, accurately documented and
thoroughly tested. Good library naming conventions are not rigorously observed
on the principle that common operations should always be easy to write and code
should always be easy to read.

*Libslack* contains the following modules:

    agent    - agent-oriented programming
    coproc   - coprocess using pipes or pseudo terminals
    daemon   - becoming a daemon
    err      - message/error/debug/verbosity/alert messaging
    fio      - fifo and file control and some I/O
    getopt   - GNU getopt_long() for systems that don't have it
    hsort    - generic heap sort
    lim      - POSIX.1 limits convenience functions
    link     - abstract linked lists with optional growable free lists
    list     - list (growable pointer array) data type
    locker   - abstract locking and reader/writer lock implementation
    map      - map (hash table) data type
    mem      - memory helper functions, secure memory, memory pools
    msg      - message handling and syslog helper functions
    net      - network functions (clients/servers, expect/send, pack/unpack, mail)
    prog     - program framework and flexible command line option handling
    prop     - program properties files
    pseudo   - pseudo terminals
    sig      - ISO C compliant signal handling
    snprintf - safe sprintf for systems that don't have it
    str      - string data type (tr, regex, regsub, fmt, trim, lc, uc, ...)
    vsscanf  - sscanf() with va_list argument for systems that don't have it

--------------------------------------------------------------------------------

    URL: http://libslack.org
    URL: http://raf.org/libslack
    GIT: https://github.com/raforg/libslack
    Date: 20210220
    Author: raf <raf@raf.org>

