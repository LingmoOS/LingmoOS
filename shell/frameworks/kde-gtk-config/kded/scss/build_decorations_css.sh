#!/bin/sh

# SPDX-FileCopyrightText: 2019 Mikhail Zolotukhin <zomial@protonmail.com>
#
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

# Usage: build_sass <source-directory> <target-directory> <include-directory>
build_sass() {
  if command -v sassc >/dev/null 2>&1; then
      sassc -I "$3" "$1" "$2"
  else
      sass -I "$3" --cache-location /tmp/sass-cache "$1" "$2"
  fi
}

# Usage: build_and_install_decorations_css <target-directory>
build_and_install_decorations_css() {
  BUILD_DIR="$(mktemp -d)"
  INSTALL_DIR="$1"

  # Build
  build_sass "window_decorations.scss" "${BUILD_DIR}/window_decorations.css"

  # Install
  mkdir -p "${INSTALL_DIR}"
  mv -f "${BUILD_DIR}/window_decorations.css" "${INSTALL_DIR}"
}

build_and_install_decorations_css "$1"
