; Debian-Installer Loader - Expert selection and early detection
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

!define BOOT_FWBOOTMGR 1
!define BOOT_BOOTMGR 2
!define BOOT_NTLDR 3
!define BOOT_DIRECT 4

; Get drive letter of device
; Parameter:
;   device - path of device, e. g. \Device\HarddiskVolume2
; Return value:
;   drive letter or error if device is not mapped to a drive letter
Function GetDriveLetterOfDevice
  Exch $0
  Push $1
  Push $2
  Push $3
  Push $4
  Push $5

  ; Go through the present logical drives and identify
  ; the matching device
  System::Call "kernel32::GetLogicalDrives() i .r5"
  StrCpy $1 0
  StrCpy $2 ""
  ${DoWhile} $1 < 26
    IntOp $4 $5 & 1
    ${If} $4 != 0
      IntOp $4 $1 + 65
      IntFmt $2 "%c:" $4
      System::Call "kernel32::QueryDosDevice(t r2, t .r3, \
        i${NSIS_MAX_STRLEN}) i .r4"
      ${If} $4 != 0
      ${AndIf} $3 == $0
        StrCpy $0 $2
        ${Break}
      ${EndIf}
    ${EndIf}
    IntOp $5 $5 >> 1
    IntOp $1 $1 + 1
  ${Loop}

  ${If} $0 != $2
    StrCpy $0 "error"
  ${EndIf}

  Pop $5
  Pop $4
  Pop $3
  Pop $2
  Pop $1
  Exch $0
FunctionEnd

; Get device path of the partition of the current BCD entry
; Return value:
;  device path or empty string in case of failure 
Function GetDevicePathOfPartition
  System::Store 'S'

  ${BOOTCFG_ConnectWMI} $R0 $R1 $0
  ${If} $0 == 0
    ${BOOTCFG_GetObject} $R1 "BcdStore" $R2 $0
    ${If} $0 == 0
      ${BOOTCFG_GetObject} $R1 "BcdObject" $R3 $0
      ${If} $0 == 0
        ${BOOTCFG_OpenDefaultBcdStore} $R1 $R2 $R4 $0
        ${If} $0 == 0
          ${BOOTCFG_GetElementFromBcd} $R1 $R2 $R4 \
            $R3 ${BOOTCFG_CURRENT_GUID} ${BOOTCFG_BCDE_DEVICE} \
            $2 $1 $0
          ${If} $0 == 0
            ; Save returned element onto stack
            Push $1
            ; Release returned bcd object
            ${BOOTCFG_ReleaseObject} $2
            StrCpy $2 $1
            ${BOOTCFG_GetObjectPropertyValue} $2 "Device" $1 $0
            ${If} $0 == 0
              ${BOOTCFG_GetObjectOfVariant} $1 $2 $0
              ; Free variant
              System::Free $1
              ${If} $0 == 0
                ${BOOTCFG_GetObjectPropertyValue} $2 "Path" $1 $0
                ${If} $0 == 0
                  ${BOOTCFG_GetStringOfVariant} $1 $2 $0
                  ; Free variant
                  System::Free $1
                  StrCpy $1 $2
                ${EndIf}
              ${EndIf}
            ${EndIf}
            ; Release returned element
            Pop $2
            ${BOOTCFG_ReleaseObject} $2
          ${EndIf}
          ${BOOTCFG_ReleaseObject} $R4
        ${EndIf}
        ${BOOTCFG_ReleaseObject} $R3
      ${EndIf}
      ${BOOTCFG_ReleaseObject} $R2
    ${EndIf}
    ${BOOTCFG_ReleaseObject} $R1
    ${BOOTCFG_ReleaseObject} $R0
  ${EndIf}

  ${If} $0 != 0
    StrCpy $0 ""
  ${Else}
    StrCpy $0 $1
  ${EndIf}

  Push $0
  System::Store 'L'
FunctionEnd

Function ShowExpert
; Do initialisations as early as possible

; ********************************************** Initialise $preseed_cmdline
  StrCpy $preseed_cmdline " "

!ifndef NOCD
; ********************************************** Initialise $d
  ${GetRoot} $EXEDIR $d

; ********************************************** Check win32-loader.ini presence
; before initialising $arch or $gtk
  IfFileExists $d\win32-loader.ini +3 0
    MessageBox MB_OK|MB_ICONSTOP "$(error_missing_ini)"
    Quit

; ********************************************** Display README.html if found
  ReadINIStr $0 $PLUGINSDIR\maps.ini "languages" "$LANGUAGE"
  IfFileExists $d\README.$0.html 0 +3
    StrCpy $0 README.$0.html
    Goto readme_file_found
  IfFileExists $d\README.html 0 readme_file_not_found
    StrCpy $0 README.html
    Goto readme_file_found

readme_file_found:
  ShowWindow $HWNDPARENT ${SW_MINIMIZE}
  ExecShell "open" "file://$d/$0"

readme_file_not_found:
!endif

; ********************************************** Initialise $arch
  StrCpy $arch "amd64"
  ${IfNot} ${RunningX64}
    ${WMIQUERY_SingleProp} "DataWidth" "Win32_Processor" $0
    ${If} $0 = ""
      ; Fall back to CPUID helper
      ${CPUID_DataWidth} $0
    ${EndIf}
    ${If} $0 != 64
      StrCpy $arch "i386"
    ${EndIf}
  ${EndIf}
!ifndef NOCD
  ReadINIStr $0 $d\win32-loader.ini "installer" "arch"
  ${If} "$0:$arch" == "amd64:i386"
    MessageBox MB_OK|MB_ICONSTOP $(amd64_on_i386)
    Quit
  ${Endif}
  ${If} "$0:$arch" == "i386:amd64"
    MessageBox MB_YESNO|MB_ICONQUESTION $(i386_on_amd64) IDNO +2
    Quit
    StrCpy $arch "i386"
  ${Endif}
!endif

  ; Windows version is another abort condition
  Var /GLOBAL windows_boot_method
  ${If} ${AtMostWinME}
    StrCpy $windows_boot_method ${BOOT_DIRECT}
    ${If} ${IsNT}
      StrCpy $windows_boot_method ${BOOT_NTLDR}
    ${Endif}
    Goto windows_version_ok
  ${Endif}
  ${If} ${AtMostWin2003}
    StrCpy $windows_boot_method ${BOOT_NTLDR}
    Goto windows_version_ok
  ${Endif}
  ; So far, all versions post Windows 7 support BCD
  ${BOOTCFG_IsUEFI} $0
  ${If} $0 != 0
    StrCpy $windows_boot_method ${BOOT_FWBOOTMGR}
  ${Else}
    StrCpy $windows_boot_method ${BOOT_BOOTMGR}
  ${EndIf}
windows_version_ok:

; ********************************************** Initialise $c
; We set it to the "System partition" (see http://en.wikipedia.org/wiki/System_partition_and_boot_partition)

  ${If} $windows_boot_method != ${BOOT_DIRECT}
    Push "\Device\HarddiskVolume1"
    Call GetDriveLetterOfDevice
    Pop $c
    ${If} $c == error
      ${If} $windows_boot_method != ${BOOT_NTLDR}
	; If we are under Vista or Win7, try reading the partition from {current} entry of default BCD store
	Call GetDevicePathOfPartition
        Pop $0
        ${If} $0 != ""
          Push $0
          Call GetDriveLetterOfDevice
          Pop $c
          ${If} $c != error
            Goto c_is_initialized
          ${EndIf}
        ${EndIf}
      ${EndIf}
      ${GetRoot} $WINDIR $c
      MessageBox MB_OK|MB_ICONEXCLAMATION $(cant_find_system_partition)
    ${Endif}
    Goto c_is_initialized
  ${Endif}
  ${If} $windows_boot_method == ${BOOT_DIRECT}
    ; Doesn't really matter.
    ${GetRoot} $WINDIR $c
    Goto c_is_initialized
  ${Endif}
c_is_initialized:
  ; For the uninstaller
  WriteRegStr HKLM "${REGSTR_WIN32}" "system_drive" "$c"

  StrCpy $INSTDIR "$c\win32-loader"
  SetOutPath $INSTDIR

!ifdef PXE
  ${Choice_Present} expert 4
!else ;PXE
  ${Choice_Present} expert 3
!endif ;PXE
  ${Choice_Get} $0

  Var /GLOBAL expert
!ifdef PXE
  Var /GLOBAL pxe_mode
  ${If} $0 == "3"
    StrCpy $pxe_mode true
    StrCpy $expert false
  ${Else}
    StrCpy $pxe_mode false
!endif ; PXE
    ${If} $0 == "2"
      StrCpy $expert true
    ${Else}
      StrCpy $expert false
    ${Endif}
!ifdef PXE
  ${Endif} ; PXE checked ?
!endif ;PXE
FunctionEnd
