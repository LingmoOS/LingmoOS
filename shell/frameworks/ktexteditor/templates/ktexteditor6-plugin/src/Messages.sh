#! /usr/bin/env bash
$XGETTEXT `find . -name \*.cpp` -o $podir/%{APPNAMELC}.pot
