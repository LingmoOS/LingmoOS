/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_POSITIONCODEC_H
#define BALOO_POSITIONCODEC_H

#include <QByteArray>
#include <QVector>

#include "positiondb.h"

namespace Baloo {

class PositionCodec
{
public:
    static QByteArray encode(const QVector<PositionInfo>& list);
    static QVector<PositionInfo> decode(const QByteArray& arr);
};
}

#endif // BALOO_POSITIONCODEC_H
