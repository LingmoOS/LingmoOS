; Licensed under the zlib/libpng license (same as NSIS)

!ifndef WININET_INCLUDED
!define WININET_INCLUDED

!ifndef WININET_CANCEL_FLAG
!define WININET_CANCEL_FLAG `""`
!endif
!ifndef WININET_MIN_BUFFER_SIZE
!define WININET_MIN_BUFFER_SIZE 8192
!endif
!ifndef WININET_MAX_BUFFER_SIZE
!define WININET_MAX_BUFFER_SIZE 262144
!endif
!ifndef WININET_PROGRESSBAR_CONTROL
!define WININET_PROGRESSBAR_CONTROL `""`
!endif
!ifndef WININET_PROXY
!define WININET_PROXY `""`
!endif
!ifndef WININET_STATUSTEXT_CONTROL
!define WININET_STATUSTEXT_CONTROL `""`
!endif
!ifndef WININET_TEXT_CONNECTING
!define WININET_TEXT_CONNECTING "Connecting ..."
!endif
!ifndef WININET_TEXT_PROGRESS
!define WININET_TEXT_PROGRESS "$1 left - $2 of $4$0$(^Byte) ($3$0$(^Byte)/s)"
!endif
!ifndef WININET_TIMEOUT
!define WININET_TIMEOUT 10000
!endif
!ifndef WININET_USER_AGENT
!define WININET_USER_AGENT "Mozilla/4.0 (compatible; Win32; WinINet)"
!endif

!define FLAGS_ERROR_UI_FILTER_FOR_ERRORS        0x01
!define FLAGS_ERROR_UI_FLAGS_CHANGE_OPTIONS     0x02
!define FLAGS_ERROR_UI_FLAGS_GENERATE_DATA      0x04

!define HTTP_E_STATUS_BASE -2145845248

!define HTTP_QUERY_CONTENT_LENGTH 5
!define HTTP_QUERY_STATUS_CODE 19 
!define HTTP_QUERY_FLAG_NUMBER 0x20000000

!define HTTP_STATUS_OK 200

!define INTERNET_DEFAULT_HTTP_PORT 80
!define INTERNET_DEFAULT_HTTPS_PORT 443 

!define INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP 0x00008000
!define INTERNET_FLAG_KEEP_CONNECTION 0x00400000
!define INTERNET_FLAG_SECURE 0x00800000
!define INTERNET_FLAG_RELOAD 0x80000000

!define INTERNET_OPEN_TYPE_PRECONFIG 0
!define INTERNET_OPEN_TYPE_PROXY 3

!define INTERNET_OPTION_CONNECT_TIMEOUT 2
!define INTERNET_OPTION_SECURITY_FLAGS 31
!define INTERNET_OPTION_CONNECTED_STATE 50

!define INTERNET_SERVICE_HTTP 3

!define SECURITY_FLAG_IGNORE_UNKNOWN_CA 0x00000100

!define WININET_SIZEOF_DWORD 4

!include include\stdmacros.nsh
!include LogicLib.nsh
!include StrFunc.nsh
!include Win\WinError.nsh
!include WinMessages.nsh

!ifndef StrLoc_INCLUDED
${StrLoc}
!endif

!include include\crypthash.nsh
!ifndef NO_SHA256_HELPER
!include include\sha256.nsh
!endif

${STDMACROS_DefIfNotDef} ERROR_CANCELLED 1223
${STDMACROS_DefIfNotDef} ERROR_INTERNET_FORCE_RETRY 12032
${STDMACROS_DefIfNotDef} ERROR_INTERNET_SEC_CERT_DATE_INVALID 12037
${STDMACROS_DefIfNotDef} ERROR_INTERNET_SEC_CERT_CN_INVALID 12038
${STDMACROS_DefIfNotDef} ERROR_INTERNET_INVALID_CA 12045

!define WININET_PREFIX "WININET_"
!define WININET_UNFUNC "un."

!define WININET_FUNCDEF \
  `!insertmacro STDMACROS_FUNCDEF ${WININET_PREFIX}`
!define WININET_FUNCINC \
  `!insertmacro STDMACROS_FUNCINC ${WININET_PREFIX}`
!define WININET_FUNCPROLOG \
  `!insertmacro STDMACROS_FUNCPROLOG ${WININET_PREFIX}`

; ${WININET_Div1024Tenth} var
${WININET_FUNCDEF} Div1024Tenth
!macro WININET_Div1024Tenth_Call UN VAR
  Push ${VAR}
  Call `${UN}${WININET_PREFIX}Div1024Tenth`
  Pop ${VAR}
!macroend

; ${WININET_SplitTenth} var
${WININET_FUNCDEF} SplitTenth
!macro WININET_SplitTenth_Call UN VAR
  Push ${VAR}
  Call `${UN}${WININET_PREFIX}SplitTenth`
  Pop ${VAR}
!macroend

; ${WININET_UpdateProgress} size current time message progress
${WININET_FUNCDEF} UpdateProgress
!macro WININET_UpdateProgress_Call UN SIZE CURRENT TIME MESSAGE PROGRESS
  Push ${SIZE}
  Push ${CURRENT}
  Push ${TIME}
  Call `${UN}${WININET_PREFIX}UpdateProgress`
  Pop ${PROGRESS}
  Pop ${MESSAGE}
!macroend

; ${WININET_Connect} handle url connection path flag
${WININET_FUNCDEF} Connect
!macro WININET_Connect_Call UN HANDLE URL CONNECTION PATH FLAG
  Push ${HANDLE}
  Push ${URL}
  Call `${UN}${WININET_PREFIX}Connect`
  Pop ${CONNECTION}
  Pop ${PATH}
  Pop ${FLAG}
!macroend

; ${WININET_Read} request file funcaddr handle size time status progress result
${WININET_FUNCDEF} Read
!macro WININET_Read_Call UN REQUEST FILE FUNCADDRESS HANDLE SIZE TIME STATUS \
  PROGRESS RESULT
  Push ${REQUEST}
  Push ${FILE}
  Push ${FUNCADDRESS}
  Push ${HANDLE}
  Push ${SIZE}
  Push ${TIME}
  Push ${STATUS}
  Push ${PROGRESS}
  Call `${UN}${WININET_PREFIX}Read`
  Pop ${RESULT}
!macroend

; ${WININET_Fetch} url filename algorithm result
${WININET_FUNCDEF} Fetch
!macro WININET_Fetch_Call UN URL FILENAME ALGORITHM RESULT
  Push "${URL}"
  Push "${FILENAME}"
  Push ${ALGORITHM}
  Call `${UN}${WININET_PREFIX}Fetch`
  Pop ${RESULT}
!macroend

; ${WININET_Get} baseurl filename basedir algorithm result
${WININET_FUNCDEF} Get
!macro WININET_Get_Call UN BASEURL FILENAME BASEDIR ALGORITHM RESULT
  Push "${BASEURL}/${FILENAME}"
  Push "${BASEDIR}\${FILENAME}"
  Push ${ALGORITHM}
  Call `${UN}${WININET_PREFIX}Fetch`
  Pop ${RESULT}
!macroend
!macro WININET_Get UN
${WININET_FUNCINC} "${UN}" Fetch
!macroend

!macro WININET_Div1024Tenth UN
; Divide by 1024 and return the result in Tenths
; Parameter:
;   var - variable
; Return value:
;   result
${WININET_FUNCPROLOG} "${UN}" Div1024Tenth
  Exch $0
  IntOp $0 $0 >> 6
  IntOp $0 $0 * 10
  IntOp $0 $0 >> 4
  Exch $0
FunctionEnd
!macroend ; WININET_Div1024Tenth

!macro WININET_SplitTenth UN
; Split Tenth from integer by separating the fraction
; part by a dot.
; For example 0 results in "0.0" and 123 in "12.3".
; Parameter:
;   var - variable
; Return value:
;   result in the form of "<number>.<digit>"
${WININET_FUNCPROLOG} "${UN}" SplitTenth
  Exch $0
  Push $1
  StrCpy $1 $0 1 -1
  ${If} $0 < 10
    StrCpy $0 0
  ${Else}
    StrCpy $0 $0 -1
  ${EndIf}
  StrCpy $0 "$0.$1"
  Pop $1
  Exch $0
FunctionEnd
!macroend ; WININET_SplitTenth

!macro WININET_UpdateProgress UN
${WININET_FUNCINC} "${UN}" Div1024Tenth
${WININET_FUNCINC} "${UN}" SplitTenth
; Update progress
; Parameters:
;   size - size of file to be downloaded in bytes
;   current - number of bytes downloaded
;   time - time since starting the download in milliseconds
; Return values:
;   message indicating the progress
;   progress in percent
${WININET_FUNCPROLOG} "${UN}" UpdateProgress
  Push $0
  Exch 3
  Push $1
  Exch 3
  Exch $2
  Exch 2
  Exch $3
  Exch
  Exch $4
  Push $5
  Push $6

  ; current=$2, time=$3, size=$4

  ; throughput = current / (now / 1000) 
  IntOp $3 $3 / 1000
  ${If} $3 != 0
    IntOp $3 $2 / $3
  ${EndIf}
  ${If} $3 != 0
  ${AndIf} $4 != 0
    ; time_left = (size - current) / throughput
    IntOp $1 $4 - $2
    IntOp $1 $1 / $3
    ${If} $1 >= 3600
      IntOp $1 $1 / 360
      !insertmacro ${WININET_PREFIX}SplitTenth_Call \
        "${UN}" $1
      StrCpy $1 "$1h"
    ${ElseIf} $1 >= 60
      IntOp $1 $1 / 6
      !insertmacro ${WININET_PREFIX}SplitTenth_Call \
        "${UN}" $1
      StrCpy $1 "$1m"
    ${Else}
      ${If} $2 < $4
        IntOp $1 $1 + 1
      ${EndIf}
      StrCpy $1 "$1s"
    ${EndIf}
  ${Else}
    StrCpy $1 "..."
  ${EndIf}

  StrCpy $5 0
  ${If} $4 == 0
  ${AndIf} $2 >= 1024
  ${OrIf} $4 >= 1024
    IntOp $5 $5 + 1
    ; Preserve first digit (1/10) after dot
    !insertmacro ${WININET_PREFIX}Div1024Tenth_Call "${UN}" $2
    !insertmacro ${WININET_PREFIX}Div1024Tenth_Call "${UN}" $3
    !insertmacro ${WININET_PREFIX}Div1024Tenth_Call "${UN}" $4
  ${EndIf}

  ; Calculate progress in percent
  ${If} $4 != 0
    IntOp $6 $2 * 100
    IntOp $6 $6 / $4
  ${Else}
    StrCpy $6 0
    StrCpy $4 $2
  ${EndIf}

  ${DoWhile} $4 >= 10240
    IntOp $2 $2 >> 10
    IntOp $3 $3 >> 10
    IntOp $4 $4 >> 10
    IntOp $5 $5 + 1
  ${Loop}
  ${Select} $5
    ${Case} 1
      StrCpy $0 $(^Kilo)
    ${Case} 2
      StrCpy $0 $(^Mega)
    ${Case} 3
      StrCpy $0 $(^Giga)
    ${CaseElse}
      StrCpy $0 " "
  ${EndSelect}
  ${If} $5 > 0
    ; Separate 1/10 part by dot
    !insertmacro ${WININET_PREFIX}SplitTenth_Call "${UN}" $2
    !insertmacro ${WININET_PREFIX}SplitTenth_Call "${UN}" $3
    !insertmacro ${WININET_PREFIX}SplitTenth_Call "${UN}" $4
  ${EndIf}

  ; If progress can not be calculated due to missing total size then
  ; set total size to the place holder as for the remaining time
  ${If} $6 == 0
  ${AndIf} $2 == $4
    StrCpy $4 $1
  ${EndIf}
  StrCpy $1 "${WININET_TEXT_PROGRESS}"

  ; Save progress in $0
  StrCpy $0 $6

  Pop $6
  Pop $5
  Pop $4
  Pop $3
  Pop $2
  Exch $1
  Exch
  Exch $0
FunctionEnd
!macroend ; WININET_UpdateProgress

!macro WININET_Connect UN
; Connect to server
; Parameter:
;   handle - handle obtained via InternetOpen
;   URL
; Return values:
;   handle - handle of connection
;   path - path to resource
;   flag - 1=https, 0=http, ""=unsupported
${WININET_FUNCPROLOG} "${UN}" Connect
  System::Store 'S'
  Pop $5
  Pop $6
  ; $5=url $6=handle

  StrCpy $0 0
  StrCpy $1 ""
  StrCpy $2 ""

  StrCpy $7 "://"
  ${StrLoc} $4 $5 $7 >
  ${If} $4 != ""
    StrLen $7 $7
    ; Get protocol from URL
    StrCpy $0 $5 $4
    ${If} $0 == "https"
      StrCpy $3 ${INTERNET_DEFAULT_HTTPS_PORT}
      StrCpy $2 1
    ${ElseIf} $0 == "http"
      StrCpy $3 ${INTERNET_DEFAULT_HTTP_PORT}
      StrCpy $2 0
    ${EndIf}
    ${If} $2 != ""
      IntOp $4 $4 + $7
      StrCpy $0 $5 "" $4
      ; Separate path component
      ${StrLoc} $4 $0 "/" >
      ${If} $4 != ""
        StrCpy $1 $0 "" $4
        StrCpy $0 $0 $4
      ${EndIf}
      StrCpy $4 $0 "" -1
      ; Extract port number in case it is not a plain IPv6 address
      ${If} $4 != "]"
        StrCpy $7 ":"
        ${StrLoc} $4 $0 $7 >
        ${If} $4 != ""
          StrLen $7 $7
          IntOp $3 $4 + $7
          StrCpy $3 $0 "" $3
          StrCpy $0 $0 $4
        ${EndIf}
      ${EndIf}
      System::Call "wininet::InternetConnect(p r6, t r0, i r3, p 0, p 0, i ${INTERNET_SERVICE_HTTP}, i 0, p 0) p.r0"
    ${Else}
      StrCpy $0 0
    ${EndIf}
  ${EndIf}

  Push $2
  Push $1
  Push $0
  System::Store 'L'
FunctionEnd
!macroend ; WININET_Connect

!macro WININET_Read UN
${WININET_FUNCINC} "${UN}" UpdateProgress
; Read data of request
; Parameters:
;   request - request handle
;   file - file handle
;   funcaddr - address of hash update function
;   handle - handle for hashing
;   size - size of file to be downloaded
;   time - time when download was started
;   status - text control receiving the progress updates
;   progress  - progress bar control
; Return value:
;   0 if successful otherwise error code
${WININET_FUNCPROLOG} "${UN}" Read
  System::Store 'S'
  Pop $8
  Pop $7
  Pop $6 
  Pop $5
  Pop $4
  Pop $3
  Pop $2
  Pop $1
  ; request=$1 file=$2 funcaddr=$3 handle=$4 size=$5 time=$6 status=$7 progress=$8

  ; Set jump targets
  ${If} $3 != ""
  ${AndIf} $4 P<> 0
    GetLabelAddress $R7 WININET_Read_ComputeHash
  ${Else}
    GetLabelAddress $R7 WININET_Read_UpdateProgress
  ${EndIf}
  ${If} $7 == 0
  ${AndIf} $8 == 0
    GetLabelAddress $R8 WININET_Read_Next
  ${Else}
    GetLabelAddress $R8 WININET_Read_DoReporting
    ${If} $8 P<> 0
      ; Take current position of the progress bar as basis
      SendMessage $8 ${PBM_GETPOS} 0 0 $R6
      ${If} $R6 != 0
        SendMessage $8 ${PBM_GETRANGE} 0 0 $0
        ${If} $R6 == $0
          ; If the position is set to the maximum then reset it to 0.
          StrCpy $R6 0
        ${EndIf}
      ${EndIf}
    ${EndIf}
  ${EndIf}

  StrCpy $R0 ${WININET_MAX_BUFFER_SIZE}
  System::Alloc $R0
  Pop $R1
  ; buffersize=$R0, buffer=$R1
  ${If} $R1 P<> 0
    ${IfNot} ${Silent}
      StrCpy $R0 ${WININET_MIN_BUFFER_SIZE}
    ${EndIf}
    ; Number of bytes downloaded
    StrCpy $R3 0
    ; Update progress at tick count
    StrCpy $R4 0
    ${Do}
      System::Call "wininet::InternetReadFile(p r1, p r11, i r10, *i .r12) i.r9?e"
      Pop $0
      ${If} $9 == 0
      ${OrIf} ${WININET_CANCEL_FLAG} != ""
        ${ExitDo}
      ${ElseIf} $R2 > 0
        System::Call "kernel32::WriteFile(p r2, p r11, i r12, *i r12, p 0) i.r9?e"
        Pop $0
        ${If} $9 == 0
        ${OrIf} ${WININET_CANCEL_FLAG} != ""
          ${ExitDo}
        ${Else}
          Goto $R7
WININET_Read_ComputeHash:
          System::Call "$3(p r4, p r11, i r12, i 0) i.r9 ?e"
          Pop $0
          ${If} $9 == 0
          ${OrIf} ${WININET_CANCEL_FLAG} != ""
            ${ExitDo}
          ${EndIf}
WININET_Read_UpdateProgress:
          Goto $R8
WININET_Read_DoReporting:
          System::Call "kernel32::GetTickCount() i.r0"
          IntOp $0 $0 - $6
          IntOp $0 $0 & 0x8fffffff
          IntOp $R3 $R3 + $R2
          ${If} $0 >= $R4
            ; Limit number of updates to no more
            ; than one update per second
            IntOp $R4 $0 + 1000
            !insertmacro ${WININET_PREFIX}UpdateProgress_Call "" \
              $5 $R3 $0 $0 $R5
            SendMessage $7 ${WM_SETTEXT} 0 "STR:$0"
            IntOp $R5 $R5 + $R6
            SendMessage $8 ${PBM_SETPOS} $R5 0
          ${Else}
	    ${If} $R0 < ${WININET_MAX_BUFFER_SIZE}
              ; Increase number of bytes to read
	      IntOp $R0 $R0 << 1
	      ${If} $R0 > ${WININET_MAX_BUFFER_SIZE}
                StrCpy $R0 ${WININET_MAX_BUFFER_SIZE}
              ${EndIf}
            ${EndIf}
          ${EndIf}
WININET_Read_Next:
        ${EndIf}
      ${Else}
        ${If} $9 != 0
          StrCpy $0 0
        ${EndIf}
        ${ExitDo}
      ${EndIf}
    ${Loop}
    System::Free $R1
  ${Else}
    StrCpy $0 ${ERROR_OUTOFMEMORY}
  ${EndIf}

  ${If} $0 == 0
  ${AndIf} ${WININET_CANCEL_FLAG} != ""
    StrCpy $0 ${ERROR_CANCELLED}
  ${EndIf}

  Push $0
  System::Store 'L'
FunctionEnd
!macroend ; WININET_Read

!macro WININET_Fetch UN
${CRYPTHASH_FUNCINC} "${UN}" ContextAcquire
${CRYPTHASH_FUNCINC} "${UN}" Init
${CRYPTHASH_FUNCINC} "${UN}" Update_GetFunctionAddress
${CRYPTHASH_FUNCINC} "${UN}" Final
${CRYPTHASH_FUNCINC} "${UN}" Destroy_GetFunctionAddress
${CRYPTHASH_FUNCINC} "${UN}" ContextRelease
!ifndef NO_SHA256_HELPER
${SHA256_FUNCINC} "${UN}" Alloc
${SHA256_FUNCINC} "${UN}" Init
${SHA256_FUNCINC} "${UN}" Update_GetFunctionAddress
${SHA256_FUNCINC} "${UN}" Final
${SHA256_FUNCINC} "${UN}" Destroy_GetFunctionAddress
${SHA256_FUNCINC} "${UN}" Free
!endif
${WININET_FUNCINC} "${UN}" Connect
${WININET_FUNCINC} "${UN}" Read
; Support functions
; Initialize the digest computation of crypthash module
; Parameters:
;   context - handle of a cryptographic service provider
;   algorithm - identifier of the hash algorithm to use
; Return value:
;   Handle to new hash object 
Function ${UN}${WININET_PREFIX}NativeHashInit
  Exch $0
  Exch
  Exch $1
  !insertmacro ${CRYPTHASH_PREFIX}Init_Call "${UN}" $1 $0 $0
  Pop $1
  Exch $0
FunctionEnd

; Release context handle
; Parameter:
;   context - handle of a cryptographic service provider
Function ${UN}${WININET_PREFIX}NativeHashFree
  Exch $0
  !insertmacro ${CRYPTHASH_PREFIX}ContextRelease_Call "${UN}" $0 $0
  Pop $0
FunctionEnd

!ifndef NO_SHA256_HELPER
; Initialize the digest computation of sha256 module
; Parameters:
;   module - handle of loaded module
;   algorithm - identifier of the hash algorithm to use
; Return value:
;   Handle to new hash object 
Function ${UN}${WININET_PREFIX}HelperHashInit
  Exch $0
  Exch
  Exch $1
  !insertmacro ${SHA256_PREFIX}Init_Call "${UN}" $1 $0
  Pop $1
  Exch $0
FunctionEnd

; Release module handle
; Parameter:
;   module - handle of loaded module
Function ${UN}${WININET_PREFIX}HelperHashFree
  Exch $0
  !insertmacro ${SHA256_PREFIX}Free_Call "${UN}" $0
  Pop $0
FunctionEnd
!endif

; Fetch file from URL
; Parameters:
;   url - internet address
;   filename - name of file
;   algorithm - identifier of the hash algorithm to use
; Return value
;   hex string of digest value or error code
${WININET_FUNCPROLOG} "${UN}" Fetch
  System::Store 'S'
  Pop $3 ; algorithm=$3
  Pop $2 ; filename=$2
  Pop $1 ; url=$1
 
  ; Handle of crypto context or loaded module
  StrCpy $R5 ""
  ; Address of hash update function
  StrCpy $R4 ""

  ${If} $3 != 0
    !insertmacro ${CRYPTHASH_PREFIX}ContextAcquire_Call "${UN}" ${PROV_RSA_AES} ${CRYPT_VERIFYCONTEXT} $R5
    ${If} $R5 P<> 0
      !insertmacro ${CRYPTHASH_PREFIX}Update_GetFunctionAddress_Call "${UN}" $R4
      GetFunctionAddress $R6 ${UN}${WININET_PREFIX}NativeHashInit
      GetFunctionAddress $R7 ${UN}${CRYPTHASH_PREFIX}Final
      !insertmacro ${CRYPTHASH_PREFIX}Destroy_GetFunctionAddress_Call "${UN}" $R8
      GetFunctionAddress $R9 ${UN}${WININET_PREFIX}NativeHashFree
!ifndef NO_SHA256_HELPER
    ${Else}
      !insertmacro ${SHA256_PREFIX}Alloc_Call "${UN}" $R5
      ${If} $R5 P<> 0
        !insertmacro ${SHA256_PREFIX}Update_GetFunctionAddress_Call "${UN}" $R4
        GetFunctionAddress $R6 ${UN}${WININET_PREFIX}HelperHashInit
        GetFunctionAddress $R7 ${UN}${SHA256_PREFIX}Final
        !insertmacro ${SHA256_PREFIX}Destroy_GetFunctionAddress_Call "${UN}" $R8
        GetFunctionAddress $R9 ${UN}${WININET_PREFIX}HelperHashFree
      ${Else}
        StrCpy $R5 ""
      ${EndIf}
!endif
    ${EndIf}
    ${If} $R4 P= 0
      StrCpy $R4 ""
    ${EndIf}
  ${EndIf}

  StrCpy $R0 0
  System::Get "wininet::InternetOpen(t '${WININET_USER_AGENT}', i ${INTERNET_OPEN_TYPE_PRECONFIG}, p 0, p 0, i 0) i.r10?e"
  Pop $0
  ${If} $0 != error
    ${If} ${WININET_PROXY} != ""
      System::Call "$0(, i ${INTERNET_OPEN_TYPE_PROXY}, t '${WININET_PROXY}', ,)"
    ${Else}
      System::Call "$0"
    ${EndIf}
    Pop $0
  ${Else}
    StrCpy $0 ${ERROR_INVALID_FUNCTION}
  ${EndIf}
  ${If} $R0 P<> 0
  ${AndIfNot} ${WININET_CANCEL_FLAG} != ""
    ; Establish connection
    System::Call "kernel32::GetTickCount() i.r7"
    FindWindow $0 "#32770" "" $HWNDPARENT
    ${If} $0 != 0
      GetDlgItem $8 $0 ${WININET_STATUSTEXT_CONTROL}
      GetDlgItem $9 $0 ${WININET_PROGRESSBAR_CONTROL}
      ; Reset progress bar
      SendMessage $9 ${PBM_SETPOS} 0 0
    ${Else}
      StrCpy $8 0
      StrCpy $9 0
    ${EndIf}
    SendMessage $8 ${WM_SETTEXT} 0 "STR:${WININET_TEXT_CONNECTING}"
    System::Call "wininet::InternetSetOption(p r10, i ${INTERNET_OPTION_CONNECT_TIMEOUT}, *i ${WININET_TIMEOUT}, i ${WININET_SIZEOF_DWORD}) i.r0"
    !insertmacro ${WININET_PREFIX}Connect_Call "${UN}" $R0 $1 $R1 $5 $6
    ${If} $R1 P<> 0
    ${AndIfNot} ${WININET_CANCEL_FLAG} != ""
      StrCpy $R2 0
      System::Get "wininet::HttpOpenRequest(p r11, t 'GET', t r5, p 0, p 0, p 0, i ${INTERNET_FLAG_KEEP_CONNECTION}|${INTERNET_FLAG_RELOAD}, p 0) p.r12?e"
      Pop $0
      ${If} $0 != error
        ${If} $6 == 1
          ; Secure HTTP connection (https)
          System::Call "$0(, , , , , , i ${INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP}|${INTERNET_FLAG_KEEP_CONNECTION}|${INTERNET_FLAG_SECURE}|${INTERNET_FLAG_RELOAD},)"
        ${Else}
          System::Call "$0"
        ${EndIf}
        Pop $0
      ${Else}
        StrCpy $0 ${ERROR_INVALID_FUNCTION}
      ${EndIf}
      ${If} $R2 P<> 0
      ${AndIfNot} ${WININET_CANCEL_FLAG} != ""
        StrCpy $0 ${ERROR_INTERNET_FORCE_RETRY}
        ${While} $0 == ${ERROR_INTERNET_FORCE_RETRY}
          System::Call "wininet::HttpSendRequest(p r12, p 0, i 0, p 0, i 0) i.r5?e"
          Pop $0
          ${If} $5 == 0
            StrCpy $5 $0
            System::Call "wininet::InternetErrorDlg(p $HWNDPARENT, p r12, i r0, i ${FLAGS_ERROR_UI_FILTER_FOR_ERRORS}|${FLAGS_ERROR_UI_FLAGS_CHANGE_OPTIONS}|${FLAGS_ERROR_UI_FLAGS_GENERATE_DATA}, p 0) i.r0"
            ${If} $0 == 0
            ${AndIf} $5 == ${ERROR_INTERNET_INVALID_CA}
              ; Allow user to accept a self signed certificate
              System::Call "wininet::InternetSetOption(p r10, i ${INTERNET_OPTION_SECURITY_FLAGS}, *i ${SECURITY_FLAG_IGNORE_UNKNOWN_CA}, i ${WININET_SIZEOF_DWORD}) i.r0"
              StrCpy $0 ${ERROR_INTERNET_FORCE_RETRY}
            ${EndIf}
          ${EndIf}
          ${If} ${WININET_CANCEL_FLAG} != ""
            StrCpy $0 ${ERROR_CANCELLED}
            ${ExitWhile}
          ${EndIf}
        ${EndWhile}
        ${If} $0 == 0
          StrCpy $6 0
          StrCpy $5 ${NSIS_MAX_STRLEN}
          System::Call "wininet::HttpQueryInfo(p r12, i ${HTTP_QUERY_CONTENT_LENGTH}|${HTTP_QUERY_FLAG_NUMBER}, *i r6r6, *i r5r5, p 0) i.r0"
          StrCpy $5 ${NSIS_MAX_STRLEN}
          System::Call "wininet::HttpQueryInfo(p r12, i ${HTTP_QUERY_STATUS_CODE}|${HTTP_QUERY_FLAG_NUMBER}, *i ss, *i r5r5, p 0) i.r0"
          Pop $5
          ${If} $0 != 0
            ${If} $5 != ${HTTP_STATUS_OK}
              IntOp $0 $5 + ${HTTP_E_STATUS_BASE}
            ${Else}
              ClearErrors
              FileOpen $R3 "$2" w
              ; filehandle=$R3
              ${IfNot} ${Errors}
              ${AndIfNot} ${WININET_CANCEL_FLAG} != ""
                ; Acquire crypto context and get handle for
                ; requested hash algorithm
                StrCpy $5 0
            
                ${If} $R5 != ""
                  ; Hash computation initializiation
                  Push $R5
                  Push $3
                  Call $R6
                  Pop $5
                ${EndIf}

                ; Read and save incoming data
	        !insertmacro WININET_Read_Call "${UN}" $R2 $R3 $R4 $5 $6 $7 $8 $9 $0
                ; Compute final digest and release associated memory
                ${If} $5 P<> 0
                  ${IfNot} ${WININET_CANCEL_FLAG} != ""
                    Push $5
                    Call $R7
                    Pop $0
                  ${EndIf}
	          ; Release hash object
		  System::Call "$R8(p r5) i.r5"
                ${EndIf}
                FileClose $R3
              ${Else}
                System::Call "kernel32::GetLastError() i.r0"
              ${EndIf}
              ClearErrors
            ${EndIf}
          ${EndIf}
          System::Call "wininet::InternetCloseHandle(p r12)"
        ${EndIf}
        System::Call "wininet::InternetCloseHandle(p r11)"
      ${EndIf}
    ${EndIf}
    System::Call "wininet::InternetCloseHandle(p r10)"
  ${EndIf}

  ${If} $R5 != ""
    Push $R5
    Call $R9
  ${EndIf}

  ${If} $0 == 0
  ${AndIf} ${WININET_CANCEL_FLAG} != ""
    StrCpy $0 ${ERROR_CANCELLED}
  ${EndIf}

  Push $0
  System::Store 'L'
FunctionEnd
!macroend ; WININET_Fetch

!endif ; WININET_INCLUDED
