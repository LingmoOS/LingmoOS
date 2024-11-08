; Licensed under the zlib/libpng license (same as NSIS)

; cpio test runner
; Run installer with the following arguments
;  /IN=<file> /OUT=<cpio archive> /RESULT=<result file>
; and for the uninstaller append the argument below:
; _?=<directory containing uninstaller>

Unicode True
Name cpio
RequestExecutionLevel user

!define UNFUNC "un."

!include FileFunc.nsh

!insertmacro GetParameters
!insertmacro GetOptions

!include include/cpio.nsh
${CPIO_Write}

Page InstFiles
UninstPage InstFiles

Var Arguments

!macro Run UN
!searchreplace INIT_FUNC "${UN}.onInit" ".." "."
Function ${INIT_FUNC}
  InitPluginsDir
  ${GetParameters} $Arguments
  ClearErrors
  SetOutPath "$EXEDIR"
FunctionEnd

${CPIO_FUNCINC} "${UN}" Write

Section ${UN}GenerateCPIO
  StrCpy $0 ${ERROR_WRITE_FAULT}
  ${GetOptions} $Arguments "/IN=" $1
  ${GetOptions} $Arguments "/OUT=" $2
  ${GetOptions} $Arguments "/RESULT=" $3
  FileOpen $4 "$3" w
  ${IfNot} ${Errors}
    FileOpen $5 "$2" w
    ${IfNot} ${Errors}
      !insertmacro ${CPIO_PREFIX}Write_Call "${UN}" $5 $1 $0
      ${If} $0 == 0
        !insertmacro ${CPIO_PREFIX}Write_Call "${UN}" $5 "" $0
      ${EndIf}
      FileClose $5
    ${EndIf}
    FileWrite $4 "$0$\n"
    FileClose $4
  ${EndIf}
SectionEnd
!macroend

!insertmacro Run ""
!insertmacro Run ${UNFUNC}

Section "Install"
  WriteUninstaller "$OUTDIR\uninstall.exe"
SectionEnd

Section "Uninstall"
SectionEnd
