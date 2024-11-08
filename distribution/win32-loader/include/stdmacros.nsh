; Licensed under the zlib/libpng license (same as NSIS)

!ifndef STDMACROS_INCLUDED
!define STDMACROS_INCLUDED

; Please use \ (backslash) in !include directives for compatibility
; between Windows and POSIX like systems

!define STDMACROS_UNFUNC "un."

; Redefine function macro (switch from include to call function mode)
!macro STDMACROS_FUNCREDEF PREFIX UN FUNCNAME
  !ifndef `${UN}${PREFIX}${FUNCNAME}_INCLUDED`
    !insertmacro ${PREFIX}${FUNCNAME} "${UN}"
  !endif
  !define /redef `${UN}${PREFIX}${FUNCNAME}` \
    `!insertmacro ${PREFIX}${FUNCNAME}_Call "${UN}"`
!macroend

; Define function macro
!macro STDMACROS_FUNCDEF PREFIX FUNCNAME
  !define `${PREFIX}${FUNCNAME}` \
    `!insertmacro STDMACROS_FUNCREDEF ${PREFIX} "" ${FUNCNAME}`
  !define `${STDMACROS_UNFUNC}${PREFIX}${FUNCNAME}` \
    `!insertmacro STDMACROS_FUNCREDEF ${PREFIX} ${STDMACROS_UNFUNC} ${FUNCNAME}`
!macroend

; Provide function prolog for installer and uninstaller
!macro STDMACROS_FUNCPROLOG PREFIX UN FUNCNAME
  !define `${UN}${PREFIX}${FUNCNAME}_INCLUDED`
  Function `${UN}${PREFIX}${FUNCNAME}`
!macroend

; Include required function
!macro STDMACROS_FUNCINC PREFIX UN FUNCNAME
  !ifndef `${UN}${PREFIX}${FUNCNAME}_INCLUDED`
    !insertmacro ${PREFIX}${FUNCNAME} "${UN}"
  !endif
!macroend

; Define identifier with value if it is not defined
; Issue an error if identifier is defined and original value differs
!macro STDMACROS_DefIfNotDef NAME VALUE
  !ifdef ${NAME}
    !if ${${NAME}} != ${VALUE}
      !error "Wrong value for ${NAME} should be ${VALUE} instead of ${${NAME}}"
    !endif
  !else
    !define ${NAME} ${VALUE}
  !endif
!macroend
!define STDMACROS_DefIfNotDef `!insertmacro STDMACROS_DefIfNotDef`

; Translate register name into System plug-in register name
; Parameter:
;   reg - register
; Return value:
;   System plug-in register name
!macro STDMACROS_ToSystemReg reg sysreg
  !searchreplace reg ${reg} "R" "1"
  !searchreplace ${sysreg} ${reg} "$" "r"
!macroend

!endif ; STDMACROS_INCLUDED
