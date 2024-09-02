; Debian-Installer Loader
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

; SetCompressor must _always_ be the first command
SetCompressor /SOLID lzma

!ifndef BUILD_DIR
!define BUILD_DIR "."
!endif

!define PRESEED_CFG "preseed.cfg"

; Needed to disable idiotic compatibility mode where Vista identifies itself
; as XP.  Otherwise useless (admin privilege is the default for installers).
RequestExecutionLevel admin

!include MUI2.nsh
!include FileFunc.nsh
!include WinVer.nsh
!include x64.nsh
!insertmacro GetRoot

!define CPUID_HELPER_DIR "${BUILD_DIR}/helpers/cpuid"
!define MINIZ_HELPER_DIR "${BUILD_DIR}/helpers/miniz"

!include include\bootcfg.nsh
!include include\choice.nsh
!include include\cpuid.nsh
!include include\miniz.nsh
!include include\security.nsh
!include include\sysinfo.nsh
!include include\wmiquery.nsh

;--------------------------------
; Pages

!define MUI_CUSTOMFUNCTION_ABORT Cancelled
Page custom ShowExpert ; expert.nsi
Page custom ShowRescue ; rescue.nsi
Page custom ShowKernel ; kernel.nsi
Page custom ShowGraphics ; graphics.nsi
!ifdef NOCD
Page custom ShowBranch ; download.nsi and branch.nsi
!endif
Page custom ShowCustom LeaveCustom ; custom.nsi
!define MUI_PAGE_CUSTOMFUNCTION_PRE Preparation
!insertmacro MUI_PAGE_INSTFILES

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!include ${BUILD_DIR}/l10n/templates/all.nsh

!include branding.nsi

;--------------------------------
Var /GLOBAL cancel
Var /GLOBAL c
!ifndef NOCD
Var /GLOBAL d
!endif
Var /GLOBAL preseed_cmdline
Var /GLOBAL proxy
Var /GLOBAL arch
Var /GLOBAL rescue
Var /GLOBAL target_distro

;--------------------------------
; Installer
!include onInit.nsi

; Include functions after .onInit and un.onInit functions allowing
; expansion of macros to occur first in these functions
${BOOTCFG_AppendBootEntry}
${BOOTCFG_AppendFwBootEntry}
${BOOTCFG_ConnectWMI}
${BOOTCFG_CreateGUID}
${BOOTCFG_CreateObject}
${BOOTCFG_GetElementFromBcd}
${BOOTCFG_GetESPVolume}
${BOOTCFG_GetFileSystemUUID}
${BOOTCFG_GetVolumeUUID}
${BOOTCFG_GetObject}
${BOOTCFG_GetObjectPropertyValue}
${BOOTCFG_GetObjectOfVariant}
${BOOTCFG_GetStringOfVariant}
${BOOTCFG_IsUEFI}
${BOOTCFG_SetDescription}
${BOOTCFG_OpenDefaultBcdStore}
${BOOTCFG_SetActiveFwBootEntry}
${BOOTCFG_SetPartition}
${BOOTCFG_SetPath}
${BOOTCFG_ReleaseObject}
${Choice_Present}
${Choice_OnClick}
${Choice_Set}
${Choice_Get}
${CPUID_DataWidth}
${MINIZ_CPIO_GZ_Close}
${MINIZ_CPIO_GZ_Open}
${MINIZ_CPIO_GZ_Write}
${SYSINFO_Domain}
${SYSINFO_HostName}
${SYSINFO_KeyboardLayout}
${SYSINFO_UserName}
${WMIQUERY_SingleProp}

!include expert.nsi
!include rescue.nsi
!include kernel.nsi
!include graphics.nsi
!ifdef NOCD
	!include download.nsi
	!include branch.nsi
!endif
!include custom.nsi
!include s_install.nsi
!include instSuccess.nsi
;--------------------------------
; Uninstaller
!include s_uninstall.nsi
