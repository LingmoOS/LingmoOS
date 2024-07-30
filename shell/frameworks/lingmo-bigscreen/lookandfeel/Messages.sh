#! /usr/bin/env bash
$XGETTEXT `find . -name \*.qml` -L Java -o $podir/lingmo_lookandfeel_org.kde.lingmo.mycroft.bigscreen.pot
rm -f rc.cpp
