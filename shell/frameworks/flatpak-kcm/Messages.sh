#! /usr/bin/env bash
# SPDX-FileCopyrightText: 2022 Suhaas Joshi <joshiesuhaas0@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

$XGETTEXT `find . -name "*.cpp" -o -name "*.qml"` -o $podir/kcm_flatpak.pot
