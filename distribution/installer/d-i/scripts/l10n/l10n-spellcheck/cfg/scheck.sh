#!/bin/sh
export PATH=${HOME}/trunk/scripts/l10n/l10n-spellcheck:$PATH

case "$1" in
l1)
	l10n-spellcheck.sh ~/trunk/scripts/l10n/l10n-spellcheck/cfg/level1
	;;
l2)
	l10n-spellcheck.sh ~/trunk/scripts/l10n/l10n-spellcheck/cfg/level2
	;;
l3)
	l10n-spellcheck.sh ~/trunk/scripts/l10n/l10n-spellcheck/cfg/level3
	;;
all)
	l10n-spellcheck.sh ~/trunk/scripts/l10n/l10n-spellcheck/cfg/level1
	l10n-spellcheck.sh ~/trunk/scripts/l10n/l10n-spellcheck/cfg/level2
	l10n-spellcheck.sh ~/trunk/scripts/l10n/l10n-spellcheck/cfg/level3
	;;
*)
	echo "Unknown parameter. Here's the list of known parameters:"
	echo ""
	echo "all - useful to force a re-run of the spellcheck"
	echo "l1"
	echo "l2"
	echo "l3"
	;;
esac
