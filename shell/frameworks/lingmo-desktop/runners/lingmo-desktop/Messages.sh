#! /usr/bin/env bash
$EXTRACTRC *.ui >> rc.cpp
$XGETTEXT *.cpp -o $podir/lingmo_runner_lingmo-desktop.pot
rm -f rc.cpp
