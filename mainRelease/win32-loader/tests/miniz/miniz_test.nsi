; Licensed under the zlib/libpng license (same as NSIS)

; miniz test runner
; Run installer with the following arguments
;  /S /IN=<file> /OUT=<gzipfile> /RESULT=<result file>
; and for the uninstaller append the argument below:
; _?=<directory containing uninstaller>
Unicode True
Name miniz
RequestExecutionLevel user

!define UNFUNC "un."

!define CPIO_GZ_EXT ".cpio.gz"
!define CPIO_GZ_LEN 8

!include FileFunc.nsh

!insertmacro GetParameters
!insertmacro GetOptions

!include include\miniz.nsh

; Pages
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

${MINIZ_FUNCINC} "${UN}" CPIO_GZ_Open
${MINIZ_FUNCINC} "${UN}" CPIO_GZ_Write
${MINIZ_FUNCINC} "${UN}" CPIO_GZ_Close

${MINIZ_FUNCINC} "${UN}" GZip

Function ${UN}${__MACRO__}
  StrCpy $0 ${ERROR_WRITE_FAULT}
  ${GetOptions} $Arguments "/IN=" $1
  ${GetOptions} $Arguments "/OUT=" $2
  ${GetOptions} $Arguments "/RESULT=" $3

  FileOpen $4 "$3" w
  ${IfNot} ${Errors}
    StrCpy $7 $2 ${CPIO_GZ_LEN} -${CPIO_GZ_LEN}
    ${If} $7 != ${CPIO_GZ_EXT}
      FileOpen $5 "$1" r
      ${IfNot} ${Errors}
        FileOpen $6 "$2" w
        ${IfNot} ${Errors}
          !insertmacro MINIZ_GZip_Call "${UN}" $5 $6 $0
          FileClose $6
        ${EndIf}
        FileClose $5
      ${Else}
        StrCpy $0 ${ERROR_FILE_NOT_FOUND}
      ${EndIf}
    ${Else}
      FileOpen $5 "$2" w
      ${IfNot} ${Errors}
        !insertmacro MINIZ_CPIO_GZ_Open_Call "${UN}" $5 $6
        ${If} $6 P<> 0
          !insertmacro MINIZ_CPIO_GZ_Write_Call "${UN}" $6 $1 $0
          ${If} $0 == 0
            !insertmacro MINIZ_CPIO_GZ_Write_Call "${UN}" $6 "" $0
          ${EndIf}
          !insertmacro MINIZ_CPIO_GZ_Close_Call "${UN}" $6 $6
	  ${If} $0 == 0
          ${AndIf} $6 != 0
            StrCpy $0 $6
          ${EndIf}
        ${EndIf}
        FileClose $5
      ${EndIf}
    ${EndIf}
    FileWrite $4 "$0$\n"
    FileClose $4
  ${EndIf}
FunctionEnd
!macroend

!insertmacro Run ""
!insertmacro Run ${UNFUNC}

Section "Install"
  Call Run
  WriteUninstaller "$OUTDIR\uninstall.exe"
SectionEnd

Section "Uninstall"
  Call ${UNFUNC}Run
SectionEnd
