#!/bin/bash

# SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

# This hook script wraps the startup of all applications within this script, 
# in order to fix the issue of some applications not being able to start without shebang in their startup scripts


# ===== Log =====
# log.info xxx
# log.warn xxx
# log.info xxx
# log.debug xxx

function log.color_output() {
    local color=$1
    shift 1

    echo >&2 -e "\033[${color}m$@\033[0m"
    return 0
}

# Log is named without prefix "utils." for convenience
# Usage: log.log <level> ...content
function log.log() {
    if [[ $# < 2 ]]; then
        return -1
    fi

    local level=$1
    shift 1

    case $level in
    error) log.color_output "0;31" "[ERROR] $@" ;;
    warn) log.color_output "1;33" "[WARN] $@" ;;
    info) log.color_output "1;37" "[INFO] $@" ;;
    debug) log.color_output "1;30" "[DEBUG] $@" ;;
    esac

    return 0
}

function log.error() { log.log "error" "$@"; }
function log.warn() { log.log "warn" $@; }
function log.info() { log.log "info" $@; }
function log.debug() { log.log "debug" $@; }


function check_file_no_shebang(){
    local file_type=$(file -b "${1}")
    # 文件类型是纯文本文件、有可执行权限，并且不是脚本文件，则可能忘记加上shebang
    if [[ ($file_type == *"ASCII text"* || $file_type == *"Unicode text"*) && -x "$exec_cmd_origin_path" && "$file_type" != *"script"* ]]; then
        true
    else
        false
    fi
}

function check_file_is_python(){
    if file -b "${1}" | grep -q "Python script" ;then
        true
    else
        false
    fi
}

function choose_python(){
    pythonInterpreter=$(command -v python3 || command -v python)
    if [ -z "${pythonInterpreter}" ]; then
        log.warn "Python interpreter not found. Trying to continue"
        exec "$@"
    fi
    log.debug "Automatically add interpreter ${pythonInterpreter} to launch app."
}

exec_cmd_origin_path=$1

if [[ -e ${exec_cmd_origin_path} ]]; then

    
##### 1. Check if the file is a script and is excutable and lack of shebang.(If not excutable, we should not let it run.)
##### 1.1 If python, then use python, else use bash.
##### 2. If do not need to fix,just try to run.

    if  check_file_no_shebang "${exec_cmd_origin_path}" ; then
        log.debug "The executable is a script without shebang. Trying to fix..."
        exec bash -c "$@"
    elif check_file_is_python "${exec_cmd_origin_path}"  ; then
        choose_python
        exec ${pythonInterpreter} "$@"
    else
        exec "$@"
    fi
else
    log.info "${exec_cmd_origin_path} maybe not an absolute path. Continue"
    exec "$@"
fi
