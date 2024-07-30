#! /usr/bin/env bash
$XGETTEXT `find lingmosearch/ plugininstaller -name "*.cpp" -o -name "*.h" -o -name "*.qml"` -o $podir/kcm_lingmosearch.pot
