; Debian-Installer Loader - Final customisation
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

${STDMACROS_DefIfNotDef} GW_HWNDNEXT 2
${STDMACROS_DefIfNotDef} GW_CHILD 5

Function WritePreSeedCfg
  System::Store 'S'

  ClearErrors
  FileOpen $3 ${PRESEED_CFG} w
  ${IfNot} ${Errors}
    ; ********************************************** preseed rescue
    ${If} $rescue == true
      FileWrite $3 "d-i rescue/enable boolean true$\n"
    ${EndIf}

    ; ********************************************** preseed locale
    ${If} $unsupported_language == false
      ReadINIStr $0 $PLUGINSDIR\maps.ini "languages" "$LANGUAGE"
      ReadRegStr $1 HKCU "Control Panel\International" iCountry
      ReadINIStr $1 $PLUGINSDIR\maps.ini "countries" "$1"
      ${If} $0 != ""
        ${If} $1 != ""
          StrCpy $0 "$0_$1"
        ${Endif}
        FileWrite $3 "d-i debian-installer/locale string $0$\n"
      ${Endif}
    ${Endif}

    ; ********************************************** preseed timezone
    ReadRegStr $0 HKLM SYSTEM\CurrentControlSet\Control\TimeZoneInformation TimeZoneKeyName
    ${If} $0 == ""
      ReadRegStr $0 HKLM SYSTEM\CurrentControlSet\Control\TimeZoneInformation StandardName
    ${Endif}
    ReadINIStr $0 $PLUGINSDIR\maps.ini "timezones" "$0"
    ${If} $0 != ""
      FileWrite $3 "d-i time/zone string $0$\nd-i time/zone seen false$\n"
    ${Endif}

    ; ********************************************** preseed keymap
    ${SYSINFO_KeyboardLayout} $0
    ReadINIStr $0 $PLUGINSDIR\maps.ini "keymaps" "$0"
    ; FIXME: do we need to support non-AT keyboards here?
    ${If} $0 != ""
      ${If} $expert == true
        MessageBox MB_YESNO|MB_ICONQUESTION $(detected_keyboard_is) IDNO keyboard_bad_guess
        FileWrite $3 "d-i console-keymaps-at/keymap select $0$\n\
d-i console-keymaps-at/keymap seen true$\n"
        Goto keyboard_end
keyboard_bad_guess:
        MessageBox MB_OK $(keyboard_bug_report)
keyboard_end:
      ${Else}
        FileWrite $3 "d-i console-keymaps-at/keymap select $0$\n\
d-i console-keymaps-at/keymap seen false$\n"
      ${Endif}
    ${Endif}

    ; ********************************************** preseed hostname
    ${SYSINFO_HostName} $0
    ${If} $0 != ""
      FileWrite $3 "d-i netcfg/get_hostname string $0$\n\
d-i netcfg/get_hostname seen false$\n"
    ${EndIf}

    ; ********************************************** preseed domain
    ${SYSINFO_Domain} $0
    ${If} $0 != ""
      FileWrite $3 "d-i netcfg/get_domain string $0$\n\
d-i netcfg/get_domain seen false$\n"
    ${EndIf}

    ; ********************************************** preseed user-fullname
    ${SYSINFO_UserName} $0
    ${If} $0 != ""
      FileWrite $3 "d-i passwd/user-fullname string $0$\n\
d-i passwd/user-fullname seen false$\n"
    ${Endif}

    ; ********************************************** preseed proxy
    ${If} $proxy == ""
      FileWrite $3 "d-i mirror/http/proxy seen true$\n"
    ${Else}
      FileWrite $3 "d-i mirror/http/proxy string http://$proxy/$\n"
    ${Endif}
    FileClose $3
  ${EndIf}

  System::Store 'L'
FunctionEnd

Function ShowCustom
; Gather all the missing information before ShowCustom is displayed
  System::Store 'S'
!ifndef NOCD

; ********************************************** Media-based install
  Var /GLOBAL g2ldr
  Var /GLOBAL g2ldr_mbr
  ReadINIStr $g2ldr	$d\win32-loader.ini "grub" "g2ldr"
  ReadINIStr $g2ldr_mbr	$d\win32-loader.ini "grub" "g2ldr.mbr"
  StrCmp $g2ldr		"" incomplete_ini
  StrCmp $g2ldr_mbr	"" incomplete_ini
  ${If} $kernel == "linux"
    Var /GLOBAL linux
    Var /GLOBAL initrd
    ReadINIStr $linux	$d\win32-loader.ini "installer" "$arch/$gtklinux"
    ReadINIStr $initrd	$d\win32-loader.ini "installer" "$arch/$gtkinitrd"
    StrCmp $linux	"" incomplete_ini
    StrCmp $initrd	"" incomplete_ini
    Goto ini_is_ok
  ${ElseIf} $kernel == "kfreebsd"
    Var /GLOBAL kfreebsd
    Var /GLOBAL kfreebsd_module
    ReadINIStr $kfreebsd	$d\win32-loader.ini "installer" "kfreebsd-$arch/$gtkkfreebsd"
    ReadINIStr $kfreebsd_module	$d\win32-loader.ini "installer" "kfreebsd-$arch/$gtkkfreebsd_module"
    StrCmp $kfreebsd		"" incomplete_ini
    StrCmp $kfreebsd_module	"" incomplete_ini
  ${ElseIf} $kernel == "hurd"
    Var /GLOBAL gnumach
    Var /GLOBAL ext2fs
    Var /GLOBAL ld
    ReadINIStr $gnumach		$d\win32-loader.ini "installer" "hurd-$arch/$gtkgnumach"
    ReadINIStr $ext2fs		$d\win32-loader.ini "installer" "hurd-$arch/$gtkext2fs"
    ReadINIStr $initrd		$d\win32-loader.ini "installer" "hurd-$arch/$gtkinitrd"
    ReadINIStr $ld		$d\win32-loader.ini "installer" "hurd-$arch/$gtklt"
    StrCmp $gnumach		"" incomplete_ini
    StrCmp $ext2fs		"" incomplete_ini
    StrCmp $initrd		"" incomplete_ini
    StrCmp $ld			"" incomplete_ini
  ${EndIf}
  Goto ini_is_ok
incomplete_ini:
  MessageBox MB_OK|MB_ICONSTOP "$(error_incomplete_ini)"
  Quit
ini_is_ok:

!endif

; ********************************************** Initialise proxy (even when in network-less mode, for the sake of preseeding)
  StrCpy $proxy ""
  ReadRegDWORD $0 HKCU "Software\Microsoft\Windows\CurrentVersion\Internet Settings" ProxyEnable
  IntCmp $0 1 0 proxyless
  ReadRegStr $0 HKCU "Software\Microsoft\Windows\CurrentVersion\Internet Settings" ProxyServer
  StrCmp $0 "" proxyless
  StrCpy $proxy "$0"
proxyless:

; ********************************************** preseed priority
  ${If} $expert == true
    StrCpy $preseed_cmdline "$preseed_cmdline priority=low"
  ${Endif}

; ********************************************** Display customisation dialog now
  ${If} $expert == true
    nsDialogs::Create ${IDC_CHILDRECT}
    Pop $0
    ${If} $0 != error
      !insertmacro MUI_HEADER_TEXT $(^NameDA) $(custom1)
      ${NSD_CreateText} 116u 10u -119u 13u "$proxy"
      Pop $1
      ${NSD_CreateLabel} 0u 12u 113u 8u $(custom2)
      Pop $1
      ${NSD_CreateLabel} 0u 48u -3u 8u $(custom5)
      Pop $1
!ifdef NOCD
      StrCpy $2 "$base_url"
!else
      StrCpy $2 ""
!endif
      ${NSD_CreateText} 0u 60u -3u 13u "$2"
      Pop $1
      ${NSD_CreateLabel} 0u 82u 113u 8u "Kernel commandline:"
      Pop $1
      ${NSD_CreateText} 0u 94u -3u 13u "$preseed_cmdline"
      Pop $1
      nsDialogs::show
    ${EndIf}
  ${Else}
    Call WritePreSeedCfg
  ${Endif}

  System::Store 'L'
FunctionEnd

Function LeaveCustom
  System::Store 'S'

  GetLabelAddress $3 _${__FUNCTION__}_SetVar
  ; Get inner dialog
  FindWindow $0 "#32770" "" $HWNDPARENT
  ${If} $0 P<> 0
    ; Retrieve content from all edit controls of the inner dialog
    ; and push it to stack
    System::Call "user32::GetWindow(p r0, i ${GW_CHILD}) p.r0"
    ${While} $0 P<> 0
      System::Call "user32::GetClassName(p r0, t .r1, i ${NSIS_MAX_STRLEN}) i.r2"
      ${If} $1 != 0
      ${AndIf} $1 == "EDIT"
        ${NSD_GetText} $0 $1
        Goto $3
_${__FUNCTION__}_SetVar:
        ; 1st edit field: proxy
        StrCpy $proxy "$1"
        Goto _${__FUNCTION__}_NextVar
        ; 2nd edit field: base_url
!ifdef NOCD
        StrCpy $base_url "$1"
!else
        StrCpy $1 ""
!endif
        Goto _${__FUNCTION__}_NextVar
        ; Last edit field: preseed_cmdline
        StrCpy $preseed_cmdline "$1"
        Goto _${__FUNCTION__}_LastVar
_${__FUNCTION__}_NextVar:
        IntOp $3 $3 + 2
_${__FUNCTION__}_LastVar:
      ${EndIf}
      System::Call "user32::GetWindow(p r0, i ${GW_HWNDNEXT}) p.r0"
    ${EndWhile}
  ${EndIf}

!ifdef NOCD
; Ensure $base_url is ending with /
  StrCpy $0 "$base_url" 1 -1
  ${If} $0 != "/"
    StrCpy $base_url "$base_url/"
  ${EndIf}
!endif

  Call WritePreSeedCfg

  System::Store 'L'
FunctionEnd
