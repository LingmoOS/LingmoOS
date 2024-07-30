/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_POSTINGCODEC_H
#define BALOO_POSTINGCODEC_H

#include <QByteArray>
#include <QVector>

namespace Baloo {

class PostingCodec
{
public:
    static QByteArray encode(const QVector<quint64>& list);
    static QVector<quint64> decode(const QByteArray& arr);
};

}

#endif // BALOO_POSTINGCODEC_H
