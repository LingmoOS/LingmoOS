; $Id: mingw-cdk.nsi,v 1.43 2023/02/02 01:15:18 tom Exp $

; TODO add examples

; Define the application name
!define APPNAME "libcdk5"
!define EXENAME "libcdk5.exe"

!define VERSION_MAJOR "5"
!define VERSION_MINOR "0"
!define VERSION_YYYY  "2023"
!define VERSION_MMDD  "201"
!define VERSION_PATCH ${VERSION_YYYY}${VERSION_MMDD}

!define SUBKEY "libcdk5"

!define INSTALL "${APPNAME} (Console)"
!define VERSION ${VERSION_MAJOR}.${VERSION_MINOR}
!define VERSION_FULL  ${VERSION}-${VERSION_PATCH}

; Main Install settings
Name "${INSTALL}"
InstallDir "c:\mingw"
InstallDirRegKey HKLM "Software\${SUBKEY}" "$INSTDIR\bin"
OutFile "NSIS-Output\${APPNAME}-${VERSION_FULL}-setup.exe"

CRCCheck on
SetCompressor /SOLID lzma

VIAddVersionKey ProductName "${SUBKEY}"
VIAddVersionKey CompanyName "http://invisible-island.net"
VIAddVersionKey LegalCopyright "© 1999-2021,2022, Thomas E. Dickey"
VIAddVersionKey FileDescription "Cdk Installer (MinGW)"
VIAddVersionKey FileVersion ${VERSION_FULL}
VIAddVersionKey ProductVersion ${VERSION_FULL}
VIAddVersionKey Comments "This installer was built with NSIS and cross-compiling to MinGW."
VIAddVersionKey InternalName "${APPNAME}-${VERSION_FULL}-setup.exe"
; This is a dotted set of numbers limited to 16-bits each
VIProductVersion "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_YYYY}.${VERSION_MMDD}"

; Modern interface settings
!include "MUI.nsh"

!define MUI_ABORTWARNING
;!define MUI_FINISHPAGE_RUN
;"$INSTDIR\bin\${EXENAME}"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\COPYING"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

; Set languages (first is default language)
!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_RESERVEFILE_LANGDLL

InstType "Full"		; SectionIn 1
InstType "Typical"	; SectionIn 2
InstType "Minimal"	; SectionIn 3

Section "${APPNAME}" Section1

	SectionIn 1 2 3

	; Set Section properties
	SetOverwrite on

	; Set Section Files and Shortcuts
	SetOutPath "$INSTDIR\bin"

	File ".\bin\*.dll"

	SetOutPath "$INSTDIR\share\${APPNAME}"
	File /oname=README.txt ".\share\doc\cdk\README"

	CreateDirectory "$SMPROGRAMS\${INSTALL}"
	CreateShortCut "$SMPROGRAMS\${INSTALL}\${APPNAME}.lnk" "$INSTDIR\bin\${EXENAME}"
	CreateShortCut "$SMPROGRAMS\${INSTALL}\Uninstall.lnk" "$INSTDIR\uninstall.exe"

SectionEnd

Section "development" Section2

	SectionIn 1 2

	; Set Section properties
	SetOverwrite on

	; Set Section Files and Shortcuts
	SetOutPath "$INSTDIR\share\${APPNAME}"

	SetOutPath "$INSTDIR\include\cdk"

	File ".\include\cdk\*.h"

	SetOutPath "$INSTDIR\lib"

	File ".\lib\*.dll"
	File ".\lib\*.a"

SectionEnd

Section "examples" Section3

	SectionIn 1

	; Set Section properties
	SetOverwrite on

	; Set Section Files and Shortcuts
	SetOutPath "$INSTDIR\lib\${APPNAME}"

SectionEnd

Section -FinishSection

	WriteRegStr HKLM "Software\${SUBKEY}" "" "$INSTDIR"
	WriteRegStr HKLM "Software\${SUBKEY}" "Environment" ""
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${INSTALL}" "DisplayName" "${APPNAME} ${VERSION_FULL} (Console)"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${INSTALL}" "UninstallString" "$INSTDIR\uninstall.exe"
	WriteUninstaller "$INSTDIR\uninstall.exe"

SectionEnd

; Modern install component descriptions
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
	!insertmacro MUI_DESCRIPTION_TEXT ${Section1} "${SUBKEY} runtime"
	!insertmacro MUI_DESCRIPTION_TEXT ${Section2} "Development headers and libraries"
	!insertmacro MUI_DESCRIPTION_TEXT ${Section3} "Examples"
!insertmacro MUI_FUNCTION_DESCRIPTION_END

;Uninstall section
Section Uninstall

	;Remove from registry...
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${INSTALL}"
	DeleteRegKey HKLM "SOFTWARE\${SUBKEY}"

	; Delete self
	Delete "$INSTDIR\uninstall.exe"

	; Delete Shortcuts
	Delete "$SMPROGRAMS\${INSTALL}\${APPNAME}.lnk"
	Delete "$SMPROGRAMS\${INSTALL}\Uninstall.lnk"

	; Clean up application
	Delete "$INSTDIR\bin\libcdk5.dll"

	Delete "$INSTDIR\include\${APPNAME}\*.h"

	Delete "$INSTDIR\lib\libcdk5.a"

	Delete "$INSTDIR\lib\libcdk5.dll.a"

	Delete "$INSTDIR\lib\${APPNAME}\*.exe"

	Delete "$INSTDIR\share\${APPNAME}\*.*"

	; Remove remaining directories
	RMDir "$SMPROGRAMS\${INSTALL}"
	RMDir "$INSTDIR\share\${APPNAME}"
	RMDir "$INSTDIR\share"
	RMDir "$INSTDIR\lib\${APPNAME}"
	RMDir "$INSTDIR\lib"
	RMDir "$INSTDIR\include\${APPNAME}"
	RMDir "$INSTDIR\include"
	RMDir "$INSTDIR\bin"
	RMDir "$INSTDIR\"

SectionEnd