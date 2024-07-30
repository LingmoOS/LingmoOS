#! /bin/sh
$XGETTEXT *.cpp -o $podir/libkcalcore.pot
if ( test -e $podir/libkcalcore.pot )
then
  echo "Error: kdepimlibs/kcalcore library should NOT have i18n strings."
fi
