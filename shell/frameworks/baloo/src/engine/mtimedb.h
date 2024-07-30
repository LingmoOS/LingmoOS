/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_MTIMEDB_H
#define BALOO_MTIMEDB_H

#include "engine_export.h"
#include <lmdb.h>
#include <QVector>
#include <QMap>

namespace Baloo {

class PostingIterator;

/**
 * The MTime DB maps the file mtime to its id. This allows us to do
 * fast searches of files between a certain time range.
 */
class BALOO_ENGINE_EXPORT MTimeDB
{
public:
    explicit MTimeDB(MDB_dbi dbi, MDB_txn* txn);
    ~MTimeDB();

    static MDB_dbi create(MDB_txn* txn);
    static MDB_dbi open(MDB_txn* txn);

    void put(quint32 mtime, quint64 docId);
    QVector<quint64> get(quint32 mtime);

    void del(quint32 mtime, quint64 docId);

    /**
      * Get documents with an mtime between \p beginTime and
      * \p endTime (inclusive)
      */
    PostingIterator* iterRange(quint32 beginTime, quint32 endTime);

    QMap<quint32, quint64> toTestMap() const;
private:
    MDB_txn* m_txn;
    MDB_dbi m_dbi;
};
}

#endif // BALOO_MTIMEDB_H
