#! /bin/sh
$XGETTEXT `find . -name "*.cpp" -o -name "*.qml"` -o $podir/cuttlefish_editorplugin.pot
