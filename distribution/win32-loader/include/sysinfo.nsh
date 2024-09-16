; Licensed under the zlib/libpng license (same as NSIS)

!ifndef SYSINFO_INCLUDED
!define SYSINFO_INCLUDED

!define SYSINFO_PREFIX "SYSINFO_"

!include include\stdmacros.nsh
!include LogicLib.nsh

!define SYSINFO_FUNCDEF \
  `!insertmacro STDMACROS_FUNCDEF ${SYSINFO_PREFIX}`
!define SYSINFO_FUNCINC \
  `!insertmacro STDMACROS_FUNCINC ${SYSINFO_PREFIX}`
!define SYSINFO_FUNCPROLOG \
  `!insertmacro STDMACROS_FUNCPROLOG ${SYSINFO_PREFIX}`

!define SYSINFO_DnsHostname 1
!define SYSINFO_DnsDomain   2
!define SYSINFO_WSADATA_SIZE 400

; ${SYSINFO_ComputerName} type result
${SYSINFO_FUNCDEF} ComputerName
!macro SYSINFO_ComputerName_Call UN TYPE RESULT
  Push ${TYPE}
  Call `${UN}${SYSINFO_PREFIX}ComputerName`
  Pop ${RESULT}
!macroend

; ${SYSINFO_Domain} result
; Get domain name
; Return value:
;   domain name
${SYSINFO_FUNCDEF} Domain
!macro SYSINFO_Domain UN
  ${SYSINFO_FUNCINC} "${UN}" ComputerName
  !define `${UN}${SYSINFO_PREFIX}Domain_INCLUDED`
!macroend
!macro SYSINFO_Domain_Call UN RESULT
  ; System::Call "kernel32::GetComputerNameEx(i ${ComputerNameDnsDomain}, t .r0, *i ${NSIS_MAX_STRLEN} r1)i.r2"
  Push ${SYSINFO_DnsDomain}
  Call `${UN}${SYSINFO_PREFIX}ComputerName`
  Pop ${RESULT}
!macroend

; ${SYSINFO_HostName} result
; Get host name
; Return value:
;   host name
${SYSINFO_FUNCDEF} HostName
!macro SYSINFO_HostName UN
  ${SYSINFO_FUNCINC} "${UN}" ComputerName
  !define `${UN}${SYSINFO_PREFIX}HostName_INCLUDED`
!macroend
!macro SYSINFO_HostName_Call UN RESULT
  Push ${SYSINFO_DnsHostname}
  Call `${UN}${SYSINFO_PREFIX}ComputerName`
  Pop ${RESULT}
!macroend

; ${SYSINFO_KeyboardLayout} result
; Get keyboard layout
; Return value:
;   keyboard layout
${SYSINFO_FUNCDEF} KeyboardLayout
!macro SYSINFO_KeyboardLayout UN
  !define `${UN}${SYSINFO_PREFIX}KeyboardLayout_INCLUDED`
!macroend
!macro SYSINFO_KeyboardLayout_Call UN RESULT
  !insertmacro STDMACROS_ToSystemReg ${RESULT} ${__MACRO__}_RESULT
  System::Call "user32::GetKeyboardLayout(i 0) i.${${__MACRO__}_RESULT}"
  IntOp ${RESULT} ${RESULT} & 0x0000FFFF
  IntFmt ${RESULT} "0x%04X" ${RESULT}
!macroend

; ${SYSINFO_UserName} result
${SYSINFO_FUNCDEF} UserName
!macro SYSINFO_UserName_Call UN RESULT
  Call `${UN}${SYSINFO_PREFIX}UserName`
  Pop ${RESULT}
!macroend

!macro SYSINFO_ComputerName UN
; Get name of specified type from computer
; Parameter:
;   type - type of name
; Return value:
;   name
${SYSINFO_FUNCPROLOG} "${UN}" ComputerName
  Exch $3
  Push $1
  Push $2
  Push $0
  Exch 3

  System::Call "kernel32::GetComputerNameEx(i r3, t .r0, *i ${NSIS_MAX_STRLEN}r1) i.r2"
  ${If} $2 = error
    ; Fall back to gethostname on Windows 95/98
    System::Alloc ${SYSINFO_WSADATA_SIZE}
    Pop $2
    ${If} $2 P<> 0
      System::Call "wsock32::WSAStartup(i 0x0101, p r2) i.r1"
      ${If} $1 = 0
        System::Call "wsock32::gethostname(m .r0, i ${NSIS_MAX_STRLEN}) i.r1"
      ${EndIf}
      ${If} $1 != 0
         ; Indicate error
         StrCpy $2 0
      ${Else}
         StrCpy $1 0
         ${Do}
           Push $1
           StrCpy $1 $0 1 $1
           ${If} $1 == "."
           ${OrIf} $1 == ""
             Pop $1
             ${ExitDo}
           ${EndIf}
           Pop $1
           IntOp $1 $1 + 1
         ${Loop}
         ${If} $3 = ${SYSINFO_DnsHostname}
           ${If} $1 != ""
             StrCpy $0 $0 $1
           ${EndIf}
           ; Indicate success
           StrCpy $2 1
         ${ElseIf} $3 = ${SYSINFO_DnsDomain}
           ${If} $1 != ""
             IntOp $1 $1 + 1
             StrCpy $0 $0 "" $1
             ; Indicate success
             StrCpy $2 1
           ${EndIf}
         ${EndIf}
      ${EndIf}
      System::Call "wsock32::WSACleanup()"
      System::Free $3
    ${Else}
      StrCpy $2 0
    ${EndIf}
    ; If querying of host name failed then give it another try 
    ; with GetComputerNameA
    ${If} $2 = 0
    ${AndIf} $3 = ${SYSINFO_DnsHostname}
      System::Call "kernel32::GetComputerNameA(m .r0, *i ${NSIS_MAX_STRLEN}r1) i.r2"
    ${EndIf}
  ${EndIf}
  ${If} $2 = 0
    StrCpy $0 ""
  ${EndIf}

  Pop $3
  Pop $2
  Pop $1
  Exch $0
FunctionEnd
!macroend ; SYSINFO_ComputerName

!macro SYSINFO_UserName UN
; Get user name
; Return value:
;   user name
${SYSINFO_FUNCPROLOG} "${UN}" UserName
  Push $0
  Push $1
  Push $2

  System::Call "advapi32::GetUserName(t .r0, *i ${NSIS_MAX_STRLEN}r1) i.r2"
  ${If} $2 = 0
    StrCpy $0 ""
  ${EndIf}

  Pop $2
  Pop $1
  Exch $0
FunctionEnd
!macroend ; SYSINFO_UserName

!endif ; SYSINFO_INCLUDED
