#!/bin/sh
# $Id: entry.sh,v 1.4 2022/10/18 22:20:44 tom Exp $

#
# Description:
#		This demonstrates the CDK command line
# interface to the entry widget.
#

#
# Create some global variables.
#
CDK_ENTRY="${CDK_BINDIR=..}/cdkentry"
CDK_LABEL="${CDK_BINDIR=..}/cdklabel"

output="${TMPDIR=/tmp}/entry_output.$$"
tmp="${TMPDIR=/tmp}/tmp.$$"

#
# Create the title.
#
title="<C>  Type a simple string.  "
buttons=" OK 
 Cancel "

#
# Create the entry box.
#
${CDK_ENTRY} -f 20 -T "${title}" -B "${buttons}" -F "</5>_ " -O "${output}" -S
selected=$?
test $selected = 255 && exit 1

answer=`cat "${output}"`

#
# Create the message for the label widget.
#
cat >"${tmp}" <<EOF
<C>Here is the string you typed in

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
