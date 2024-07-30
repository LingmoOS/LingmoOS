#! /usr/bin/env bash
$EXTRACTRC *.ui >> rc.cpp
$XGETTEXT *.cpp *.h -o $podir/plasmaengineexplorer.pot
rm -f rc.cpp

