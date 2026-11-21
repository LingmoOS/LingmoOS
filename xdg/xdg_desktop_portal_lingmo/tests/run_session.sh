#!/bin/sh
# Runs the tests on a private session bus whose activated helpers (the
# frontend's document portal among them) get private XDG directories too.
#
#   run_session.sh <dbus-run-session> <test> [arguments...]
set -eu

runner=$1
shift
directory=$(mktemp -d /tmp/lingmo-portal-session-XXXXXX)
cleanup() {
    # The frontend's document helper may leave a FUSE mount after the bus exits.
    fusermount3 -u "$directory/runtime/doc" 2>/dev/null || fusermount -u "$directory/runtime/doc" 2>/dev/null || true
    rm -rf "$directory"
}
trap cleanup EXIT

for name in config data cache state runtime; do
    mkdir -m 700 "$directory/$name"
done
export XDG_CONFIG_HOME="$directory/config"
export XDG_DATA_HOME="$directory/data"
export XDG_CACHE_HOME="$directory/cache"
export XDG_STATE_HOME="$directory/state"
export XDG_RUNTIME_DIR="$directory/runtime"

"$runner" -- "$@"
