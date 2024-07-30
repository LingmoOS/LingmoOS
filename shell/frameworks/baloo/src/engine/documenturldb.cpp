/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "documenturldb.h"
#include "postingiterator.h"
#include "enginedebug.h"

#include <algorithm>

using namespace Baloo;

DocumentUrlDB::DocumentUrlDB(MDB_dbi idTreeDb, MDB_dbi idFilenameDb, MDB_txn* txn)
    : m_txn(txn)
    , m_idFilenameDbi(idFilenameDb)
    , m_idTreeDbi(idTreeDb)
{
}

DocumentUrlDB::~DocumentUrlDB()
{
}

bool DocumentUrlDB::addPath(const QByteArray& url)
{
    Q_ASSERT(!url.isEmpty());
    Q_ASSERT(!url.endsWith('/'));
    if (url.isEmpty() || url.endsWith('/')) {
        return false;
    }

    IdFilenameDB idFilenameDb(m_idFilenameDbi, m_txn);

    QByteArray arr = url;
    quint64 id = filePathToId(arr);

    while (id) {
        if (idFilenameDb.contains(id)) {
            return true;
        }

        int pos = arr.lastIndexOf('/');
        QByteArray name = arr.mid(pos + 1);

        if (pos == 0) {
            add(id, 0, name);
            return true;
        }

        arr.resize(pos);
        auto parentId = filePathToId(arr);
        if (!parentId) {
            return false;
        }
        add(id, parentId, name);

        id = parentId;
    }
    return false;
}

bool DocumentUrlDB::put(quint64 docId, quint64 parentId, const QByteArray& filename)
{
    Q_ASSERT(docId);
    Q_ASSERT(!filename.contains('/'));
    Q_ASSERT(!filename.isEmpty());
    if (!docId || filename.isEmpty() || filename.contains('/')) {
        return false;
    }

    add(docId, parentId, filename);
    return true;
}

void DocumentUrlDB::updateUrl(quint64 id, quint64 newParentId, const QByteArray& newName)
{
    IdFilenameDB idFilenameDb(m_idFilenameDbi, m_txn);
    IdTreeDB idTreeDb(m_idTreeDbi, m_txn);

    // Sanity checks
    auto path = idFilenameDb.get(id);
    if (path.parentId != newParentId) {
        // Remove from old parent
        QVector<quint64> subDocs = idTreeDb.get(path.parentId);
        if (subDocs.removeOne(id)) {
            idTreeDb.set(path.parentId, subDocs);
        }
        // Add to new parent
        subDocs = idTreeDb.get(newParentId);
        sortedIdInsert(subDocs, id);
        idTreeDb.set(newParentId, subDocs);
    }

    if ((newName != path.name) || (newParentId != path.parentId)) {
        qCDebug(ENGINE) << id << "renaming" << path.name << "to" << newName;
        path.parentId = newParentId;
        path.name = newName;
        idFilenameDb.put(id, path);
    }
}

void DocumentUrlDB::del(quint64 id)
{
    if (!id) {
        qCWarning(ENGINE) << "del called with invalid docId";
        return;
    }

    IdFilenameDB idFilenameDb(m_idFilenameDbi, m_txn);
    IdTreeDB idTreeDb(m_idTreeDbi, m_txn);

    auto path = idFilenameDb.get(id);

    // Remove from parent
    QVector<quint64> subDocs = idTreeDb.get(path.parentId);
    if (subDocs.removeOne(id)) {
        idTreeDb.set(path.parentId, subDocs);
    }

    qCDebug(ENGINE) << id << "deleting" << path.name;
    idFilenameDb.del(id);
}

void DocumentUrlDB::add(quint64 id, quint64 parentId, const QByteArray& name)
{
    if (!id || name.isEmpty()) {
        return;
    }

    IdFilenameDB idFilenameDb(m_idFilenameDbi, m_txn);
    IdTreeDB idTreeDb(m_idTreeDbi, m_txn);

    QVector<quint64> subDocs = idTreeDb.get(parentId);

    // insert if not there
    sortedIdInsert(subDocs, id);

    idTreeDb.set(parentId, subDocs);

    // Update the IdFileName
    IdFilenameDB::FilePath path;
    path.parentId = parentId;
    path.name = name;

    idFilenameDb.put(id, path);
}

bool DocumentUrlDB::contains(quint64 docId) const
{
    if (!docId) {
        return false;
    }

    IdFilenameDB idFilenameDb(m_idFilenameDbi, m_txn);

    return idFilenameDb.contains(docId);
}

QByteArray DocumentUrlDB::get(quint64 docId) const
{
    if (!docId) {
        return QByteArray();
    }

    IdFilenameDB idFilenameDb(m_idFilenameDbi, m_txn);

    IdFilenameDB::FilePath path;
    if (!idFilenameDb.get(docId, path)) {
        return QByteArray();
    }

    QByteArray ret = '/' + path.name;
    quint64 id = path.parentId;
    // arbitrary path depth limit - we have to deal with
    // possibly corrupted DBs out in the wild
    int depth_limit = 512;

    while (id) {
        if (!idFilenameDb.get(id, path)) {
            return QByteArray();
        }
        if (!depth_limit--) {
            return QByteArray();
        }
        path.name.prepend('/');

        ret.prepend(path.name);
        id = path.parentId;
    }

    return ret;
}

QVector<quint64> DocumentUrlDB::getChildren(quint64 docId) const
{
    IdTreeDB idTreeDb(m_idTreeDbi, m_txn);
    return idTreeDb.get(docId);
}

quint64 DocumentUrlDB::getId(quint64 docId, const QByteArray& fileName) const
{
    if (fileName.isEmpty()) {
        return 0;
    }

    IdFilenameDB idFilenameDb(m_idFilenameDbi, m_txn);
    IdTreeDB idTreeDb(m_idTreeDbi, m_txn);

    const QVector<quint64> subFiles = idTreeDb.get(docId);
    IdFilenameDB::FilePath path;
    for (quint64 id : subFiles) {
        if (idFilenameDb.get(id, path) && (path.name == fileName)) {
            return id;
        }
    }

    return 0;
}

QMap<quint64, QByteArray> DocumentUrlDB::toTestMap() const
{
    IdTreeDB idTreeDb(m_idTreeDbi, m_txn);

    QMap<quint64, QVector<quint64>> idTreeMap = idTreeDb.toTestMap();
    QSet<quint64> allIds;

    for (auto it = idTreeMap.cbegin(); it != idTreeMap.cend(); it++) {
        allIds.insert(it.key());
        for (quint64 id : it.value()) {
            allIds.insert(id);
        }
    }

    QMap<quint64, QByteArray> map;
    for (quint64 id : allIds) {
        if (id) {
            QByteArray path = get(id);
            // FIXME: this prevents sanitizing
            // reactivate  Q_ASSERT(!path.isEmpty());
            map.insert(id, path);
        }
    }

    return map;
}
