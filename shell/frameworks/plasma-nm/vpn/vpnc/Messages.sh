#! /usr/bin/env bash
$EXTRACTRC `find . -name "*.ui" -o -name "*.rc"` >> rc.cpp
$XGETTEXT `find . -name "*.cpp"` -o $podir/plasmanetworkmanagement_vpncui.pot
rm -f rc.cpp
