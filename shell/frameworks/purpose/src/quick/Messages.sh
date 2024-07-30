#!/bin/sh
$XGETTEXT $(find . -name "*.cpp" -o -name "*.h" -o -name "*.qml") -o $podir/libpurpose6_quick.pot
