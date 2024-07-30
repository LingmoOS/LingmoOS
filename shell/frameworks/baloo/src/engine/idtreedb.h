/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_IDTREEDB_H
#define BALOO_IDTREEDB_H

#include "engine_export.h"
#include <lmdb.h>
#include <QVector>
#include <QMap>

namespace Baloo {

class PostingIterator;

class BALOO_ENGINE_EXPORT IdTreeDB
{
public:
    IdTreeDB(MDB_dbi dbi, MDB_txn* txn);

    static MDB_dbi create(MDB_txn* txn);
    static MDB_dbi open(MDB_txn* txn);

    void set(quint64 docId, const QVector<quint64> &subDocIds);
    QVector<quint64> get(quint64 docId);

    /**
     * Returns an iterator which will return all the docIds which use \p docId
     * are the parent docID.
     */
    PostingIterator* iter(quint64 docId);

    QMap<quint64, QVector<quint64>> toTestMap() const;
private:
    MDB_txn* m_txn;
    MDB_dbi m_dbi;
};
}

#endif // BALOO_IDTREEDB_H
