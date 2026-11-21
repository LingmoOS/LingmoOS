#!/bin/bash

generate_build_ver() {
    echo "$(date +%y%m%d%H%M).$(git rev-parse --short HEAD 2>/dev/null || echo "build")"
}