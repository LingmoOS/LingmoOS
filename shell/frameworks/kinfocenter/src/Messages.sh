#!/bin/sh
# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2023 Emir SARI <emir_sari@icloud.com>

$XGETTEXT `find . -name \*.cc -o -name \*.cpp -o -name \*.h -o -name \*.qml` -o $podir/kinfocenter.pot
