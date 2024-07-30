#! /bin/sh

# SPDX-FileCopyrightText: 2024 Emir SARI <emir_sari@icloud.com>
# SPDX-License-Identifier: GPL-2.0-or-later

$XGETTEXT `find -name \*.cpp -o -name \*.h` -o $podir/powerdevil_osd.pot
