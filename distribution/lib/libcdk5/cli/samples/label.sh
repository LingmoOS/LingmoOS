#!/bin/sh
# $Id: label.sh,v 1.4 2022/10/19 00:11:34 tom Exp $

#
# Description:
#		This demonstrates the CDK command line
# interface to the label widget.
#

#
# Create some global variables.
#
CDK_LABEL="${CDK_BINDIR=..}/cdklabel"

dfTmp="${TMPDIR=/tmp}/label.$$"
tmp="${TMPDIR=/tmp}/tmp.$$"
top="${TMPDIR=/tmp}/dfTop.$$"
bottom="${TMPDIR=/tmp}/dfBottom.$$"

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
   elif [ "$machine" = "AIX" ]; then
      command="df -i"
   elif [ "$machine" = "Linux" ]; then
      command="df -i"
   fi

   #
   # Run the command.
   #
   ${command} > "${fileName}"
}

#
# Get the disk information.
#
getDiskInfo "${dfTmp}"

#
# Bold the first line of the df command.
#
head -1 "${dfTmp}" | awk '{printf "</B>%s\n", $0}' > "${top}"
tail +2 "${dfTmp}" > "${bottom}"

#
# Create the message for the label widget.
#
cat >"${tmp}" <<EOF
<C>This is the current
<C>status of your local filesystems.
<C><#HL(2)>
EOF
cat "${top}"    >> "${tmp}"
cat "${bottom}" >> "${tmp}"
cat >>"${tmp}" <<EOF
<C><#HL(2)>

<C>Hit </R>space<!R> to continue.
EOF

#
# Create the label widget to display the information.
#
${CDK_LABEL} -f "${tmp}" -p " " -c "ls" -S

#
# Clean up.
#
rm -f "${tmp}" "${dfTmp}" "${top}" "${bottom}"
