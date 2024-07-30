#! /bin/sh
$EXTRACTRC `find . -name \*.rc -o -name \*.ui -o -name \*.kcfg` >> rc.cpp
$XGETTEXT `find . -path ./src/applet -prune -name \*.cc -o -name \*.cpp -o -name \*.h` -o $podir/wacomtablet.pot
$XGETTEXT `find ./src/applet -name \*.qml -o -name \*.js` -o $podir/lingmo_applet_org.kde.lingmo.wacomtablet.pot
rm -f rc.cpp
