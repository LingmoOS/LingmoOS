#!/bin/sh
# $Id: alphalist.sh,v 1.9 2022/10/18 22:08:00 tom Exp $

#
# Description:
#		This demonstrates the CDK command line
# interface to the alphalist widget.
#

#
# This gets the password file.
#
getPasswordFile()
{
   system=$1
   file=$2

   #
   # Depending on the system, get the password file
   # using nicat, ypcat, or just plain old /etc/passwd
   #
   if [ "$system" = "NIS" ]; then
      niscat passwd.org_dir > "$file"
   elif [ "$system" = "YP" ]; then
      ypcat passwd > "$file"
   else
      cp /etc/passwd "$file"
   fi
}

#
# This displays account information.
#
displayAccountInformation()
{
   userAccount=$1
   passwordFile=$2

   #
   # Get the user account information.
   #
   line=`grep "^${userAccount}" "$passwordFile"`
   uid=`echo "$line" | cut -d: -f3`
   gid=`echo "$line" | cut -d: -f4`
   info=`echo "$line" | cut -d: -f5`
   home=`echo "$line" | cut -d: -f6`
   shell=`echo "$line" | cut -d: -f7`

   #
   # Create the label message information.
   #
   accountMessage="<C></U>Account
<C><#HL(30)>
Account: </U>${userAccount}
UID    : </U>${uid}
GID    : </U>${gid}
Info   : </U>${info}
Home   : </U>${home}
Shell  : </U>${shell}
<C><#HL(30)>
<C>Hit </R>space<!R> to continue"

   #
   # Create the popup label.
   #
   ${CDK_LABEL} -m "${accountMessage}" -p " "
}

#
# Create some global variables.
#
CDK_ALPHALIST="${CDK_BINDIR=..}/cdkalphalist"
CDK_LABEL="${CDK_BINDIR=..}/cdklabel"

tmpPass="${TMPDIR=/tmp}/sl.$$"
output="${TMPDIR=/tmp}/alphalist.$$"
userAccounts="${TMPDIR=/tmp}/ua.$$"

#
# Create the message for the scrolling list.
#
title="<C>Pick an account
<C>you want to view."

#
# Get the password file and stick it into the temp file.
#
#getPasswordFile "YP" "$tmpPass"
#getPasswordFile "NIS" "$tmpPass"
getPasswordFile "Other" "$tmpPass"

#
# Get the user account from the password file.
#
awk 'BEGIN {FS=":"} {printf "%s\n", $1}' "$tmpPass" | sort > "${userAccounts}"

#
# Create the scrolling list.
#
${CDK_ALPHALIST} -T "${title}" -f "${userAccounts}" -H -10 -W -20 2> "${output}"
selected=$?
test $selected = 255 && exit 1

answer=`sed -e 's/^[ ]*//' -e 's/[ ]*$//' "${output}"`

#
# Display the account information.
#
if [ -n "$answer" ]; then
    displayAccountInformation "$answer" "$tmpPass"
fi

#
# Clean up.
#
rm -f "${output}" "${tmpPass}" "${userAccounts}"
