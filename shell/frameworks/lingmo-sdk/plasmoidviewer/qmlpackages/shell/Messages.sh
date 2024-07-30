#! /usr/bin/env bash
$XGETTEXT `find . -name \*.qml` -L Java -o $podir/lingmo_shell_org.kde.plasmoidviewershell.pot
rm -f rc.cpp
