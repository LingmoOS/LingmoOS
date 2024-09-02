# Usage: po_finder  <D-I repository path> <lang>
#
# This module selects a list of (po/pot) files for
# the debian installer manual 

DI_COPY=$1
LANG=$2

if [ ${LANG} = en ] ; then
    find ${DI_COPY}/POT -name "*.pot"
else
    find ${DI_COPY}/files/${LANG} -name "*.po"
fi
