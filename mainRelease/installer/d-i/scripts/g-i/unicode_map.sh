#!/bin/bash
# -*-sh-*-
#
#     Copyright (C) 2005 Davide Viti <zinosat@tiscali.it>
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
# showttf was grabbed from:
#     http://cvs.sourceforge.net/viewcvs.py/fontforge/fontforge/fonttools/showttf.c
#
# to compile it:
#     gcc -o showttf showttf.c
#
# unicode_map.sh <ttf files path> 
#
# Ex: FORMAT=xml sh unicode_map.sh ~/g-i/installer/build/tmp/gtk-miniiso/tree/usr/share/fonts/truetype/
#
# produces various files (needs to be cleaned up)
# 
# map.txt shows which font files contain a particular glyph (the glyph listed are at least on one
# ttf file: i.e. it's not from U+0000 to U+ffff)
#
# the colums correspond (same order) to the font names listed in the header

create_css() {
exec 7>&1 
exec > ${CSS_FILE}
echo "${TABLETAG} {	display: table; }"
echo "${ROWTAG} { display: table-row; }"
echo ""
echo "${COORDTAG}"
echo "{"
echo "	display: table-cell;"
echo "	vertical-align:middle;"
echo "	font-size: 8pt;"
echo "	border-width:thin;"
echo "}"
echo ""
echo "${FOUNDTAG}"
echo "{"
echo "	border-style:solid;"
echo "	border-color: green; "
echo "	empty-cells: show;"
echo "	display: table-cell;"
echo "	vertical-align:top;"
echo "	font-size: 9pt;"
echo "	background-color: black;"
echo "	color: black;"
echo "}"
echo ""
echo "${NOTFOUNDTAG}"
echo "{"
echo "	border-style:solid;"
echo "	border-color: green; "
echo "	empty-cells: show;"
echo "	display: table-cell;"
echo "	vertical-align:top;"
echo "	font-size: 9pt;"
echo "	background-color: white;"
echo "	color: white;"
echo "}"
exec 1>&7 7>&-
}

if type showttf >/dev/null 2>&1; then
    SHOWTTF="showttf"
elif [ -f ./showttf ] ; then
    SHOWTTF="./showttf"
else
    echo "Error: showttf is needed to run this script"
    exit 1
fi

if [ ! -d "$1" ] ; then
    echo "Error: directory '$1' does not exist"
    exit 1
fi

[ "${FORMAT}" ] || export FORMAT=text

rm -fr ranges all_ranges.txt rangefiles.txt
mkdir ranges

FONTFILES="$(find $1 -name "*.ttf")"
if [ -z "$FONTFILES" ] ; then
    echo "No .ttf files found under '$1'"
    exit 1
fi

# associate a .lst file to each .ttf file
# the lst file contains a sorted list of the unicode coordinates contained in the ttf file
tot_size=0
for FONTFILE in ${FONTFILES} ; do
    RANGEFILE="ranges/$(basename ${FONTFILE} | sed "s:\.ttf$:.lst:")"
    echo $RANGEFILE >> rangefiles.txt
    ${SHOWTTF} ${FONTFILE} | grep "^Glyph" | awk '{print $7}' | sort | uniq | sed "s|^U+||" > ${RANGEFILE}

    nglyphs=$(wc -l ${RANGEFILE} | awk '{print $1}')
    size=$(ls -la ${FONTFILE} | awk '{print $5}')
    tot_size=`expr $tot_size + $size`
    echo "$(basename ${FONTFILE}): ${nglyphs} glyphs   [${size} bytes]"
    cat ${RANGEFILE} >> all_ranges.txt
done

# create a list of the unicode coords used at least on one font file
sort all_ranges.txt | uniq > avail_codes.lst

echo ""
echo "Font files: $(wc -l rangefiles.txt | awk '{print $1}')"
echo "Total size: $tot_size bytes"
echo "Unique Glyphs: $(wc -l avail_codes.lst | awk '{print $1}')"
echo "Bytes per glyph: $(expr $tot_size / $(wc -l avail_codes.lst | awk '{print $1}'))"

# For each unicode coordinate used in one or more ttf files produce this stats
#
# 0000 1 0 1 0 1
# 0020 0 0 0 1 1 
echo ""
echo "Generating unicode map file..."

i=0
num_loops=5
ranges=$(cat rangefiles.txt)

exec 6>&1 
if [ "${FORMAT}" = "xml" ] ; then
    exec > map.xml
else
    exec > map.txt
fi


if [ "${FORMAT}" = "xml" ] ; then

    CSS_FILE="unicode_map.css"

    TABLETAG="unicode_table"
    ROWTAG="glyph_row"
    COORDTAG="coord"
    FOUNDTAG="full"
    NOTFOUNDTAG="empty"

    create_css

    echo "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>"
    echo "<?xml-stylesheet type=\"text/css\" href=\"${CSS_FILE}\"?>"

    echo "<${TABLETAG}>"
else
    EGLYPHTAG=":"
    ECELLTAG=":"
fi

for COORD in $(cat avail_codes.lst) ; do
    echo -n "<${ROWTAG}><${COORDTAG}>${COORD}</${COORDTAG}>"

    for RANGEFILE in $ranges ; do
	grep --mmap -F -q -w ${COORD} ${RANGEFILE}
	if [ $? = 0 ] ; then
	    echo -n "<${FOUNDTAG}>$?</${FOUNDTAG}>"
	else
	    echo -n "<${NOTFOUNDTAG}>$?</${NOTFOUNDTAG}>"
	fi
    done

    echo "</${ROWTAG}>"

##
## For profiling: 
##                time * num_lines(avail_codes.lst) / num_loops
##

#    i=`expr $i + 1`
#    if [ $i -eq ${num_loops} ] ; then
#	 if [ "${FORMAT}" = "xml" ] ; then
#	     echo "</${TABLETAG}>"
#	 fi
#	 exit 0
#    fi
done

if [ "${FORMAT}" = "xml" ] ; then
    echo "</${TABLETAG}>"
fi

exec 1>&6 6>&-

## FIXME: filenames need to be shown on a separate top-frame
# cat rangefiles.txt | sed "s|ranges/||" > tmp.1
# mv tmp.1 rangefiles.txt
#cat rangefiles.txt tbl.txt | sed "s|:$||" > map.txt
