#! /usr/bin/env bash

# SPDX-FileCopyrightText: 2021 Daniel Vrátil <dvratil@kde.org>
#
# SPDX-License-Identifier: LGPL-2.1-or-later

$XGETTEXT `find . -name \*.qml -o -name \*.cpp` -o $podir/lingmo_applet_org.kde.lingmo.pass.pot
