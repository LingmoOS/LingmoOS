; Licensed under the zlib/libpng license (same as NSIS)

!ifndef SECURITY_INCLUDED
!define SECURITY_INCLUDED

!define SECURITY_PREFIX "Security_"

!include include\stdmacros.nsh
!include LogicLib.nsh
!include Win\WinError.nsh
!include Win\WinNT.nsh

!define SECURITY_FUNCDEF \
  `!insertmacro STDMACROS_FUNCDEF ${SECURITY_PREFIX}`
!define SECURITY_FUNCINC \
  `!insertmacro STDMACROS_FUNCINC ${SECURITY_PREFIX}`
!define SECURITY_FUNCPROLOG \
  `!insertmacro STDMACROS_FUNCPROLOG ${SECURITY_PREFIX}`

${STDMACROS_DefIfNotDef} EOAC_NONE 0
${STDMACROS_DefIfNotDef} RPC_C_AUTHN_LEVEL_DEFAULT 0
${STDMACROS_DefIfNotDef} RPC_C_IMP_LEVEL_IMPERSONATE 3
${STDMACROS_DefIfNotDef} RPC_E_TOO_LATE -2147417831 ; 0x80010119

${STDMACROS_DefIfNotDef} LUID_AND_ATTRIBUTES_SIZEOF 12
${STDMACROS_DefIfNotDef} TOKEN_PRIVILEGES_COUNT_SIZEOF 4

; ${Security_COMSetDefaultLevel} result
; Set default COM security level
; Return value:
;   0 if successful otherwise error code
; Notes:
;   Initialization of COM is done via OleInitialize in NSIS installer code.
;   This macro has to be used at the earliest possible stage (.onInit).
;   The default COM security level is only set at first use. Subsequent
;   use does nothing else but returns 0.
!macro Security_COMSetDefaultLevel UN
  !define ${UN}${SECURITY_PREFIX}COMSetDefaultLevel_INCLUDED
!macroend
${SECURITY_FUNCDEF} COMSetDefaultLevel
!macro Security_ComSetDefaultLevel_Call UN RESULT
  !ifndef `${UN}${SECURITY_PREFIX}COMSetDefaultLevel_INVOKED`
    !define ${UN}${SECURITY_PREFIX}COMSetDefaultLevel_INVOKED
    !insertmacro STDMACROS_ToSystemReg ${RESULT} ${__MACRO__}_RESULT
    System::Call "ole32::CoInitializeSecurity( \
      p 0, i -1, p 0, p 0, i ${RPC_C_AUTHN_LEVEL_DEFAULT}, \
      i ${RPC_C_IMP_LEVEL_IMPERSONATE}, p 0, i ${EOAC_NONE}, p 0) \
      i.${${__MACRO__}_RESULT}"
  !else
    ; Only call CoInitializeSecurity once
    StrCpy ${RESULT} 0
  !endif
!macroend

; ${Security_PrivilegeValueToStack} privilege counter
; Convert privilege name into its value and put this value onto the stack
; Parameter:
;   privilege - name of privilege
;   counter - value of counter
; Return value:
;   counter incremented by one if successful and value put onto stack
!macro Security_PrivilegeValueToStack UN
  !define ${UN}${SECURITY_PREFIX}PrivilegeValueToStack_INCLUDED
!macroend
${SECURITY_FUNCDEF} PrivilegeValueToStack
!macro Security_PrivilegeValueToStack_Call UN PRIVILEGE COUNTER
  Push "${PRIVILEGE}"
  System::Call 'advapi32::LookupPrivilegeValue(p 0, t s, *l .s) i.s'
  Exch ${COUNTER}
  ${If} ${COUNTER} != 0
    Pop ${COUNTER}
    IntOp ${COUNTER} ${COUNTER} + 1
  ${Else}
    Pop ${COUNTER}
  ${EndIf}
!macroend

; ${Security_AcquirePrivileges} privilege1 privilege2 privilege3 ... count
${SECURITY_FUNCDEF} AcquirePrivileges
!macro Security_AcquirePrivileges_Call UN RESULT
  Call `${UN}${SECURITY_PREFIX}AcquirePrivileges`
  Pop ${RESULT}
!macroend

!macro Security_AcquirePrivileges UN
; Acquire privileges
; Parameters:
;   1st privilege value
;   2nd privilege value
;   ...
;   count - number of privilege values
;   privilege value for example of SE_BACKUP_NAME could be determined via:
;     ${Security_PrivilegeValueToStack} SE_BACKUP_NAME $0
; Return value:
;   If the function succeeds, the return value is zero.
;   Otherwise return value represents an error code.
${SECURITY_FUNCPROLOG} "${UN}" AcquirePrivileges
  System::Store 'S'
  Pop $3 ; count
  ${If} $3 > 0
    System::Call "kernel32::GetCurrentProcess() p.r0"
    System::Call "advapi32::OpenProcessToken(p r0, \
      i ${TOKEN_ADJUST_PRIVILEGES}, *p .r1) i.r0?e"
    ${If} $0 != 0
      Pop $0 ; Successful therefore ignore error code
      IntOp $4 ${LUID_AND_ATTRIBUTES_SIZEOF} * $3
      IntOp $4 $4 + ${TOKEN_PRIVILEGES_COUNT_SIZEOF}
      System::Alloc $4 ; Allocate TOKEN_PRIVILEGES structure
      Pop $5
      ${If} $5 P<> 0
        StrCpy $4 $5
        System::Call "*$4(&i${TOKEN_PRIVILEGES_COUNT_SIZEOF} r3)"
        IntOp $4 $4 + ${TOKEN_PRIVILEGES_COUNT_SIZEOF}
        ${While} $3 > 0
          System::Call "*$4(l s, &i4 ${SE_PRIVILEGE_ENABLED})"
          IntOp $4 $4 + ${LUID_AND_ATTRIBUTES_SIZEOF}
          IntOp $3 $3 - 1
        ${EndWhile}
        ; Acquire new set of privileges
        System::Call "advapi32::AdjustTokenPrivileges(p r1, i 0, \
          p r5, i 0, p 0, p 0) i.r0?e"
        ${If} $0 != 0
          StrCpy $0 0
          Exch $0
        ${EndIf}
        System::Free $5
      ${Else}
        Push ${ERROR_OUTOFMEMORY}
      ${EndIf}
    ${Else}
      Push ${ERROR_INVALID_DATA}
    ${EndIf}
    System::Call "kernel32::CloseHandle(p r1)"
  ${EndIf}
  System::Store 'L'
FunctionEnd
!macroend ; Security_AcquirePrivileges

!endif ; SECURITY_INCLUDED
