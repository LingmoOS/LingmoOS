#! /usr/bin/env bash
$EXTRACT_TR_STRINGS `find . -name \*.qml -o -name \*.cpp` -o  $podir/qqc2desktopstyle_qt.pot
