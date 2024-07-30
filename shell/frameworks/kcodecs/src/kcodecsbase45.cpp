
/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kcodecs.h"
#include "kcodecs_debug.h"

#include <QDebug>

static constexpr const char base45Table[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ $%*+-./:";

static uint8_t base45MapFromChar(char c)
{
    const auto it = std::find(std::begin(base45Table), std::end(base45Table), c);
    if (it == std::end(base45Table)) {
        qCWarning(KCODECS_LOG) << "invalid base45 character:" << c;
        return 0;
    }
    return std::distance(std::begin(base45Table), it);
}

QByteArray KCodecs::base45Decode(QByteArrayView in)
{
    QByteArray out;
    out.reserve(((in.size() / 3) + 1) * 2);

    for (qsizetype i = 0; i + 1 < in.size(); i += 3) {
        uint32_t n = base45MapFromChar(in[i]) + base45MapFromChar(in[i + 1]) * 45;
        if (i + 2 < in.size()) {
            n += 45 * 45 * base45MapFromChar(in[i + 2]);
            out.push_back(n >> 8);
        } else {
            if (n >> 8) {
                out.push_back(n >> 8);
            }
        }
        out.push_back(n % 256);
    }

    return out;
}
