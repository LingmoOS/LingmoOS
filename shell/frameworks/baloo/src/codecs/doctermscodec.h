/*
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_DOCTERMSCODEC_H
#define BALOO_DOCTERMSCODEC_H

#include <QVector>
#include <QByteArray>

namespace Baloo {

class DocTermsCodec
{
public:
    static QByteArray encode(const QVector<QByteArray>& terms);
    static QVector<QByteArray> decode(const QByteArray& arr);
};
}

#endif // BALOO_DOCTERMSCODEC_H
