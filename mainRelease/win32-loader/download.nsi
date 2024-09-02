; Debian-Installer Loader - Download functions
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

${STDMACROS_DefIfNotDef} IDCANCEL 2
${STDMACROS_DefIfNotDef} IDC_STATIC1 1001
${STDMACROS_DefIfNotDef} IDC_PROGRESS 1004
${STDMACROS_DefIfNotDef} IDC_PROGRESS1 1005
${STDMACROS_DefIfNotDef} IDC_INTROTEXT 1006
${STDMACROS_DefIfNotDef} IDC_SHOWDETAILS 1027

!ifndef NO_SHA256_HELPER
!define SHA256_HELPER_DIR "${BUILD_DIR}/helpers/sha256"
!endif

!define WININET_CANCEL_FLAG $cancel
!define WININET_PROGRESSBAR_CONTROL ${IDC_PROGRESS1}
!define WININET_PROXY $proxy
!define WININET_STATUSTEXT_CONTROL ${IDC_STATIC1}
!define WININET_TEXT_CONNECTING $(connecting)
!define WININET_TEXT_PROGRESS $(progress)
!define WININET_TIMEOUT 10000
!define WININET_USER_AGENT "Mozilla/4.0 (compatible; Win32; ${_PRODUCT_NAME})"

!include include\wininet.nsh
${WININET_Get}

ChangeUI IDD_INSTFILES "${BUILD_DIR}/helpers/instdlg/instdlg.exe"

; Show window controls for reporting progress of download
; Parameter:
;   flag - 0: hide 1: show
Function Download_ShowControls
  Exch $0
!ifdef MUI_INCLUDED
  ${IfNot} ${Silent}
  ${AndIf} $mui.InstFilesPage P<> 0
    ${If} $0 != 0
      StrCpy $0 ${SW_NORMAL}
      SetDetailsView hide
      EnableWindow $mui.InstFilesPage.ShowLogButton 0
    ${Else}
      StrCpy $0 ${SW_HIDE}
      EnableWindow $mui.InstFilesPage.ShowLogButton 1
    ${EndIf}

    Push $1
    GetDlgItem $1 $mui.InstFilesPage ${IDC_STATIC1}
    ${If} $1 P<> 0
      ShowWindow $1 $0
    ${EndIf}
    GetDlgItem $1 $mui.InstFilesPage ${IDC_PROGRESS1}
    ${If} $1 P<> 0
      ShowWindow $1 $0
    ${EndIf}
    Pop $1
  ${EndIf}
!endif ; MUI_INCLUDED

  Push $0
FunctionEnd

Function Download
    System::Store 'S'
    ; Base URL
    Pop $0
    ; Target dir
    Pop $6
    ; Filename
    Pop $2
    ; Allow it to fail?
    Pop $3
    ; Check its sha256sum ?
    Pop $4

    DetailPrint "GET: $0/$2"
    ${If} $4 != "false"
      StrCpy $5 ${CALG_SHA256}
    ${Else}
      StrCpy $5 0
      StrCpy $4 0
    ${EndIf}
    ${WININET_Get} $0 $2 $6 $5 $1
    ${If} $1 != ${ERROR_CANCELLED}
      ${If} $1 != $4
        ${If} $4 == 0
          MessageBox MB_OK|MB_ICONSTOP "$(error): $0/$2 $1"
          ${If} $3 == "false"
            Quit
          ${EndIf}
        ${Else}
          MessageBox MB_OK|MB_ICONSTOP "$(checksum_mismatch)"
          Quit
        ${EndIf}
      ${Else}
        StrCpy $1 "success"
      ${EndIf}
    ${Else}
      Quit
    ${EndIf}

    Push "$1"
    System::Store 'L'
FunctionEnd

Function Get_SHA256_ref
;Line is like:
; d62f7850ef1a0a381c1e856936761ddd678e16dfdab90685de2deaf248f8d655      108 contrib/debian-installer/binary-amd64/Release
;or like;
;8716fec256b2df0b3e7c2f45a90813285b28fe7a72e4925c14d456d40af4caa1  ./netboot/debian-installer/amd64/linux
    System::Store 'S'
    ; Url we are looking for
    Pop $0
    ; Release file
    Pop $1
    ClearErrors
    FileOpen $3 $1 r
    IfErrors sha256_done ; Make sure it's readable
  sha256_readline:
    FileRead $3 $4
    IfErrors sha256_endoffile
    StrCpy $5 $4 -1 75 ; space + sha256sum + space-leftpadded size, -1 to get rid of newline
    ; Compare what we want with what we have
    StrCmpS $0 $5 0 sha256_alternativesyntax
    StrCpy $0 $4 64 1 ; Copying the found sha256sum to $0
    Push $0
    Goto sha256_done
  sha256_alternativesyntax:
    StrCpy $5 $4 -1 68 ; sha256sum + spaces and ./, -1 to get rid of newline
    ; Compare what we want with what we have
    StrCmpS $0 $5 0 sha256_readline
    StrCpy $0 $4 64 ; Copying the found sha256sum to $0
    Push $0
    Goto sha256_done
  sha256_endoffile:
    Push "false-sha256"
  sha256_done:
    FileClose $3
    System::Store 'L'
FunctionEnd
