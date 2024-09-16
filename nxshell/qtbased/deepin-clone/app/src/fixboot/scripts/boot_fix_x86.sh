#!/bin/bash

# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-only

error () {
    echo $1
    exit $2
}

config_item() {
  if [ -f /etc/default/grub ]; then
    . /etc/default/grub || return
  fi
  eval echo "\$$1"
}

DI_BOOTLOADER=$1
disk_is_efi=$2
repo_path=$3

if [[ $disk_is_efi == "true" ]]; then
    echo "disk is efi mode"
    disk_is_efi=1
else
    echo "disk is legacy mode"
    disk_is_efi=0
fi

dpkg-query -l grub-pc|egrep -i i\\w\\s+grub-pc

if [[ $? == 0 ]]; then
    echo "system is legacy mode"
    system_is_efi=0
else
    if [[ `uname -m` == "x86_64" ]]; then
        dpkg-query -l grub-efi-amd64-signed|egrep -i i\\w\\s+grub-efi-amd64-signed
    else
        dpkg-query -l grub-efi-ia32|egrep -i i\\w\\s+grub-efi-ia32
    fi

    if [[ $? == 0 ]]; then
        echo "system is efi mode"
        system_is_efi=1
    else
        echo "system not install grub"
        system_is_efi=2
    fi
fi

set -e

#install pageack
if [[ $disk_is_efi != $system_is_efi ]]; then
    cp /etc/apt/sources.list /tmp/sources.list.bak
    set +e
    echo "deb file://${repo_path} unstable main contrib non-free" > /etc/apt/sources.list
    apt-get update
    echo "restore sources.list file"
    mv -f /tmp/sources.list.bak /etc/apt/sources.list
    set -e

    export DEBIAN_FRONTEND="noninteractive"

    if [[ $disk_is_efi == 0 ]]; then
        echo "INFO: Detected legacy machine, installing grub to ${DI_BOOTLOADER}"
        apt-get -y -o Dpkg::Options::="--force-confdef" \
            -o Dpkg::Options::="--force-confold" --no-install-recommends \
            --allow-unauthenticated install grub-pc
    else
        if [[ `uname -m` == "x86_64" ]]; then
            # Clover efi loader cannot use grub.efi correctly,
            # so we may patch grub or use grub.efi.signed.
            apt-get install --no-install-recommends -y --allow-unauthenticated \
                shim-signed grub-efi-amd64-signed efibootmgr
        else
            apt-get install --no-install-recommends -y --allow-unauthenticated \
                grub-efi-ia32
        fi
    fi
fi

echo "install grub"
if [[ $disk_is_efi == 0 ]]; then
    set +e
    grub-install --no-floppy ${DI_BOOTLOADER} --target=i386-pc --force|| \
        error "grub-install failed! ${DI_BOOTLOADER}" 1
    set -e
else
    BOOTLOADER_ID="$(config_item GRUB_DISTRIBUTOR | tr A-Z a-z | cut -d' ' -f1)"
    BOOTLOADER_ID="${BOOTLOADER_ID:-deepin}"

    if [[ `uname -m` == "x86_64" ]]; then
        # uefi-secure-boot options is enabled by default
        set +e
        grub-install --target=x86_64-efi --uefi-secure-boot \
            --efi-directory=/boot/efi --bootloader-id="${BOOTLOADER_ID}" \
            --recheck || error "grub-install failed with efi64 ${BOOTLOADER_ID}" 1
        set -e

        # Copy signed grub efi file.
        [ -d /boot/efi/EFI/ubuntu ] || mkdir -p /boot/efi/EFI/ubuntu
        cp /boot/efi/EFI/${BOOTLOADER_ID}/grub* /boot/efi/EFI/ubuntu/
        [ -d /boot/efi/EFI/boot ] || mkdir -p /boot/efi/EFI/boot
        cp -f /boot/efi/EFI/${BOOTLOADER_ID}/grub* /boot/efi/EFI/boot/

        # Backup fallback efi first.
        fallback_efi=/boot/efi/EFI/boot/bootx64.efi
        fallback_efi_bak="${fallback_efi}-$(date +%s).bak"
        [ -f "${fallback_efi}" ] && cp "${fallback_efi}" "${fallback_efi_bak}"
        # Override fallback efi with shim.
        cp /boot/efi/EFI/${BOOTLOADER_ID}/shim*.efi "${fallback_efi}"
    else
        set +e
        grub-install --target=i386-efi --efi-directory=/boot/efi \
            --bootloader-id="${BOOTLOADER_ID}" --recheck || \
            error "grub-install failed with efi! ${BOOTLOADER_ID}" 1
        set -e

        # Copy signed grub efi file.
        [ -d /boot/efi/EFI/ubuntu ] || mkdir -p /boot/efi/EFI/ubuntu
        cp /boot/efi/EFI/${BOOTLOADER_ID}/grub* /boot/efi/EFI/ubuntu/
        [ -d /boot/efi/EFI/boot ] || mkdir -p /boot/efi/EFI/boot
        cp -f /boot/efi/EFI/${BOOTLOADER_ID}/grub* /boot/efi/EFI/boot/
        # Backup fallback efi first.
        fallback_efi=/boot/efi/EFI/boot/bootia32.efi
        fallback_efi_bak="${fallback_efi}-$(date +%s).bak"
        [ -f "${fallback_efi}" ] && cp "${fallback_efi}" "${fallback_efi_bak}"
        # Override fallback efi with shim.
        cp /boot/efi/EFI/${BOOTLOADER_ID}/shim*.efi "${fallback_efi}"
    fi
fi

echo "update grub"
set +e
update-grub || error "Failed update-grub" 2
set -e

exit 0
