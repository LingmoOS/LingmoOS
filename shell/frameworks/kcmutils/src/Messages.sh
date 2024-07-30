#!/bin/sh

# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: CC0-1.0

# Invoke the extractrc script on all .ui, .rc, and .kcfg files in the sources.
# The results are stored in a pseudo .cpp file to be picked up by xgettext.
lst=`find . -name \*.rc -o -name \*.ui -o -name \*.kcfg`
if [ -n "$lst" ] ; then
    $EXTRACTRC $lst >> rc.cpp
fi

# Extract strings from all source files.
# If your framework depends on KI18n, use $XGETTEXT. If it uses Qt translation
# system, use $EXTRACT_TR_STRINGS.
$XGETTEXT ` find -not -path "./kcmshell/*" -name \*.cpp -o  -not -path "./kcmshell/*" -name \*.h -o -name \*.qml` -o $podir/kcmutils6.pot
