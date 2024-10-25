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
#
# Convert a list of unicode coordinates into a set of unicode ranges
# 
# 0000
# 000C
# 0010
# 0011
# 0012
#
# becomes: 0000 000C 0010:0012
#

RANGEFILE=$1

glyphs=($(cat ${RANGEFILE}))
num_glyphs=${#glyphs[@]}
glyphs[$num_glyphs]=0

start=0
end=$start

for ((j=`expr $start + 1`;j<=${num_glyphs};j+=1)); do
    if  [ $(printf "%d" 0x${glyphs[$j]}) -eq `expr $(printf "%d" 0x${glyphs[$end]}) + 1` ] ; then
	end=$j
    else
	if [ $end -eq $start ] ; then
	    echo -n "u${glyphs[$start]} "
	else
	    echo -n "u${glyphs[$start]}:u${glyphs[$end]} "
	fi
	start=$j
	end=$j
    fi
done

echo ""

