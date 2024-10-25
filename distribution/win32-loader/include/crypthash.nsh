; Licensed under the zlib/libpng license (same as NSIS)

!ifndef CRYPTHASH_INCLUDED
!define CRYPTHASH_INCLUDED

!define CALG_MD5 0x00008003
!define CALG_SHA1 0x00008004
!define CALG_SHA256 0x0000800C

!define CRYPT_STRING_HEXRAW 0x0000000C
!define CRYPT_STRING_NOCRLF 0x40000000
!define CRYPT_VERIFYCONTEXT 0xF0000000
!define HP_HASHVAL 0x0002
!define PROV_RSA_AES 24

!define CRYPTHASH_BUFFER_SIZE 262144
!define CRYPTHASH_PREFIX "CRYPTHASH_"

!include include\stdmacros.nsh
!include LogicLib.nsh
!include WinVer.nsh

!define CRYPTHASH_FUNCDEF \
  `!insertmacro STDMACROS_FUNCDEF ${CRYPTHASH_PREFIX}`
!define CRYPTHASH_FUNCINC \
  `!insertmacro STDMACROS_FUNCINC ${CRYPTHASH_PREFIX}`
!define CRYPTHASH_FUNCPROLOG \
  `!insertmacro STDMACROS_FUNCPROLOG ${CRYPTHASH_PREFIX}`

; ${CRYPTHASH_ContextAcquire} type flags context
; Acquire a context of a particular key container within the provided
; cryptographic service provider
; Parameters:
;   type - type of provider to acquire
;   flags - usally set to CRYPT_VERIFYCONTEXT for hashing
; Return value:
;   context or 0 in case of failure
!macro CRYPTHASH_ContextAcquire UN
  !define `${UN}${CRYPTHASH_PREFIX}ContextAcquire_INCLUDED`
!macroend
${CRYPTHASH_FUNCDEF} ContextAcquire
!macro CRYPTHASH_ContextAcquire_Call UN TYPE FLAGS CONTEXT
  !insertmacro STDMACROS_ToSystemReg ${CONTEXT} ${__MACRO__}_CONTEXT
  ; Check for availability of CryptAcquireContext function
  System::Get "advapi32::CryptAcquireContext()"
  Pop ${CONTEXT}
  ${If} ${CONTEXT} != error
    System::Call "${CONTEXT}(*p .${${__MACRO__}_CONTEXT}, p 0, p 0, i ${TYPE}, i ${FLAGS})"
  ${Else}
    StrCpy ${CONTEXT} 0
  ${EndIf}
!macroend

; ${CRYPTHASH_Init} context algorithm handle
; Initialize the digest computation
; Parameters:
;   context - handle of a cryptographic service provider
;   algorithm - identifier of the hash algorithm to use
; Return value:
;   Handle to new hash object
!macro CRYPTHASH_Init UN
  !define ${UN}CRYPTHASH_Init_INCLUDED
!macroend
${CRYPTHASH_FUNCDEF} Init
!macro CRYPTHASH_Init_Call UN CONTEXT ALGORITHM HANDLE
  !insertmacro STDMACROS_ToSystemReg ${CONTEXT} ${__MACRO__}_CONTEXT
  !insertmacro STDMACROS_ToSystemReg ${HANDLE} ${__MACRO__}_HANDLE
  System::Call "advapi32::CryptCreateHash(p ${${__MACRO__}_CONTEXT}, i ${ALGORITHM}, p 0, i 0, *p .${${__MACRO__}_HANDLE})"
!macroend

; ${CRYPTHASH_Update} handle data len result
; Update digest computation with the provided data
; Parameters:
;   handle - hash object
;   data - byte array of data
;   len - size of data byte array
; Return value:
;   Non zero value if successful
!macro CRYPTHASH_Update UN
  !define ${UN}CRYPTHASH_Update_INCLUDED
!macroend
${CRYPTHASH_FUNCDEF} Update
!macro CRYPTHASH_Update_Call UN HANDLE DATA LEN RESULT
  !insertmacro STDMACROS_ToSystemReg ${HANDLE} ${__MACRO__}_HANDLE
  !insertmacro STDMACROS_ToSystemReg ${DATA} ${__MACRO__}_DATA
  !insertmacro STDMACROS_ToSystemReg ${LEN} ${__MACRO__}_LEN
  !insertmacro STDMACROS_ToSystemReg ${RESULT} ${__MACRO__}_RESULT
  System::Call "advapi32::CryptHashData(p ${${__MACRO__}_HANDLE}, p ${${__MACRO__}_DATA}, i ${${__MACRO__}_LEN}, i 0) i.${${__MACRO__}_RESULT}"
!macroend

; ${CRYPTHASH_Update_GetFunctionAddress} address
; Get address of update function
; Return value:
;   address of update function
!macro CRYPTHASH_Update_GetFunctionAddress UN
  !define ${UN}CRYPTHASH_Update_GetFunctionAddress_INCLUDED
!macroend
${CRYPTHASH_FUNCDEF} Update_GetFunctionAddress
!macro CRYPTHASH_Update_GetFunctionAddress_Call UN ADDRESS
  System::Get "advapi32::CryptHashData"
  Pop ${ADDRESS}
!macroend

; ${CRYPTHASH_Final} handle digest
${CRYPTHASH_FUNCDEF} Final
!macro CRYPTHASH_Final_Call UN HANDLE DIGEST
  Push ${HANDLE}
  Call `${UN}${CRYPTHASH_PREFIX}Final`
  Pop ${DIGEST}
!macroend

; ${CRYPTHASH_SHA256Digest} context filehandle digest
${CRYPTHASH_FUNCDEF} SHA256Digest
!macro CRYPTHASH_SHA256Digest_Call UN CONTEXT FILEHANDLE DIGEST
  Push ${CONTEXT}
  Push ${FILEHANDLE}
  Call `${UN}${CRYPTHASH_PREFIX}SHA256Digest`
  Pop ${DIGEST}
!macroend

; ${CRYPTHASH_Destroy} handle result
; Destroys the hash object
; Parameter:
;   handle - hash object
; Return value:
;  0 if successful otherwise error code
!macro CRYPTHASH_Destroy UN
  !define ${UN}CRYPTHASH_Destroy_INCLUDED
!macroend
${CRYPTHASH_FUNCDEF} Destroy
!macro CRYPTHASH_Destroy_Call UN HANDLE RESULT
  !insertmacro STDMACROS_ToSystemReg ${HANDLE} ${__MACRO__}_HANDLE
  !insertmacro STDMACROS_ToSystemReg ${RESULT} ${__MACRO__}_RESULT
  System::Call "advapi32::CryptDestroyHash(p ${${__MACRO__}_HANDLE}) i.${${__MACRO__}_RESULT}?e"
  ${If} ${RESULT} != 0
    StrCpy ${RESULT} 0
    Exch ${RESULT}
  ${EndIf}
  Pop ${RESULT}
!macroend

; ${CRYPTHASH_Destroy_GetFunctionAddress} address
; Get address of destroy function
; Return value:
;   address of destroy function
!macro CRYPTHASH_Destroy_GetFunctionAddress UN
  !define ${UN}CRYPTHASH_Destroy_GetFunctionAddress_INCLUDED
!macroend
${CRYPTHASH_FUNCDEF} Destroy_GetFunctionAddress
!macro CRYPTHASH_Destroy_GetFunctionAddress_Call UN ADDRESS
  System::Get "advapi32::CryptDestroyHash"
  Pop ${ADDRESS}
!macroend

; ${CRYPTHASH_ContextRelease} context result
; Releases the context of a cryptographic service provider and a key container.
; Parameter:
;   context - handle of a cryptographic service provider
; Return value:
;  0 if successful otherwise error code
!macro CRYPTHASH_ContextRelease UN
  !define ${UN}CRYPTHASH_ContextRelease_INCLUDED
!macroend
${CRYPTHASH_FUNCDEF} ContextRelease
!macro CRYPTHASH_ContextRelease_Call UN CONTEXT RESULT
  !insertmacro STDMACROS_ToSystemReg ${CONTEXT} ${__MACRO__}_CONTEXT
  !insertmacro STDMACROS_ToSystemReg ${RESULT} ${__MACRO__}_RESULT
  System::Call "advapi32::CryptReleaseContext(p ${${__MACRO__}_CONTEXT}, i 0) i.${${__MACRO__}_RESULT}?e"
  ${If} ${RESULT} != 0
    StrCpy ${RESULT} 0
    Exch ${RESULT}
  ${EndIf}
  Pop ${RESULT}
!macroend

!macro CRYPTHASH_Final UN
; Complete hashing and get digest value
; Parameters:
;   handle - hash object
; Return value:
;   Hex string of digest value or empty string in case of failure
${CRYPTHASH_FUNCPROLOG} "${UN}" Final
  Exch $0
  Push $1
  Push $2
  Push $3

  ; handle=$0
  System::Call "advapi32::CryptGetHashParam(p r0, i ${HP_HASHVAL}, p 0, *i .r2, i 0) i.r1"
  ${If} $1 != 0
    System::Alloc $2
    Pop $3
    ${If} $3 P<> 0
      System::Call "advapi32::CryptGetHashParam(p r0, i ${HP_HASHVAL}, p r3, *i r2r2, i 0) i.r1"
      ${If} $1 != 0
        Push ${NSIS_MAX_STRLEN}
        ${IfNot} ${AtLeastWinVista}
          StrCpy $1 0
        ${Else}
          System::Call "crypt32::CryptBinaryToString(p r3, i r2, i ${CRYPT_STRING_HEXRAW}|${CRYPT_STRING_NOCRLF}, t .r0, *i ss) i.r1"
        ${EndIf}
        ${If} $1 == 0
          ; CryptBinaryToString of Wine 4.0 and any version prior
          ; Windows Vista do not support CRYPT_STRING_HEXRAW flag
          ; Fall back to generic binary to hex implementation
          StrCpy $0 ""
          IntOp $3 $3 + $2
          ${While} $2 > 0
            IntOp $2 $2 - 1
            IntOp $3 $3 - 1
            System::Call "*$3(&i1 .r1)"
            IntFmt $1 "%02x" $1
            StrCpy $0 "$1$0"
          ${EndWhile}
        ${EndIf}
        Pop $1
      ${EndIf}
      System::Free $3
    ${EndIf}
  ${EndIf}

  Pop $3
  Pop $2
  Pop $1
  Exch $0
FunctionEnd
!macroend ; CRYPTHASH_Final

!macro CRYPTHASH_SHA256Digest UN
${CRYPTHASH_FUNCINC} "${UN}" Init
${CRYPTHASH_FUNCINC} "${UN}" Update
${CRYPTHASH_FUNCINC} "${UN}" Final
${CRYPTHASH_FUNCINC} "${UN}" Destroy
; Compute SHA256 hash value
; Parameters:
;   context - acquired crypto context
;   filehandle - handle of file to read from
; Return value:
;   Hex string of digest value or empty string in case of failure
${CRYPTHASH_FUNCPROLOG} "${UN}" SHA256Digest
  Exch $0
  Exch
  Exch $1
  Push $2
  Push $3
  Push $4
  Push $5

  ; filehandle=$0, context=$1

  Push ""
  ${If} $0 P<> 0
  ${AndIf} $1 != ""
    !insertmacro ${CRYPTHASH_PREFIX}Init_Call "${UN}" $1 ${CALG_SHA256} $1
    ${If} $1 != 0
      ; Allocate buffer
      StrCpy $3 ${CRYPTHASH_BUFFER_SIZE}
      System::Alloc $3
      Pop $4
      ${If} $4 P<> 0
        ${Do}
          System::Call "kernel32::ReadFile(p r0, p r4, i r3, *i .r5, p 0) i.r2"
          ${If} $2 != 0
            !insertmacro ${CRYPTHASH_PREFIX}Update_Call "${UN}" $1 $4 $5 $2
            ${If} $2 != 0
              ${If} $5 == 0
              ${OrIf} $5 < $3
                ; Reached end of file
                !insertmacro ${CRYPTHASH_PREFIX}Final_Call "${UN}" $1 $0
                Exch $0
                ${ExitDo}
              ${EndIf}
            ${Else}
	      System::Call "kernel32::GetLastError() i.r0"
	      DetailPrint "${__FUNCTION__} CRYPTHASH_Update: $0"
              ${ExitDo}
            ${EndIf}
          ${Else}
            ; Read failure
	    System::Call "kernel32::GetLastError() i.r0"
	    DetailPrint "${__FUNCTION__} Readfile: $0"
            ${ExitDo}
          ${EndIf}
        ${Loop}
	System::Free $4
      ${EndIf}
      !insertmacro ${CRYPTHASH_PREFIX}Destroy_Call "${UN}" $1 $0
    ${EndIf}
  ${EndIf}
  Pop $0

  Pop $5
  Pop $4
  Pop $3
  Pop $2
  Pop $1
  Exch $0
FunctionEnd
!macroend ; CRYPTHASH_SHA256Digest

!endif ; CRYPTHASH_INCLUDED
