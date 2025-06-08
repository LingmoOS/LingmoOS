#!/usr/bin/env bash

$XGETTEXT `find . -name "*.cpp" -o -name "*.qml"` -o $podir/kscreen_common.pot
