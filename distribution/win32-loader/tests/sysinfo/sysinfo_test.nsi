; Licensed under the zlib/libpng license (same as NSIS)

; System information test runner
; Run installer with the following arguments
;  /TEST=<test> /RESULT=<result file>
;  <test> can be one of the following:
;  Domain HostName KeyboardLayout UserName
; and for the uninstaller append the argument below:
; _?=<directory containing uninstaller>

Unicode True
Name sysinfo
RequestExecutionLevel user

!define UNFUNC "un."

!include FileFunc.nsh

!insertmacro GetParameters
!insertmacro GetOptions

!include include\sysinfo.nsh

Page InstFiles
UninstPage InstFiles

Var Arguments

; Perfect mapping of test name to section identifier
;   Formula: upper case ordinal value of first character modulo 6
!macro MAP VALUE
  !insertmacro STDMACROS_ToSystemReg ${VALUE} ${__MACRO__}_VALUE
  ; Get ordinal value of first character
  System::Call "*(t ${${__MACRO__}_VALUE}) p.${${__MACRO__}_VALUE}"
  ${If} ${VALUE} P<> 0
    Push ${VALUE}
    System::Call "*${VALUE}(p .${${__MACRO__}_VALUE})"
    System::Call "*${VALUE}(&i${NSIS_CHAR_SIZE} .s)"
    Exch
    Pop ${VALUE}
    System::Free ${VALUE}
    Pop ${VALUE}
  ${EndIf}
  ; Convert to upper case ordinal value
  IntOp ${VALUE} ${VALUE} & 0xDF
  ; Modulo 6 operation
  IntOp ${VALUE} ${VALUE} % 6
!macroend

!macro INSERT_SECTION UN FUNCTION
Section /o ${UN}${FUNCTION} Sec${UN}${FUNCTION}
  Push $0
  !insertmacro `${SYSINFO_PREFIX}${FUNCTION}_Call` "${UN}" $0
  Push "$0"
  Call ${UN}WriteResult
  Pop $0
SectionEnd
!macroend

!macro Run UN
!searchreplace INIT_FUNC "${UN}.onInit" ".." "."
Function ${INIT_FUNC}
  Push $0
  InitPluginsDir
  ${GetParameters} $Arguments
  ClearErrors
  SetOutPath "$EXEDIR"
  ${GetOptions} $Arguments "/TEST=" $0
  ${If} $0 != ""
    !insertmacro MAP $0
    SectionSetFlags $0 ${SF_SELECTED}
  ${EndIf}
  Pop $0
FunctionEnd

${SYSINFO_FUNCINC} "${UN}" Domain
${SYSINFO_FUNCINC} "${UN}" HostName
${SYSINFO_FUNCINC} "${UN}" KeyboardLayout
${SYSINFO_FUNCINC} "${UN}" UserName

Function ${UN}WriteResult
  Exch $0
  Push $1
  ${GetOptions} $Arguments "/RESULT=" $1
  ClearErrors
  ${If} $1 != ""
    FileOpen $1 "$1" w
    ${IfNot} ${Errors}
      FileWrite $1 "$0$\n"
      FileClose $1
    ${EndIf}
  ${EndIf}
  Pop $1
  Pop $0
FunctionEnd

; Domain -> D -> 68 -> 68 % 6 -> 2 -> 3rd section
; HostName -> H -> 72 -> 72 % 6 -> 0 -> 1st section
; KeyboardLayout -> K -> 75 -> 75 % 6 -> 3 -> 4th section
; UserName -> U -> 85 -> 85 % 6 -> 1 -> 2nd section
!insertmacro INSERT_SECTION "${UN}" HostName
!insertmacro INSERT_SECTION "${UN}" UserName
!insertmacro INSERT_SECTION "${UN}" Domain
!insertmacro INSERT_SECTION "${UN}" KeyboardLayout
!macroend

!insertmacro Run ""
!insertmacro Run ${UNFUNC}

Section "Install"
  WriteUninstaller "$OUTDIR\uninstall.exe"
SectionEnd

Section "Uninstall"
SectionEnd
