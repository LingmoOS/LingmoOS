#! /usr/bin/env bash
$EXTRACTRC *.ui >> rc.cpp || exit 11
$XGETTEXT *.cpp -o $podir/lingmo-kcm_kwintabbox.pot
rm -f rc.cpp
