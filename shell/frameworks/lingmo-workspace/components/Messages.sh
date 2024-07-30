#! /bin/sh
$EXTRACTRC `find . -name \*.kcfg` >>rc.cpp
$XGETTEXT  `find . -name \*.cpp -o -name \*.qml` -o $podir/lingmoshellprivateplugin.pot
rm -f rc.cpp
