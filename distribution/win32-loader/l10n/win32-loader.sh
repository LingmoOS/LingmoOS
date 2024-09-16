#!/bin/bash -e
#
# l10n support for win32-loader
# Copyright (C) 2007,2009  Robert Millan <rmh@aybabtu.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# Got this from our parent
export LANGUAGE

# We need this for gettext to work.  Why not matching $LANGUAGE as passed
# by our parent?  Because then we'd have to guess country, and en_US.UTF-8
# just works.
export LANG=en_US.UTF-8

# LC_ALL is usually undefined in user shells.  Hence it's easy to experience
# the illusion that this line is unnecessary.  pbuilder thinks otherwise (as
# it exports LC_ALL=C breaking it).
export LC_ALL=$LANG

. /usr/bin/gettext.sh
export TEXTDOMAIN=win32-loader
export TEXTDOMAINDIR=${PWD}/locale

# translate:
# This must be a valid string recognised by Nsis.  If your
# language is not yet supported by Nsis, please translate the
# missing Nsis part first.
nsis_lang=$(gettext LANG_ENGLISH)

langstring()
{
  echo LangString $1 \${$nsis_lang} \
    "\"$(sed -e ':l;s&\\\|\("\)&$\\\1&g;N;$!bl;s&\n&$\\n&g')\""
  # :l
  # Label l
  # s&\\\|\("\)&$\\\1&g
  # Subsitute \ with $\ and " with $\"
  # N;$!bl
  # Put all lines together
  # s&\n&$\\n&g
  # Substitue line feed with $\n
}

# translate:
# This must be the string used by GNU iconv to represent the charset used
# by Windows for your language.  If you don't know, check
# [wine]/tools/wmc/lang.c, or http://www.microsoft.com/globaldev/reference/WinCP.mspx
#
# IMPORTANT: In the rest of this file, only the subset of UTF-8 that can be
# converted to this charset should be used.
charset=`gettext windows-1252`

# translate:
# Charset used by NTLDR in your localised version of Windows XP.  If you
# don't know, maybe http://en.wikipedia.org/wiki/Code_page helps.
ntldr_charset=`gettext cp437`

# Were we asked to translate a single string?
if [ "$1" != "" ] ; then
  exec gettext -s "$1"
fi

# May be requested by our parent makefile (see above)
# translate:
# The name of your language _in English_ (must be restricted to ascii)
gettext English > /dev/null

# translate:
# The MUI_LANGUAGE macro expects the nlf file without extension for your
# language as a parameter. The nlf file should be found in
# /usr/share/nsis/Contrib/Language files/
echo "!insertmacro MUI_LANGUAGE \"$(eval_pgettext NLFID English)\""

# Now comes a string that may be used by NTLDR (or not).  So we need both
# samples.

#  - First we get the string.

# translate:
# IMPORTANT: only the subset of UTF-8 that can be converted to NTLDR charset
# (e.g. cp437) should be used in this string.  If you don't know which charset
# applies, limit yourself to ascii. $target_distro; will be "Debian" and $kernel_name;
# will be either "GNU/Linux", "GNU/kFreeBSD" or "GNU/Hurd" (in ASCII)
d_i=`gettext '$target_distro $kernel_name - Continue with install process'`

# translate:
# IMPORTANT: only the subset of UTF-8 that can be converted to NTLDR charset
# (e.g. cp437) should be used in this string.  If you don't know which charset
# applies, limit yourself to ascii.
pxe=`gettext "PXE - Network boot"`

#  - Then we get a sample for bootmgr in the native charset.
echo "${d_i}" | langstring d-i
echo "${d_i}" | langstring d-i_ntldr
echo "${pxe}" | langstring pxe
echo "${pxe}" | langstring pxe_ntldr

# The bulk of the strings

# translate:
# This is the program name, that appears in the installer windows captions and in the Windows Uninstaller dialog.
# Ampersands (&) are _forbidden_ in that string.
gettext "Debian-Installer loader" | langstring program_name
gettext "Cannot find win32-loader.ini." | langstring error_missing_ini
gettext "win32-loader.ini is incomplete.  Contact the provider of this medium." | langstring error_incomplete_ini
gettext "This program has detected that your keyboard type is \"\$0\".  Is this correct?" | langstring detected_keyboard_is
gettext "Please send a bug report with the following information:

 - Version of Windows.
 - Country settings.
 - Real keyboard type.
 - Detected keyboard type.

Thank you." | langstring keyboard_bug_report
gettext "There doesn't seem to be enough free disk space in drive \$c.  For a complete desktop install, it is recommended to have at least 3 GB.  If there is already a separate disk or partition for this install, or if you plan to replace Windows completely, you can safely ignore this warning." | langstring not_enough_space_for_debian
gettext "Error: not enough free disk space.  Aborting install." | langstring not_enough_space_for_loader
gettext "This program doesn't support Windows \$windows_version yet." | langstring unsupported_version_of_windows
gettext "The system version you're trying to install is designed to run on modern, 64-bit computers.  However, your computer is incapable of running 64-bit programs.

Use the 32-bit (\"i386\") version, or the Multi-arch version which is able to install either of them.

This installer will abort now." | langstring amd64_on_i386
gettext "Your computer is capable of running modern, 64-bit operating systems.  However, the system version you're trying to install is designed to run on older, 32-bit hardware.

You may still proceed with this install, but in order to take the most advantage of your computer, we recommend that you use the 64-bit (\"amd64\") version instead, or the Multi-arch version which is able to install either of them.

Would you like to abort now?" | langstring i386_on_amd64

# translate:
# "system partition" and "boot partition" are defined by Microsoft to mean the opposite of what a
# normal person would expect they mean, so please keep the quotes, and translate them as literally
# as possible.  Also, I suggest you use the same term used in http://support.microsoft.com/kb/314470
# if that is available for your language.
gettext "Unable to find \"system partition\", assuming it is the same as the \"boot partition\" (\$c)." | langstring cant_find_system_partition
gettext "Select install mode:" | langstring expert1
gettext "Normal mode.  Recommended for most users." | langstring expert2
gettext "Expert mode.  Recommended for expert users who want full control of the install process." | langstring expert3
gettext "PXE mode: install a PXE loader to allow remote kernel loading." | langstring expert4
gettext "Select action:" | langstring rescue1
gettext "Begin install on this computer." | langstring rescue2
gettext "Repair an installed system (rescue mode)." | langstring rescue3
gettext "Select the kernel:" | langstring kernel1
gettext "GNU/Linux" | langstring kernel2
gettext "GNU/kFreeBSD" | langstring kernel3
gettext "GNU/Hurd" | langstring kernel4
gettext "Select install mode:" | langstring graphics1
gettext "Graphical install" | langstring graphics2
gettext "Text install" | langstring graphics3
gettext "Connecting ..." | langstring connecting

# translate:
# $(^Byte), $0, $1, $4 must not be translated.
# These are substitutes as the following:
# $(^Byte) -> B
# $0 -> unit prefix such as G (giga), M (mega), K (kilo) or empty
# $1 -> estimated time remaining for the download
#    It includes the unit. Examples are 1h, 5m, 3s
# $2 -> current size of downloaded content
# $4 -> total size of file
# Example: 3s left - 49.2 of 55.0 MB (2.2 MB/s)
gettext "\$1 left - \$2 of \$4\$0\$(^Byte) (\$3\$0\$(^Byte)/s)" | langstring progress
gettext "Select which version of Debian-Installer to use:" | langstring di_branch1
gettext "Stable release.  This will install Debian \"stable\"." | langstring di_branch2
gettext "Daily build.  This is the development version of Debian-Installer.  It will install Debian \"testing\" by default, and may be capable of installing \"stable\" or \"unstable\" as well." | langstring di_branch3

# translate:
# You might want to mention that so-called "known issues" page is only available in English.
gettext "It is recommended that you check for known issues before using a daily build.  Would you like to do that now?" | langstring di_branch4
gettext "The following parameters will be used.  Do NOT change any of these unless you know what you're doing." | langstring custom1
gettext "Proxy settings (host:port):" | langstring custom2
gettext "Location of boot.ini:" | langstring custom3
gettext "Base URL for netboot images (linux and initrd.gz):" | langstring custom5
gettext "Error" | langstring error
gettext "Error: failed to copy \$0 to \$1." | langstring error_copyfiles

# translate:
# $0 will be "Release" (it is a filename).
gettext "Checking GPG signature on \$0." | langstring gpg_checking_release

# translate:
# $0 will be "Release" (it is a filename).
gettext "The downloaded \$0 file cannot be trusted! Aborting." | langstring unsecure_release

# translate:
# This appears in a MessageBox when the md5 checksum verification failed. $0 is a url; $2 is the filename $1 is the
# computed checksum and $4 is the expected one.
gettext "Checksum mismatch for \$0/\$2. Got \$1 when expecting \$4. Aborting." | langstring checksum_mismatch

# translate:
# $2 is a filename
gettext "Computing checksum for \$2" | langstring computing_checksum

gettext "Generating \$0" | langstring generating
gettext "Appending preseeding information to \$0" | langstring appending_preseeding
gettext "Error: unable to run \$0." | langstring error_exec
gettext "Disabling NTFS compression in bootstrap files" | langstring disabling_ntfs_compression
gettext "Registering in NTLDR" | langstring registering_ntldr
gettext "Registering in BootMgr" | langstring registering_bootmgr
gettext "Error: failed to parse bcdedit.exe output." | langstring error_bcdedit_extract_id
gettext "Error: \$0 not found.  Is this really Windows \$windows_version?" | langstring system_file_not_found
gettext "VERY IMPORTANT NOTICE:\\n\\n" | langstring warning1

# translate:
# The following two strings are mutualy exclusive.  win32-loader
# will display one or the other depending on version of Windows.
# Take into account that either option has to make sense in our
# current context (i.e. be careful when using pronouns, etc).
gettext "The second stage of this install process will now be started.  After your confirmation, this program will restart Windows in DOS mode, and automatically load the next part of the install process.\\n\\n" | langstring warning2_direct
gettext "You need to reboot in order to proceed with the install process.  During your next boot, you will be asked whether you want to start Windows or continue with the install process.\\n\\n" | langstring warning2_reboot
gettext "During the install process, you will be offered the possibility of either reducing your Windows partition or completely replacing it.  In both cases, it is STRONGLY RECOMMENDED that you have previously made a backup of your data.  The authors of this software will NOT take ANY RESPONSIBILITY in the event of data loss.\\n\\nOnce your install is complete (and if you have chosen to keep Windows in your disk), you can uninstall this loader through the Windows Add/Remove Programs dialog in Control Panel." | langstring warning3
gettext "Do you want to reboot now?" | langstring reboot_now
