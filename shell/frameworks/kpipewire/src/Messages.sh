#!/bin/sh

# Extract strings from all source files.
# If your framework depends on KI18n, use $XGETTEXT. If it uses Qt translation
# system, use $EXTRACT_TR_STRINGS.
$XGETTEXT `find . -name \*.cpp -o -name \*.h` -o $podir/kpipewire6.pot
