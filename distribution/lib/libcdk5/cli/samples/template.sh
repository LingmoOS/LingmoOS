#!/bin/sh
# $Id: template.sh,v 1.4 2022/10/18 23:55:40 tom Exp $

#
# Description:
#		This demonstrates the CDK command line
# interface to the template widget.
#

#
# Create some global variables.
#
CDK_TEMPLATE="${CDK_BINDIR=..}/cdktemplate"
CDK_LABEL="${CDK_BINDIR=..}/cdklabel"

output="${TMPDIR=/tmp}/template_output.$$"
tmp="${TMPDIR=/tmp}/tmp.$$"

#
# Create the title.
#
title="<C>               Type in the                 
<C>IP Address for this machine."
buttons=" OK 
 Cancel "

#
# Create the template box.
#
${CDK_TEMPLATE} -p "###.###.###.###" -o "___.___.___.___" -T "${title}" -P -B "${buttons}" 2> "${output}"
selected=$?
test $selected = 255 && exit 1

answer=`cat "${output}"`

#
# Create the message for the label widget.
#
cat >"${tmp}" <<EOF
<C>Here is the IP you typed in

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
