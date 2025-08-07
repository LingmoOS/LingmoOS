#! /bin/sh
$PREPARETIPS > tips.cpp
$EXTRACTRC `find . -name \*.rc -o -name \*.ui -o -name \*.kcfg` >> rc.cpp
$XGETTEXT *.h *.cpp core/*.cpp editor/*.cpp model/*.cpp transactions/*.cpp -o $podir/kgpg.pot
rm -f tips.cpp
