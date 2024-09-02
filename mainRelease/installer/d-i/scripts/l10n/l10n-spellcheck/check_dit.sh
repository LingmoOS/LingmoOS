#!/bin/bash
#
# -*-sh-*-
#
#     Copyright (C) 2004-2006 Davide Viti <zinosat@tiscali.it>
#
#     
#     This program is free software; you can redistribute it and/or
#     modify it under the terms of the GNU General Public License
#     as published by the Free Software Foundation; either version 2
#     of the License, or (at your option) any later version.
#     
#     This program is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#     GNU General Public License for more details.
#     
#     You should have received a copy of the GNU General Public License
#     along with this program; if not, write to the Free Software
#     Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
#

usage() {
    echo  "Usage:"
    echo  "$0 <lang> <aspell-suffix> <D-I repository path> <output dir>"
}

initialise() {
CHECK_VAR=check_var.pl

ALL_STRINGS=${DEST_DIR}/${LANG}_all.txt
FILES_TO_KEEP="${ALL_STRINGS} ${FILES_TO_KEEP}"

NO_VARS=${DEST_DIR}/1_no_vars_${LANG}

ALL_UNKNOWN=${DEST_DIR}/2_all_unkn_${LANG}
NEEDS_RM="${ALL_UNKNOWN} ${NEEDS_RM}"

UNKN=${DEST_DIR}/${LANG}_unkn_wl.txt
FILES_TO_KEEP="${UNKN} ${FILES_TO_KEEP}"

SUSPECT_VARS=${DEST_DIR}/${LANG}_var.txt
LANG_SPECIFIC=${DEST_DIR}/${LANG}_lspec.txt
}

checks(){

if [ ! -d ${BASE_SEARCH_DIR} ] ; then
    echo ${BASE_SEARCH_DIR} does not exist
    exit 1
fi

if [ ! -d ${DEST_DIR} ] ; then
    mkdir ${DEST_DIR}
fi

}

if [ -z "$4" ] ; then
    usage
    exit 1
fi

LANG=$1
DICT=$2
BASE_SEARCH_DIR=$3
DEST_DIR=$4

initialise	# initialise some variables
checks		# do an environment check

PO_FILE_LIST="${DEST_DIR}/${LANG}_file_list.txt"
NEEDS_RM="${PO_FILE_LIST} ${NEEDS_RM}"

sh ${PO_FINDER} ${BASE_SEARCH_DIR} ${LANG} > ${PO_FILE_LIST} 2> /dev/null

# Check if exist any po file for $LANG 
if [ $(wc -c ${PO_FILE_LIST} | cut -d ' ' -f1) = 0 ] ; then
    rm ${PO_FILE_LIST}
    exit 1
fi

rm -f ${ALL_STRINGS}

while read LANG_FILE; do
    ENC=$(grep -e "^\"Content-Type:" ${LANG_FILE} | sed 's|.*charset=\(.*\)\\n\"$|\1|')

    echo "${LANG_FILE}" | grep -e ".po$" > /dev/null
    if  [ $? = 0 ] ; then

	echo ${ENC} | grep -iw "utf-8" > /dev/null
	if [ $? = 0 ]  ; then
	    if [ ${HANDLE_SUSPECT_VARS} = "yes" ] ; then
		${CHECK_VAR} -s ${LANG_FILE} >> ${SUSPECT_VARS}

	        # Level-specific check (only for po files)
		if [ -f ${SPECIFIC_CHECK} ] ; then
		    ${SPECIFIC_CHECK} ${LANG_FILE} ${LANG} >> ${LANG_SPECIFIC}
		fi
	    fi
	    extract_msg.pl -msgstr ${LANG_FILE} >> ${ALL_STRINGS}
	else
	    if [ ${HANDLE_SUSPECT_VARS} = "yes" ] ; then
		${CHECK_VAR} -s ${LANG_FILE} | iconv --from ${ENC} --to utf-8 >> ${SUSPECT_VARS}

	        # Level-specific check (only for po files)
		if [ -f ${SPECIFIC_CHECK} ] ; then
		    ${SPECIFIC_CHECK} ${LANG_FILE} ${LANG} | iconv --from ${ENC} --to utf-8 >> ${LANG_SPECIFIC}
		fi
	    fi
	    extract_msg.pl -msgstr ${LANG_FILE} | iconv --from ${ENC} --to utf-8 >> ${ALL_STRINGS}
	fi

    else			# now deal with ".pot" files
	if [ ${HANDLE_SUSPECT_VARS} = "yes" ] ; then
	    ${CHECK_VAR} -s ${LANG_FILE} >> ${SUSPECT_VARS}
	fi
	extract_msg.pl -msgid ${LANG_FILE} >> ${ALL_STRINGS}
    fi
done < ${PO_FILE_LIST}

#remove empty files
find ${DEST_DIR} -empty -exec rm '{}' ';'

if [ ${HANDLE_SUSPECT_VARS} = "yes" ] ; then
    if [ -f ${SUSPECT_VARS} ]; then
	FILES_TO_KEEP="${SUSPECT_VARS} ${FILES_TO_KEEP}" 
	NUM_SUSPECT=$(msgfmt -o /dev/null --statistics ${SUSPECT_VARS} 2>&1 | sed -e "s|^\([0-9]*\) .*|\1|")
    else
	NUM_SUSPECT=0
    fi
fi

if [ -f ${LANG_SPECIFIC} ]; then
    FILES_TO_KEEP="${LANG_SPECIFIC} ${FILES_TO_KEEP}" 
    NUM_SPECIFIC=$(msgfmt -o /dev/null --statistics ${LANG_SPECIFIC} 2>&1 | sed -e "s|^\([0-9]*\) .*|\1|")
else
    NUM_SPECIFIC=0
fi

# remove ${ALL_THESE_VARIABLES} if they do not need to be spell checked
# remove %s, %c, %d (to get better count of unicode points)
if [ ${REMOVE_VARS} = "yes" ] ; then
    NEEDS_RM="${NO_VARS} ${NEEDS_RM}"
    grep -e "^-" ${ALL_STRINGS} | \
    sed -e s/\$\{[a-zA-Z0-9_]*\}//g > ${NO_VARS}

    FILE_TO_CHECK=${NO_VARS}
else
    FILE_TO_CHECK=${ALL_STRINGS}
fi

# Find unicode points
if [ ${REMOVE_VARS} = "yes" ] ; then
    FILE_CODEPOINTS=${FILE_TO_CHECK}
else
    # remove header
    FILE_CODEPOINTS=${DEST_DIR}/temp-codes.txt
    sed "1,2d" ${FILE_TO_CHECK} > ${FILE_CODEPOINTS}
fi

    sed -e "s|^- \"\(.*\)\"$|\1|" \
	-e "s|\$TCPIP|TCPIP|" \
	-e "s/%l*[scdbniuBFNS]//g" \
	-e "s|\\\\\"|\"|g" \
	-e "s|\\\n||g" ${FILE_CODEPOINTS} > ${DEST_DIR}/fully_stripped.txt

    iconv -f utf8 -t ucs-4le ${DEST_DIR}/fully_stripped.txt | \
	od -v -tx2 -An -w2 | \
	sed "s|\(....\)$|<U\1>|" > ${DEST_DIR}/${LANG}_codes1.txt

    sort ${DEST_DIR}/${LANG}_codes1.txt | \
	uniq -c | \
	grep -vw "<U0000>" | \
	grep -vw "<U000a>" > ${DEST_DIR}/${LANG}_codes.txt

# Add a third column with the encoded character
#  567 - <U0033> "3"
    sed "s|\(.* \)\(<U....>\)|\1\2  \"\2\" |" ${DEST_DIR}/${LANG}_codes.txt | \
	uxx2utf > ${DEST_DIR}/${LANG}_codes1.txt 2> /dev/null

    mv ${DEST_DIR}/${LANG}_codes1.txt ${DEST_DIR}/${LANG}_codes.txt

    FILES_TO_KEEP="${DEST_DIR}/${LANG}_codes.txt ${FILES_TO_KEEP}" 
    CODEPOINTS=$(wc -l ${DEST_DIR}/${LANG}_codes.txt | awk '{print $1}')
    
    awk '{print $2}' ${DEST_DIR}/${LANG}_codes.txt >> ${DEST_DIR}/all_codes-tmp.txt
    rm -f ${DEST_DIR}/fully_stripped.txt

    if [ ${REMOVE_VARS} != "yes" ] ; then
	rm ${FILE_CODEPOINTS}
    fi

if [ ${DICT} != "null" ] ; then
	EXTRADICT=`mktemp`
	cat ${WLS_PATH}/${LANG}_wl.txt  ${WLS_PATH}/di_common_wl.txt 2>/dev/null | \
		grep -E -v "^#.*" | \
		grep -v -E "^$" | \
		aspell --lang=${DICT} --encoding=utf-8 --dont-validate-words create master ${EXTRADICT}
	if [ -f ${EXTRADICT} ] ; then
	WL_PARAM="--add-extra-dicts ${EXTRADICT}"
	fi
# spell check the selected strings eventually using a custom wl 
    aspell list --dont-validate-words --lang=${DICT} --encoding=utf-8 ${WL_PARAM} ${ASPELL_EXTRA_PARAM} < ${FILE_TO_CHECK} > ${ALL_UNKNOWN}

# sort all the unrecognized words (don't care about upper/lower case)
# count duplicates
# take note of unknown words
    sort -f ${ALL_UNKNOWN} | uniq -c > ${UNKN}

# if we're *not* handling suspecet vars (i.e. d-i manual), make this an empty string
    if [ ${HANDLE_SUSPECT_VARS} = "no" ] ; then
	NUM_SUSPECT=-1
    fi

# build the entry of stats.txt for the current language (i.e "395 it 1 134 0")
    echo ${LANG} $(wc -l ${UNKN} | awk '{print $1}') ${NUM_SUSPECT} ${CODEPOINTS} ${NUM_SPECIFIC} >> ${DEST_DIR}/stats.txt
else
    if [ ${HANDLE_SUSPECT_VARS} = "no" ] ; then
	NUM_SUSPECT=-1
    fi

    echo "${LANG} -1 ${NUM_SUSPECT} ${CODEPOINTS} ${NUM_SPECIFIC}" >> ${DEST_DIR}/stats.txt

    NEEDS_RM=`echo ${NEEDS_RM} | sed "s:${ALL_UNKNOWN}::"`
    FILES_TO_KEEP=`echo ${FILES_TO_KEEP} | sed "s:${UNKN}::"`
fi

rm ${EXTRADICT} >/dev/null 2>&1

rm ${NEEDS_RM}

if [ ! -d  ${DEST_DIR}/zip ] ; then
    mkdir ${DEST_DIR}/zip
fi

if [ ! -d  ${DEST_DIR}/nozip ] ; then
    mkdir ${DEST_DIR}/nozip
fi

# in the tgz file WL is iso-8859-1 (the way it has to be)
# in "nozip" it's utf-8 like the other files 
if [ -f ${WLS_PATH}/${LANG}_wl.txt ] ; then
    WORDLIST=${WLS_PATH}/${LANG}_wl.txt
    iconv --from iso-8859-1 --to utf-8 ${WORDLIST} > ${DEST_DIR}/nozip/${LANG}_wl.txt
    cp ${WORDLIST} ${DEST_DIR}
    WL_ISO8859=${DEST_DIR}/${LANG}_wl.txt
fi

# now let's copy the common wl *only* to nozip
if [ -f ${WLS_PATH}/di_common_wl.txt ] ; then
    cp ${WLS_PATH}/di_common_wl.txt ${DEST_DIR}/nozip
fi

pushd ${DEST_DIR} > /dev/null

FILES_TO_KEEP=`echo ${FILES_TO_KEEP} | sed "s|${DEST_DIR}\/||g"`
WL_ISO8859=`echo ${WL_ISO8859} | sed "s|${DEST_DIR}\/||g"`
tar czf zip/${LANG}.tar.gz ${FILES_TO_KEEP} ${WL_ISO8859}

mv ${FILES_TO_KEEP} ${DEST_DIR}/nozip
rm -f ${WL_ISO8859}

popd > /dev/null

echo "AddCharset UTF-8 .txt" > ${DEST_DIR}/nozip/.htaccess
