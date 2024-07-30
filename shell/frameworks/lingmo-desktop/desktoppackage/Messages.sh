#! /usr/bin/env bash
$XGETTEXT `find . -name \*.qml` -o $podir/lingmo_shell_org.kde.lingmo.desktop.pot
rm -f rc.cpp
