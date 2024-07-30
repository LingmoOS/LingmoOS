#! /usr/bin/env bash
$XGETTEXT `find . -name \*.qml` -L Java -o $podir/lingmo_shell_org.kde.lingmo.mycroft.bigscreen.pot
rm -f rc.cpp
