; Debian-Installer Loader - Branch selection
; 
; Copyright (C) 2007,2008,2009  Robert Millan <rmh@aybabtu.com>
; Copyright (C) 2010,2011       Didier Raboud <odyx@debian.org>
;
; This program is free software: you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation, either version 3 of the License, or
; (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program.  If not, see <http://www.gnu.org/licenses/>.

Function ShowBranch
  Var /GLOBAL di_branch
  StrCpy $di_branch stable
  ${If} ${AtLeastWin7}
!ifdef DAILIES
  ${AndIf} $expert == true
!endif
    ${If} $kernel == ${KERNEL_LINUX}
      ${Choice_Present} di_branch 3
      ${Choice_Get} $0
      ${If} $0 == "2"
        StrCpy $di_branch daily
      ${Endif}
    ${Else}
      ; Only Debian GNU/Linux will have a stable branch for the stretch cycle
      StrCpy $di_branch daily
    ${EndIf}
  ${EndIf}

; ********************************************** Initialise base_url
  Var /GLOBAL base_url ; URL to the Release{,.gpg} files
  Var /GLOBAL base_path_hashes ; further path to the various ${HASH}SUMS files
  Var /GLOBAL base_path_images ; (even) further path to the images
  ReadINIStr $0 $PLUGINSDIR\di_branch.ini "Field 3" "State"
  ${If} $di_branch == "daily"
    MessageBox MB_YESNO|MB_ICONQUESTION $(di_branch4) IDNO +2
    ExecShell "open" "https://www.debian.org/devel/debian-installer/errata"

    ; We have no Release.gpg files for dailies…
    StrCpy $base_path_hashes ""

    ; Daily images URL
    ; See http://svn.debian.org/viewsvn/d-i/trunk/scripts/daily-build-aggregator for the canonical list
    ${If} $kernel == ${KERNEL_LINUX}
      StrCpy $base_url         "https://d-i.debian.org/daily-images/$arch/daily/"
      StrCpy $base_path_images "netboot/$gtkdebian-installer/$arch"
    ${ElseIf} $kernel == ${KERNEL_KFREEBSD}
      StrCpy $base_url          "https://d-i.debian.org/daily-images/kfreebsd-$arch/daily/"
      StrCpy $base_path_images  "netboot/$gtkdebian-installer/kfreebsd-$arch"
    ${ElseIf} $kernel == ${KERNEL_HURD}
      StrCpy $base_url          "https://d-i.debian.org/daily-images/hurd-$arch/daily/"
      StrCpy $base_path_images  "netboot/debian-installer/hurd-$arch"
    ${EndIf}
  ${Else}
    ${If} ${AtLeastWin7}
      StrCpy $base_url "https://"
    ${Else}
      StrCpy $base_url "http://"
    ${EndIf}
    StrCpy $base_url         "$base_urldeb.debian.org/debian/dists/stable/"
    ${If} $kernel == ${KERNEL_LINUX}
      ; Only Debian GNU/Linux will have a stable branch for the stretch cycle
      StrCpy $base_path_hashes    "main/installer-$arch/current/images/"
      StrCpy $base_path_images "netboot/$gtkdebian-installer/$arch"
    ${ElseIf} $kernel == ${KERNEL_KFREEBSD}
      ; Debian GNU/kFreeBSD will have no stable branch for the stretch cycle, this branch can't happen
      StrCpy $base_path_hashes    "main/installer-kfreebsd-$arch/current/images/"
      StrCpy $base_path_images "netboot/$gtkdebian-installer/kfreebsd-$arch"
    ${ElseIf} $kernel == ${KERNEL_HURD}
      ; Debian GNU/Hurd will have no stable branch for the stretch cycle, this branch can't happen
      StrCpy $base_path_hashes    "main/installer-hurd-$arch/current/images/"
      StrCpy $base_path_images "netboot/$gtkdebian-installer/hurd-$arch"
    ${EndIf}
  ${Endif}
FunctionEnd
