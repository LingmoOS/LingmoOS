#! /usr/bin/env bash
$EXTRACTRC `find . -name "*.ui" -o -name "*.rc"` >> rc.cpp
$XGETTEXT `find . -name "*.cpp"` -o $podir/lingmonetworkmanagement-libs.pot
rm -f rc.cpp
