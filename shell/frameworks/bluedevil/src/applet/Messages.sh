#!/usr/bin/env bash

$XGETTEXT `find . -name '*.js' -o -name '*.qml'` -o $podir/lingmo_applet_org.kde.lingmo.bluetooth.pot
