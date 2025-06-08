#!/usr/bin/env bash

$XGETTEXT `find . -name "*.cpp" -o -name "*.qml"` -L Java -o $podir/kscreen.pot
