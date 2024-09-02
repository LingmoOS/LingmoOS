; Licensed under the zlib/libpng license (same as NSIS)

!ifndef SHA256_INCLUDED
!define SHA256_INCLUDED

!define SHA256_BLOCK_LEN 64
!define SHA256_LENGTH_SIZE 8
!define SHA256_HELPER_DLL sha256.dll
!searchreplace SHA256_NAME ${SHA256_HELPER_DLL} ".dll" ""
!ifndef SHA256_HELPER_DIR
  !error "Please define the directory containing ${SHA256_HELPER_DLL}"
!endif

!define SHA256_PREFIX "SHA256_"
!define SHA256_UNFUNC "un."

!include include\stdmacros.nsh
!include LogicLib.nsh

!define SHA256_FUNCDEF \
  `!insertmacro STDMACROS_FUNCDEF ${SHA256_PREFIX}`
!define SHA256_FUNCINC \
  `!insertmacro STDMACROS_FUNCINC ${SHA256_PREFIX}`
!define SHA256_FUNCPROLOG \
  `!insertmacro STDMACROS_FUNCPROLOG ${SHA256_PREFIX}`

; ${SHA256_Int32ArrayAsHexString} array size string
${SHA256_FUNCDEF} Int32ArrayAsHexString
!macro SHA256_Int32ArrayAsHexString_Call UN ARRAY SIZE STRING
  Push ${ARRAY}
  Push ${SIZE}
  Call `${UN}${SHA256_PREFIX}Int32ArrayAsHexString`
  Pop ${STRING}
!macroend

; ${SHA256_Alloc} handle
${SHA256_FUNCDEF} Alloc
!macro SHA256_Alloc_Call UN HANDLE
  Call `${UN}${SHA256_PREFIX}Alloc`
  Pop ${HANDLE}
!macroend

; ${SHA256_Init} handle
; Parameter:
;   module - handle of loaded module
; Initialize the digest computation
; Return value:
;   Handle to new hash object
!macro SHA256_Init UN
  !define ${UN}SHA256_Init_INCLUDED
!macroend
${SHA256_FUNCDEF} Init
!macro SHA256_Init_Call UN MODULE HANDLE
  !insertmacro STDMACROS_ToSystemReg ${HANDLE} ${__MACRO__}_HANDLE
  System::Call "${SHA256_NAME}::init() p.${${__MACRO__}_HANDLE}"
!macroend

; ${SHA256_Update} handle data len result
; Update digest computation with the provided data
; Parameters:
;   handle - hash object
;   data - byte array of data
;   len - size of data byte array
; Return value:
;   Non zero value if successful
!macro SHA256_Update UN
  !define ${UN}SHA256_Update_INCLUDED
!macroend
${SHA256_FUNCDEF} Update
!macro SHA256_Update_Call UN HANDLE DATA LEN RESULT
  !insertmacro STDMACROS_ToSystemReg ${HANDLE} ${__MACRO__}_HANDLE
  !insertmacro STDMACROS_ToSystemReg ${DATA} ${__MACRO__}_DATA
  !insertmacro STDMACROS_ToSystemReg ${LEN} ${__MACRO__}_LEN
  !insertmacro STDMACROS_ToSystemReg ${RESULT} ${__MACRO__}_RESULT
  System::Call "${SHA256_NAME}::update(p ${${__MACRO__}_HANDLE}, p ${${__MACRO__}_DATA}, i ${${__MACRO__}_LEN}) i.${${__MACRO__}_RESULT}"
!macroend

; ${SHA256_Update_GetFunctionAddress} address
; Get address of update function
; Return value:
;   address of update function
!macro SHA256_Update_GetFunctionAddress UN
  !define ${UN}SHA256_Update_GetFunctionAddress_INCLUDED
!macroend
${SHA256_FUNCDEF} Update_GetFunctionAddress
!macro SHA256_Update_GetFunctionAddress_Call UN ADDRESS
  System::Get "${SHA256_NAME}::update"
  Pop ${ADDRESS}
!macroend

; ${SHA256_Final} module handle result
${SHA256_FUNCDEF} Final
!macro SHA256_Final_Call UN HANDLE RESULT
  Push ${HANDLE}
  Call `${UN}${SHA256_PREFIX}Final`
  Pop ${RESULT}
!macroend

; ${SHA256_Destroy} module handle
; Parameter:
;   handle - hash object
; Return value:
;  0 if successful otherwise error code
!macro SHA256_Destroy UN
  !define ${UN}SHA256_Destroy_INCLUDED
!macroend
${SHA256_FUNCDEF} Destroy
!macro SHA256_Destroy_Call UN HANDLE RESULT
  !insertmacro STDMACROS_ToSystemReg ${HANDLE} ${__MACRO__}_HANDLE
  !insertmacro STDMACROS_ToSystemReg ${RESULT} ${__MACRO__}_RESULT
  System::Call "${SHA256_NAME}::destroy(p ${${__MACRO__}_HANDLE}) i.${${__MACRO__}_RESULT}?e"
  ${If} ${RESULT} != 0
    StrCpy ${RESULT} 0
    Exch ${RESULT}
  ${EndIf}
  Pop ${RESULT}
!macroend

; ${SHA256_Destroy_GetFunctionAddress} address
; Get address of destroy function
; Return value:
;   address of destroy function
!macro SHA256_Destroy_GetFunctionAddress UN
  !define ${UN}SHA256_Destroy_GetFunctionAddress_INCLUDED
!macroend
${SHA256_FUNCDEF} Destroy_GetFunctionAddress
!macro SHA256_Destroy_GetFunctionAddress_Call UN ADDRESS
  System::Get "${SHA256_NAME}::destroy"
  Pop ${ADDRESS}
!macroend

; ${SHA256_Hash} module filehandle result
${SHA256_FUNCDEF} Hash
!macro SHA256_Hash_Call UN MODULE FILEHANDLE RESULT
  Push ${MODULE}
  Push ${FILEHANDLE}
  Call `${UN}${SHA256_PREFIX}Hash`
  Pop ${RESULT}
!macroend

; ${SHA256_Free} module
; Release the loaded sha256 helper module context
; Parameter:
;   module - handle to the loaded module
!macro SHA256_Free UN
  !define ${UN}SHA256_Free_INCLUDED
!macroend
${SHA256_FUNCDEF} Free
!macro SHA256_Free_Call UN MODULE
  !insertmacro STDMACROS_ToSystemReg ${MODULE} ${__MACRO__}_MODULE
  System::Call "kernel32::FreeLibrary(p ${${__MACRO__}_MODULE})"
!macroend

!macro SHA256_Int32ArrayAsHexString UN
; Get hex string from an array of 32-bit integers
; Parameters:
;   array - array holding the 32-bit integers
;   size - size of arry in bytes
; Return value:
;   Hex string
${SHA256_FUNCPROLOG} "${UN}" Int32ArrayAsHexString
  Push $0
  Exch 2
  Exch $1
  Exch
  Exch $2
  Push $3
  Push $4

  ; array=$1, size=$2
  StrCpy $0 ""
  StrCpy $3 $1
  ${While} $2 > 3
    System::Call "*$3(&i4 .r4)"
    IntFmt $4 "%08x" $4
    StrCpy $0 "$0$4"
    IntOp $3 $3 + 4
    IntOp $2 $2 - 4
  ${EndWhile}

  Pop $4
  Pop $3
  Pop $2
  Pop $1
  Exch $0
FunctionEnd
!macroend ; SHA256_Int32ArrayAsHexString

!macro SHA256_Alloc UN
; Unpack and load sha256 helper dll
; Result:
;   Handle of loaded module
${SHA256_FUNCPROLOG} "${UN}" Alloc
  Push $0
  Push $1

  StrCpy $1 "$PLUGINSDIR\${SHA256_HELPER_DLL}"
  ${IfNot} ${FileExists} "$1"
    ; Unpack sha256 helper dll
    Push $OUTDIR
    SetOutPath $PLUGINSDIR
    ReserveFile "${SHA256_HELPER_DIR}\${SHA256_HELPER_DLL}"
    File "/oname=${SHA256_HELPER_DLL}" "${SHA256_HELPER_DIR}\${SHA256_HELPER_DLL}"
    Pop $0
    SetOutPath $0
  ${EndIf}
  System::Call "kernel32::LoadLibrary(t r1) p.r0 ?e"
  Pop $1

  Pop $1
  Exch $0
FunctionEnd
!macroend ; SHA256_Alloc

!macro SHA256_Final UN
${SHA256_FUNCINC} "${UN}" Int32ArrayAsHexString
; Complete hashing and get digest value
; Parameters:
;  handle  - hash object
; Return value:
;   Hex string of digest value or empty string in case of failure
${SHA256_FUNCPROLOG} "${UN}" Final
  Push $0
  Exch
  Exch $1
  Push $2
  Push $3

  StrCpy $0 ""
  ${If} $1 P<> 0
    ; Allocate 32-bit integer array
    System::Call "*(&l .r2, &i4,&i4,&i4,&i4,&i4,&i4,&i4,&i4) p.r3"
    ${If} $3 P<> 0
      System::Call "${SHA256_NAME}::final(p r1, p r3)"
      ; Get hash value
      !insertmacro ${SHA256_PREFIX}Int32ArrayAsHexString_Call "${UN}" $3 $2 $0
      System::Free $3
    ${EndIf}
  ${EndIf}

  Pop $3
  Pop $2
  Pop $1
  Exch $0
FunctionEnd
!macroend ; SHA256_Final

!macro SHA256_Hash UN
${SHA256_FUNCINC} "${UN}" Int32ArrayAsHexString
; Compute SHA256 hash value
; Parameters:
;   module - handle of loaded module
;   filehandle - handle of file to read from
; Return value:
;   Hex string of digest value or empty string in case of failure
${SHA256_FUNCPROLOG} "${UN}" Hash
  Push $0
  Exch 2
  Exch $1
  Exch
  Exch $2
  Push $3
  Push $4

  ; module=$1, filehandle=$2
  StrCpy $0 ""
  ${If} $1 P<> 0
  ${AndIf} $2 P<> 0
    System::Call "kernel32::GetProcAddress(p r1, m 'hash') p.r1"
    ${If} $1 P<> 0
      ; Allocate 32-bit integer array
      System::Call "*(&l .r3, &i4,&i4,&i4,&i4,&i4,&i4,&i4,&i4) p.r4"
      ${If} $4 P<> 0
        System::Call "::$1(p r2, p r4)"
        ; Get hash value
        !insertmacro ${SHA256_PREFIX}Int32ArrayAsHexString_Call "${UN}" $4 $3 $0
	System::Free $4
      ${EndIf}
    ${EndIf}
  ${EndIf}

  Pop $4
  Pop $3
  Pop $2
  Pop $1
  Exch $0
FunctionEnd
!macroend ; SHA256_Hash

!endif ; SHA256_INCLUDED
