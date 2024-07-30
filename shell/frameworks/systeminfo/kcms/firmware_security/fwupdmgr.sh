#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
# SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>

set -e

# For sed testing please use --posix on GNU sed so you don't end up using gnu-isms.
fwupdmgr security --force 2>&1 | aha | sed -E 's@(https:[^[:space:]]+)@<a href="\1">\1</a>@g'
