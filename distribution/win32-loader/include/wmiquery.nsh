; Licensed under the zlib/libpng license (same as NSIS)

!ifndef WMIQUERY_INCLUDED
!define WMIQUERY_INCLUDED

!define WMIQUERY_PREFIX "WMIQUERY_"

!include include\stdmacros.nsh
!include LogicLib.nsh
!include Win\COM.nsh

!define WMIQUERY_FUNCDEF \
  `!insertmacro STDMACROS_FUNCDEF ${WMIQUERY_PREFIX}`
!define WMIQUERY_FUNCPROLOG \
  `!insertmacro STDMACROS_FUNCPROLOG ${WMIQUERY_PREFIX}`

${STDMACROS_DefIfNotDef} VT_I4       3 ; 0x0003
${STDMACROS_DefIfNotDef} VT_BSTR     8 ; 0x0008

${STDMACROS_DefIfNotDef} WBEM_FLAG_FORWARD_ONLY       0x20
${STDMACROS_DefIfNotDef} WBEM_FLAG_RETURN_IMMEDIATELY 0x10
${STDMACROS_DefIfNotDef} WBEM_INFINITE                0xffffffff

${STDMACROS_DefIfNotDef} \
  CLSID_WbemLocator {4590f811-1d3a-11d0-891f-00aa004b2e24}

${STDMACROS_DefIfNotDef} \
  IID_IWbemLocator {dc12a687-737f-11cf-884d-00aa004b2e24}
!ifndef IWbemLocator::ConnectServer
${NSISCOMIFACEDECL}IWbemLocator ConnectServer 3 (w,p,p,p,i,p,p,*p)i
!endif

!ifndef IWbemServices::ExecQuery
${NSISCOMIFACEDECL}IWbemServices ExecQuery 20 (w,w,i,p,*p)i
!endif

!ifndef IEnumWbemClassObject::Next
${NSISCOMIFACEDECL}IEnumWbemClassObject Next 4 (i,i,*p,*p)i
!endif

!ifndef IWbemClassObject::Get
${NSISCOMIFACEDECL}IWbemClassObject Get 4 (w,i,p,i,i)i
!endif

; ${WMIQUERY_SingleProp} property class value
${WMIQUERY_FUNCDEF} SingleProp
!macro WMIQUERY_SingleProp_Call UN PROPERTY CLASS VALUE
  Push "${PROPERTY}"
  Push "${CLASS}"
  Call `${UN}${WMIQUERY_PREFIX}SingleProp`
  Pop ${VALUE}
!macroend

!macro WMIQUERY_SingleProp UN
; Query provided Windows Management Instrumentation (WMI) class for a single property
; Parameters:
;   property - name of the property to be retrieved from WMI class
;   class - WMI class to query
; Return:
;   Empty string in case of error otherwise value of queried property
${WMIQUERY_FUNCPROLOG} "${UN}" SingleProp
  ; Save registers
  System::Store 'S'

  Pop $8
  Pop $7
  ; property=$7, class=$8

  ; Create IWbemLocator interface
  System::Call "ole32::CoCreateInstance(g '${CLSID_WbemLocator}', p 0, \
    i ${CLSCTX_INPROC_SERVER}, g '${IID_IWbemLocator}', *p .r1) i.r0"
  ; locator=$1
  ${If} $0 != 0
    StrCpy $0 "CoCreateInstance: $0"
  ${Else}
    ; IWbemLocator::ConnectServer(objectpath, user, password, locale,
    ;   securityflags, authority, context, services)
    ${IWbemLocator::ConnectServer} $1 "('root/cimv2',0,0,0,0,0,0,.r2).r0"
    ; services=$2
    ${If} $0 != 0
      DetailPrint "IWbemLocator::ConnectServer: $0"
      StrCpy $0 ""
    ${Else}
      ${IWbemServices::ExecQuery} $2 "('WQL','Select $7 from $8',\
	${WBEM_FLAG_FORWARD_ONLY}|${WBEM_FLAG_RETURN_IMMEDIATELY},0,.r3)r0"
      ${If} $0 != 0
        DetailPrint "IWbemServices::ExecQuery: $0"
        StrCpy $0 ""
      ${Else}
        ${IEnumWbemClassObject::Next} $3 "(${WBEM_INFINITE},1,.r4,.r5).r0"
        ${If} $4 != 0
          ; Allocate memory for Variant
	  System::Call "*(&i2 0, &i2 0, &i2 0, &i2 0, p 0, p 0) p.r6"
          ${If} $6 P<> 0
            ${IWbemClassObject::Get} $4 "(r7,0,r6,0,0).r0"
            ${If} $0 != 0
              DetailPrint "IWbemClassObject::Get: $0"
              StrCpy $0 ""
            ${Else}
              ; Check type of variant
              System::Call "*$6(i .r0)"
              IntOp $0 $0 & 0xFFFF
              ${If} $0 = ${VT_I4}
                ; Access 32 bit integer from Variant
                System::Call "*$6(&i2, &i2, &i2, &i2, i .r0)"
              ${ElseIf} $0 = ${VT_BSTR}
                ; Access bstrVal from Variant
                System::Call "*$6(i, i, w .r0)"
              ${Else}
                DetailPrint "VARTYPE: $0"
                StrCpy $0 ""
	      ${EndIf}
              System::Call "ole32::VariantClear(p r6)"
            ${EndIf}
            ; Free variant
            System::Free $6
	    ; Release IWbemClassObject
            ${IUnknown::Release} $4 ""
          ${Else}
            StrCpy $0 ""
          ${EndIf}
        ${Else}
          StrCpy $0 "IEnumWbemClassObject::Next: $0"
        ${EndIf}
        ; Release IEnumWbemClassObject
        ${IUnknown::Release} $3 ""
      ${EndIf}
      ; Release IWbemService
      ${IUnknown::Release} $2 ""
    ${EndIf}
    ; Release IWbemLocator
    ${IUnknown::Release} $1 ""
  ${EndIf}

  Push $0

  ; Restore registers
  System::Store 'L'
FunctionEnd
!macroend ; WMIQUERY_SingleProp

!endif ; WMIQUERY_INCLUDED
