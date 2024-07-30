#! /usr/bin/env bash
$EXTRACTRC *.ui >> rc.cpp
$XGETTEXT `find . -name \*.qml -o -name \*.cpp` -o $podir/plasma_applet_org.kde.plasma.timer.pot
