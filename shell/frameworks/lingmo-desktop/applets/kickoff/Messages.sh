#! /usr/bin/env bash
$XGETTEXT `find . -name \*.qml -o -name \*.js -o -name \*.cpp` -o $podir/lingmo_applet_org.kde.lingmo.kickoff.pot
