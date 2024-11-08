; Debian-Installer Loader - Graphical installer choice
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


Function ShowGraphics
  System::Store 'S'
  Var /GLOBAL user_interface
  Var /GLOBAL gtk

!ifndef NOCD
  Var /GLOBAL predefined_user_interface
  ReadINIStr $predefined_user_interface $d\win32-loader.ini "installer" "user_interface"

  ${If} $predefined_user_interface == ""
!endif
    ${If} $expert == true
      nsDialogs::Create ${IDC_CHILDRECT}
      Pop $0
      ${If} $0 != error
        !insertmacro MUI_HEADER_TEXT $(^NameDA) $(graphics1)
        StrCpy $2 8
        IntOp $6 ${CHOICE_INNER_DIALOG_HEIGHT} / 2
        ; Graphical installation option
        IntOp $4 $6 - $2
        StrCpy $5 "$PLUGINSDIR\gtk.bmp"
        IntOp $3 $4 / 2
        File /oname=$5 templates/gtk.bmp
        ${NSD_CreateBitmap} 10u $2u 75u $4u ""
        Pop $1
        ${NSD_SetImage} $1 $5 $R0
        ${NSD_CreateRadioButton} 90u $3u -93u 9u $(graphics2)
        Pop $1
        nsDialogs::SetUserData $1 1
        ${NSD_OnClick} $1 Choice_OnClick
        ; Select graphical installation by default
        ${Choice_Set} 1
        ${NSD_SetState} $1 1
        ; Text installation option
        IntOp $2 $2 + $6
        IntOp $3 $3 + $6
        StrCpy $5 "$PLUGINSDIR\text.bmp"
        File /oname=$5 templates/text.bmp
        ${NSD_CreateBitmap} 10u $2u 75u $4u ""
        Pop $1
        ${NSD_SetImage} $1 "$5" $R1
        ${NSD_CreateRadioButton} 90u $3u -93u 9u $(graphics3)
        Pop $1
        nsDialogs::SetUserData $1 2
        ${NSD_OnClick} $1 Choice_OnClick
        nsDialogs::Show
        ; Release previously loaded image handles
        ${NSD_FreeImage} $R1
        ${NSD_FreeImage} $R0
        ${Choice_Get} $0
        ${If} $0 == "1"
          StrCpy $user_interface graphical
        ${EndIf}
      ${EndIf}
    ${Else}
      ; ** Default to graphical interface for all kernels
      StrCpy $user_interface graphical
    ${Endif}
!ifndef NOCD
  ${Else}
    StrCpy $user_interface $predefined_user_interface
  ${Endif}
!endif

  ${If} $user_interface == "graphical"
    StrCpy $gtk "gtk/"
  ${Endif}

  ; ********************************************** Preseed vga mode
  ${If} $kernel == "linux"
    ; See debian-installer/build/config/{i386,amd64}.cfg
    StrCpy $preseed_cmdline "$preseed_cmdline vga=788"
  ${ElseIf} $kernel == "hurd"
    ${If} "$user_interface" != "graphical"
      StrCpy $preseed_cmdline "$preseed_cmdline GTK_NOVESA=1"
    ${Endif}
  ${Endif}
  System::Store 'L'
FunctionEnd
