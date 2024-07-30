#! /usr/bin/env bash
$EXTRACTRC *.ui >> rc.cpp || exit 11
$XGETTEXT `find . -name \*.cpp -o -name \*.qml` -o $podir/kwin_effect_cube.pot
