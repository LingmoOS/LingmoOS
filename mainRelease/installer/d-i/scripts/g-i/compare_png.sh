#!/bin/bash
#
# -*-sh-*-
#
#     Copyright (C) 2007 Davide Viti <zinosat@tiscali.it>
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
# 
# the script expects two direcories as argument; 
# it expects "skeleton.html" in the same dir and
# each of the input directories has to be organized this way:
#
# 2.15-1
#  |
#  --- deb
#  |
#  --- udeb
#
# It creates an index.hml file which links other pages.
# I still need to make sure that arguments are right

old=$1
new=$2
cmp="${old}_vs_${new}"

TMPDIR=tmp_html

rm -fr $TMPDIR
mkdir $TMPDIR

names=(
    old_deb:${old}/deb 
    old_udeb:${old}/udeb 
    new_deb:${new}/deb 
    new_udeb:${new}/udeb 
    udeb_diff:${cmp}/udeb 
    deb_diff:${cmp}/deb 
)

#
# Creating pages containing all pngs each
#
for i in $( seq 0 $((${#names[@]}-1)) ) ; do
    file=$( echo ${names[$i]}  | awk -F: '{print $1}' )
    png_dir=$( echo ${names[$i]} | awk -F: '{print $2}' )
    
    LOGFILE=${TMPDIR}/${file}.html
    exec 6>&1
    exec > ${LOGFILE}

    for png in $(find ${png_dir} -name '*.png' | sort) ; do	
	png=$(basename $png)	
        lang=$(echo $png | sed -e 's|diff_||' -e 's|.png||')
	echo    "${lang}"
	echo    "<p><img src=../${png_dir}/${png}></p>"
	echo    "<hr color=\"#eeaaaa\">"
    done
    exec 1>&6 6>&-
done


LOGFILE=${TMPDIR}/tmp.html
exec 6>&1
exec > ${LOGFILE}

echo "<table>"
echo "  <tr>"
echo "   <th class=\"col1\">Language</th>"

echo "   <th class=\"t1\">deb [${old}]</th>"
echo "   <th class=\"t1\">udeb [${old}]</th>"

echo "   <th class=\"t1\">deb [${new}]</th>"
echo "   <th class=\"t1\">udeb [${new}]</th>"

echo "   <th class=\"t1\">deb [${cmp}]</th>"
echo "   <th class=\"t1\">udeb [${cmp}]</th>"
echo " </tr>"

for lang in $(find ${old}/deb/ -name '*.png' | sort) ; do

    lang=$(basename $lang)

    echo "  <tr>"
    echo "   <td class=\"col1\">${lang/.png/}</td>"

    echo "   <td><a href="${old}/deb/${lang}">${lang}</a></td>"
    echo "   <td><a href="${old}/udeb/${lang}">${lang}</a></td>"

    echo "   <td><a href="${new}/deb/${lang}">${lang}</a></td>"
    echo "   <td><a href="${new}/udeb/${lang}">${lang}</a></td>"


if [ -f ${cmp}/deb/diff_${lang} ] ; then
    echo "   <td><a href="${cmp}/deb/diff_${lang}">${lang}</a></td>"
else
    echo "   <td>-</a></td>"
fi

if [ -f ${cmp}/udeb/diff_${lang} ] ; then
    echo "   <td><a href="${cmp}/udeb/diff_${lang}">${lang}</a></td>"
else
    echo "   <td>-</a></td>"
fi

    echo "  </tr>"
done

echo "</table>"

exec 1>&6 6>&-


HTML_DIR=html_files

sed -e "/<!-- HTML TABLE STARTS HERE -->/r ${LOGFILE}" \
    -e "s|<!-- TODAY DATE -->|$(date --utc)|" \
    -e "s|<!-- OLD -->|${old}|" \
    -e "s|<!-- NEW -->|${new}|" \
    skeleton.html > index.html

rm ${TMPDIR}/tmp.html
rm -fr ${HTML_DIR}
mv ${TMPDIR} ${HTML_DIR}
