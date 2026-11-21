#!/usr/bin/env bash
# Copy only diagnostic logs, never DKMS signing keys or the full chroot.
set -euo pipefail
build=${1:-build}
output=${2:-build-diagnostics}
mkdir -p -- "$output"
copy_log() {
    local relative=$1
    if [[ -f "$build/$relative" ]]; then
        mkdir -p -- "$output/$(dirname -- "$relative")"
        cp -- "$build/$relative" "$output/$relative"
    fi
}
copy_log build.log
copy_log chroot/var/log/dpkg.log
copy_log chroot/var/log/apt/term.log
copy_log chroot/var/log/apt/history.log
if [[ -d "$build/chroot/var/lib/dkms" ]]; then
    while IFS= read -r -d '' log; do
        copy_log "${log#"$build/"}"
    done < <(find "$build/chroot/var/lib/dkms" -type f -name make.log -print0)
fi
if [[ -d "$build/chroot" ]]; then
    {
        chroot "$build/chroot" dpkg-query -W -f='${binary:Package} ${Version} ${db:Status-Status}\n' \
            broadcom-sta-dkms dkms build-essential gcc gcc-14 'linux-headers-*' 'linux-image-*' || true
        chroot "$build/chroot" sh -c 'command -v gcc; command -v gcc-14; gcc --version; gcc-14 --version' || true
    } > "$output/toolchain.txt" 2>&1
fi
find "$output" -type f -print
