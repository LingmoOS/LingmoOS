#! /usr/bin/env bash
$EXTRACTRC *.ui >> rc.cpp
$XGETTEXT *.cpp `find . -name '*.qml'` -o $podir/lingmo_applet_org.kde.lingmo.comic.pot
