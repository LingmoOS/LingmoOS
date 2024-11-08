; Debian-Installer Loader - Rescue mode selection
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


Function ShowRescue
  StrCpy $rescue false
!ifdef PXE
  ${If} $pxe_mode == "false"
!endif ;PXE
    ${Choice_Present} rescue 3
    ${Choice_Get} $0

    ${If} $0 == "2"
      StrCpy $rescue true
    ${Endif}
!ifdef PXE
  ${Endif} ; $pxe_mode == "false"
!endif ;PXE
FunctionEnd
