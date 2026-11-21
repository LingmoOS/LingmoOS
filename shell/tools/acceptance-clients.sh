#!/usr/bin/env bash
# Run under tools/run-vt-test.sh; both clients use the harness Wayland socket.
set -euo pipefail
command -v qdbusviewer >/dev/null
command -v gtk4-demo >/dev/null
pids=()
cleanup() {
    trap - EXIT TERM INT
    for pid in "${pids[@]}"; do kill "$pid" 2>/dev/null || true; done
    for pid in "${pids[@]}"; do wait "$pid" 2>/dev/null || true; done
}
trap cleanup EXIT
trap 'exit 0' TERM INT
if command -v wayland-info >/dev/null; then
    wayland-info
fi
env QT_QPA_PLATFORM=wayland qdbusviewer &
pids+=("$!")
env GDK_BACKEND=wayland gtk4-demo &
pids+=("$!")
for pid in "${pids[@]}"; do wait "$pid"; done
