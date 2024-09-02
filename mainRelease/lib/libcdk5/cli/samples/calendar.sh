#!/bin/sh
# $Id: calendar.sh,v 1.4 2022/10/18 23:14:12 tom Exp $

#
# Description:
#		This demonstrates the CDK command line
# interface to the celendar widget.
#

#
# Create some global variables.
#
CDK_CALENDAR="${CDK_BINDIR=..}/cdkcalendar"
CDK_LABEL="${CDK_BINDIR=..}/cdklabel"

date="${TMPDIR=/tmp}/cal.$$"
tmp="${TMPDIR=/tmp}/tmp.$$"

xpos=CENTER
ypos=CENTER

#
# Get today's date.
#
day=`date +%d`
month=`date +%m`
year=`date +%Y`

#
# Chop up the command line.
#
if set -- `getopt d:m:y:X:Y: "$@"`
then
for c in "$@"
do
    case $c in
         -d) day=$2; shift 2;;
         -m) month=$2; shift 2;;
         -y) year=$2; shift 2;;
         -X) xpos=$2; shift 2;;
         -Y) ypos=$2; shift 2;;
         --) shift; break;;
    esac
done
else
   echo "Usage: $0 [-d day] [-m mon] [-y year] [-X xpos] [-Y ypos]"
   exit 2
fi

#
# Create the title and buttons.
#
title="<C><#HL(22)>
<C>Select a date
<C><#HL(22)>"
buttons=" OK 
Cancel "

#
# Create the calendar widget.
#
${CDK_CALENDAR} -B "${buttons}" -d "${day}" -m "${month}" -y "${year}" -T "${title}" -X "${xpos}" -Y "${ypos}" -O "${date}" -S
selected=$?
test $selected = 255 && exit 1

answer=`cat "${date}"`

#
# Create the message for the label widget.
#
cat >"${tmp}" <<EOF
<C>You chose the following date

<C><#HL(10)>
<C>${answer}
<C><#HL(10)>

<C>You chose button #${selected}

<C>Hit </R>space<!R> to continue.
EOF

#
# Create the label widget to display the information.
#
${CDK_LABEL} -f "${tmp}" -p " "

#
# Clean up.
#
rm -f "${tmp}" "${date}"
