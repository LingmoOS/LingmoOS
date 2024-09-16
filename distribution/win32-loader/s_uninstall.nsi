; Debian-Installer Loader - Uninstallation
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

; Include required functions
${un.BOOTCFG_ConnectWMI}
${un.BOOTCFG_DeleteObject}
${un.BOOTCFG_GetObject}
${un.BOOTCFG_OpenDefaultBcdStore}
${un.BOOTCFG_ReleaseObject}

Function un.RemoveBootEntry
  System::Store 'S'
  ReadRegStr $2 HKLM "${REGSTR_WIN32}" "bootmgr"
  ${If} $2 != ""
    ${un.BOOTCFG_ConnectWMI} $R0 $R1 $0
    ${If} $0 == 0
      ${un.BOOTCFG_GetObject} $R1 "BcdStore" $R2 $0
      ${If} $0 == 0
        ${un.BOOTCFG_GetObject} $R1 "BcdObject" $R3 $0
        ${If} $0 == 0
          ${un.BOOTCFG_OpenDefaultBcdStore} $R1 $R2 $R4 $0
          ${If} $0 == 0
            ${un.BOOTCFG_DeleteObject} $R1 $R2 $R4 $2 $1 $0
            ${If} $0 != 0
              IntFmt $0 "0x%08X" $0
              DetailPrint "$1: $0"
            ${EndIf}
            ${un.BOOTCFG_ReleaseObject} $R4
          ${Endif}
          ${un.BOOTCFG_ReleaseObject} $R3
        ${EndIf}
        ${un.BOOTCFG_ReleaseObject} $R2
      ${EndIf}
      ${un.BOOTCFG_ReleaseObject} $R1
      ${un.BOOTCFG_ReleaseObject} $R0
    ${EndIf}
  ${EndIf}
  System::Store 'L'
FunctionEnd

Section "Uninstall"
  ; Initialise $c
  ReadRegStr $c HKLM "${REGSTR_WIN32}" "system_drive"

  Call un.RemoveBootEntry

  IfFileExists "$c\boot.ini" 0 no_saved_boot_ini_timeout
  SetFileAttributes "$c\boot.ini" NORMAL
  SetFileAttributes "$c\boot.ini" SYSTEM|HIDDEN
  DeleteINIStr "$c\boot.ini" "operating systems" "$INSTDIR\g2ldr.mbr"
    
  ReadIniStr $0 "$c\boot.ini" "boot loader" "old_timeout_win32-loader"
  IfErrors 0 no_saved_boot_ini_timeout
     ; Restore original timeout
     ClearErrors
     WriteIniStr "$c\boot.ini" "boot loader" "timeout" $0
     DeleteINIStr "$c\boot.ini" "boot loader" "old_timeout_win32-loader"
  no_saved_boot_ini_timeout:

  DeleteRegKey HKLM "${REGSTR_WIN32}"
  DeleteRegKey HKLM "${REGSTR_UNINST}"
  Delete $c\g2ldr
  RMDir /r /REBOOTOK $INSTDIR
SectionEnd
