; Debian-Installer Loader - Initialisation
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

${Security_COMSetDefaultLevel}
${un.Security_COMSetDefaultLevel}

Function .onInit
  InitPluginsDir
  ${Security_COMSetDefaultLevel} $0

!ifdef TARGET_DISTRO
  StrCpy $target_distro "${TARGET_DISTRO}"
!else
  StrCpy $target_distro "Debian"
!endif

  ; needed by: timezones, keymaps, languages
  File /oname=$PLUGINSDIR\maps.ini maps.ini

  ; default to English
  StrCpy $LANGUAGE ${LANG_ENGLISH}

  ; Language selection dialog
  Push ""
!include ${BUILD_DIR}/l10n/templates/dialog.nsh
  Push unsupported
  Push "-- Not in this list --"

  Push A ; A means auto count languages
         ; for the auto count to work the first empty push (Push "") must remain
  LangDLL::LangDialog "Choose language" "Please choose the language used for \
the installation process. This language will be the default language for the \
final system."

  Pop $LANGUAGE
  ${If} $LANGUAGE == "cancel"
    Abort
  ${Endif}

  ; Note: Possible API abuse here.  Nsis *seems* to fallback sanely to English
  ; when $LANGUAGE == "unsupported", so we'll use that to decide wether to
  ; preseed later.

  Var /GLOBAL unsupported_language
  ${If} $LANGUAGE == "unsupported"
    StrCpy $unsupported_language true
    ; Translators: your language is supported by d-i, but not yet by nsis.
    ; Please get your translation in nsis before translating win32-loader.
    MessageBox MB_OK "Because your language is not supported by this \
stage of the installer, English will be used for now.  On the second \
(and last) stage of the install process, you will be offered a much \
wider choice, where your language is more likely to be present."
    ; Test in Windows 7 shows a somehow random choice, force it to LANG_ENGLISH.
    StrCpy $LANGUAGE ${LANG_ENGLISH}
  ${Else}
    StrCpy $unsupported_language false
  ${Endif}
FunctionEnd

Function Cancelled
  ${If} $cancel == ""
    StrCpy $cancel 1
    ${If} $mui.InstFilesPage != ""
      EnableWindow $mui.Button.Cancel 0
      Abort
    ${EndIf}
  ${EndIf}
FunctionEnd

Function un.onInit
  ${un.Security_COMSetDefaultLevel} $0
FunctionEnd
