#!/bin/sh
# $Id: selection.sh,v 1.5 2022/10/18 23:52:42 tom Exp $

#
# Description:
#	This demonstrates the CDK command line
#	interface to the selection widget.
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
   totalSelections=$1
   currentSelection=$2
   userAccount=$3
   passwordFile=$4

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
   accountMessage="<C></U>Account ${currentSelection}/${totalSelections}
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
# Define where the CDK widgets are.
#
CDK_SELECTION="${CDK_BINDIR=..}/cdkselection"
CDK_LABEL="${CDK_BINDIR=..}/cdklabel"

TYPE="Other"

#
# Define the output files.
#
accountList="${TMPDIR=/tmp}/accList.$$"
userAccounts="${TMPDIR=/tmp}/userAccList.$$"
output="${TMPDIR=/tmp}/selection_output.$$"
tmpPass="${TMPDIR=/tmp}/ps.$$"
tmp="${TMPDIR=/tmp}/tmp.$$"

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
# Create the message for the selection list.
#
title="<C>Pick a user account to view."

#
# Get the password file and stick it into the temp file.
#
getPasswordFile "${TYPE}" "$tmpPass"

#
# Create the user account list.
#
awk 'BEGIN {FS=":"} {printf "%s\n", $1}' "$tmpPass" | sort > "${userAccounts}"
awk '{printf "<C></B>%s\n", $1}' "${userAccounts}" > "${accountList}"
accounts=`cat "${userAccounts}"`

#
# Create the choices list.
#
choices="<C></B>No 
<C></B> Yes "
buttons=" OK 
 Cancel "

#
# Create the selection list.
#
${CDK_SELECTION} -T "${title}" -f "${accountList}" -c "${choices}" -B "${buttons}"  2> "$output"
selected=$?
test $selected = 255 && exit 1

#
# Initialize the variables.
#
selection=""
value=""

#
# Count how many were selected.
#
count=`grep -c "^1" "${output}"`
current=0

#
# Create the label.
#
for i in `cat "${output}"`
do
   #
   # Since every other variable is value/selection, we
   # store every other value in the correct variable.
   #
   if [ "$value" = "" ]; then
      value="$i"
   else
      selection="$i"

      #
      # Only display the selected accounts.
      #
      if [ "$value" -eq 1 ]; then
         #
         # Increment our counter.
         #
         current=`expr $current + 1`

         #
         # Display the account information.
         #
         displayAccountInformation "$count" "$current" "$selection" "$tmpPass"
      fi

      #
      # Reset the variables.
      #
      value=""
      selection=""
   fi
done

#
# Clean up.
#
rm -f "${accountList}" "${userAccounts}" "${output}" "${tmpPass}" "${tmp}"
