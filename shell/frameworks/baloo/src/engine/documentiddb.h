/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_DOCUMENTIDDB_H
#define BALOO_DOCUMENTIDDB_H

#include "engine_export.h"
#include <QVector>
#include <lmdb.h>

/**
 * Implements storage for docIds without any associated data
 * Instantiated for:
 * - content indexing (files to be reindexed)
 * - failed ids (files no indexable, e.g. due to crashing indexers)
 */
namespace Baloo {

class BALOO_ENGINE_EXPORT DocumentIdDB
{
public:
    DocumentIdDB(MDB_dbi dbi, MDB_txn* txn);
    ~DocumentIdDB();

    static MDB_dbi create(const char* name, MDB_txn* txn);
    static MDB_dbi open(const char* name, MDB_txn* txn);

    void put(quint64 docId);
    bool contains(quint64 docId);
    void del(quint64 docID);

    QVector<quint64> fetchItems(int size);
    uint size();

    QVector<quint64> toTestVector() const;
private:
    MDB_txn* m_txn;
    MDB_dbi m_dbi;
};

}

#endif // BALOO_DOCUMENTIDDB_H
