#! /usr/bin/env bash

# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: CC0-1.0

$EXTRACTRC `find . -name \*.ui -o -name \*.rc -o -name \*.kcfg` >> rc.cpp
$XGETTEXT `find . -name \*.cpp -o -name \*.qml` -o $podir/systemsettings.pot
rm -f rc.cpp
