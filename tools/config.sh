#!/bin/bash

SRC_DIRS=(
    "boot/splash/BootScreen"
    "core"
    "daemon"
    "fm"
    "libcoreui"
    "posic"
    "server/asyncjobprog"
    "server/itframework"
)

DEST_DIR="buildpkg"
SPCASE_BUILD_DIR="buildpkg_spcase"

for dir in "${SRC_DIRS[@]}"; do
   rsync -av --relative "$dir" "$DEST_DIR"
done

SRC_OTHER_DIRS=(
    "core32/"
    "shell/bc/*"
    "shell/cc/*"
    "windows/*"
    "lib/*"
)

DEST_O_DIR="$DEST_O_DIR"
SPCASE_BUILD_DIR="$SPCASE_BUILD_DIR"

rsync -av --relative "${SRC_OTHER_DIRS[0]}" "$SPCASE_BUILD_DIR"

for dir in "${SRC_OTHER_DIRS[@]:1}"; do
    cp -r "$dir" "$DEST_O_DIR"
done

ls buildpkg/
ls buildpkg_spcase/

mkdir pkg_out/

# Build core32
cd buildpkg_spcase/ &&
mv core32 lingmo-workspace/ &&
tar -Jcvf lingmo-workspace_3.0.5.orig.tar.xz --exclude='debian/' lingmo-workspace/ &&
(cd lingmo-workspace/ && apt build-dep ./ -y && dpkg-buildpackage -j10) &&
cp *.deb *.changes *.buildinfo *.dsc *.xz ../pkg_out