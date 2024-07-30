#! /usr/bin/env bash

# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: CC0-1.0

$XGETTEXT `find . -name "*.cpp" -o -name "*.qml"` -o $podir/kcm_bolt.pot
$XGETTEXT -j -L JavaScript `find . -name "*.js"` -o $podir/kcm_bolt.pot

