; Licensed under the zlib/libpng license (same as NSIS)

; SHA256 test runner
; Run installer with the following arguments
;  /S /FILE=<reference file> /RESULT=<result file>
; and for the uninstaller append the argument below:
; _?=<directory containing uninstaller>

Unicode True
Name sha256
RequestExecutionLevel user

!define UNFUNC "un."

!include FileFunc.nsh

!insertmacro GetParameters
!insertmacro GetOptions

!ifndef NO_SHA256_HELPER
!include include\sha256.nsh
!else
!include include\crypthash.nsh
!endif

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

!ifndef NO_SHA256_HELPER
${SHA256_FUNCINC} "${UN}" Alloc
${SHA256_FUNCINC} "${UN}" Hash
${SHA256_FUNCINC} "${UN}" Free
!else
${CRYPTHASH_FUNCINC} "${UN}" ContextAcquire
${CRYPTHASH_FUNCINC} "${UN}" SHA256Digest
${CRYPTHASH_FUNCINC} "${UN}" ContextRelease
!endif

Function ${UN}${__MACRO__}
  ${GetOptions} $Arguments /FILE= $1
  ${GetOptions} $Arguments /RESULT= $2
  ClearErrors

  ${If} $1 != ""
  ${AndIf} $2 != ""
    FileOpen $3 "$2" w
    ${IfNot} ${Errors}
      FileOpen $4 "$1" r
      ${IfNot} ${Errors}
!ifndef NO_SHA256_HELPER
        !insertmacro ${SHA256_PREFIX}Alloc_Call "${UN}" $5
        ${If} $5 P<> 0
          !insertmacro ${SHA256_PREFIX}Hash_Call "${UN}" $5 $4 $0
          !insertmacro ${SHA256_PREFIX}Free_Call "${UN}" $4
        ${Else}
          FileWrite $3 "Failed to open SHA256 hash module$\n"
        ${EndIf}
!else
	!insertmacro CRYPTHASH_ContextAcquire_Call "${UN}" ${PROV_RSA_AES} ${CRYPT_VERIFYCONTEXT} $5
        ${If} $5 P<> 0
          !insertmacro ${CRYPTHASH_PREFIX}SHA256Digest_Call "${UN}" $5 $4 $0
          !insertmacro ${CRYPTHASH_PREFIX}ContextRelease_Call "${UN}" $5 $5
        ${Else}
          FileWrite $3 "Failed to acquire crypto context$\n"
        ${EndIf}
!endif
        FileClose $4
      ${Else}
        StrCpy $0 "Unable to open $1"
      ${EndIf}
      FileWrite $3 "$0$\n"
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
