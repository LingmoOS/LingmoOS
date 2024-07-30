#! /usr/bin/env bash
for file in data/*.dat
do
  awk -F'|' '$0 ~ /\|/ {
                         print "// i18n: file: '`basename $file`':"NR;
                         printf("i18nc(\"%s\", \"%s\");\n", $1, $2)
                       }' $file >> rc.cpp
done

$XGETTEXT `find . -name \*.js -o -name \*.qml -o -name \*.cpp` -o $podir/plasma_applet_org.kde.plasma.weather.pot
rm -f rc.cpp
