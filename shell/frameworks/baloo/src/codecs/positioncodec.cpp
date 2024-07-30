/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "positioncodec.h"
#include "positioninfo.h"
#include "coding.h"

using namespace Baloo;

QByteArray PositionCodec::encode(const QVector<PositionInfo>& list)
{
    QByteArray data;
    QByteArray temporaryStorage;

    for (const PositionInfo& pos : list) {
        putFixed64(&data, pos.docId);
        putDifferentialVarInt32(temporaryStorage, &data, pos.positions);
   }

    return data;
}

QVector<PositionInfo> PositionCodec::decode(const QByteArray& arr)
{
    char* data = const_cast<char*>(arr.data());
    char* end = data + arr.size();

    QVector<PositionInfo> vec;
    while (data < end) {
        PositionInfo info;

        info.docId = decodeFixed64(data);
        data += sizeof(quint64);
        data = getDifferentialVarInt32(data, end, &info.positions);
        if (!data) {
            return QVector<PositionInfo>();
        }

        vec << info;
    }

    return vec;
}
