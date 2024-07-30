# SPDX-License-Identifier: CC0-1.0
# SPDX-FileCopyrightText: none
[General]
LogoPath=/home/me/kubuntu-circle-small.png
Website=http://www.kubuntu.org
Version=1.2.3
Variant=Unstable Branches
UseOSReleaseVersion=true # controls if os-release's VERSION is used (default=false = use VERSION_ID)
# Scripts to run to get extra software data to show. The scripts need to take care of l10n. When en_US output
# is required the LANGUAGE environment variable will contain **at least** en_US. Simply follow the environment.
# Script output is expected to be a \n separated set of one label (including a terminal colon) and one value e.g.
#   VLC Version:\n
#   4.0.0
# Further output is ignored.
ExtraSoftwareData=/usr/bin/myscript;/usr/bin/myotherscript
