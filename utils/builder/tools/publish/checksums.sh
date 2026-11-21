#!/usr/bin/env bash
# Generate portable manifests containing ISO basenames, never build paths.
set -euo pipefail
cd -- "${1:-artifact}"
export LC_ALL=C
shopt -s nullglob
images=(*.iso)
(( ${#images[@]} > 0 )) || { echo 'No ISO files for checksums' >&2; exit 1; }
for image in "${images[@]}"; do [[ -s "$image" ]] || { echo "Empty ISO: $image" >&2; exit 1; };done
for tool in sha256sum sha512sum md5sum; do
    "$tool" -- "${images[@]}" > "$tool.txt"
    "$tool" --check -- "$tool.txt"
    cat > "$tool.README" <<EOF
Lingmo OS ISO verification / ISO 完整性校验

Download your ISO and $tool.txt into the same directory. Keep the ISO filename unchanged.
将 ISO 和 $tool.txt 下载到同一目录，保持 ISO 文件名不变。

Linux / GNU coreutils:
  $tool --check --ignore-missing $tool.txt

Only downloaded ISO files are checked. At least one file must be checked successfully.
仅检查已下载的 ISO；至少应有一个文件校验成功。OK 表示匹配，FAILED 表示不匹配。

Windows PowerShell:
EOF
    case "$tool" in sha256sum) algorithm=SHA256;;sha512sum) algorithm=SHA512;;md5sum) algorithm=MD5;;esac
    printf '  Get-FileHash -Algorithm %s -LiteralPath "YOUR-IMAGE.iso"\n' "$algorithm" >> "$tool.README"
    cat >> "$tool.README" <<EOF
Compare the Hash value with the corresponding ISO entry in $tool.txt (case does not matter).
将 Hash 值与 $tool.txt 中对应 ISO 的一行比较，忽略字母大小写。

macOS:
EOF
    case "$tool" in
      sha256sum) echo '  shasum -a 256 -c sha256sum.txt' >> "$tool.README";;
      sha512sum) echo '  shasum -a 512 -c sha512sum.txt' >> "$tool.README";;
      md5sum) echo '  md5 "YOUR-IMAGE.iso"' >> "$tool.README";;
    esac
    cat >> "$tool.README" <<EOF
Replace YOUR-IMAGE.iso with the exact downloaded filename. If a checksum fails, download again.
将 YOUR-IMAGE.iso 替换为实际文件名；校验失败请重新下载。

These unsigned checksums detect accidental corruption; they do not prove publisher authenticity.
这些未签名的校验值用于检测传输损坏，不用于证明发布者身份。
EOF
    if [[ "$tool" == md5sum ]]; then
        printf '\nMD5 is for compatibility only. Prefer SHA-256 or SHA-512.\nMD5 仅供兼容，请优先使用 SHA-256 或 SHA-512。\n' >> "$tool.README"
    fi
done
