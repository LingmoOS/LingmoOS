/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_DOCUMENTTIMEDB_H
#define BALOO_DOCUMENTTIMEDB_H

#include "engine_export.h"

#include <QMap>
#include <QDebug>
#include <lmdb.h>

namespace Baloo {

class BALOO_ENGINE_EXPORT DocumentTimeDB
{
public:
    DocumentTimeDB(MDB_dbi dbi, MDB_txn* txn);
    ~DocumentTimeDB();

    static MDB_dbi create(MDB_txn* txn);
    static MDB_dbi open(MDB_txn* txn);

    struct TimeInfo
    {
        /** Tracking of file time stamps
          *
          * @sa QDateTime::toSecsSinceEpoch()
          * @sa QFileInfo::lastModified()
          * @sa QFileInfo::metadataChangeTime()
          */
        quint32 mTime; /**< file (data) modification time */
        quint32 cTime; /**< metadata (e.g. XAttr) change time */
        /* No birthtime yet */

        explicit TimeInfo(quint32 mt = 0, quint32 ct = 0) : mTime(mt), cTime(ct) {}

        bool operator == (const TimeInfo& rhs) const {
            return mTime == rhs.mTime && cTime == rhs.cTime;
        }
    };
    void put(quint64 docId, const TimeInfo& info);
    TimeInfo get(quint64 docId);

    void del(quint64 docId);
    bool contains(quint64 docId);

    QMap<quint64, TimeInfo> toTestMap() const;
private:
    MDB_txn* m_txn;
    MDB_dbi m_dbi;
};

inline QDebug operator<<(QDebug dbg, const DocumentTimeDB::TimeInfo &time) {
    dbg << "(" << time.mTime << "," << time.cTime << ")";
    return dbg;
}

}

#endif // BALOO_DOCUMENTTIMEDB_H
