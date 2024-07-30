/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_IDFILENAMEDB_H
#define BALOO_IDFILENAMEDB_H

#include "engine_export.h"
#include <lmdb.h>
#include <QByteArray>
#include <QMap>

namespace Baloo {

class BALOO_ENGINE_EXPORT IdFilenameDB
{
public:
    IdFilenameDB(MDB_dbi dbi, MDB_txn* txn);
    ~IdFilenameDB();

    static MDB_dbi create(MDB_txn* txn);
    static MDB_dbi open(MDB_txn* txn);

    struct FilePath {
        quint64 parentId = 0;
        QByteArray name;

        bool operator == (const FilePath& fp) const {
            return parentId == fp.parentId && name == fp.name;
        }
    };
    void put(quint64 docId, const FilePath& path);
    FilePath get(quint64 docId);
    bool get(quint64 docId, FilePath& path);
    bool contains(quint64 docId);
    void del(quint64 docId);

    QMap<quint64, FilePath> toTestMap() const;
private:
    MDB_txn* m_txn;
    MDB_dbi m_dbi;
};

}

#endif // BALOO_IDFILENAMEDB_H
