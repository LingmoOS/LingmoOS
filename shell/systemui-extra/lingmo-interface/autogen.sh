#!/bin/sh
#aclocal
#autoconf
#autoheader
#libtoolize --copy
#automake --copy --add-missing

echo "Regenerating autotools files"
autoreconf --force --install || exit 1
