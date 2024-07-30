#!/usr/bin/env bash

$EXTRACTRC `find . -name "*.ui" -o -name "*.rc"` >> rc.cpp
$XGETTEXT `find . -name '*.cpp'` -o $podir/plasmanetworkmanagement-kcm.pot
$XGETTEXT `find . -name '*.qml'` -j -L Java -o $podir/plasmanetworkmanagement-kcm.pot
rm -f rc.cpp
