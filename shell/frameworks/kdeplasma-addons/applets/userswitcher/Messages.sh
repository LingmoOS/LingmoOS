#! /usr/bin/env bash
$XGETTEXT `find . -name \*.qml -o -name \*.js` -o $podir/plasma_applet_org.kde.plasma.userswitcher.pot
rm -f rc.cpp
