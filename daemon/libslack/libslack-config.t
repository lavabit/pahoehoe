#!/bin/bash
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

var() { eval $1='$2'; export $1; }
die() { echo "$@" >&2; exit 1; }

var url     "@@URL@@"
var name    "@@NAME@@"
var version "@@VERSION@@"
var prefix  "@@PREFIX@@"
var cflags  "@@CFLAGS@@"
var libs    "@@LIBS@@"

if test "$prefix" != "/usr" -a "$prefix" != "/usr/local"
then
	cflags="-I$prefix/include $cflags"
	libs="-L$prefix/lib $libs"
fi

usage()
{
	cat <<EOF
usage: libslack-config [options]
options:
    -h, --help      - Print this help and exit
    -v, --version   - Print the version of the currently installed libslack
    -L, --latest    - Print the latest version of libslack (uses wget)
    -D, --download  - Download the latest version of libslack (uses wget)
    -U, --upgrade   - Upgrade to the latest version of libslack (uses wget)
    -p, --prefix    - Print the prefix directory of the libslack installation
    -c, --cflags    - Print CFLAGS needed to compile clients of libslack
    -l, --libs      - Print LDFLAGS needed to link against libslack
    -l, --ldflags   - Identical to --libs
    -u, --uninstall - Uninstall libslack

Note: the dashes are optional for long option names

Command line example:
    gcc -o app app.c \`libslack-config --cflags --libs\`

Makefile example:
    CFLAGS  += \$(shell libslack-config --cflags)
    LDFLAGS += \$(shell libslack-config --libs)

EOF
	exit $1
}

latest()
{
	wget -q -O- "${url}download/" | \
	perl -e '

		$ENV{suffix} =~ s/\./\\./g;

		while (<>)
		{
			$version{$1} = 1 if /[Hh][Rr][Ee][Ff]=".*$ENV{name}-([\d.]+)\.tar\.gz"/;
		}

		sub version_sort
		{
			my @anum = split /\./, $a;
			my @bnum = split /\./, $b;

			while ($#anum != -1 && $#bnum != -1)
			{
				return $x if $x = $bnum[0] - $anum[0];
				shift @anum;
				shift @bnum;
			}

			return -1 if $#anum != -1;
			return  1 if $#bnum != -1;
			return 0;
		}

		@version = sort { version_sort } keys %version;
		die "No versions found at $ENV{url}download/\n" if $#version == -1;
		print "$ENV{url}download/$ENV{name}-$version[0].tar.gz\n";
		exit 0;
	'
}

download()
{
	latest="`latest 2>&1`"
	test "$latest" = "No versions found at ${url}download" && die "$latest"
	file="`echo $latest | sed 's/^.*\///'`"
	test -f "$file" && die "The file $file already exists"
	wget "$latest"
}

upgrade()
{
	latest="`latest 2>&1`"
	test "$latest" = "No versions found at ${url}/download" && die "$latest"
	file="`echo $latest | sed 's/^.*\///'`"
	dir="`echo $file | sed 's/\.tar\.gz$//'`"
	test -f "$file" || wget "$latest"
	test -s "$file" || die "Failed to download $latest"
	tar xzf "$file" || die "Failed to untar $file"
	cd "$dir" || die "Failed to cd $dir"
	./configure || die "Failed to configure $dir"
	make || die "Failed to make $dir"
	uninstall || die "Failed to uninstall current version"
	make PREFIX="$prefix" install || die "Failed to install $dir into $prefix"
	cd .. && rm -rf "$dir"
}

uninstall()
{
@@UNINSTALL@@
}

test $# -eq 0 && usage 1 1>&2

while test $# -gt 0
do
	case "$1" in
		-h|--help|help)           usage 0;;
		-v|--version|version)     echo "$version";;
		-L|--latest|latest)       latest;;
		-D|--download|download)   download;;
		-U|--upgrade|upgrade)     upgrade;;
		-p|--prefix|prefix)       echo "$prefix";;
		-c|--cflags|cflags)       echo "$cflags";;
		-l|--libs|libs)           echo "$libs";;
		-l|--ldflags|ldflags)     echo "$libs";;
		-u|--uninstall|uninstall) uninstall;;
		*) usage 1 >&2;;
	esac
	shift
done

exit 0

# vim:set ts=4 sw=4:
