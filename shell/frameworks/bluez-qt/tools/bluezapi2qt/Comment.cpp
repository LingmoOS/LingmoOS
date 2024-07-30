/*
 * BluezQt - Asynchronous BlueZ wrapper library
 *
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "Comment.h"

bool Comment::finalize()
{
    // Delete last empty lines from comment
    while (last().isEmpty()) {
        removeLast();
    }

    // Find indents
    qsizetype indents = 255;
    for (const auto &line : *this) {
        if (line.isEmpty()) {
            continue;
        }
        indents = std::min(indents, line.count(QStringLiteral("\t")));
    }

    // Remove indents
    for (auto &line : *this) {
        line.remove(0, indents);
    }

    // Replace indents
    for (auto &line : *this) {
        line.replace(QStringLiteral("\t"), QStringLiteral("    "));
    }

    return true;
}
