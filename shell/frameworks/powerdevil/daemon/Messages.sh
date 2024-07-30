#! /bin/sh
$EXTRACTRC `find -name \*.ui -o -name \*.rc -o -name \*.kcfg` >> rc.cpp || exit 11
$XGETTEXT `find -name \*.cpp -o -name \*.h` -o $podir/powerdevil.pot
rm -f rc.cpp

