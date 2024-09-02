#! /bin/sh
# $Id: gen-scale.sh,v 1.7 2022/10/18 21:34:09 tom Exp $
#
# This script generates source variations from scale.c for different datatypes.

UPPER="$1"
MIXED="$2"
DTYPE="$3"
CTYPE="$4"

# The widget provides for setting the digits.  Turn on/off the corresponding
# ifdef.
case $CTYPE in
float|*double)
	float=1
	print=f
	;;
long*long)
	float=0
	print=ll
	;;
unsigned*long)
	float=0
	print=lu
	;;
unsigned)
	float=0
	print=u
	;;
long)
	float=0
	print=l
	;;
*)
	float=0
	print=d
	;;
esac

if test "$print" = "d" ; then
	MODEL=
elif test "$CTYPE" = double; then
	MODEL="d"
else
	MODEL=$print
fi

if test $float = 0 ; then
sed	-e '/^#if <FLOAT>/,/^#endif <FLOAT>/d' \
	-e '/^#if <INT>/d' \
	-e '/^#endif <INT>/d' \
	-e 's/<UPPER>/'"$UPPER"'/g' \
	-e 's/<MIXED>/'"$MIXED"'/g' \
	-e 's/<DTYPE>/'"$DTYPE"'/g' \
	-e 's/<CTYPE>/'"$CTYPE"'/g' \
	-e 's/<MODEL>/'"$MODEL"'/g' \
	-e 's/<PRINT>/'"$print"'/g' \
	"$5"
else
sed	-e '/^#if <INT>/,/^#endif <INT>/d' \
	-e '/^#if <FLOAT>/d' \
	-e '/^#endif <FLOAT>/d' \
	-e 's/<UPPER>/'"$UPPER"'/g' \
	-e 's/<MIXED>/'"$MIXED"'/g' \
	-e 's/<DTYPE>/'"$DTYPE"'/g' \
	-e 's/<CTYPE>/'"$CTYPE"'/g' \
	-e 's/<MODEL>/'"$MODEL"'/g' \
	-e 's/<PRINT>/'"$print"'/g' \
	"$5"
fi
