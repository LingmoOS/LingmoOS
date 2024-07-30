#! /usr/bin/env bash
$XGETTEXT `find . -name \*.qml` `find ../sddm-theme -name \*.qml` -o $podir/lingmo_lookandfeel_org.kde.lookandfeel.pot
