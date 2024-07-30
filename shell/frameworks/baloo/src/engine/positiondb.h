/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_POSITIONDB_H
#define BALOO_POSITIONDB_H

#include "engine_export.h"

#include <QByteArray>
#include <QMap>
#include <QVector>
#include <lmdb.h>

namespace Baloo {

class PositionInfo;
class VectorPositionInfoIterator;

class BALOO_ENGINE_EXPORT PositionDB
{
public:
    explicit PositionDB(MDB_dbi dbi, MDB_txn* txn);
    ~PositionDB();

    static MDB_dbi create(MDB_txn* txn);
    static MDB_dbi open(MDB_txn* txn);

    void put(const QByteArray& term, const QVector<PositionInfo>& list);
    QVector<PositionInfo> get(const QByteArray& term);
    void del(const QByteArray& term);

    VectorPositionInfoIterator* iter(const QByteArray& term);

    QMap<QByteArray, QVector<PositionInfo>> toTestMap() const;
private:
    MDB_txn* m_txn;
    MDB_dbi m_dbi;
};

}

#endif // BALOO_POSITIONDB_H
