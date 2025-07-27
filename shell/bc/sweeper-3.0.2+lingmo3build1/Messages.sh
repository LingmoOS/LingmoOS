#! /bin/sh
$EXTRACTRC *.rc *.ui >> rc.cpp
$XGETTEXT *.cpp *.h -o $podir/sweeper.pot
