#!/bin/bash
#
# Build word lists: each wl is composed by words common to all languages
# AND words specific for each language:
#
# ${LANG}_wl = common_wl + ${LANG}_wl
#
# Davide Viti <zinosat@tiscali.it> 2005, for the Debian Project
LANGUAGE_LIST=lang2dict.txt
COMMON_WL=./wls/di_common_wl.txt

for LANGUAGE in `cat ${LANGUAGE_LIST} | sed "s:\(^#.*\)::"`; do
    LANG=`echo  ${LANGUAGE} | awk -F, '{print $1}'`
    DICT=`echo  ${LANGUAGE} | awk -F, '{print $2}'`
    
    if [ ${#DICT} -gt 0 ] ; then    
	SPECIFIC_WL=./wls/${LANG}_wl.txt
	WLIST=./wls/${LANG}_di_wl
    
	WL_WARN=0
	if [ ! -f ${COMMON_WL} ] ; then
	    echo "can't find di_common_wl.txt. You'll get a lot of unknown technical words!"
	    COMMON_WL=
	    WL_WARN=`expr ${WL_WARN} + 1`
	fi
	
	if [ ! -f ${SPECIFIC_WL} ] ; then
	    echo "can't find ${LANG}_wl.txt. You'll get a lot of unknown words in \"${LANG}\" language!"
	    SPECIFIC_WL=
	    WL_WARN=`expr ${WL_WARN} + 1`    
	fi
	
# build wl only if there's at least one of the "private" wls
# "| perl -00lne'/\n/&&print'" would remove blank lines from wl: is it needed?
	if [ ${WL_WARN} -ne 2 ] ; then
	    echo "[${LANG}]"
	    cat ${COMMON_WL} ${SPECIFIC_WL} | sort -f | sed "s:\(^#.*\)::" | grep -v ^$ > ${WLIST}.txt
	    
# NB: --lang uses $LANG and not $DICT
	    aspell --lang=${LANG} create master ${WLIST} < ${WLIST}.txt
	    rm ${WLIST}.txt
	fi	
    fi
done
