#!/bin/bash

set -e

# Build ISO for BIOS

mkdir -vp build/{bios,uefi}

cp -rv auto/ build/bios/
cp -rv auto/ build/uefi/
cp -rv config/ build/bios/
cp -rv config/ build/uefi/

echo "[CONFIG]config build-work for bios..."

cp -v release/bios/config build/bios/auto/
cp -v release/bios/live.cfg build/bios/config/includes.binary/isolinux/
cp -v release/bios/grub.cfg build/bios/config/includes.binary/boot/grub/
cp -v release/bios/live.list.chroot build/bios/config/package-lists/

echo "[BUILD] Start Build-work for bios"

cd build/bios/
chmod +x auto/*
lingmo-sys-build clean
cp -v ../../source config
lingmo-sys-build config
lingmo-sys-build build
cd ../..

echo "[BUILD] Build-work for bios Successful!"

# Build ISO for UEFI
#
echo "[CONFIG]config build-work for uefi..."
cp -v release/uefi/live.cfg build/uefi/config/includes.binary/isolinux/
cp -v release/uefi/grub.cfg build/uefi/config/includes.binary/boot/grub/

echo "[BUILD] Start Build-work for uefi"

cd build/uefi/
chmod +x auto/*
lingmo-sys-build clean
cp -v ../../source config
lingmo-sys-build config
lingmo-sys-build build
cd ../..

echo "[BUILD] Build-work for bios Successful!"
