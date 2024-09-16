; Licensed under the zlib/libpng license (same as NSIS)

!ifndef CPIO_INCLUDED
!define CPIO_INCLUDED

!include include\stdmacros.nsh
!include LogicLib.nsh
!include Win\WinError.nsh

!define CPIO_PREFIX "CPIO_"
!define CPIO_UNFUNC "un."

!ifndef CPIO_BUFFER_SIZE
!define CPIO_BUFFER_SIZE 262144
!endif

!define CPIO_BY_HANDLE_FILEINFO_SIZE 52

!define CPIO_MODE_REGULAR_FILE 0x8000
!define CPIO_MODE_USER_WRITE   0x0080
!define CPIO_MODE_USER_READ    0x0100
!define CPIO_MODE_GROUP_WRITE  0x0010
!define CPIO_MODE_GROUP_READ   0x0020
!define CPIO_MODE_OTHER_WRITE  0x0002
!define CPIO_MODE_OTHER_READ   0x0004
!define /math CPIO_MODE_DEFAULT_FILE ${CPIO_MODE_REGULAR_FILE} + ${CPIO_MODE_USER_WRITE}
!define /redef /math CPIO_MODE_DEFAULT_FILE ${CPIO_MODE_DEFAULT_FILE} + ${CPIO_MODE_USER_READ}
!define /redef /math CPIO_MODE_DEFAULT_FILE ${CPIO_MODE_DEFAULT_FILE} + ${CPIO_MODE_GROUP_READ}
!define /redef /math CPIO_MODE_DEFAULT_FILE ${CPIO_MODE_DEFAULT_FILE} + ${CPIO_MODE_OTHER_READ}

!define CPIO_NEWC_MAGIC "070701"

!define CPIO_FILETIME_TICKS_PER_SEC 10000000
!define CPIO_SEC_TO_UNIX_EPOCH 11644473600

!define CPIO_FUNCDEF \
  `!insertmacro STDMACROS_FUNCDEF ${CPIO_PREFIX}`
!define CPIO_FUNCINC \
  `!insertmacro STDMACROS_FUNCINC ${CPIO_PREFIX}`
!define CPIO_FUNCPROLOG \
  `!insertmacro STDMACROS_FUNCPROLOG ${CPIO_PREFIX}`

!macro CPIO_Padding BUFFER COUNT
  IntOp ${COUNT} ${COUNT} & 3
  System::Call "*${BUFFER}(&i${COUNT} 0)"
!macroend

; ${CPIO_Header} fileinfo filemode filesize namesize result
${CPIO_FUNCDEF} Header
!macro CPIO_Header_Call UN FILEINFO FILEMODE FILESIZE NAMESIZE RESULT
  Push ${FILEINFO}
  Push ${FILEMODE}
  Push ${FILESIZE}
  Push ${NAMESIZE}
  Call `${UN}${CPIO_PREFIX}Header`
  Pop ${RESULT}
!macroend

; ${CPIO_Filter} filehandle buffer len result
${CPIO_FUNCDEF} Filter
!macro CPIO_Filter_Call UN FILEHANDLE BUFFER LEN RESULT
  Push ${FILEHANDLE}
  Push ${BUFFER}
  Push ${LEN}
  Call `${UN}${CPIO_PREFIX}Filter`
  Pop ${RESULT}
!macroend

; ${CPIO_Write_Filter} filehandle filename filter result
${CPIO_FUNCDEF} Write_Filter
!macro CPIO_Write_Filter_Call UN FILEHANDLE FILENAME RESULT
  Push ${FILEHANDLE}
  Push "${FILENAME}"
  Push ${FILTER}
  Call `${UN}${CPIO_PREFIX}Write_Filter`
  Pop ${RESULT}
!macroend

; ${CPIO_Write} filehandle filename result
${CPIO_FUNCDEF} Write
!macro CPIO_Write_Call UN FILEHANDLE FILENAME RESULT
  Push ${FILEHANDLE}
  Push "${FILENAME}"
  Call `${UN}${CPIO_PREFIX}Write`
  Pop ${RESULT}
!macroend

!macro CPIO_Header UN
; Create CPIO header
; Parameters:
;   fileinfo - file information obtained via GetFileInformationByHandle
;   filemode - file mode definition
;   filesize - size of file in bytes
;   namesize - size of file name including terminating zero
; Return value:
;   CPIO header
${CPIO_FUNCPROLOG} "${UN}" Header
  System::Store 'S'
  Pop $5
  Pop $4
  Pop $3
  Pop $2

  StrCpy $0 "${CPIO_NEWC_MAGIC}"
  ; Use nFileIndexLow as inode
  System::Call "*$2(&v48,&i4 .r1)"
  IntFmt $0 "$0%08X" $1
  ; File mode
  IntFmt $0 "$0%08X" $3
  ; User identifier
  IntFmt $1 "%08X" 0
  StrCpy $0 "$0$1"
  ; Group identifier
  StrCpy $0 "$0$1"
  ; Number of links
  System::Call "*$2(&v40,&i4 .r1)"
  IntFmt $0 "$0%08X" $1
  ; Last modified time
  ${If} $3 != 0
    System::Call "*$2(&v20,l .r1)"
    ; Convert file to unix time
    System::Int64Op $1 / ${CPIO_FILETIME_TICKS_PER_SEC}
    Pop $1
    System::Int64Op $1 - ${CPIO_SEC_TO_UNIX_EPOCH}
    Pop $1
  ${Else}
    StrCpy $1 0
  ${EndIf}
  IntFmt $0 "$0%08X" $1
  ; Size of file
  IntFmt $0 "$0%08X" $4
  ; Device major and minor identifier
  System::Call "*$2(&v28,&i4 .r1)"
  Push $1
  IntOp $1 $1 & 0xFF
  IntFmt $0 "$0%08X" $1
  Pop $1
  IntOp $1 $1 >>> 8
  IntFmt $0 "$0%08X" $1
  ; Special device major and minor identifier
  IntFmt $1 "%08X" 0
  StrCpy $0 "$0$1"
  StrCpy $0 "$0$1"
  ; Size of name
  IntFmt $0 "$0%08X" $5
  ; Check
  IntFmt $0 "$0%08X" 0

  Push $0
  System::Store 'L'
FunctionEnd
!macroend ; CPIO_Header

!macro CPIO_Filter UN
; Write buffer to file
; Parameters:
;   filehandle - writable file handle
;   buffer - buffer to be written
;   len - size of buffer in bytes
; Return value:
;   Non zero value if successful
${CPIO_FUNCPROLOG} "${UN}" Filter
  Push $0
  Exch 3
  Exch $1
  Exch 2
  Exch $2
  Exch
  Exch $3

  System::Call "kernel32::WriteFile(p r1, p r2, i r3, *i .r3, p 0) i.r0"
  Pop $3
  Pop $2
  Pop $1
  Exch $0
FunctionEnd
!macroend ; CPIO_Filter

!macro CPIO_Run_Filter FUNCTION FILEHANDLE BUFFER LEN RESULT
  Push ${FILEHANDLE}
  Push ${BUFFER}
  Push ${LEN}
  Call ${FUNCTION}
  Pop ${RESULT}
!macroend

!macro CPIO_Write_Filter UN
${CPIO_FUNCINC} "${UN}" Header
; Write CPIO entry of given file name to file handle via filter function
; Parameters:
;   filehandle - writable file handle
;   filename - name of file to be added
;   filter - output filter function
; Return value:
;   0 if successful otherwise error code
${CPIO_FUNCPROLOG} "${UN}" Write_Filter
  System::Store 'S'
  Pop $R0
  Pop $4
  Pop $1

  StrCpy $0 ${ERROR_OUTOFMEMORY}
  System::Alloc ${CPIO_BY_HANDLE_FILEINFO_SIZE}
  Pop $3
  ${If} $3 P<> 0
    StrCpy $0 0
    ${If} $4 != ""
      ClearErrors
      FileOpen $2 "$4" r
      ${If} ${Errors}
        StrCpy $0 ${ERROR_FILE_NOT_FOUND}
      ${Else}
        System::Call "kernel32::GetFileSize(p r2, p 0) i.r6"
        System::Call "kernel32::GetFileInformationByHandle(p r2, p r3) i.r5"
        ${If} $5 != 0
          StrCpy $0 0
          StrCpy $5 ${CPIO_MODE_DEFAULT_FILE}
        ${Else}
          System::Call "kernel32::GetLastError() i.r0"
        ${EndIf}
      ${EndIf}
    ${Else}
      StrCpy $2 ""
      StrCpy $4 "TRAILER!!!"
      StrCpy $5 0
      StrCpy $6 0
    ${EndIf}
    ${If} $0 == 0
      StrLen $7 $4
      IntOp $7 $7 + 1
      !insertmacro CPIO_Header_Call "${UN}" $3 $5 $6 $7 $5
    ${EndIf}
    System::Free $3
  ${EndIf}
  ${If} $0 == 0
    StrCpy $9 ${CPIO_BUFFER_SIZE}
    System::Alloc $9
    Pop $3
    ${If} $3 P<> 0
      StrLen $8 $5
      System::Call "*$3(&m$8 r5)"
      !insertmacro CPIO_Run_Filter $R0 $1 $3 $8 $5
      ${If} $5 != 0
        ; Name of file
        StrCpy $8 $7
        System::Call "*$3(&m$8 r4)"
        !insertmacro CPIO_Run_Filter $R0 $1 $3 $8 $5
        ${If} $5 != 0
          ; Padding
          IntOp $8 2 - $7
          !insertmacro CPIO_Padding $3 $8
          !insertmacro CPIO_Run_Filter $R0 $1 $3 $8 $5
        ${EndIf}
      ${EndIf}
      ${If} $2 != ""
        ${DoWhile} $5 != 0
          System::Call "kernel32::ReadFile(p r2, p r3, i r9, *i .r8, p 0) i.r5"
          ${If} $5 != 0
            !insertmacro CPIO_Run_Filter $R0 $1 $3 $8 $5
            ${If} $8 < $9
              ${ExitDo}
            ${EndIf}
          ${Else}
            ${ExitDo}
          ${EndIf}
        ${Loop}
        ${If} $5 != 0
          ; Padding
          IntOp $8 4 - $6
          !insertmacro CPIO_Padding $3 $8
          !insertmacro CPIO_Run_Filter $R0 $1 $3 $8 $5
        ${EndIf}
        ${If} $5 != 0
          StrCpy $0 0
        ${Else}
          StrCpy $0 ${ERROR_WRITE_FAULT}
        ${EndIf}
        FileClose $2
      ${EndIf}
      System::Free $3
    ${Else}
      StrCpy $0 ${ERROR_OUTOFMEMORY}
    ${EndIf}
  ${EndIf}

  Push $0
  System::Store 'L'
FunctionEnd
!macroend ; CPIO_Write_Filter

!macro CPIO_Write UN
; Write CPIO entry of given file name to file handle
; Parameters:
;   filehandle - writable file handle
;   filename - name of file to be added
; Return value:
;   0 if successful otherwise error code
${CPIO_FUNCINC} "${UN}" Filter
${CPIO_FUNCINC} "${UN}" Write_Filter
${CPIO_FUNCPROLOG} "${UN}" Write
   Push $0
   GetFunctionAddress $0 `${UN}${CPIO_PREFIX}Filter`
   Exch $0
   Call `${UN}${CPIO_PREFIX}Write_Filter`
FunctionEnd
!macroend ; CPIO_Write

!endif ; CPIO_INCLUDED
