#! /usr/bin/env bash
$EXTRACTRC `find . -name "*.rc" -o -name "*.ui" | grep -v "src/plasmoid"` >> rc.cpp
$XGETTEXT `find . -name "*.cpp" | grep -v "src/plasmoid"` -o $podir/print-manager.pot
rm -f rc.cpp

$XGETTEXT `find src/plasmoid -name "*.qml"` -L Java -o $podir/plasma_applet_org.kde.plasma.printmanager.pot
