#! /usr/bin/env bash
$EXTRACTRC `find . -name \*.ui` >> rc.cpp || exit 11
$XGETTEXT `find . -name "*.cpp" -o -name "*.cc" | grep -v "/tests"` -o $podir/kwalletd6.pot
