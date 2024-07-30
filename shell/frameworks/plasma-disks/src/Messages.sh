#!/bin/sh
# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2020-2021 Harald Sitter <sitter@kde.org>

$XGETTEXT `find . -name \*.cpp -o -name \*.h` -o $podir/kcm_disks.pot
# Extract JavaScripty files as what they are, otherwise for example template literals won't work correctly (by default we extract as C++).
# https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Template_literals
$XGETTEXT --join-existing --language=JavaScript `find . -name \*.qml -o -name \*.js` -o $podir/kcm_disks.pot
