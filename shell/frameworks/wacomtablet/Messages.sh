#! /bin/sh
$EXTRACTRC `find . -name \*.rc -o -name \*.ui -o -name \*.kcfg` >> rc.cpp
$XGETTEXT `find . -path ./src/applet -prune -name \*.cc -o -name \*.cpp -o -name \*.h` -o $podir/wacomtablet.pot
$XGETTEXT `find ./src/applet -name \*.qml -o -name \*.js` -o $podir/plasma_applet_org.kde.plasma.wacomtablet.pot
rm -f rc.cpp
