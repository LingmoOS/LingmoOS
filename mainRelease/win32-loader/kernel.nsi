; Debian-Installer Loader - Kernel choice
; 
; Copyright (C) 2007,2008,2009  Robert Millan <rmh@aybabtu.com>
; Copyright (C) 2010,2011       Didier Raboud <odyx@debian.org>
;
; This program is free software: you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation, either version 3 of the License, or
; (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program.  If not, see <http://www.gnu.org/licenses/>.


!define KERNEL_LINUX 1
!define KERNEL_KFREEBSD 2
!define KERNEL_HURD 3

Function ShowKernel
  Var /GLOBAL kernel 
!ifdef NOCD
 !ifdef ALLKERNELS
  ${If} $expert == true
  ${AndIf} $windows_boot_method != direct ; loadlin can only load linux
    ${Choice_Present} kernel 4
    ${Choice_Get} $kernel

    ${If} $kernel == ${KERNEL_HURD}
      StrCpy $kernel hurd
      # hurd doesn't exist in other architectures other than i386
      StrCpy $arch i386
    ${ElseIf} $kernel == 0
      StrCpy $kernel ${KERNEL_LINUX}
    ${Endif}
  ${Else}
    ; ** Default to GNU/Linux
    StrCpy $kernel ${KERNEL_LINUX}
  ${Endif}
 !else ; ALLKERNELS
    StrCpy $kernel ${KERNEL_LINUX}
 !endif ; ALLKERNELS
!else ; NOCD
  ; When on CD, the win32-loader.ini contains a kernel={linux,kfreebsd} in the [installer] section
  ReadINIStr $kernel $d\win32-loader.ini "installer" "kernel"
  ; If there is nothing, default to linux
  ${If} $kernel == "kfreebsd"
    StrCpy $kernel ${KERNEL_KFREEBSD}
  ${ElseIf} $kernel == "hurd"
    StrCpy $kernel ${KERNEL_HURD}
  ${Else}
    StrCpy $kernel ${KERNEL_LINUX}
  ${EndIf}
!endif ; NOCD
  Var /GLOBAL kernel_name
  ${If} $kernel == ${KERNEL_KFREEBSD}
    StrCpy $kernel_name "GNU/kFreeBSD"
  ${ElseIf} $kernel == ${KERNEL_LINUX}
    StrCpy $kernel_name "GNU/Linux"
  ${ElseIf} $kernel == ${KERNEL_HURD}
    StrCpy $kernel_name "GNU/Hurd"
    # hurd doesn't exist in other architectures other than i386
    StrCpy $arch i386
  ${EndIf}
FunctionEnd
