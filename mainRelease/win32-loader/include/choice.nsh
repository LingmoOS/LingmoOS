; Licensed under the zlib/libpng license (same as NSIS)

!ifndef CHOICE_INCLUDED
!define CHOICE_INCLUDED

!ifndef CHOICE_INNER_DIALOG_HEIGHT
!define CHOICE_INNER_DIALOG_HEIGHT 125
!endif

!include include\stdmacros.nsh
!include nsDialogs.nsh
!include Win\WinUser.nsh

!define CHOICE_PREFIX "Choice_"
!define CHOICE_UNFUNC "un."

${STDMACROS_DefIfNotDef} IDC_CHILDRECT 1018

!define CHOICE_FUNCDEF \
  `!insertmacro STDMACROS_FUNCDEF ${CHOICE_PREFIX}`
!define CHOICE_FUNCINC \
  `!insertmacro STDMACROS_FUNCINC ${CHOICE_PREFIX}`
!define CHOICE_FUNCPROLOG \
  `!insertmacro STDMACROS_FUNCPROLOG ${CHOICE_PREFIX}`

; ${Choice_Present} name total
${CHOICE_FUNCDEF} Present
!macro Choice_Present_Call UN NAME TOTAL
  !ifndef ${__MACRO__}_COUNTER
    !define ${__MACRO__}_COUNTER ${TOTAL}
  !else
    !define /redef /math ${__MACRO__}_COUNTER ${${__MACRO__}_COUNTER} - 1
  !endif
  !if ${${__MACRO__}_COUNTER} > 0
    Push $(${NAME}${${__MACRO__}_COUNTER})
    !insertmacro ${__MACRO__} "${UN}" ${NAME} ${TOTAL}
  !else
    Push ${TOTAL}
    Call `${UN}${CHOICE_PREFIX}Present`
    !undef ${__MACRO__}_COUNTER
  !endif
!macroend

; ${Choice_OnClick} hwnd
${CHOICE_FUNCDEF} OnClick
!macro Choice_OnClick_Call UN HWND
  Push ${HWND}
  Call `${UN}${CHOICE_PREFIX}OnClick`
!macroend

; ${Choice_Get} value
; Get choice
; Return value:
;   index number of choice
!macro Choice_Get UN
  !define ${UN}${CHOICE_PREFIX}Get_INCLUDED
!macroend
${CHOICE_FUNCDEF} Get
!macro Choice_Get_Call UN VALUE
  !insertmacro STDMACROS_ToSystemReg ${VALUE} ${__MACRO__}_VALUE
!if ${NSIS_PTR_SIZE} != 4
  System::Call 'user32::GetWindowLongPtr(p $HWNDPARENT, i ${DWLP_USER}) p.${${__MACRO__}_VALUE}'
!else
  System::Call 'user32::GetWindowLong(p $HWNDPARENT, i ${DWLP_USER}) i.${${__MACRO__}_VALUE}'
!endif
!macroend

; ${Choice_Set} value
; Save choice in user data of dialog
; Parameter:
;   value - index number of choice
!macro Choice_Set UN
  !define ${UN}${CHOICE_PREFIX}Set_INCLUDED
!macroend
${CHOICE_FUNCDEF} Set
!macro Choice_Set_Call UN VALUE
  !insertmacro STDMACROS_ToSystemReg ${VALUE} ${__MACRO__}_VALUE
!if ${NSIS_PTR_SIZE} != 4
  System::Call 'user32::SetWindowLongPtr(p $HWNDPARENT, i ${DWLP_USER}, p ${${__MACRO__}_VALUE})'
!else
  System::Call 'user32::SetWindowLong(p $HWNDPARENT, i ${DWLP_USER}, i ${${__MACRO__}_VALUE})'
!endif
!macroend

!macro Choice_OnClick UN
${CHOICE_FUNCINC} "${UN}" Set
; Click event handler
; Parameter:
;   hwnd - handle of window control
${CHOICE_FUNCPROLOG} "${UN}" OnClick
  Exch $0
  Push $1
  ${NSD_GetState} $0 $1
  ${If} $1 != 0
    nsDialogs::GetUserData $0
    Pop $0
    !insertmacro ${CHOICE_PREFIX}Set_Call "${UN}" $0
  ${EndIf}
  Pop $1
  Pop $0
FunctionEnd
!macroend ; Choice_OnClick

!macro Choice_Present UN
${CHOICE_FUNCINC} "${UN}" OnClick
; Create and show choice dialog
; Parameters:
;   text for last option
;   ...
;   text for 2nd option
;   text for 1st option
;   sub text for header
;   total number of elements on stack excluding this one
${CHOICE_FUNCPROLOG} "${UN}" Present
  System::Store 'S'
  nsDialogs::Create ${IDC_CHILDRECT}
  Pop $0

  Pop $4
  ${If} $0 != error
  ${AndIf} $4 > 1
    StrCpy $2 ${CHOICE_INNER_DIALOG_HEIGHT}
    ; Size per item
    IntOp $3 $2 / $4
    ; Vertical position
    IntOp $2 $3 / 2
    ; Sub text for header
    Pop $6
    IntOp $4 $4 - 1
    !insertmacro MUI_HEADER_TEXT $(^NameDA) $6
    ; 1st option
    Pop $6
    IntOp $4 $4 - 1
    StrCpy $5 1
    ${NSD_CreateRadioButton} 3u $2u -6u $3u $6
    Pop $1
    nsDialogs::SetUserData $1 $5
    ${NSD_OnClick} $1 `${UN}${CHOICE_PREFIX}OnClick`
    ; Select 1st option
    ${NSD_SetState} $1 1
    !insertmacro ${CHOICE_PREFIX}Set_Call "${UN}" $5
    ${While} $4 > 0
      IntOp $2 $2 + $3
      IntOp $4 $4 - 1
      IntOp $5 $5 + 1
      Pop $6
      ${NSD_CreateRadioButton} 3u $2u -6u $3u $6
      Pop $1
      nsDialogs::SetUserData $1 $5
      ${NSD_OnClick} $1 `${UN}${CHOICE_PREFIX}OnClick`
    ${EndWhile}
    nsDialogs::Show
  ${EndIf}
  ; Clear arguments from stack
  ${While} $4 > 0
    Pop $6
    IntOp $4 $4 - 1
  ${EndWhile}
  System::Store 'L'
FunctionEnd
!macroend ; Choice_Present

!endif ; CHOICE_INCLUDED
