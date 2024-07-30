/*
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_POSITIONINFO_H
#define BALOO_POSITIONINFO_H

#include <QVector>
#include <QDebug>

namespace Baloo {

class PositionInfo {
public:
    quint64 docId;
    QVector<uint> positions;

    PositionInfo(quint64 id = 0, const QVector<uint> posList = QVector<uint>())
        : docId(id), positions(posList) {}

    bool operator ==(const PositionInfo& rhs) const {
        return docId == rhs.docId;
    }
    bool operator !=(const PositionInfo& rhs) const {
        return docId != rhs.docId;
    }

    bool operator <(const PositionInfo& rhs) const {
        return docId < rhs.docId;
    }
};

inline QDebug operator<<(QDebug dbg, const PositionInfo &pos) {
    QDebugStateSaver saver(dbg);
    dbg.nospace() << Qt::hex << "(" << pos.docId << ": "
                  << Qt::dec << pos.positions << ")";
    return dbg;
}

}

Q_DECLARE_TYPEINFO(Baloo::PositionInfo, Q_RELOCATABLE_TYPE);

#endif // BALOO_POSITIONINFO_H
