#!/bin/sh

aclocal --force
autoheader --force
autoconf --force
automake --foreign --add-missing --copy --force-missing
