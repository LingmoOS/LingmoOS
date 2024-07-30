#! /usr/bin/env bash
$EXTRACT_TR_STRINGS `find . -name \*.qml -o -name \*.cpp` -o $podir/liblingmoui6_qt.pot
rm -f rc.cpp

