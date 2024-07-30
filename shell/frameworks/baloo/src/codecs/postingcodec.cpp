/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "postingcodec.h"

using namespace Baloo;

QByteArray PostingCodec::encode(const QVector<quint64>& list)
{
    uint size = list.size() * sizeof(quint64);
    const char* ptr = reinterpret_cast<const char*>(list.constData());

    return QByteArray(ptr, size);
}

QVector<quint64> PostingCodec::decode(const QByteArray& arr)
{
    QVector<quint64> vec;
    vec.resize(arr.size() / sizeof(quint64));

    memcpy(vec.data(), arr.constData(), arr.size());
    return vec;
}
