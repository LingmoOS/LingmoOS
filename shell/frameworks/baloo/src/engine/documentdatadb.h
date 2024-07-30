/*
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_DOCUMENTDATADB_H
#define BALOO_DOCUMENTDATADB_H

#include "engine_export.h"
#include <lmdb.h>
#include <QByteArray>
#include <QMap>

namespace Baloo {

class BALOO_ENGINE_EXPORT DocumentDataDB
{
public:
    explicit DocumentDataDB(MDB_dbi dbi, MDB_txn* txn);
    ~DocumentDataDB();

    static MDB_dbi create(MDB_txn* txn);
    static MDB_dbi open(MDB_txn* txn);

    void put(quint64 docId, const QByteArray& data);
    QByteArray get(quint64 docId);

    void del(quint64 docId);
    bool contains(quint64 docId);

    QMap<quint64, QByteArray> toTestMap() const;
private:
    MDB_txn* m_txn;
    MDB_dbi m_dbi;
};

}

#endif // BALOO_DOCUMENTDATADB_H
