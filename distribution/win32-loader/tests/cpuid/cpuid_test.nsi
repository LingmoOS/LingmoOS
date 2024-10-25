; Licensed under the zlib/libpng license (same as NSIS)

; CPUID test runner
; Run installer with the following arguments
;  /RESULT=<result file>
; and for the uninstaller append the argument below:
; _?=<directory containing uninstaller>

Unicode True
Name cpuid
RequestExecutionLevel user

!define UNFUNC "un."

!include FileFunc.nsh

!insertmacro GetParameters
!insertmacro GetOptions

!include include\cpuid.nsh

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

${CPUID_FUNCINC} "${UN}" Alloc
${CPUID_FUNCINC} "${UN}" Free

Function ${UN}${__MACRO__}
  ${GetOptions} $Arguments /RESULT= $1
  ClearErrors

  ${If} $1 != ""
    FileOpen $2 "$1" w
    ${IfNot} ${Errors}
      !insertmacro ${CPUID_PREFIX}Alloc_Call "${UN}" $3 $4
      ${If} $3 != error
      ${AndIf} $4 != error
      ${AndIf} $4 P<> 0
        System::Call "*(&i4, &i4, &i4, &i4) p.r5"
        ${If} $5 P<> 0
          ; Get vendor ID via cpuid function
          System::Call "::$4(p r5, i 0)"
          System::Call "*$5(&i4, &m4 .s, &m4 .s, &m4 .s)"
          Pop $0
          Exch
          Pop $1
          StrCpy $0 "$0$1"
          Pop $1
          StrCpy $0 "$0$1"
          System::Free $5
        ${EndIf}
        FileWrite $2 "$0$\n"
        !insertmacro ${CPUID_PREFIX}Free_Call "${UN}" $3
      ${Else}
        FileWrite $2 "Failed to open cpuid module$\n"
      ${EndIf}
      FileClose $3
    ${EndIf}
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
  Call un.Run
SectionEnd
