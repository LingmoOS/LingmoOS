# Usage: po_finder  <repository path> <lang>
#

DI_COPY=$1
LANG=$2

# English master file "template.pot" (before we found "en.po")
if [ ${LANG} = en ] ; then
    i# echo "${DI_COPY}/master/template.pot"
    find ${DI_COPY}/POT -name "*.pot"
    exit
fi

# look for a master file
if [ -f ${DI_COPY}/master/${LANG}.po ] ; then
    echo "${DI_COPY}/master/${LANG}.po"
else # master file not available: look for sparse po files
    find ${DI_COPY}/files/${LANG} -name "*.po"
fi
