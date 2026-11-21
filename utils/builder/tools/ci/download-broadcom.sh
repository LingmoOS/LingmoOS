#!/usr/bin/env bash
# Download the kernel-compatible driver before scripts/ is copied to build/.
set -euo pipefail

root=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd)
destination=${1:-"$root/scripts/lingmo-config/common/packages.chroot"}
package=broadcom-sta-dkms
version=6.30.223.271-32
architecture=amd64
filename="${package}_${version}_${architecture}.deb"
url="https://deb.debian.org/debian/pool/non-free/b/broadcom-sta/$filename"
sha256=0b4e801fda736715cfd56d98a4ddd6a0564c6ebc77c041a58f74642f7630d870

mkdir -p -- "$destination"
temporary=$(mktemp "$destination/.broadcom-download.XXXXXX")
trap 'rm -f -- "$temporary"' EXIT
curl --fail --location --silent --show-error --proto '=https' --proto-redir '=https' \
    --retry 3 --connect-timeout 30 --max-time 300 "$url" --output "$temporary"
printf '%s  %s\n' "$sha256" "$temporary" | sha256sum --check --status
[[ $(dpkg-deb -f "$temporary" Package) == "$package" ]]
[[ $(dpkg-deb -f "$temporary" Version) == "$version" ]]
[[ $(dpkg-deb -f "$temporary" Architecture) == "$architecture" ]]
chmod 0644 "$temporary"
mv -f -- "$temporary" "$destination/$filename"
printf 'Staged %s (%s, %s) in %s\n' "$package" "$version" "$architecture" "$destination"
