#!/bin/sh
# $Id: mentry.sh,v 1.4 2022/10/18 22:30:31 tom Exp $

#
# Description:
#		This demonstrates the CDK command line
# interface to the multiple line entry widget.
#

#
# Create some global variables.
#
CDK_MENTRY="${CDK_BINDIR=..}/cdkmentry"
CDK_LABEL="${CDK_BINDIR=..}/cdklabel"

output="${TMPDIR=/tmp}/mentry_output.$$"
tmp="${TMPDIR=/tmp}/tmp.$$"

#
# Create the title.
#
title="<C>Type in a short message."
buttons=" OK 
 Cancel "

#
# Create the mentry box.
#
${CDK_MENTRY} -s 5 -v 10 -f 20 -T "${title}" -B "${buttons}" -F '_' -O "${output}"
selected=$?
test $selected = 255 && exit 1

answer=`cat "${output}"`

#
# Create the message for the label widget.
#
cat >"${tmp}" <<EOF
<C>Here is the message you typed in

<C></R>${answer}

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
rm -f "${tmp}" "${output}"
