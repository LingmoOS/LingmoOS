#! /usr/bin/env bash
$EXTRACTRC `find . -name "*.ui"` >> rc.cpp
$XGETTEXT `find . -name "*.cpp"` -o $podir/kpeople6.pot
