#! /usr/bin/env bash
$EXTRACTRC *.ui >> rc.cpp
$XGETTEXT `find . -name \*.qml -o -name \*.cpp` -o $podir/lingmo_applet_org.kde.lingmo.timer.pot
