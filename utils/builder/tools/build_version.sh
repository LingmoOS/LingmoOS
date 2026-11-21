#!/bin/bash

REVISION_FILE=".version.build"

generate_build_ver() {
    if [[ -f "$REVISION_FILE" ]]; then
        REVISION=$(<"$REVISION_FILE")
    else
        REVISION="2501"
    fi

    ((REVISION++))

    echo "$REVISION" > "$REVISION_FILE"

    local year=$(date +"%y")
    local month=$(date +"%m")
    local day=$(date +"%d")
    local hour=$(date +"%H")
    local minute=$(date +"%M")

    local minute_ten=$((minute / 100))

    local build_ver="${year}${month#0}${day#0}${hour#0}${minute_ten}"

    echo "$build_ver.$REVISION"
}

