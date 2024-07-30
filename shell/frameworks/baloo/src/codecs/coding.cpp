/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>
    SPDX-FileCopyrightText: 2011 The LevelDB Authors. All rights reserved.

    SPDX-License-Identifier: LGPL-2.1-or-later AND BSD-3-Clause
*/

#include "coding.h"

namespace Baloo {

static inline int encodeVarint32Internal(char* dst, quint32 v) {
    // Operate on characters as unsigneds
    unsigned char* ptr = reinterpret_cast<unsigned char*>(dst);
    static const int B = 128;
    if (v < (1<<7)) {
        ptr[0] = v;
        return 1;
    }
    if (v < (1<<14)) {
        ptr[0] = v | B;
        ptr[1] = v>>7;
        return 2;
    }
    if (v < (1<<21)) {
        ptr[0] = v | B;
        ptr[1] = (v>>7) | B;
        ptr[2] = v>>14;
        return 3;
    }
    if (v < (1<<28)) {
        ptr[0] = v | B;
        ptr[1] = (v>>7) | B;
        ptr[2] = (v>>14) | B;
        ptr[3] = v>>21;
        return 4;
    }

    ptr[0] = v | B;
    ptr[1] = (v>>7) | B;
    ptr[2] = (v>>14) | B;
    ptr[3] = (v>>21) | B;
    ptr[4] = v>>28;
    return 5;
}

static inline void putVarint32Internal(char* dst, quint32 v, int &pos)
{
    pos += encodeVarint32Internal(&dst[pos], v);
}

void putDifferentialVarInt32(QByteArray &temporaryStorage, QByteArray* dst, const QVector<quint32>& values)
{
    temporaryStorage.resize((values.size() + 1) * 5);  // max size, correct size will be held in pos
    int pos = 0;
    putVarint32Internal(temporaryStorage.data(), values.size(), pos);

    quint32 v = 0;
    const auto itEnd = values.cend();
    for (auto it = values.cbegin(); it != itEnd; ++it) {
        const quint32 n = *it;
        putVarint32Internal(temporaryStorage.data(), n - v, pos);
        v = n;
    }
    dst->append(temporaryStorage.constData(), pos);
}

char* getDifferentialVarInt32(char* p, char* limit, QVector<quint32>* values)
{
    quint32 size = 0;
    p = getVarint32Ptr(p, limit, &size);
    values->resize(size);

    auto it = values->begin();
    auto end = values->end();

    quint32 v = 0;
    while (p && it != end) {
        quint32 n = 0;
        p = getVarint32Ptr(p, limit, &n);

        *it = (n + v);
        v += n;
        ++it;
    }
    values->erase(it, end);

    return p;
}

char* getVarint32PtrFallback(char* p, char* limit, quint32* value)
{
    quint32 result = 0;
    for (quint32 shift = 0; shift <= 28 && p < limit; shift += 7) {
        quint32 byte = *(reinterpret_cast<const unsigned char*>(p));
        p++;
        if (byte & 128) {
            // More bytes are present
            result |= ((byte & 127) << shift);
        } else {
            result |= (byte << shift);
            *value = result;
            return p;
        }
    }
    return nullptr;
}

}
