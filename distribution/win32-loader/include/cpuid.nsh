; Licensed under the zlib/libpng license (same as NSIS)

!ifndef CPUID_INCLUDED
!define CPUID_INCLUDED

!define CPUID_HELPER_DLL cpuid.dll
!ifndef CPUID_HELPER_DIR
  !error "Please define the directory containing ${CPUID_HELPER_DLL}"
!endif

!define CPUID_LONG_MODE 0x20000000

!define CPUID_PREFIX "CPUID_"

!include include\stdmacros.nsh
!include LogicLib.nsh

!define CPUID_FUNCDEF \
  `!insertmacro STDMACROS_FUNCDEF ${CPUID_PREFIX}`
!define CPUID_FUNCINC \
  `!insertmacro STDMACROS_FUNCINC ${CPUID_PREFIX}`
!define CPUID_FUNCPROLOG \
  `!insertmacro STDMACROS_FUNCPROLOG ${CPUID_PREFIX}`

; ${CPUID_Alloc} handle func
${CPUID_FUNCDEF} Alloc
!macro CPUID_Alloc_Call UN HANDLE FUNC
  Call `${UN}${CPUID_PREFIX}Alloc`
  Pop ${HANDLE}
  Pop ${FUNC}
!macroend

; ${CPUID_Free} handle
${CPUID_FUNCDEF} Free
!macro CPUID_Free_Call UN HANDLE
  Push ${HANDLE}
  Call `${UN}${CPUID_PREFIX}Free`
!macroend

; ${CPUID_DataWidth} width
${CPUID_FUNCDEF} DataWidth
!macro CPUID_DataWidth_Call UN WIDTH
  Call `${UN}${CPUID_PREFIX}DataWidth`
  Pop ${WIDTH}
!macroend

!macro CPUID_Alloc UN
${CPUID_FUNCINC} "${UN}" Free
; Unpack and load cpuid helper dll
; Result:
;   Handle to the loaded module or error if it could not be loaded
;   Function address to perform CPUID operation in case it is available otherwise error
${CPUID_FUNCPROLOG} "${UN}" Alloc
  Push $0
  Push $1

  StrCpy $1 "$PLUGINSDIR\${CPUID_HELPER_DLL}"
  ${IfNot} ${FileExists} "$1"
    ; Unpack cpuid helper dll
    Push $OUTDIR
    SetOutPath $PLUGINSDIR
    ReserveFile "${CPUID_HELPER_DIR}\${CPUID_HELPER_DLL}"
    File "/oname=${CPUID_HELPER_DLL}" "${CPUID_HELPER_DIR}\${CPUID_HELPER_DLL}"
    Pop $0
    SetOutPath $0
  ${EndIf}
  System::Call "kernel32::LoadLibrary(t r1) p.r0"
  ${If} $0 P<> 0
    System::Call "kernel32::GetProcAddress(p r0, m 'cpuid_available') p.r1"
    ${If} $1 P<> 0
      System::Call "::$1() i.r1"
      ${If} $1 != 0
        System::Call "kernel32::GetProcAddress(p r0, m 'cpuid') p.r1"
        ${If} $1 P= 0
          !insertmacro ${CPUID_PREFIX}Free_Call "${UN}" $0
	  StrCpy $0 0
        ${EndIf}
      ${EndIf}
    ${Else}
      StrCpy $1 error
    ${EndIf}
  ${Else}
    StrCpy $1 error
    StrCpy $0 error
  ${EndIf}

  Exch $1
  Exch
  Exch $0
FunctionEnd
!macroend ; CPUID_Alloc

!macro CPUID_Free UN
; Release the loaded cpuid helper module context
; Parameter:
;   handle - handle to the loaded module
${CPUID_FUNCPROLOG} "${UN}" Free
  Exch $0
  System::Call "kernel32::FreeLibrary(p r0)"
  Pop $0
FunctionEnd
!macroend ; CPUID_Free

!macro CPUID_DataWidth UN
${CPUID_FUNCINC} "${UN}" Alloc
${CPUID_FUNCINC} "${UN}" Free
; Get data width of x86/x86_64 processor
; Return value:
;   Data width of processor
${CPUID_FUNCPROLOG} "${UN}" DataWidth
  Push $0
  Push $1
  Push $2
  Push $3
  Push $4

  ; Set 32 bit as default
  StrCpy $0 32

  !insertmacro ${CPUID_PREFIX}Alloc_Call "${UN}" $1 $2
  ${If} $1 != error
  ${AndIf} $2 != error
  ${AndIf} $2 P<> 0
    System::Call "*(&i4, &i4, &i4, &i4) p.r3"
    ${If} $3 P<> 0
      ; Get highest function parameter
      System::Call "::$2(p r3, i 0)"
      System::Call "*$3(&i4 .r4)"
      ${If} $4 != 0
        ; Get highest implemented extended function
        System::Call "::$2(p r3, i 0x80000000)"
        System::Call "*$3(&i4 .r4)"
        IntOp $4 $4 & 0x80000000
        ${If} $4 != 0
          ; Get extended processor information and feature bits
          System::Call "::$2(p r3, i 0x80000001)"
          System::Call "*$3(&i4,&i4,&i4,&i4.r4)"
          IntOp $4 $4 & ${CPUID_LONG_MODE}
          ${If} $4 != 0
            StrCpy $0 64
          ${EndIf}
        ${EndIf}
      ${EndIf}
      System::Free $3
    ${EndIf}
    !insertmacro ${CPUID_PREFIX}Free_Call "${UN}" $1
  ${EndIf}

  Pop $4
  Pop $3
  Pop $2
  Pop $1
  Exch $0
FunctionEnd
!macroend ; CPUID_DataWidth

!endif ; CPUID_INCLUDED
