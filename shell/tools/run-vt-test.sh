#!/usr/bin/env bash
#
# Development-only harness for running lingmo-aurora against real DRM/KMS
# hardware from an SSH session, and pointing a Wayland test client at it.
#
# It picks a free virtual terminal, starts the compositor there under
# LIBSEAT_BACKEND=builtin (which needs root, because the builtin libseat backend
# does the VT ioctls and takes DRM master itself), waits for the Wayland socket,
# runs the client from the calling session, and tears everything down again.
#
# This is a TEST harness only. Production stays:
#
#     normal user session -> libseat/logind -> DRM/KMS
#
# Nothing here should ever become a runtime requirement.
#
# Usage:
#   tools/run-vt-test.sh [options] -- <client> [client args...]
#
# Options:
#   --socket NAME          Wayland socket name (default: lingmo-vt-test)
#   --compositor PATH      compositor binary (default: build/lingmo-aurora)
#   --compositor-arg ARG   extra argument for the compositor; repeatable
#   --drm-device PATH      passed through as --drm-device
#   --vt N                 force a VT instead of searching for a free one
#   --client-timeout SECS  how long the client may run (default: 20)
#   --startup-timeout SECS how long to wait for the socket (default: 15)
#   --continuous-client    the client runs until stopped, so hitting the client
#                          timeout counts as success rather than failure
#   --verbose              turn on the compositor's debug logging categories
#
# sudo is used for openvt, for the compositor, and for chvt. Cache credentials
# first if the shell is not interactive:
#
#   sudo -v && exec tools/run-vt-test.sh ...

set -uo pipefail

SOCKET="lingmo-vt-test"
COMPOSITOR="$(dirname "$(readlink -f "$0")")/../build/lingmo-aurora"
DRM_DEVICE=""
FORCED_VT=""
COMPOSITOR_ARGS=()
CLIENT_TIMEOUT=20
STARTUP_TIMEOUT=15
CONTINUOUS_CLIENT=0
VERBOSE=0

die() {
    printf 'run-vt-test: %s\n' "$*" >&2
    exit 1
}

info() {
    printf '==> %s\n' "$*"
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --socket) SOCKET="${2:?}"; shift 2 ;;
        --compositor) COMPOSITOR="${2:?}"; shift 2 ;;
        --compositor-arg) COMPOSITOR_ARGS+=("${2:?}"); shift 2 ;;
        --drm-device) DRM_DEVICE="${2:?}"; shift 2 ;;
        --vt) FORCED_VT="${2:?}"; shift 2 ;;
        --client-timeout) CLIENT_TIMEOUT="${2:?}"; shift 2 ;;
        --startup-timeout) STARTUP_TIMEOUT="${2:?}"; shift 2 ;;
        --continuous-client) CONTINUOUS_CLIENT=1; shift ;;
        --verbose) VERBOSE=1; shift ;;
        --) shift; break ;;
        -h|--help) sed -n '2,40p' "$0"; exit 0 ;;
        *) die "unknown option '$1' (client arguments go after --)" ;;
    esac
done

[[ $# -gt 0 ]] || die "no test client given; put it after --"
CLIENT=("$@")

[[ -x "$COMPOSITOR" ]] || die "compositor '$COMPOSITOR' is missing or not executable"
# Resolve the client through PATH when it is not given as a path, so that
# `-- env FOO=bar app` and `-- gtk4-demo` work as well as `-- build/tests/x`.
if [[ "${CLIENT[0]}" != */* ]]; then
    CLIENT_PATH="$(command -v "${CLIENT[0]}" 2>/dev/null)"
    [[ -n "$CLIENT_PATH" ]] || die "client '${CLIENT[0]}' was not found in PATH"
    CLIENT[0]="$CLIENT_PATH"
fi
[[ -x "${CLIENT[0]}" ]] || die "client '${CLIENT[0]}' is missing or not executable"

command -v openvt >/dev/null || die "openvt is not installed (package kbd)"
command -v chvt >/dev/null || die "chvt is not installed (package kbd)"

# Prompt when there is a human at the terminal, otherwise insist on a cached
# ticket. Note that with no tty sudo keys its ticket on the parent pid, so a
# non-interactive caller has to hand this script the same pid it cached under:
#
#   sudo -v && exec tools/run-vt-test.sh ...
if ! sudo -n true 2>/dev/null; then
    if [[ -t 0 ]]; then
        sudo -v || die "sudo authentication failed"
    else
        die "sudo credentials are not cached; run 'sudo -v' first"
    fi
fi

################################################################################
# VT selection
#
# A VT is considered busy if a logind session sits on it, if a getty is running
# on it, if someone is logged in on it, or if it is the foreground VT. The
# graphical session is therefore never a candidate, and neither is the terminal
# the user is looking at.
################################################################################

foreground_vt() {
    local active
    active="$(cat /sys/class/tty/tty0/active 2>/dev/null)"
    [[ "$active" =~ ^tty([0-9]+)$ ]] && printf '%s' "${BASH_REMATCH[1]}"
}

busy_vts() {
    local vt session

    vt="$(foreground_vt)"
    [[ -n "$vt" ]] && printf '%s\n' "$vt"

    if command -v loginctl >/dev/null; then
        while read -r session _; do
            [[ -n "$session" ]] || continue
            vt="$(loginctl show-session "$session" -p VTNr --value 2>/dev/null)"
            [[ -n "$vt" && "$vt" != 0 ]] && printf '%s\n' "$vt"
        done < <(loginctl list-sessions --no-legend 2>/dev/null)
    fi

    if command -v systemctl >/dev/null; then
        while read -r unit _; do
            [[ "$unit" =~ ^getty@tty([0-9]+)\.service$ ]] || continue
            systemctl is-active --quiet "$unit" && printf '%s\n' "${BASH_REMATCH[1]}"
        done < <(systemctl list-units 'getty@*' --all --no-legend 2>/dev/null)
    fi

    who 2>/dev/null | awk '$2 ~ /^tty[0-9]+$/ { sub(/^tty/, "", $2); print $2 }'
}

ORIGINAL_VT="$(foreground_vt)"

# Collected once into an array rather than tested through a pipeline: with
# pipefail set, a `busy_vts | grep -q` condition reports the SIGPIPE from the
# producer instead of grep's match, and silently never fires.
mapfile -t BUSY < <(busy_vts | sort -un)

vt_is_busy() {
    local candidate="$1" used
    for used in ${BUSY[@]+"${BUSY[@]}"}; do
        [[ "$candidate" == "$used" ]] && return 0
    done
    return 1
}

if [[ -n "$FORCED_VT" ]]; then
    VT="$FORCED_VT"
    if vt_is_busy "$VT"; then
        die "VT $VT is in use; refusing to take it (busy: ${BUSY[*]})"
    fi
else
    # High numbers first: on a desktop they are almost always unallocated,
    # which keeps the search away from getty and display-manager territory.
    VT=""
    for candidate in 8 9 10 11 12 7 6 5 4; do
        if ! vt_is_busy "$candidate"; then
            VT="$candidate"
            break
        fi
    done
    [[ -n "$VT" ]] || die "no free VT found (busy: ${BUSY[*]})"
fi

if [[ -n "$ORIGINAL_VT" && "$VT" == "$ORIGINAL_VT" ]]; then
    die "refusing to use VT $VT: it is the foreground VT"
fi

################################################################################
# Working directory and logs
################################################################################

WORKDIR="$(mktemp -d /tmp/lingmo-vt-test.XXXXXX)"
RUNTIME_DIR="$WORKDIR/runtime"
COMPOSITOR_LOG="$WORKDIR/compositor.log"
CLIENT_LOG="$WORKDIR/client.log"
PID_FILE="$WORKDIR/compositor.pid"
LAUNCHER="$WORKDIR/launch.sh"

# 0700 keeps libwayland from complaining; root can still create the socket here.
mkdir -p "$RUNTIME_DIR"
chmod 0700 "$RUNTIME_DIR"
: > "$COMPOSITOR_LOG"
: > "$CLIENT_LOG"

info "VT:              $VT (foreground VT is ${ORIGINAL_VT:-unknown}, left alone)"
info "compositor:      $COMPOSITOR"
info "client:          ${CLIENT[*]}"
info "socket:          $SOCKET"
info "XDG_RUNTIME_DIR: $RUNTIME_DIR"
info "logs:            $WORKDIR"

################################################################################
# Teardown
################################################################################

CLIENT_PID=""
CLEANUP_FAILED=0
CLEANUP_DONE=0

stop_client() {
    [[ -n "$CLIENT_PID" ]] || return 0
    kill -0 "$CLIENT_PID" 2>/dev/null || return 0
    kill -TERM "$CLIENT_PID" 2>/dev/null
    for _ in $(seq 20); do
        kill -0 "$CLIENT_PID" 2>/dev/null || return 0
        sleep 0.1
    done
    kill -KILL "$CLIENT_PID" 2>/dev/null
}

stop_compositor() {
    local pid
    pid="$(cat "$PID_FILE" 2>/dev/null)"
    [[ -n "$pid" ]] || return 0

    sudo -n kill -TERM "$pid" 2>/dev/null
    for _ in $(seq 50); do
        sudo -n kill -0 "$pid" 2>/dev/null || return 0
        sleep 0.1
    done

    printf 'run-vt-test: compositor %s ignored SIGTERM, sending SIGKILL\n' "$pid" >&2
    sudo -n kill -KILL "$pid" 2>/dev/null
    sleep 0.5
    if sudo -n kill -0 "$pid" 2>/dev/null; then
        printf 'run-vt-test: compositor %s survived SIGKILL\n' "$pid" >&2
        return 1
    fi
    return 0
}

restore_vt() {
    [[ -n "$ORIGINAL_VT" ]] || return 0
    sudo -n chvt "$ORIGINAL_VT" 2>/dev/null || {
        printf 'run-vt-test: failed to switch back to VT %s\n' "$ORIGINAL_VT" >&2
        return 1
    }
    return 0
}

# Both ends matter: startup lives at the top of the log and failures at the
# bottom, so a long log is shown head and tail rather than tail alone.
dump_log_file() {
    local label="$1" file="$2" lines
    printf '\n===== %s (%s) =====\n' "$label" "$file"
    lines="$(wc -l < "$file" 2>/dev/null || echo 0)"
    if (( lines > 120 )); then
        head -n 60 "$file"
        printf '... %d lines elided ...\n' "$(( lines - 120 ))"
        tail -n 60 "$file"
    else
        cat "$file" 2>/dev/null
    fi
}

dump_logs() {
    dump_log_file "compositor log" "$COMPOSITOR_LOG"
    dump_log_file "client log" "$CLIENT_LOG"
    printf '\n'
}

cleanup() {
    local status=$?
    (( CLEANUP_DONE )) && return
    CLEANUP_DONE=1
    trap - EXIT INT TERM

    stop_client
    stop_compositor || CLEANUP_FAILED=1
    restore_vt || CLEANUP_FAILED=1

    dump_logs

    if (( status == 0 && CLEANUP_FAILED )); then
        info "RESULT: cleanup failed"
        status=1
    fi
    exit "$status"
}
trap cleanup EXIT INT TERM

################################################################################
# Start the compositor on the chosen VT
################################################################################

# Left at the compositor's own defaults unless asked: the debug categories log
# once per surface commit, which is once per frame.
QT_RULES=""
(( VERBOSE )) && QT_RULES="lingmo.*.debug=true"

# openvt hands the program the VT as its stdio, so the redirection has to happen
# inside the launched shell for the logs to be capturable.
{
    printf '#!/bin/sh\n'
    printf 'echo $$ > %s\n' "$(printf '%q' "$PID_FILE")"
    printf 'exec >>%s 2>&1\n' "$(printf '%q' "$COMPOSITOR_LOG")"
    printf 'export LIBSEAT_BACKEND=builtin\n'
    printf 'export XDG_RUNTIME_DIR=%s\n' "$(printf '%q' "$RUNTIME_DIR")"
    [[ -n "$QT_RULES" ]] && printf 'export QT_LOGGING_RULES=%s\n' "$(printf '%q' "$QT_RULES")"
    printf 'exec %s --socket %s' "$(printf '%q' "$(realpath "$COMPOSITOR")")" \
        "$(printf '%q' "$SOCKET")"
    [[ -n "$DRM_DEVICE" ]] && printf ' --drm-device %s' "$(printf '%q' "$DRM_DEVICE")"
    for arg in ${COMPOSITOR_ARGS[@]+"${COMPOSITOR_ARGS[@]}"}; do
        printf ' %s' "$(printf '%q' "$arg")"
    done
    printf '\n'
} > "$LAUNCHER"
chmod 0755 "$LAUNCHER"

info "starting the compositor on VT $VT"
# No --force: openvt refuses a VT that is already allocated, which is one more
# guard against landing on someone else's terminal.
if ! sudo -n openvt -c "$VT" -s -- "$LAUNCHER"; then
    die "openvt failed to start the compositor on VT $VT"
fi

SOCKET_PATH="$RUNTIME_DIR/$SOCKET"
deadline=$(( SECONDS + STARTUP_TIMEOUT ))
while [[ ! -S "$SOCKET_PATH" ]]; do
    if (( SECONDS >= deadline )); then
        die "the Wayland socket did not appear within ${STARTUP_TIMEOUT}s"
    fi
    pid="$(cat "$PID_FILE" 2>/dev/null)"
    if [[ -n "$pid" ]] && ! sudo -n kill -0 "$pid" 2>/dev/null; then
        die "the compositor exited during startup"
    fi
    sleep 0.25
done

# The compositor runs as root, so the socket it creates is not writable by the
# calling user until we say so.
sudo -n chmod 0666 "$SOCKET_PATH" || die "failed to make the socket connectable"

info "socket is up: $SOCKET_PATH"

################################################################################
# Run the client from this session
################################################################################

info "running the client for up to ${CLIENT_TIMEOUT}s"
env XDG_RUNTIME_DIR="$RUNTIME_DIR" WAYLAND_DISPLAY="$SOCKET" \
    timeout --signal=TERM --kill-after=5 "$CLIENT_TIMEOUT" "${CLIENT[@]}" \
    >>"$CLIENT_LOG" 2>&1 &
CLIENT_PID=$!
wait "$CLIENT_PID"
CLIENT_STATUS=$?
CLIENT_PID=""

RESULT=0
if (( CLIENT_STATUS == 124 )); then
    if (( CONTINUOUS_CLIENT )); then
        info "client ran for the full ${CLIENT_TIMEOUT}s, as expected"
    else
        info "RESULT: client timed out after ${CLIENT_TIMEOUT}s"
        RESULT=1
    fi
elif (( CLIENT_STATUS != 0 )); then
    info "RESULT: client exited with status $CLIENT_STATUS"
    RESULT=1
fi

pid="$(cat "$PID_FILE" 2>/dev/null)"
if [[ -n "$pid" ]] && ! sudo -n kill -0 "$pid" 2>/dev/null; then
    info "RESULT: the compositor died before the client finished"
    RESULT=1
fi

(( RESULT == 0 )) && info "RESULT: ok"
exit "$RESULT"
