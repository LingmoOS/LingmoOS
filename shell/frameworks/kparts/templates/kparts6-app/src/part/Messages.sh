#! /usr/bin/env bash
$EXTRACTRC `find . -name \*.rc` >> rc.cpp
$XGETTEXT `find . -name \*.cpp` -o $podir/%{APPNAMELC}part.pot
