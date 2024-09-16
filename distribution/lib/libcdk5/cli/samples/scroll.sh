#!/bin/sh
# $Id: scroll.sh,v 1.5 2022/10/18 23:47:13 tom Exp $

#
# Description:
# 		This demonstrates the CDK command line
# interface to the scrolling list widget.
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
CDK_SCROLL="${CDK_BINDIR=..}/cdkscroll"
CDK_LABEL="${CDK_BINDIR=..}/cdklabel"

tmpPass="${TMPDIR=/tmp}/sl.$$"
output="${TMPDIR=/tmp}/output.$$"
userAccounts="${TMPDIR=/tmp}/ua.$$"

TYPE="Other"

#
# Chop up the command line.
#
if set -- `getopt nNh "$@"`
then
for c in "$@"
do
    case $c in
         -n) TYPE="YP"; shift;;
         -N) TYPE="NIS"; shift;;
         -h) echo "$0 [-n YP] [-N NIS+] [-h]"; exit 0;;
         --) shift; break;;
    esac
done
else
   echo "Usage: $0 [-n] [-N] [-h]"
   exit 2
fi

#
# Create the message for the scrolling list.
#
title="<C><#HL(30)>
<C>Pick an account you want to view.
<C><#HL(30)>"
buttons=" OK 
 Cancel "

#
# Get the password file and stick it into the temp file.
#
getPasswordFile "${TYPE}" "$tmpPass"

#
# Get the user account from the password file.
#
awk 'BEGIN {FS=":"} {printf "%s\n", $1}' "$tmpPass" | sort > "${userAccounts}"

#
# Create the scrolling list.
#
${CDK_SCROLL} -T "${title}" -f "${userAccounts}" -n -B "${buttons}" 2> "${output}"
selected=$?
test $selected = 255 && exit 1

answer=`cat "${output}"`

#
# Display the account information.
#
displayAccountInformation "$answer" "$tmpPass"

#
# Clean up.
#
rm -f "${output}" "${tmpPass}" "${userAccounts}"
