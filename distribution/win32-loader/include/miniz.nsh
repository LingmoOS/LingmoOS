; Licensed under the zlib/libpng license (same as NSIS)

!ifndef MINIZ_INCLUDED
!define MINIZ_INCLUDED

!ifndef MINIZ_BUFFER_SIZE
!define MINIZ_BUFFER_SIZE 262144
!endif
!ifndef MINIZ_COMP_FLAGS
!define MINIZ_COMP_FLAGS 1500
!endif
!define MINIZ_CRC32_INIT 0
!define MINIZ_FINISH 4
!define MINIZ_NO_FLUSH 0

!define MINIZ_HELPER_DLL miniz.dll
!searchreplace MINIZ_NAME ${MINIZ_HELPER_DLL} ".dll" ""
!ifndef MINIZ_HELPER_DIR
  !error "Please define the directory containing ${MINIZ_HELPER_DLL}"
!endif

!define MINIZ_PREFIX "MINIZ_"
!define MINIZ_UNFUNC "un."

!define /math CPIO_BUFFER_SIZE ${MINIZ_BUFFER_SIZE} >>> 1

!include include\stdmacros.nsh
!include LogicLib.nsh
!include Win\WinError.nsh
!include include\cpio.nsh

!define MINIZ_FUNCDEF \
  `!insertmacro STDMACROS_FUNCDEF ${MINIZ_PREFIX}`
!define MINIZ_FUNCINC \
  `!insertmacro STDMACROS_FUNCINC ${MINIZ_PREFIX}`
!define MINIZ_FUNCPROLOG \
  `!insertmacro STDMACROS_FUNCPROLOG ${MINIZ_PREFIX}`

; ${MINIZ_Alloc} handle
${MINIZ_FUNCDEF} Alloc
!macro MINIZ_Alloc_Call UN HANDLE
  Call `${UN}${MINIZ_PREFIX}Alloc`
  Pop ${HANDLE}
!macroend

; ${MINIZ_Init} flags handle
; Create and initalize compressor object
; Parameter:
;   flags - compression flags
; Return value:
;   Handle to compressor object
!macro MINIZ_Init UN
  !define ${UN}MINIZ_Init_INCLUDED
!macroend
${MINIZ_FUNCDEF} Init
!macro MINIZ_Init_Call UN FLAGS HANDLE
  !insertmacro STDMACROS_ToSystemReg ${FLAGS} ${__MACRO__}_FLAGS
  !insertmacro STDMACROS_ToSystemReg ${HANDLE} ${__MACRO__}_HANDLE
  System::Call "${MINIZ_NAME}::init(i ${${__MACRO__}_FLAGS}) p.${${__MACRO__}_HANDLE}"
!macroend

; ${MINIZ_CRC32} crc data len
; Compute cyclic redundancy check (CRC) value
; Parameter:
;   crc - initial value of CRC
;   data - buffer containing the data
;   len - size of data in bytes
; Return value:
;   updated CRC value
!macro MINIZ_CRC32 UN
  !define ${UN}MINIZ_CRC32_INCLUDED
!macroend
${MINIZ_FUNCDEF} CRC32
!macro MINIZ_CRC32_Call UN CRC DATA LEN RESULT
  !insertmacro STDMACROS_ToSystemReg ${CRC} ${__MACRO__}_CRC
  !insertmacro STDMACROS_ToSystemReg ${DATA} ${__MACRO__}_DATA
  !insertmacro STDMACROS_ToSystemReg ${LEN} ${__MACRO__}_LEN
  !insertmacro STDMACROS_ToSystemReg ${RESUlT} ${__MACRO__}_RESUlT
  System::Call "${MINIZ_NAME}::crc32(i ${${__MACRO__}_CRC}, p ${${__MACRO__}_DATA}, i ${${__MACRO__}_LEN}) i.${${__MACRO__}_RESULT}"
!macroend

; ${MINIZ_Compress} handle input inputlen output ouputlen flush result
; Compress data
; Parameters:
;   handle - compressor object
;   input - input data
;   inputlen - size of input in bytes
;   output - output buffer
;   outputlen - size of output buffer in bytes
;   flush - flag to perform flush operation
; Return value:
;   Negative value in case of an error
!macro MINIZ_Compress UN
  !define ${UN}MINIZ_Compress_INCLUDED
!macroend
${MINIZ_FUNCDEF} Compress
!macro MINIZ_Compress_Call UN HANDLE INPUT INPUTLEN OUTPUT OUTPUTLEN FLUSH RESULT
  !insertmacro STDMACROS_ToSystemReg ${HANDLE} ${__MACRO__}_HANDLE
  !insertmacro STDMACROS_ToSystemReg ${INPUT} ${__MACRO__}_INPUT
  !insertmacro STDMACROS_ToSystemReg ${INPUTLEN} ${__MACRO__}_INPUTLEN
  !insertmacro STDMACROS_ToSystemReg ${OUTPUT} ${__MACRO__}_OUTPUT
  !insertmacro STDMACROS_ToSystemReg ${OUTPUTLEN} ${__MACRO__}_OUTPUTLEN
  !insertmacro STDMACROS_ToSystemReg ${FLUSH} ${__MACRO__}_FLUSH
  !insertmacro STDMACROS_ToSystemReg ${RESULT} ${__MACRO__}_RESULT
  System::Call "${MINIZ_NAME}::compress(p ${${__MACRO__}_HANDLE}, p ${${__MACRO__}_INPUT}, *i ${${__MACRO__}_INPUTLEN}${${__MACRO__}_INPUTLEN}, p ${${__MACRO__}_OUTPUT}, *i ${${__MACRO__}_OUTPUTLEN}${${__MACRO__}_OUTPUTLEN}, i ${${__MACRO__}_FLUSH}) i.${${__MACRO__}_RESULT}"
!macroend

; ${MINIZ_Destroy} handle
; Destroy compressor object
; Parameter:
;   handle - compressor object
!macro MINIZ_Destroy UN
  !define ${UN}MINIZ_Destroy_INCLUDED
!macroend
${MINIZ_FUNCDEF} Destroy
!macro MINIZ_Destroy_Call UN HANDLE
  !insertmacro STDMACROS_ToSystemReg ${HANDLE} ${__MACRO__}_HANDLE
  System::Call "${MINIZ_NAME}::destroy(p ${${__MACRO__}_HANDLE})"
!macroend

; ${MINIZ_Free} module
; Release the loaded miniz helper module
; Parameter:
;   module - handle to the loaded module
!macro MINIZ_Free UN
  !define ${UN}MINIZ_Free_INCLUDED
!macroend
${MINIZ_FUNCDEF} Free
!macro MINIZ_Free_Call UN MODULE
  !insertmacro STDMACROS_ToSystemReg ${MODULE} ${__MACRO__}_MODULE
  System::Call "kernel32::FreeLibrary(p ${${__MACRO__}_MODULE})"
!macroend

; ${MINIZ_GZip_Header} output buffer len result
${MINIZ_FUNCDEF} GZip_Header
!macro MINIZ_GZip_Header_Call UN OUTPUT RESULT
  Push ${OUTPUT}
  Call `${UN}${MINIZ_PREFIX}GZip_Header`
  Pop ${RESULT}
!macroend

; ${MINIZ_GZip} input output result
${MINIZ_FUNCDEF} GZip
!macro MINIZ_GZip_Call UN INPUT OUTPUT RESULT
  Push ${INPUT}
  Push ${OUTPUT}
  Call `${UN}${MINIZ_PREFIX}GZip`
  Pop ${RESULT}
!macroend

; $[MINIZ_CPIO_GZ_Open} output result
${MINIZ_FUNCDEF} CPIO_GZ_Open
!macro MINIZ_CPIO_GZ_Open_Call UN OUTPUT RESULT
  Push ${OUTPUT}
  Call `${UN}${MINIZ_PREFIX}CPIO_GZ_Open`
  Pop ${RESULT}
!macroend

; ${MINIZ_CPIO_GZ_Filter} handle buffer len result
${MINIZ_FUNCDEF} CPIO_GZ_Filter
!macro MINIZ_CPIO_GZ_Filter_Call UN HANDLE BUFFER LEN RESULT
  Push ${HANDLE}
  Push ${BUFFER}
  Push ${LEN}
  Call `${UN}${MINIZ_PREFIX}CPIO_GZ_Filter`
  Pop ${RESULT}
!macroend

; ${MINIZ_CPIO_GZ_Write} handle filename result
${MINIZ_FUNCDEF} CPIO_GZ_Write
!macro MINIZ_CPIO_GZ_Write_Call UN HANDLE FILENAME RESULT
  Push ${HANDLE}
  Push "${FILENAME}"
  Call `${UN}${MINIZ_PREFIX}CPIO_GZ_Write`
  Pop ${RESULT}
!macroend

; ${MINIZ_CPIO_GZ_Close} handle result
${MINIZ_FUNCDEF} CPIO_GZ_Close
!macro MINIZ_CPIO_GZ_Close_Call UN HANDLE RESULT
  Push ${HANDLE}
  Call `${UN}${MINIZ_PREFIX}CPIO_GZ_Close`
  Pop ${RESULT}
!macroend

!macro MINIZ_Alloc UN
; Unpack and load miniz helper dll
; Result:
;   Handle of loaded module
${MINIZ_FUNCPROLOG} "${UN}" Alloc
  Push $0
  Push $1

  StrCpy $1 "$PLUGINSDIR\${MINIZ_HELPER_DLL}"
  ${IfNot} ${FileExists} "$1"
    ; Unpack miniz helper dll
    Push $OUTDIR
    SetOutPath $PLUGINSDIR
    ReserveFile "${MINIZ_HELPER_DIR}\${MINIZ_HELPER_DLL}"
    File "/oname=${MINIZ_HELPER_DLL}" "${MINIZ_HELPER_DIR}\${MINIZ_HELPER_DLL}"
    Pop $0
    SetOutPath $0
  ${EndIf}
  System::Call "kernel32::LoadLibrary(t r1) p.r0"

  Pop $1
  Exch $0
FunctionEnd
!macroend ; MINIZ_Alloc

!macro MINIZ_GZip_Header UN
; Write gzip header into the provided buffer
; Parameter:
;   output - writable file handle
; Return value:
;   Non zero value if successful
${MINIZ_FUNCPROLOG} "${UN}" GZip_Header
  Exch $0
  Push $1
  Push $2

  ; GZip header
  ; offset size value               description
  ; 0      2    0x1F 0x8b           identificatioon bytes
  ; 2      1    0x08                compression method (0x08 = deflate)
  ; 3      1    0x00                flags (0x00 = no flags set)
  ; 4      4    0x00 0x00 0x00 0x00 last modification time
  ; 8      1    0x02                compression flags (0x02 = max compression)
  ; 9      1    0x00                operationg system (0x00 = MS/DOS, NT/Win32)
  System::Call "*(&l .r2, &i1 31, &i1 139, &i1 8, &i1 0, &i4 0, &i1 2, &i1 0) p.r1"
  ${If} $1 P<> 0
    System::Call "kernel32::WriteFile(p r0, p r1, i r2, *i .r2, p 0) i.r0"
    System::Free $1
  ${Else}
    StrCpy $0 0
  ${EndIf}

  Pop $2
  Pop $1
  Exch $0
FunctionEnd
!macroend ; MINIZ_GZip_Header

!macro MINIZ_GZip UN
; Read input file, compress its content and write the compressed data
; to a gzip formatted output file
; Parameters:
;   input - handle of file to read from
;   output - writable file handle
; Return value:
;   0 if successful otherwise error code
${MINIZ_FUNCINC} "${UN}" Alloc
${MINIZ_FUNCINC} "${UN}" Init
${MINIZ_FUNCINC} "${UN}" CRC32
${MINIZ_FUNCINC} "${UN}" GZip_Header
${MINIZ_FUNCINC} "${UN}" Compress
${MINIZ_FUNCINC} "${UN}" Destroy
${MINIZ_FUNCINC} "${UN}" Free
${MINIZ_FUNCPROLOG} "${UN}" GZip
  System::Store 'S'

  Pop $2
  Pop $1
  ; input=$1, output=$2

  StrCpy $0 ${ERROR_INVALID_HANDLE}
  ${If} $1 P<> 0
  ${AndIf} $2 P<> 0
    !insertmacro ${MINIZ_PREFIX}Alloc_Call "${UN}" $3
    ${If} $3 P<>  0
      !insertmacro ${MINIZ_PREFIX}Init_Call "${UN}" ${MINIZ_COMP_FLAGS} $4
      ${If} $4 P<> 0
        ; Allocate input buffer (half of output buffer size)
        StrCpy $6 ${MINIZ_BUFFER_SIZE}
        IntOp $6 $6 >> 1
        System::Alloc $6
        Pop $5
        ${If} $5 P<> 0
          ; Allocate output buffer
          StrCpy $8 ${MINIZ_BUFFER_SIZE}
          System::Alloc $8
          Pop $7
          ${If} $7 P<> 0
            !insertmacro ${MINIZ_PREFIX}GZip_Header_Call "${UN}" $2 $0
            ${If} $0 != 0
              StrCpy $R1 ${MINIZ_CRC32_INIT}
              StrCpy $R2 0
              ${DoWhile} $0 != 0
                System::Call "kernel32::ReadFile(p r1, p r5, i r6, *i .r9, p 0) i.r0"
                ${If} $0 != 0
                  !insertmacro ${MINIZ_PREFIX}CRC32_Call "${UN}" $R1 $5 $9 $R1
                  IntOp $R2 $R2 + $9
                  ${If} $9 < $6
                     ; Reached end of file
                     !insertmacro ${MINIZ_PREFIX}Compress_Call "${UN}" $4 $5 $9 $7 $8 ${MINIZ_FINISH} $0
                     ${If} $0 < 0
                       ${ExitDo}
                     ${EndIf}
                     System::Call "kernel32::WriteFile(p r2, p r7, i r8, *i .r9, p 0) i.r0"
                     ${ExitDo}
                  ${Else}
		     StrCpy $R0 $8 ; Reset size to size of output buffer
                     !insertmacro ${MINIZ_PREFIX}Compress_Call "${UN}" $4 $5 $9 $7 $R0 ${MINIZ_NO_FLUSH} $0
                     ${If} $0 < 0
                       ${ExitDo}
                     ${EndIf}
                     System::Call "kernel32::WriteFile(p r2, p r7, i r10, *i .r9, p 0) i.r0"
                  ${EndIf}
                ${Else}
                  ${ExitDo}
                ${EndIf}

              ${Loop}
              ${If} $0 > 0
                ; Write gzip footer
                System::Call "*$7(&l .r8, &i4 r11)"
                System::Call "kernel32::WriteFile(p r2, p r7, i r8, *i .r8, p 0) i.r0"
                ${If} $0 != 0
                  System::Call "*$7(&i4 r12)"
                  System::Call "kernel32::WriteFile(p r2, p r7, i r8, *i .r8, p 0) i.r0"
                  ${If} $0 != 0
                    ; Indicate success
                    StrCpy $0 0
                  ${EndIf}
                ${EndIf}
              ${ElseIf} $0 == 0
                System::Call "kernel32::GetLastError() i.r0"
              ${EndIf}
            ${EndIf}
            System::Free $7
          ${EndIf}
          System::Free $5
        ${EndIf}
        !insertmacro ${MINIZ_PREFIX}Destroy_Call "${UN}" $4
      ${EndIf}
      !insertmacro ${MINIZ_PREFIX}Free_Call "${UN}" $3
    ${EndIf}
  ${EndIf}

  Push $0
  System::Store 'L'
FunctionEnd
!macroend ; MINIZ_GZip

!macro MINIZ_CPIO_GZ_Open UN
; Allocate and intialize handle for creating a gzip compressed cpio archive
; Parameter:
;   output - writable file handle
; Return value:
;   Handle for creating a gzip compressed cpio archive
${MINIZ_FUNCINC} "${UN}" Alloc
${MINIZ_FUNCINC} "${UN}" Init
${MINIZ_FUNCINC} "${UN}" GZip_Header
${MINIZ_FUNCINC} "${UN}" Destroy
${MINIZ_FUNCINC} "${UN}" Free
${MINIZ_FUNCPROLOG} "${UN}" CPIO_GZ_Open
  System::Store 'S'
  Pop $1

  StrCpy $0 0
  ${If} $1 P<> 0
    !insertmacro ${MINIZ_PREFIX}Alloc_Call "${UN}" $2
    ${If} $2 P<>  0
      !insertmacro ${MINIZ_PREFIX}Init_Call "${UN}" ${MINIZ_COMP_FLAGS} $3
      ${If} $3 P<> 0
        StrCpy $5 ${MINIZ_BUFFER_SIZE}
	System::Alloc $5
	Pop $4
        ${If} $4 P<> 0
          !insertmacro ${MINIZ_PREFIX}GZip_Header_Call "${UN}" $1 $0
	  ${If} $0 != 0
            System::Call "*(p r1, p r2, p r3, p r4, &i${NSIS_PTR_SIZE} r5, &i${NSIS_PTR_SIZE} 0, &i4 ${MINIZ_CRC32_INIT}) p.r0"
          ${EndIf}
        ${EndIf}
      ${EndIf}
    ${EndIf}
  ${EndIf}

  ${If} $0 == 0
    ${If} $4 P<> 0
      System::Free $4
    ${EndIf}
    ${If} $3 P<> 0
      !insertmacro ${MINIZ_PREFIX}Destroy_Call "${UN}" §4
    ${EndIf}
    ${If} $2 P<> 0
      !insertmacro ${MINIZ_PREFIX}Free_Call "${UN}" §3
    ${EndIf}
  ${EndIf}

  Push $0
  System::Store 'L'
FunctionEnd
!macroend ; MINIZ_CPIO_GZ_Open

!macro MINIZ_CPIO_GZ_Filter UN
; Write buffer in compressed format to file
; Parameters:
;   handle - handle obtained via MINIZ_CPIO_GZ_Open
;   buffer - buffer to be written
;   len - size of buffer in bytes
; Return value:
;   Non zero value if successful
${MINIZ_FUNCINC} "${UN}" CRC32
${MINIZ_FUNCPROLOG} "${UN}" CPIO_GZ_Filter
  System::Store 'S'
  Pop $R0
  Pop $9
  Pop $8

  System::Call "*$8(p .r1, p, p .r3, p .r4, &i${NSIS_PTR_SIZE} .r5, &i${NSIS_PTR_SIZE} .r6, &i4 .r7)"
  !insertmacro ${MINIZ_PREFIX}CRC32_Call "${UN}" $7 $9 $R0 $7
  IntOp $6 $6 + $R0
  !insertmacro ${MINIZ_PREFIX}Compress_Call "${UN}" $3 $9 $R0 $4 $5 ${MINIZ_NO_FLUSH} $0
  ${If} $0 >= 0
    System::Call "kernel32::WriteFile(p r1, p r4, i r5, *i .r5, p 0) i.r0"
    ${If} $0 != 0
      ; Update size and CRC32 value
      System::Call "*$8(p, p, p, p, &i${NSIS_PTR_SIZE}, &i${NSIS_PTR_SIZE} r6, &i4 r7)"
    ${EndIf}
  ${EndIf}

  Push $0
  System::Store 'L'
FunctionEnd
!macroend ; MINIZ_CPIO_GZ_Filter

!macro MINIZ_CPIO_GZ_Write UN
; Write CPIO entry of given file name to handle
; Parameters:
;   handle - handle obtained via MINIZ_CPIO_GZ_Open
;   filename - name of file to be added
; Return value:
;   0 if successful otherwise error code
${MINIZ_FUNCINC} "${UN}" CPIO_GZ_Filter
${CPIO_FUNCINC} "${UN}" Write_Filter
${MINIZ_FUNCPROLOG} "${UN}" CPIO_GZ_Write
   Push $0
   GetFunctionAddress $0 `${UN}${MINIZ_PREFIX}CPIO_GZ_Filter`
   Exch $0
   Call `${UN}${CPIO_PREFIX}Write_Filter`
FunctionEnd
!macroend ; MINIZ_CPIO_GZ_Write

!macro MINIZ_CPIO_GZ_Close UN
; Release handle and its associated memory
; Parameters:
;   handle - handle obtained via MINIZ_CPIO_GZ_Open
; Return value:
;   0 if successful otherwise error code
${MINIZ_FUNCINC} "${UN}" Compress
${MINIZ_FUNCINC} "${UN}" Destroy
${MINIZ_FUNCINC} "${UN}" Free
${MINIZ_FUNCPROLOG} "${UN}" CPIO_GZ_Close
  System::Store 'S'
  Pop $8

  StrCpy $0 ${ERROR_INVALID_HANDLE}
  ${If} $8 P<> 0
    System::Call "*$8(p .r1, p .r2, p .r3, p .r4, &i${NSIS_PTR_SIZE} .r5, &i${NSIS_PTR_SIZE} .r6, &i4 .r7)"
    StrCpy $9 0
    !insertmacro ${MINIZ_PREFIX}Compress_Call "${UN}" $3 0 $9 $4 $5 ${MINIZ_FINISH} $0
    ${If} $0 >= 0
      System::Call "kernel32::WriteFile(p r1, p r4, i r5, *i .r5, p 0) i.r0"
      ${If} $0 != 0
        ; Write gzip footer
        System::Call "*$4(&l .r5, &i4 r7)"
        System::Call "kernel32::WriteFile(p r1, p r4, i r5, *i .r5, p 0) i.r0"
        ${If} $0 != 0
          System::Call "*$4(&i4 r6)"
          System::Call "kernel32::WriteFile(p r1, p r4, i r5, *i .r5, p 0) i.r0"
        ${EndIf}
      ${EndIf}
    ${EndIf}
    ${If} $0 != 0
      ; Indicate success
      StrCpy $0 0
    ${Else}
      System::Call "kernel32::GetLastError() i.r0"
    ${EndIf}
    ${If} $4 P<> 0
      System::Free $4
    ${EndIf}
    ${If} $3 P<> 0
      !insertmacro ${MINIZ_PREFIX}Destroy_Call "${UN}" $3
    ${EndIf}
    ${If} $2 P<> 0
      !insertmacro ${MINIZ_PREFIX}Free_Call "${UN}" $2
    ${EndIf}
    System::Free $8
  ${EndIf}

  Push $0
  System::Store 'L'
FunctionEnd
!macroend ; MINIZ_CPIO_GZ_Close

!endif ; MINIZ_INCLUDED
