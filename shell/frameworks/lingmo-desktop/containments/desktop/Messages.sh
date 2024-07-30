#! /usr/bin/env bash
$XGETTEXT `find . -name \*.cpp -o -name \*.h -o -name \*.qml` -o $podir/lingmo_applet_org.kde.desktopcontainment.pot
