; Debian-Installer Loader - Installation
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

!define EFI_FOLDER "EFI\debian"
!define EFI_LOADER "shimx64.efi"
!define GRUB2_CFG_FILE "grub.cfg"
!define GRUB2_EFI_FILE "grubx64.efi"
!define GRUB2_PREFIX_DI "debian-installer\amd64\grub"
!ifdef NOCD
!define SHIM_EFI_FILE "bootnetx64.efi"
!else
!define CD_EFI_FOLDER "EFI\boot"
!define SHIM_EFI_FILE "bootx64.efi"
!endif

!include StrFunc.nsh
!ifndef StrRep_INCLUDED
${StrRep}
!endif

; Create boot entry and add it to display list of boot manager
; Parameter:
;   guid - global unique identifier of boot entry
;   label - brief description of boot entry
Function AddBootEntry
  System::Store 'S'
  Pop $3
  Pop $2

  System::Call 'kernel32::QueryDosDevice(t "$c", t .r4, \
    i${NSIS_MAX_STRLEN}) i.r0'

  ; guid=$2, label=$3, systempartition=$4
  ${If} $2 != ""
  ${AndIf} $0 != 0
    ${BOOTCFG_ConnectWMI} $R0 $R1 $0
    ${If} $0 == 0
      ${BOOTCFG_GetObject} $R1 "BcdStore" $R2 $0
      ${If} $0 == 0
        ${BOOTCFG_GetObject} $R1 "BcdObject" $R3 $0
        ${If} $0 == 0
          ${BOOTCFG_OpenDefaultBcdStore} $R1 $R2 $R4 $0
          ${If} $0 == 0
            ${BOOTCFG_CreateObject} $R1 $R2 $R4 \
              ${BOOTCFG_BOOT_SECTOR} $2 $5 $0
            ${If} $0 == 0
              ; Associate data with boot sector object
              ${BOOTCFG_SetDescription} $R1 $R3 $5 "$3" $1 $0
              ${If} $0 == 0
                ${BOOTCFG_SetPartition} $R1 $R3 $5 $4 $1 $0
                ${If} $0 == 0
                  StrLen $0 $c
                  StrCpy $0 "$INSTDIR" "" $0
                  ; $0=Directory without drive letter
                  ${BOOTCFG_SetPath} $R1 $R3 $5 "$0\g2ldr.mbr" $1 $0
                ${EndIf}
              ${EndIf}
              ; Release boot sector object and its associated data
              ${BOOTCFG_ReleaseObject} $5
              ${If} $0 == 0
                ; Append boot entry to list
                ${BOOTCFG_AppendBootEntry} $R1 $R2 \
                  $R4 $R3 $2 $1 $0
                ${If} $0 == 0
                  ; Release result
                  ${BOOTCFG_ReleaseObject} $1
                ${EndIf}
              ${EndIf}
            ${EndIf}
            ${BOOTCFG_ReleaseObject} $R4
          ${EndIf}
          ${BOOTCFG_ReleaseObject} $R3
        ${EndIf}
        ${BOOTCFG_ReleaseObject} $R2
      ${EndIf}
      ${BOOTCFG_ReleaseObject} $R1
      ${BOOTCFG_ReleaseObject} $R0
    ${EndIf}
  ${EndIf}

  ${If} $0 != 0
    IntFmt $2 "0x%08X" $0
    DetailPrint "$1: $2"
  ${EndIf}

  System::Store 'L'
FunctionEnd

; Write GUID to the registry
; Parameter:
;   guid - GUID of boot entry
Function WriteRegGUID
  Exch $0
  WriteRegStr HKLM "${REGSTR_WIN32}" "bootmgr" "$0"
  Pop $0
FunctionEnd

Function CreateBootEntry
  ReadRegStr $0 HKLM "${REGSTR_WIN32}" "bootmgr"
  ${If} $0 == ""
    ${BOOTCFG_CreateGUID} $2
    ${If} $2 != ""
      Push "$2"
!ifdef PXE
      ${If} $pxe_mode == "true"
        Push "$(pxe)"
      ${Else}
!endif ; PXE
        Push "$(d-i)"
!ifdef PXE
      ${EndIf} ; $pxe_mode == "true"
!endif ; PXE
      Call AddBootEntry
      Push $2
      Call WriteRegGUID
    ${Else}
      MessageBox MB_OK|MB_ICONSTOP "$(error_bcdedit_extract_id)"
      Quit
    ${EndIf}
  ${Endif}
FunctionEnd

; ********************************************** grub.cfg
; Generate grub.cfg file
; Return value:
;   file handle of grub.cfg file
Function GenerateGrubConfig
  Push $0
  StrCpy $0 "$INSTDIR\${GRUB2_CFG_FILE}"
  DetailPrint "$(generating)"
  ClearErrors
  FileOpen $0 "$0" w
  ${IfNot} ${Errors}
    FileWrite $0 "set gfxpayload=auto$\n"
  ${Else}
    DetailPrint "$(error) ${GRUB2_CFG_FILE}"
    StrCpy $0 ""
  ${EndIf}
  Exch $0
FunctionEnd

; Write grub.cfg file for Linux kernel
Function WriteGrubConfigLinux
  Call GenerateGrubConfig
  Exch $0
  ${If} $0 != ""
    FileWrite $0 "\
linux	$$prefix/linux $preseed_cmdline$\n\
initrd	$$prefix/initrd.gz$\n\
boot$\n"
    FileClose $0
  ${EndIf}
  Pop $0
FunctionEnd

; Write grub.cfg file for kFreeBSD kernel
Function WriteGrubConfigkFreeBSD
  Call GenerateGrubConfig
  Exch $0
  ${If} $0 != ""
    FileWrite $0 "\
kfreebsd	$$prefix/kfreebsd.gz$\n\
kfreebsd_module	$$prefix/initrd.gz type=mfs_root$\n\
set kFreeBSD.vfs.root.mountfrom=ufs:/dev/md0$\n\
set kFreeBSD.hw.ata.ata_dma=0   # needed for qemu hard disk # TODO: delete$\n\
set kFreeBSD.hw.ata.atapi_dma=0 # needed for qemu cd # TODO: 1$\n\
boot$\n"
    FileClose $0
  ${EndIf}
  Pop $0
FunctionEnd

; Write grub.cfg file for Hurd kernel
Function WriteGrubConfigHurd
  Call GenerateGrubConfig
  Exch $0
  ${If} $0 != ""
    FileWrite $0 "\
multiboot		$$prefix/gnumach.gz root=gunzip:device:rd0 $preseed_cmdline$\n\
module --nounzip	$$prefix/initrd.gz initrd '$$(ramdisk-create)'$\n\
module 			$$prefix/ext2fs.static ext2fs \$\n\
			--multiboot-command-line='$${kernel-command-line}' \$\n\
			--host-priv-port='$${host-port}' \$\n\
			--device-master-port='$${device-port}' \$\n\
			--exec-server-task='$${exec-task}' -T typed '$${root}' \$\n\
			'$$(task-create)' '$$(task-resume)'$\n\
module			$$prefix/ld.so.1 exec /hurd/exec '$$(exec-task=task-create)'$\n\
boot$\n"
    FileClose $0
  ${EndIf}
  Pop $0
FunctionEnd

!ifdef PXE
Function WriteGrubConfigPXE
  Call GenerateGrubConfig
  Exch $0
  ${If} $0 != ""
    FileWrite $0 "\
linux16 $$prefix/pxe.lkrn$\n\
boot$\n"
    FileClose $0
  ${EndIf}
  Pop $0
FunctionEnd
!endif ; PXE

; Adding the preseed file to the installer's initrd.gz
Function PreSeedLinux
  System::Store 'S'
  StrCpy $1 ${PRESEED_CFG}
  StrCpy $2 "initrd.gz"

  StrCpy $0 "$INSTDIR\$2"
  DetailPrint "$(appending_preseeding)"

  ; IMPORTANT!!  All accessed files must be in the same
  ; filesystem as the script itself, because cmd.exe/command.com gets
  ; confused when using absolute paths.  This is why $INSTDIR is used
  ; instead of $PLUGINSDIR.

  StrCpy $0 "$1"
  ${If} ${FileExists} $1
    StrCpy $0 "$2"
    ClearErrors
    FileOpen $3 "$2" a
    ${IfNot} ${Errors}
      FileSeek $3 0 END
      ${MINIZ_CPIO_GZ_Open} $3 $4
      ${If} $4 P<> 0
        ${MINIZ_CPIO_GZ_Write} $4 $1 $0
        ${If} $0 == 0
          ${MINIZ_CPIO_GZ_Write} $4 "" $0
        ${EndIf}
        ${MINIZ_CPIO_GZ_Close} $4 $4
        ${If} $0 != 0
        ${OrIf} $4 != 0
          StrCpy $0 "$2"
        ${Else}
          ; Indicate success
          StrCpy $0 ""
        ${EndIf}
      ${EndIf}
      FileClose $3
    ${EndIf}
  ${EndIf}

  ${If} $0 != ""
    MessageBox MB_OK|MB_ICONSTOP "$(^ErrorCreating)$0"
    Quit
  ${EndIf}
  System::Store 'L'
FunctionEnd

; ******************************************************************************
; ***************************************** THIS IS WHERE THE REAL ACTION STARTS
; ******************************************************************************
Section "Preparation" sec_prep
  ; We're about to write down our preseed line.  This would be a nice place
  ; to add post-install parameters.
  StrCpy $preseed_cmdline "$preseed_cmdline ---"

  ; ********************************************** preseed quietness
  ${If} $expert == false
    StrCpy $preseed_cmdline "$preseed_cmdline quiet"
  ${Endif}

  ; Up to this point, we haven't modified host system.  The first modification
  ; we want to do is preparing the Uninstaller (so that in case something went
  ; wrong, our half-install can be undone).
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  WriteRegStr HKLM "${REGSTR_UNINST}" "DisplayName" $(program_name)
  WriteRegStr HKLM "${REGSTR_UNINST}" "UninstallString" "$INSTDIR\Uninstall.exe"
  ; Add more branding (see branding.nsi)
  WriteRegStr HKLM "${REGSTR_UNINST}" "Publisher" "${_COMPANY_NAME}"
  WriteRegStr HKLM "${REGSTR_UNINST}" "DisplayIcon" "$INSTDIR\Uninstall.exe"
  WriteRegStr HKLM "${REGSTR_UNINST}" "NoModify" "1"
  WriteRegStr HKLM "${REGSTR_UNINST}" "NoRepair" "1"
  WriteRegStr HKLM "${REGSTR_UNINST}" "DisplayVersion" "${VERSION_OPT}"
  WriteRegStr HKLM "${REGSTR_UNINST}" "ProductID" "${4DIGITS_DATE}"
SectionEnd

!ifndef NOCD
!macro CopyFile SOURCE DEST
  StrCpy $0 "$EXEDIR\${SOURCE}"
  StrCpy $1 "$INSTDIR\${DEST}"
  CopyFiles /FILESONLY "$0" "$1"
  ${If} ${Errors}
    MessageBox MB_OK|MB_ICONSTOP "$(error_copyfiles)"
    Quit
  ${EndIf}
!macroend

Function CopyGrub2Bootloader
  Push $0
  Push $1
  ClearErrors
  !insertmacro CopyFile "$g2ldr_mbr" g2ldr_mbr
  StrCpy $0 "$EXEDIR\$g2ldr"
  StrCpy $1 "$c\g2ldr"
  CopyFiles /FILESONLY "$0" "$1"
  ${If} ${Errors}
    MessageBox MB_OK|MB_ICONSTOP "$(error_copyfiles)"
    Quit
  ${EndIf}
  Pop $1
  Pop $0
FunctionEnd

Section /o "Linux" sec_linux
  ClearErrors
  !insertmacro CopyFile "$linux" linux
  !insertmacro CopyFile "$initrd" initrd.gz
  Call WriteGrubConfig${__SECTION__}
  Call PreSeed${__SECTION__}
SectionEnd

Section /o "kFreeBSD" sec_kfreebsd
  ClearErrors
  !insertmacro CopyFile "$kfreebsd" kfreebsd.gz
  !insertmacro CopyFile "$kfreebsd_module" initrd.gz
  Call WriteGrubConfig${__SECTION__}
SectionEnd

Section /o "Hurd" sec_hurd
  ClearErrors
  !insertmacro CopyFile "$gnumach" gnumach.gz
  !insertmacro CopyFile "$ext2fs" ext2fs.static
  !insertmacro CopyFile "$initrd" initrd.gz
  !insertmacro CopyFile "$ld" ld.so.1
  Call WriteGrubConfig${__SECTION__}
SectionEnd

Section /o "GRUB 2 EFI" sec_grub2_efi
  ${If} $arch == "amd64"
    ClearErrors
    !insertmacro CopyFile "${CD_EFI_FOLDER}\${GRUB2_EFI_FILE}" "${GRUB2_EFI_FILE}"
    !insertmacro CopyFile "${CD_EFI_FOLDER}\${SHIM_EFI_FILE}" "${SHIM_EFI_FILE}"
  ${EndIf}
SectionEnd

Section /o "GRUB 2 boot loader for bootmgr" sec_g2ldr_bootmgr
  Call CopyGrub2Bootloader
SectionEnd

Section /o "GRUB 2 boot loader for NTLDR" sec_g2ldr_ntldr
  Call CopyGrub2Bootloader
SectionEnd

Section /o "Linux boot loader loadlin" sec_loadlin
  File "${BUILD_DIR}/loadlin.exe"
  File "${BUILD_DIR}/loadlin.pif"
SectionEnd
!else ;NOCD

Function ExtractGrub2Bootloader
  File /oname=$c\g2ldr "${BUILD_DIR}/g2ldr"
  File "${BUILD_DIR}/g2ldr.mbr"
FunctionEnd

Section "Download" sec_download
!ifdef PXE
  ${If} $pxe_mode != "true"
!endif
    ; Enable cancel button and make download controls visible
    EnableWindow $mui.Button.Cancel 1
    Push 1
    Call Download_ShowControls
    ${If} $base_path_hashes != ""
      ; Remove trailing / from $base_url
      StrCpy $1 "$base_url" -1
      ; Download the Release and Release.gpg files
      Push "false"
      Push "false"
      Push "Release"
      Push "$PLUGINSDIR"
      Push "$1"
      Call Download
      Pop $0
      Push "false"
      Push "false"
      Push "Release.gpg"
      Push "$PLUGINSDIR"
      Push "$1"
      Call Download
      Pop $0

      EnableWindow $mui.Button.Cancel 0
      ; Now check gpg validity of this
      File /oname=$PLUGINSDIR\gpgv.exe /usr/share/win32/gpgv.exe
      File /oname=$PLUGINSDIR\debian-archive-keyring.gpg /usr/share/keyrings/debian-archive-keyring.gpg
      File /oname=$PLUGINSDIR\debian-archive-removed-keys.gpg /usr/share/keyrings/debian-archive-removed-keys.gpg
      StrCpy $0 "Release"
      DetailPrint $(gpg_checking_release)
      Push "$OUTDIR"
      SetOutPath "$PLUGINSDIR"
      nsExec::Exec 'gpgv.exe --keyring ".\debian-archive-removed-keys.gpg" --keyring ".\debian-archive-keyring.gpg" Release.gpg Release'
      Pop $0
      ${If} $0 != 0
        StrCpy $0 "Release"
        MessageBox MB_OK|MB_ICONSTOP "$(unsecure_release)"
        Quit
      ${EndIf}
      Pop $0
      SetOutPath "$0"
      EnableWindow $mui.Button.Cancel 1

      ; Get SHA256SUMS file
      Push "$PLUGINSDIR\Release"
      Push "$base_path_hashesSHA256SUMS"
      Call Get_SHA256_ref
      ; SHA-256 is on the stack.
    ${Else}
      Push "false" ; Don't try to compare the SHA256SUM (daily images don't have that).
    ${EndIf}

    ; Download the SHA256SUMS file
    ; SHA-256 or "false" is on the stack
    Push "false"
    Push "SHA256SUMS"
    Push "$PLUGINSDIR"
    Push "$base_url$base_path_hashes"
    Call Download
    Pop $0
!ifdef PXE
  ${EndIf}
!endif
SectionEnd

!ifdef NETWORK_BASE_URL
; ********************************************** if ${NETWORK_BASE_URL} provides a preseed.cfg, use it
Section "Custom preseeding" sec_preseeding
  Push ${NETWORK_BASE_URL_CHECKSUM}
  Push true
  Push "preseed.cfg"
  Push "$PLUGINSDIR"
  Push "${NETWORK_BASE_URL}"
  Call Download
  Pop $0
  ${If} $0 == "success"
    StrCpy $preseed_cfg "$preseed_cfg$\n$\n"
    ClearErrors
    FileOpen $0 $PLUGINSDIR\preseed.cfg r
    ${Do}
      FileRead $0 $1
      ${If} ${Errors}
        ${ExitDo}
      ${EndIf}
      StrCpy $preseed_cfg "\
$preseed_cfg\
$1"
    ${Loop}
    FileClose $0
    StrCpy $preseed_cfg "$preseed_cfg$\n"
  ${Endif}
SectionEnd
!endif ; NETWORK_BASE_URL

Section /o "Linux" sec_linux
  Push "$PLUGINSDIR\SHA256SUMS"
  Push "$base_path_images/linux"
  Call Get_SHA256_ref
  ; SHA256 is on stack
  Push "false"
  Push "linux"
  Push "$INSTDIR"
  Push "$base_url$base_path_hashes$base_path_images"
  Call Download
  Pop $0

  Push "$PLUGINSDIR\SHA256SUMS"
  Push "$base_path_images/initrd.gz"
  Call Get_SHA256_ref
  ; SHA256 is on stack
  Push "false"
  Push "initrd.gz"
  Push "$INSTDIR"
  Push "$base_url$base_path_hashes$base_path_images"
  Call Download
  Pop $0
  Call WriteGrubConfig${__SECTION__}
  Call PreSeed${__SECTION__}
SectionEnd

Section /o "kFreeBSD" sec_kfreebsd
  Push "$PLUGINSDIR\SHA256SUMS"
  Push "$base_path_images/kfreebsd.gz"
  Call Get_SHA256_ref
  ; SHA256 is on stack
  Push "false"
  Push "kfreebsd.gz"
  Push "$INSTDIR"
  Push "$base_url$base_path_hashes$base_path_images"
  Call Download
  Pop $0

  Push "$PLUGINSDIR\SHA256SUMS"
  Push "$base_path_images/initrd.gz"
  Call Get_SHA256_ref
  ; SHA256 is on stack
  Push "false"
  Push "initrd.gz"
  Push "$INSTDIR"
  Push "$base_url$base_path_hashes$base_path_images"
  Call Download
  Pop $0
  Call WriteGrubConfig${__SECTION__}
SectionEnd

Section /o "Hurd" sec_hurd
  Push "$PLUGINSDIR\SHA256SUMS"
  Push "$base_path_images/gnumach.gz"
  Call Get_SHA256_ref
  ; SHA256 is on stack
  Push "false"
  Push "gnumach.gz"
  Push "$INSTDIR"
  Push "$base_url$base_path_hashes$base_path_images"
  Call Download
  Pop $0

  Push "$PLUGINSDIR\SHA256SUMS"
  Push "$base_path_images/ext2fs.static"
  Call Get_SHA256_ref
  ; SHA256 is on stack
  Push "false"
  Push "ext2fs.static"
  Push "$INSTDIR"
  Push "$base_url$base_path_hashes$base_path_images"
  Call Download
  Pop $0

  Push "$PLUGINSDIR\SHA256SUMS"
  Push "$base_path_images/initrd.gz"
  Call Get_SHA256_ref
  ; SHA256 is on stack
  Push "false"
  Push "initrd.gz"
  Push "$INSTDIR"
  Push "$base_url$base_path_hashes$base_path_images"
  Call Download
  Pop $0

  Push "$PLUGINSDIR\SHA256SUMS"
  Push "$base_path_images/ld.so.1"
  Call Get_SHA256_ref
  ; SHA256 is on stack
  Push "false"
  Push "ld.so.1"
  Push "$INSTDIR"
  Push "$base_url$base_path_hashes$base_path_images"
  Call Download
  Pop $0
  Call WriteGrubConfig${__SECTION__}
SectionEnd
!endif

!ifdef PXE
Section /o "PXE" sec_pxe
  File /oname=$INSTDIR\pxe.lkrn "${BUILD_DIR}/pxe.lkrn"
  Call WriteGrubConfig${__SECTION__}
SectionEnd
!endif

!ifdef NOCD
Section /o "GRUB 2 EFI" sec_grub2_efi
  ${If} $arch == "amd64"
; TODO: Remove work around
; Use signed grub efi file of testing instead of stable branch (buster)
;
; https://deb.debian.org/debian/dists/buster/main/installer-amd64/
; current/images/netboot/gtk/debian-installer/amd64/grubx64.efi
; sets the prefix to ($root)debian-installer/amd64/grub
; This prefix lacks the leading /.
;
; ---
    StrCpy $0 $base_url
    StrLen $1 $0
    ${If} $1 > 0
      ; Skip trailing /
      IntOp $1 $1 - 1
      ${DoWhile} $1 > 0
        IntOp $1 $1 - 1
        StrCpy $2 $0 1 $1
        ${If} $2 == '/'
          IntOp $1 $1 + 1
          StrCpy $2 $0 -1 $1
          StrCpy $1 $0 $1
          ${ExitDo}
        ${EndIf}
      ${Loop}
    ${EndIf}
    Push "$PLUGINSDIR\SHA256SUMS"
    Push "$base_path_images/${GRUB2_EFI_FILE}"
    Call Get_SHA256_ref
    Pop $0
    ${If} $2 == "stable"
    ${AndIf} $0 == "bef7969d0425f89417ab0dd90542a1bace4ebc592ec4ff2d8641795bc8d06001"
      StrCpy $1 "$1testing/"
      Push "false"
      Push "false"
      Push "${GRUB2_EFI_FILE}"
      Push "$INSTDIR"
      Push "$1$base_path_hashes$base_path_images"
      Call Download
      Pop $0
    ${Else}
      Push $0 ; SHA256 fingerprint
      Push "false"
      Push "${GRUB2_EFI_FILE}"
      Push "$INSTDIR"
      Push "$base_url$base_path_hashes$base_path_images"
      Call Download
      Pop $0
    ${EndIf}
; ---
    Push "$PLUGINSDIR\SHA256SUMS"
    Push "$base_path_images/${SHIM_EFI_FILE}"
    Call Get_SHA256_ref
    ; SHA256 is on stack
    Push "false"
    Push "${SHIM_EFI_FILE}"
    Push "$INSTDIR"
    Push "$base_url$base_path_hashes$base_path_images"
    Call Download
    Pop $0
  ${EndIf}
SectionEnd

Section /o "GRUB 2 boot loader for bootmgr" sec_g2ldr_bootmgr
  Call ExtractGrub2Bootloader
SectionEnd

Section /o "GRUB 2 boot loader for NTLDR" sec_g2ldr_ntldr
  Call ExtractGrub2Bootloader
SectionEnd

Section /o "Linux boot loader loadlin" sec_loadlin
  File "${BUILD_DIR}/loadlin.exe"
  File "${BUILD_DIR}/loadlin.pif"
SectionEnd

Section "Download completed" sec_download_done
  ; Hide download controls and disable cancel button
  Push 0
  Call Download_ShowControls
  EnableWindow $mui.Button.Cancel 0
SectionEnd
!endif

; ********************************************** Do bootloader last, because it's the most dangerous
Section /o "Firmware Boot Manager" sec_fwbootmgr
  System::Store 'S'
  DetailPrint "${__SECTION__}"
!ifdef PXE
  ${If} $pxe_mode == "true"
    DetailPrint "$(^Skipped)$(pxe)"
    Return
  ${EndIf}
!endif
  ${BOOTCFG_ConnectWMI} $R0 $R1 $0
  ${If} $0 == 0
    ${BOOTCFG_GetObject} $R1 "BcdStore" $R2 $0
    ${If} $0 == 0
      ${BOOTCFG_GetObject} $R1 "BcdObject" $R3 $0
      ${If} $0 == 0
        ${BOOTCFG_OpenDefaultBcdStore} $R1 $R2 $R4 $0
        ${If} $0 == 0
          ${BOOTCFG_GetESPVolume} $R1 $R2 $R4 $R3 $1 $0
          ${If} $0 != 0
            DetailPrint "Unable to identify ESP volume: $1 [$0]"
          ${Else}
            DetailPrint "ESP volume: $1"
            ; Save ESP volume
            Push $1
            ${BOOTCFG_GetVolumeUUID} "\\?\GLOBALROOT$1" $1 $0
            ${If} $0 == 0
              ; Terminating backslash is required for SetVolumeMountPoint
              StrCpy $2 "$PLUGINSDIR_esp\"
              ClearErrors
              CreateDirectory $2
              ${If} ${Errors}
                DetailPrint "Unable to create $2 directory"
                StrCpy $0 ${ERROR_ACCESS_DENIED}
              ${Else}
                ; Terminating backslash is required for SetVolumeMountPoint
                StrCpy $1 "\\?\Volume$1\"
                DetailPrint "Volume Name: $1"
                System::Call \
                  "kernel32::SetVolumeMountPoint(t r2, t r1) i.r0 ?e"
                ${If} $0 != 0
                  StrCpy $0 0
                  Exch $0
                ${EndIf}
                Pop $0
                ${If} $0 == 0
                  Push $2
                  StrCpy $2 "$2${EFI_FOLDER}"
                  ClearErrors
                  CreateDirectory $2
                  ${If} ${Errors}
                    DetailPrint \
                      "Unable to create ${EFI_FOLDER} directory"
                    StrCpy $0 ${ERROR_ACCESS_DENIED}
                  ${Else}
                    CopyFiles /FILESONLY "$INSTDIR\${SHIM_EFI_FILE}" "$2\${EFI_LOADER}"
                    CopyFiles /FILESONLY "$INSTDIR\${GRUB2_EFI_FILE}" "$2\${GRUB2_EFI_FILE}"
                    ${BOOTCFG_GetFileSystemUUID} "C:" $3
                    ${If} $3 != ""
!ifdef NOCD
                      Pop $2
                      Push $2
                      StrCpy $2 "$2${GRUB2_PREFIX_DI}"
                      CreateDirectory $2
!endif
                      ClearErrors
                      FileOpen $4 "$2\${GRUB2_CFG_FILE}" w
                      ${IfNot} ${Errors}
                        StrLen $0 $c
                        StrCpy $0 "$INSTDIR" "" $0
                        ${StrRep} $0 "$0" "\" "/"
                        FileWrite $4 "\
search --set --fs-uuid $3$\n\
set prefix=($$root)$0$\n\
normal$\n"
                        FileClose $4
                      ${EndIf}
                    ${EndIf}
                  ${EndIf}
                  Pop $2
                  System::Call "kernel32::DeleteVolumeMountPoint(t r2)"
                ${EndIf}
              ${EndIf}
            ${EndIf}
            ; Restore ESP volume
            Pop $3
            ${BOOTCFG_CreateGUID} $2
            DetailPrint "GUID=$2 Label=$(d-i) Volume=$3"
            ${BOOTCFG_CreateObject} $R1 $R2 $R4 \
              ${BOOTCFG_BOOT_MANAGER} $2 $5 $0
            ${If} $0 == 0
              ; Associate data with boot manager object
              ${BOOTCFG_SetDescription} $R1 $R3 $5 "$(d-i)" $1 $0
              ${If} $0 == 0
                ${BOOTCFG_SetPartition} $R1 $R3 $5 $3 $1 $0
                ${If} $0 == 0
                  ${BOOTCFG_SetPath} $R1 $R3 $5 "\${EFI_FOLDER}\${EFI_LOADER}" $1 $0
                ${EndIf}
              ${EndIf}
              ; Release boot manager object and its associated data
              ${BOOTCFG_ReleaseObject} $5
              ${BOOTCFG_AppendFwBootEntry} $R1 $R2 $R4 $R3 $2 $1 $0
              ${If} $0 == 0
                ${BOOTCFG_ReleaseObject} $1
              ${Else}
                DetailPrint "AppendFwBootEntry: $1 [$0]"
              ${EndIf}
              ${BOOTCFG_SetActiveFwBootEntry} $R1 $R2 $R4 $R3 $2 $1 $0
              ${If} $0 == 0
                ${BOOTCFG_ReleaseObject} $1
              ${Else}
                DetailPrint "SetActiveFwBootEntry: $1 [$0]"
              ${EndIf}
            ${EndIf}
            Push $2
            Call WriteRegGUID
            ${BOOTCFG_ReleaseObject} $R4
          ${EndIf}
          ${BOOTCFG_ReleaseObject} $R3
        ${EndIf}
        ${BOOTCFG_ReleaseObject} $R2
      ${EndIf}
      ${BOOTCFG_ReleaseObject} $R1
      ${BOOTCFG_ReleaseObject} $R0
    ${EndIf}
  ${EndIf}
  System::Store 'L'
SectionEnd

Section /o "Boot Manager" sec_bootmgr
  DetailPrint "$(registering_bootmgr)"
  Call CreateBootEntry
SectionEnd

Section /o "Windows NT Loader" sec_ntldr
  DetailPrint "$(registering_ntldr)"
  SetFileAttributes "$c\boot.ini" NORMAL
  SetFileAttributes "$c\boot.ini" SYSTEM|HIDDEN

  ; Sometimes timeout isn't set.  This may result in ntldr booting straight to
  ; Windows (bad) or straight to Debian-Installer (also bad)!  Force it to 30
  ; just in case. Store its eventual old value alongside
  ; Read the already defined timeout
  ReadIniStr $0 "$c\boot.ini" "boot loader" "timeout"
  ${If} ${Errors}
    ClearErrors
    WriteIniStr "$c\boot.ini" "boot loader" "old_timeout_win32-loader" $0
  ${EndIf}
  WriteIniStr "$c\boot.ini" "boot loader" "timeout" "30"
!ifdef PXE
  ${If} $pxe_mode == "true"
    StrCpy $1 "$(pxe)"
  ${Else}
!endif ; PXE
    StrCpy $1 "$(d-i)"
!ifdef PXE
  ${EndIf} ; $pxe_mode == "true"
!endif ; PXE
  System::Call "kernel32::WideCharToMultiByte(i 850, i 0, w r1, i -1, \
    m.r2, i${NSIS_MAX_STRLEN}, p 0, p 0) i.r0"
  WriteIniStr "$c\boot.ini" "operating systems" "$INSTDIR\g2ldr.mbr" '"$2"'
SectionEnd

Section /o "Direct via loadlin" sec_direct
SectionEnd

; Make preparations for installation
; Note:
; Function has to be placed below defintion of sections
; in order to select sections based on their identifiers.
Function Preparation
!ifdef PXE
  ${If} $pxe_mode != "true"
!endif
    ${If} $kernel != ""
    ${AndIf} $kernel > 0
      Push $0
      IntOp $0 $kernel - 1
      IntOp $0 $0 + ${sec_linux}
      SectionSetFlags $0 ${SF_SELECTED}
      Pop $0
    ${Else}
      SectionSetFlags ${sec_linux} ${SF_SELECTED}
    ${EndIf}
!ifdef PXE
  ${Else}
    SectionSetFlags ${sec_pxe} ${SF_SELECTED}
  ${EndIf}
  ${If} $windows_boot_method != ""
  ${AndIf} $windows_boot_method > 0
    Push $0
    Push $1
    ; Select section to copy respectively download
    ; required files for booting
    IntOp $0 $windows_boot_method - 1
    IntOp $1 $0 + ${sec_grub2_efi}
    SectionSetFlags $1 ${SF_SELECTED}
    ; Select section to configure boot loader
    IntOp $1 $0 + ${sec_fwbootmgr}
    SectionSetFlags $1 ${SF_SELECTED}
    Pop $1
    Pop $0
  ${EndIf}
!endif
FunctionEnd
