#!/bin/sh
# $Id: fselect.sh,v 1.6 2022/10/19 00:21:44 tom Exp $

#
# Description:
#	This demonstrates the CDK command line
#	interface to the file selection widget.
#

#
# Create some global variables.
#
CDK_FSELECT="${CDK_BINDIR=..}/cdkfselect"
CDK_LABEL="${CDK_BINDIR=..}/cdklabel"

directory="."
label="File: "
title="<C>Select a file"

buttons=" OK 
 Cancel "

xpos="CENTER"
ypos="CENTER"

width=0
height=-5

tmp="${TMPDIR=/tmp}/tmp.$$"
file="${TMPDIR=/tmp}/fs.$$"

#
# Chop up the command line.
#
if set -- `getopt d:L:T:X:Y:W:H: "$@"`
then
for c in "$@"
do
    case $c in
         -d) directory=$2; shift 2;;
         -T) title=$2; shift 2;;
         -L) label=$2; shift 2;;
         -X) xpos=$2; shift 2;;
         -Y) ypos=$2; shift 2;;
         -W) width=$2; shift 2;;
         -H) height=$2; shift 2;;
         --) shift; break;;
    esac
done
else
    echo "Usage: $0 [-d dir] [-L label] [-T title] [-X xpos] [-Y ypos] [-W width] [-H height]"
    exit 1
fi

#
# Create the CDK file selector.
#
${CDK_FSELECT} -d "${directory}" -T "${title}" -L "${label}" -X "${xpos}" -Y "${ypos}" -W "${width}" -H "${height}" -B "${buttons}" 2> "${file}"
selected=$?
test $selected = 255 && exit 1

answer=`cat "${file}"`

#
# Display the file the user selected.
#
cat >"${tmp}" <<EOF
<C>You selected the following file

<C><#HL(10)>
<C></B>${answer}
<C><#HL(10)>

<C>You chose button #${selected}

<C>Press </R>space<!R> to continue.
EOF

${CDK_LABEL} -f "${tmp}" -p " "

#
# Clean up.
#
rm -f "${tmp}" "${file}"
