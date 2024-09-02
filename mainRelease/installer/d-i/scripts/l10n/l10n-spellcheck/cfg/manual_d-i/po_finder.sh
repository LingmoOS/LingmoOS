# Usage: pof_di-manual  <D-I repository path> <lang>
#
# This module selects a list of (po/pot) files for
# the debian installer manual 

MANUAL_COPY=$1
LANG=$2

# English master file "template.pot" (before we found "en.po")
if [ $LANG = en ] ; then
    find $MANUAL_COPY/pot -name "*.pot"
else
    find $MANUAL_COPY/$LANG -name "*.po"
fi

