#! /usr/bin/env bash

$EXTRACTRC --context="Category" --tag-group=none --tag=Name `find libdiscover -name "*-categories.xml"` >> rc.cpp
$XGETTEXT rc.cpp `find libdiscover -name \*.cpp -o -name \*.qml` -o $podir/libdiscover.pot
$XGETTEXT `find discover -name \*.cpp -o -name \*.qml -o -name \*.js` -o $podir/lingmo-discover.pot
$XGETTEXT `find notifier -name \*.cpp` -o $podir/lingmo-discover-notifier.pot
$XGETTEXT `find kcm -name \*.cpp -o -name \*.qml` -o $podir/kcm_updates.pot
