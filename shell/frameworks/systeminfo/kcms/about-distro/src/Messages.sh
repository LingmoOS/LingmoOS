#!/bin/sh
# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2012 Harald Sitter <sitter@kde.org>

$XGETTEXT `find . -name \*.cc -o -name \*.cpp -o -name \*.h -o -name \*.qml` -o $podir/kcm_about-distro.pot
