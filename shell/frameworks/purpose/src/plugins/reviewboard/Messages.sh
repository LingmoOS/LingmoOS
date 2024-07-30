#!/bin/sh
$EXTRACTRC `find . -name \*.rc` `find . -name \*.ui` >> rc.cpp
$XGETTEXT `find . -not -path \*/tests/\* -name \*.cpp -o -name \*.qml -o -name \*.h` -o $podir/purpose6_reviewboard.pot
rm -f rc.cpp
