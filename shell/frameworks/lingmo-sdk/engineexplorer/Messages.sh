#! /usr/bin/env bash
$EXTRACTRC *.ui >> rc.cpp
$XGETTEXT *.cpp *.h -o $podir/lingmoengineexplorer.pot
rm -f rc.cpp

