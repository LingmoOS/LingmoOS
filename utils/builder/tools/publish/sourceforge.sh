#!/usr/bin/env bash
# Upload complete ISO files and checksums; never split the SourceForge copy.
set -euo pipefail
: "${SF_USERNAME:?Set SF_USERNAME repository variable}"
: "${SF_PROJECT:?Set SF_PROJECT repository variable}"
: "${SF_REMOTE_DIR:?Set SF_REMOTE_DIR to the project files base directory}"
: "${SF_SSH_PRIVATE_KEY:?Set SF_SSH_PRIVATE_KEY repository secret}"
: "${SF_KNOWN_HOSTS:?Set verified SF_KNOWN_HOSTS repository secret}"
[[ "$SF_USERNAME" =~ ^[a-zA-Z0-9_-]+$ ]] || { echo 'Invalid SourceForge username' >&2; exit 2; }
[[ "$SF_PROJECT" =~ ^[a-zA-Z0-9_-]+$ ]] || { echo 'Invalid SourceForge project' >&2; exit 2; }
[[ "$SF_REMOTE_DIR" =~ ^[a-zA-Z0-9_./-]+$ && "$SF_REMOTE_DIR" != /* && "/$SF_REMOTE_DIR/" != *"/../"* ]] || { echo 'Invalid project-relative directory' >&2; exit 2; }
# CI supplies the canonical version from the same check that guarded the ISO.
# Do not derive a timestamp or clone version metadata again after the build.
if [ -n "${SF_BUILD_VERSION:-}" ]; then
    [[ "$SF_BUILD_VERSION" =~ ^[a-zA-Z0-9][a-zA-Z0-9_.-]*$ && "$SF_BUILD_VERSION" != *..* ]] || { echo 'Invalid build version' >&2; exit 2; }
    SF_REMOTE_DIR="${SF_REMOTE_DIR%/}/$SF_BUILD_VERSION"
fi
source_dir="${1:-artifact}"
shopt -s nullglob
images=("$source_dir"/*.iso)
(( ${#images[@]} > 0 )) || { echo 'No ISO files to publish' >&2; exit 1; }
for image in "${images[@]}"; do
    [[ -s "$image" ]] || { echo "Empty ISO: $image" >&2; exit 1; }
done
work=$(mktemp -d)
cleanup() { rm -rf -- "$work"; }
trap cleanup EXIT
chmod 700 "$work"
printf '%s\n' "$SF_SSH_PRIVATE_KEY" > "$work/key"
printf '%s\n' "$SF_KNOWN_HOSTS" > "$work/known_hosts"
chmod 600 "$work/key" "$work/known_hosts"
# rsync invokes SSH using an argument-preserving wrapper; key material is never logged.
cat > "$work/ssh" <<'SSH'
#!/bin/sh
exec ssh -i "$SF_UPLOAD_KEY" -o IdentitiesOnly=yes -o BatchMode=yes -o StrictHostKeyChecking=yes -o UserKnownHostsFile="$SF_UPLOAD_HOSTS" "$@"
SSH
chmod 700 "$work/ssh"
export SF_UPLOAD_KEY="$work/key" SF_UPLOAD_HOSTS="$work/known_hosts"
bash "$(dirname "$0")/checksums.sh" "$source_dir"
# SourceForge's rsync server does not support --mkpath. Its SFTP service
# can create each level. Ignore mkdir's existing-directory error, but require
# the following cd to succeed: missing permissions must stop before uploading.
remote_root="/home/frs/project/$SF_PROJECT"
printf 'cd "%s"\n' "$remote_root" > "$work/directories.sftp"
IFS=/ read -r -a components <<< "$SF_REMOTE_DIR"
for component in "${components[@]}"; do
    [[ -n "$component" && "$component" != . ]] || continue
    remote_root="$remote_root/$component"
    printf -- '-mkdir "%s"\ncd "%s"\n' "$remote_root" "$remote_root" >> "$work/directories.sftp"
done
sftp -b "$work/directories.sftp" -i "$work/key" \
    -o IdentitiesOnly=yes -o BatchMode=yes -o StrictHostKeyChecking=yes \
    -o "UserKnownHostsFile=$work/known_hosts" "$SF_USERNAME@frs.sourceforge.net"
destination="$SF_USERNAME,$SF_PROJECT@frs.sourceforge.net:$remote_root/"
# SourceForge's restricted rsync cannot create partial/delayed-update
# directories. Use ordinary temporary-file + rename behaviour in the target.
# Some server failures have returned zero: check diagnostics AND perform a
# checksum dry-run before advertising any files as successfully published.
upload_and_verify() {
    local log="$work/transfer.log" differences="$work/differences.txt"
    rsync -v --checksum --no-perms --no-owner --no-group \
        -e "$work/ssh" -- "$@" "$destination" 2>&1 | tee "$log"
    if grep -Eiq 'discarding completed file|Unable to create|Permission denied|rsync error:' "$log"; then
        echo 'SourceForge reported a transfer failure; refusing to publish success.' >&2
        return 1
    fi
    rsync --dry-run --checksum --itemize-changes --out-format='%i %n' \
        --no-perms --no-owner --no-group -e "$work/ssh" \
        -- "$@" "$destination" > "$differences"
    if [[ -s "$differences" ]]; then
        echo 'Remote files are missing or differ from local files after upload:' >&2
        cat "$differences" >&2
        return 1
    fi
}
upload_and_verify "${images[@]}"
checksums=();for tool in sha256sum sha512sum md5sum; do
    checksums+=("$source_dir/$tool.txt" "$source_dir/$tool.README")
done
upload_and_verify "${checksums[@]}"
echo "Published and remotely verified ${#images[@]} ISO file(s), checksum manifests and instructions on SourceForge."
