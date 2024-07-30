#! /bin/sh

# SPDX-License-Identifier: CC0-1.0
# SPDX-FileCopyrightText: none

$EXTRACTRC `find -name \*.ui -o -name \*.rc -o -name \*.kcfg` >> rc.cpp || exit 11
$XGETTEXT `find -name \*.cpp -o -name \*.h  -o -name \*.qml` -o $podir/polkit-kde-authentication-agent-1.pot
rm -f rc.cpp
