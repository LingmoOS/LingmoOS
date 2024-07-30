/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_DOCUMENTURLDB_H
#define BALOO_DOCUMENTURLDB_H

#include "idtreedb.h"
#include "idfilenamedb.h"
#include "idutils.h"

#include <QDebug>
#include <QFile>

namespace Baloo {

class PostingIterator;

class BALOO_ENGINE_EXPORT DocumentUrlDB
{
public:
    explicit DocumentUrlDB(MDB_dbi idTreeDb, MDB_dbi idFileNameDb, MDB_txn* txn);
    ~DocumentUrlDB();

    /**
     * Returns true if added
     */
    bool put(quint64 docId, quint64 parentId, const QByteArray& fileName);
    bool addPath(const QByteArray& url);

    QByteArray get(quint64 docId) const;
    QVector<quint64> getChildren(quint64 docId) const;
    bool contains(quint64 docId) const;

    /**
     * Move the document \p id to directory \p newParentId, set its name
     * to \p newName.
     */
    void updateUrl(quint64 id, quint64 newParentId, const QByteArray& newName);

    void del(quint64 docId);

    quint64 getId(quint64 docId, const QByteArray& fileName) const;

    PostingIterator* iter(quint64 docId) {
        IdTreeDB db(m_idTreeDbi, m_txn);
        return db.iter(docId);
    }

    QMap<quint64, QByteArray> toTestMap() const;

private:
    BALOO_ENGINE_NO_EXPORT void add(quint64 id, quint64 parentId, const QByteArray& name);

    MDB_txn* m_txn;
    MDB_dbi m_idFilenameDbi;
    MDB_dbi m_idTreeDbi;

};

}

#endif // BALOO_DOCUMENTURLDB_H
