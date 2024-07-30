/*
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "doctermscodec.h"

using namespace Baloo;

QByteArray DocTermsCodec::encode(const QVector<QByteArray>& terms)
{
    Q_ASSERT(!terms.isEmpty());

    QByteArray full;
    full.append(terms.first());
    full.append('\0');

    for (int i = 1; i < terms.size(); i++) {
        const QByteArray term = terms[i];
        const QByteArray prevTerm = terms[i-1];

        if (term.startsWith(prevTerm)) {
            full.append(term.mid(prevTerm.size()));
            full.append(static_cast<char>(1));
        } else {
            full.append(term);
            full.append('\0');
        }
    }

    return full;
}

QVector<QByteArray> DocTermsCodec::decode(const QByteArray& full)
{
    Q_ASSERT(full.size());

    QVector<QByteArray> list;

    int prevWordBoundary = 0;
    for (int i = 0; i < full.size(); i++) {
        if (full[i] == 1) {
            if (list.isEmpty()) {
                // corrupted entry - no way to recover
                return list;
            }

            QByteArray arr(full.constData() + prevWordBoundary, i - prevWordBoundary);

            list << list.last() + arr;
            prevWordBoundary = i + 1;
            continue;
        }

        if (full[i] == '\0') {
            QByteArray arr(full.constData() + prevWordBoundary, i - prevWordBoundary);

            list << arr;
            prevWordBoundary = i + 1;
            continue;
        }
    }

    return list;
}
