#! /usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2020 Tomaz Canabrava <tcanabrava@kde.org>

$XGETTEXT `find . -name \*.cpp -o -name \*.qml` -o $podir/kcm_firewall.pot
