#! /usr/bin/env bash
$EXTRACTRC *.ui >> rc.cpp
$XGETTEXT *.cpp -o $podir/lingmo_runner_kwin.pot
rm -f rc.cpp
