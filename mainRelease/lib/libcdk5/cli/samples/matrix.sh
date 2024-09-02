#!/bin/sh
# $Id: matrix.sh,v 1.5 2022/10/18 22:28:29 tom Exp $

#
# Description:
#		This demonstrates the CDK command line
# interface to the matrix widget.
#

#
# Create some global variables.
#
CDK_MATRIX="${CDK_BINDIR=..}/cdkmatrix"
CDK_LABEL="${CDK_BINDIR=..}/cdklabel"

fileSystemList="${TMPDIR=/tmp}/fsList.$$"
info="${TMPDIR=/tmp}/fsInfo.$$"
diskInfo="${TMPDIR=/tmp}/diskInfo.$$"
tmp="${TMPDIR=/tmp}/tmp.$$"

#
# Get the filesystem information.
#
getDiskInfo()
{
   fileName=$1;
   command="df"

   #
   # Determine the type of operating system.
   #
   machine=`uname -s`
   if [ "$machine" = "SunOS" ]; then
      level=`uname -r`

      if [ "$level" -gt 4 ]; then
         command="df -kl"
      fi
   else
      if [ "$machine" = "AIX" ]; then
         command="df -i"
      fi
   fi

   #
   # Run the command.
   #
   ${command} > "${fileName}"
}

#
# Get the disk information.
#
getDiskInfo "${diskInfo}"

#
# Create the row titles.
#
rowTitles=`tail +2 "${diskInfo}" | awk '{printf "%s\n", $1}'`

#
# Create the column titles.
#
colTitles="KBytes
Used
Avail
Capacity
Mounted on"

#
# Define the matrix title.
#
title="<C></U>Current Filesystem Information"

#
# Create the data file to fill in the matrix.
#
tail +2 "${diskInfo}" | awk '{printf "%s%s%s%s%s\n", $2, $3, $4, $5, $6}' > "${info}"

#
# Set the widths of the columns.
#
colWidths="5
5
5
5
5"

#
# Set the cells as uneditable.
#
types="VIEWONLY
VIEWONLY
VIEWONLY
VIEWONLY
VIEWONLY"

buttons=" OK 
 Cancel "

#
# Start the matrix.
#
${CDK_MATRIX} -r "${rowTitles}" -c "${colTitles}" -v 3 -w "${colWidths}" -d "${info}" -t "${types}" -T "${title}" -B "${buttons}" -O"$tmp"
selected=$?
test $selected = 255 && exit 1

#
# Create the message for the label widget.
#
cat >"${tmp}" <<EOF
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
rm -f "${info}" "${fileSystemList}" "${tmp}" "${diskInfo}"
