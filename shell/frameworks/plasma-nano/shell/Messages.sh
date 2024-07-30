#! /usr/bin/env bash
$XGETTEXT `find . -name \*.qml` -L Java -o $podir/plasma_shell_org.kde.plasma.nano.pot
rm -f rc.cpp
