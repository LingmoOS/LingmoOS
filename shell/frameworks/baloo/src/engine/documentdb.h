/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_DOCUMENTDB_H
#define BALOO_DOCUMENTDB_H

#include "engine_export.h"
#include <lmdb.h>
#include <QVector>
#include <QMap>

namespace Baloo {

/**
 * Implements storage for a set of <term>s for the given docId
 * Instantiated for:
 * - document (content) terms
 * - filename terms
 * - xattr terms
 */
class BALOO_ENGINE_EXPORT DocumentDB
{
public:
    DocumentDB(MDB_dbi dbi, MDB_txn* txn);
    ~DocumentDB();

    static MDB_dbi create(const char* name, MDB_txn* txn);
    static MDB_dbi open(const char* name, MDB_txn* txn);

    void put(quint64 docId, const QVector< QByteArray >& list);
    QVector<QByteArray> get(quint64 docId);

    bool contains(quint64 docId);
    void del(quint64 docId);
    uint size();

    QMap<quint64, QVector<QByteArray>> toTestMap() const;
private:
    MDB_txn* m_txn;
    MDB_dbi m_dbi;
};
}

#endif // BALOO_DOCUMENTDB_H
