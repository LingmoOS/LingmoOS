#!/bin/bash
#
# -*-sh-*-
#
#     Copyright (C) 2004-2005 Davide Viti <zinosat@tiscali.it>
# 
#     This program is free software; you can redistribute it and/or modify
#     it under the terms of the GNU General Public License as published by
#     the Free Software Foundation; either version 2 of the License, or
#     (at your option) any later version.
# 
#     This program is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#     GNU General Public License for more details.
# 
#     You should have received a copy of the GNU General Public
#     License along with this program; if not, write to the Free
#     Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA 02111-1307 USA
#

SCRIPT_WITHOUT_PATH=`basename $0`
SCRIPTS_PATH=`echo $0 | sed "s:\(.*\)${SCRIPT_WITHOUT_PATH}\(.*\):\1\2:"`

if [ $# -ne 1 ]; then
    echo "Usage: $0 <cfg path>"
    exit 1
fi


export CFG_DIR=$1
if [ ! -d ${CFG_DIR} ] ; then
    echo "${CFG_DIR} does not exist"
    exit 1
fi

. ${CFG_DIR}/setup.sh
export LANGUAGE_LIST="${CFG_DIR}/lang2dict.txt"
export WLS_PATH="${CFG_DIR}/wls"
export PO_FINDER="${CFG_DIR}/po_finder.sh"
export SPECIFIC_CHECK="${CFG_DIR}/specific.pl"
export HTML_PAGE="${CFG_DIR}/report_page.html"

if [ -z ${LC_ALL} ] ; then
    export LC_ALL="C"
fi

NEW="check_$(date '+%Y%m%d')"
WORK_DIR=`mktemp -u -d`

# update local copy of the repository (if necessary)
${REFRESH_CMD}

# spellcheck and move data to public_html
check_all.sh ${LOCAL_REPOSITORY} ${WORK_DIR}

umask 0002

BRAND_NEW=0
if [ ! -d ${OUT_DIR} ] ; then
    mkdir -p ${OUT_DIR}/history
    BRAND_NEW=1
fi

OLD_PATH=${OUT_DIR}/latest/nozip
NEW_PATH=${WORK_DIR}/nozip

if [ ${BRAND_NEW} -ne 1 ] ; then
    for ROW in `sed 's: :,:g' ${WORK_DIR}/stats.txt`; do
	LANG=`echo ${ROW} | awk -F, '{print $1}'`
	VAL=`echo ${ROW} | awk -F, '{print $2}'`
	
	LALL=${LANG}_all.txt
	LUWL=${LANG}_unkn_wl.txt
	LVAR=${LANG}_var.txt
	LSPC=${LANG}_lspec.txt

	for TO_DIFF in ${LALL} ${LUWL} ${LVAR} ${LSPC}; do
	    if [ -f ${OLD_PATH}/${TO_DIFF} -o -f ${NEW_PATH}/${TO_DIFF} ] ; then
		
		diff -U 0 -N \
		    --label old ${OLD_PATH}/${TO_DIFF} \
		    --label new ${NEW_PATH}/${TO_DIFF} > \
		    ${NEW_PATH}/${TO_DIFF/.txt/.diff}
	fi
	done

    done
fi

# remove empty files
find ${NEW_PATH} -empty -exec rm '{}' ';'

# build "index.html" with the new results
export DIFF_DIR=${WORK_DIR}
${SCRIPTS_PATH}/build_index.sh ${WORK_DIR}/stats.txt ${HTML_PAGE} ${WORK_DIR}/index.html

SAVED_STATS=`echo ${NEW}.txt | sed "s:check_:stats_:"`
cp ${WORK_DIR}/stats.txt ${OUT_DIR}/history/${SAVED_STATS}
mv ${WORK_DIR}/index.html ${OUT_DIR}

#echo ""
#echo "***  $SAVED_STATS  ***"

if [ ${BRAND_NEW} -ne 1 ] ; then    
    diff_stats.sh ${OUT_DIR}/latest/stats.txt ${WORK_DIR}/stats.txt

    LATEST=`ls -l ${OUT_DIR}/latest | sed "s:.*-> ::"`
    rm ${OUT_DIR}/latest 
    rm -fr $LATEST
else
    echo "${OUT_DIR} created"
fi

mv ${WORK_DIR} ${OUT_DIR}/${NEW}
cd ${OUT_DIR}
ln -s ${NEW} latest
