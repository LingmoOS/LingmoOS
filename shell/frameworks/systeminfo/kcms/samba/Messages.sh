#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>
$XGETTEXT  `find . -name '*.cpp' -o -name '*.h' -o -name '*.qml' -o -name '*.js'` -o $podir/kcm_samba.pot
