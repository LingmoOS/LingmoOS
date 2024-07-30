#! /usr/bin/env bash
$EXTRACTRC *.ui >> rc.cpp
$XGETTEXT `find . -name \*.qml -o -name \*.cpp` -o $podir/kcm_pulseaudio.pot
rm -f rc.cpp
