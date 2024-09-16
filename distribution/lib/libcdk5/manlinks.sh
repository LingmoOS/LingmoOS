#! /bin/sh
# $Id: manlinks.sh,v 1.9 2022/10/19 01:03:33 tom Exp $
# install/uninstall aliases for one or more manpages,
#
# Copyright 2002-2005,2022 Thomas E. Dickey
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, distribute with
# modifications, sublicense, and/or sell copies of the Software, and to permit
# persons to whom the Software is furnished to do so, subject to the following
# conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
# ABOVE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
# IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
# Except as contained in this notice, the name(s) of the above copyright
# holders shall not be used in advertising or otherwise to promote the sale,
# use or other dealings in this Software without prior written authorization.

CDPATH=

verb=$1
shift

srcdir=$1
shift

mandir=$1
shift

mansect=$1
shift

parent=`pwd`
script=$srcdir/manlinks.sed

for source in "$@" ; do
case $source in #(vi
*.orig|*.rej) ;; #(vi
*.[0-9]*)
	cd "$parent" || exit 1
	# section=`expr "$source" : '.*\.\([0-9]\)[xm]*'`;
	aliases=
	inalias=$source
	test ! -f "$inalias" && inalias="$srcdir/$inalias"
	if test ! -f "$inalias" ; then
		echo "	.. skipped $source"
		continue
	fi
	aliases=`sed -f "$script" "$inalias" | sort -u`

	suffix=`basename "$source" | sed -e 's/^[^.]*//'`
	cd "$mandir" || exit 1
	if test "$verb" = installing ; then
		test -n "$aliases" && (
			if test "$suffix" = ".$mansect" ; then
				target=`basename "$source"`
			else
				target=`basename "$source" "$suffix"`.$mansect
				suffix=".$mansect"
			fi
			for cf_alias in $aliases
			do
				cf_doit=no
				if test -f "$cf_alias${suffix}" ; then
					if ( cmp -s "$target" "$cf_alias${suffix}" )
					then
						:
					else
						cf_doit=yes
					fi
				else
					cf_doit=yes
				fi
				if test $cf_doit = yes ; then
					echo "	... alias $cf_alias${suffix}"
					rm -f "$cf_alias${suffix}"
					if (ln -s "$target" "$cf_alias${suffix}") ; then
						:
					else
						echo ".so $target" > "$cf_alias${suffix}"
					fi
				fi
			done
		)
	elif test "$verb" = removing ; then
		suffix=".$mansect"
		test -n "$aliases" && (
			for cf_alias in $aliases
			do
				echo "	... alias $cf_alias${suffix}"
				rm -f "$cf_alias${suffix}"
			done
		)
	fi
	;;
esac
done
exit 0
# vile:ts=4
