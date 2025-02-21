#!/bin/bash

SRC_DIRS=(
    "boot/splash/BootScreen"
    "core"
    "core32"
    "daemon"
    "fm"
    "lib/*"
    "libcoreui"
    "posic"
    "server/asyncjobprog"
    "server/itframework"
    "shell/bc/*"
    "shell/cc/*"
    "windows/*"
)

DEST_DIR="buildpkg"

for dir in "${SRC_DIRS[@]}"; do

   rsync -av --relative "$dir" "$DEST_DIR"
                                                            done

ls buildpkg/