/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "idfilenamedb.h"
#include "enginedebug.h"

using namespace Baloo;

IdFilenameDB::IdFilenameDB(MDB_dbi dbi, MDB_txn* txn)
    : m_txn(txn)
    , m_dbi(dbi)
{
    Q_ASSERT(txn != nullptr);
    Q_ASSERT(dbi != 0);
}

IdFilenameDB::~IdFilenameDB()
{
}

MDB_dbi IdFilenameDB::create(MDB_txn* txn)
{
    MDB_dbi dbi = 0;
    int rc = mdb_dbi_open(txn, "idfilename", MDB_CREATE | MDB_INTEGERKEY, &dbi);
    if (rc) {
        qCWarning(ENGINE) << "IdFilenameDB::create" << mdb_strerror(rc);
        return 0;
    }

    return dbi;
}

MDB_dbi IdFilenameDB::open(MDB_txn* txn)
{
    MDB_dbi dbi = 0;
    int rc = mdb_dbi_open(txn, "idfilename", MDB_INTEGERKEY, &dbi);
    if (rc) {
        qCWarning(ENGINE) << "IdFilenameDB::open" << mdb_strerror(rc);
        return 0;
    }

    return dbi;
}

void IdFilenameDB::put(quint64 docId, const FilePath& path)
{
    Q_ASSERT(docId > 0);
    Q_ASSERT(!path.name.isEmpty());

    MDB_val key;
    key.mv_size = sizeof(quint64);
    key.mv_data = static_cast<void*>(&docId);

    QByteArray data(8 + path.name.size(), Qt::Uninitialized);
    memcpy(data.data(), &path.parentId, 8);
    memcpy(data.data() + 8, path.name.data(), path.name.size());

    MDB_val val;
    val.mv_size = data.size();
    val.mv_data = static_cast<void*>(data.data());

    int rc = mdb_put(m_txn, m_dbi, &key, &val, 0);
    if (rc) {
        qCWarning(ENGINE) << "IdFilenameDB::put" << mdb_strerror(rc);
    }
}

IdFilenameDB::FilePath IdFilenameDB::get(quint64 docId)
{
    FilePath path;
    if (get(docId, path)) {
        return path;
    }
    return {};
}

bool IdFilenameDB::get(quint64 docId, FilePath& path)
{
    Q_ASSERT(docId > 0);

    MDB_val key;
    key.mv_size = sizeof(quint64);
    key.mv_data = static_cast<void*>(&docId);

    MDB_val val{0, nullptr};
    int rc = mdb_get(m_txn, m_dbi, &key, &val);
    if (rc) {
        if (rc != MDB_NOTFOUND) {
            qCDebug(ENGINE) << "IdfilenameDB::get" << docId << mdb_strerror(rc);
        }
        return false;
    }
    if (val.mv_size < 8) {
        return false;
    }

    path.name.resize(val.mv_size - 8);
    memcpy(&path.parentId, val.mv_data, 8);
    memcpy(path.name.data(), static_cast<const char*>(val.mv_data) + 8, path.name.size());

    return true;
}

bool IdFilenameDB::contains(quint64 docId)
{
    Q_ASSERT(docId > 0);

    MDB_val key;
    key.mv_size = sizeof(quint64);
    key.mv_data = static_cast<void*>(&docId);

    MDB_val val{0, nullptr};
    int rc = mdb_get(m_txn, m_dbi, &key, &val);
    if (rc) {
        if (rc != MDB_NOTFOUND) {
            qCDebug(ENGINE) << "IdfilenameDB::contains" << docId << mdb_strerror(rc);
        }
        return false;
    }
    return true;
}

void IdFilenameDB::del(quint64 docId)
{
    Q_ASSERT(docId > 0);

    MDB_val key;
    key.mv_size = sizeof(quint64);
    key.mv_data = static_cast<void*>(&docId);

    int rc = mdb_del(m_txn, m_dbi, &key, nullptr);
    if (rc != 0 && rc != MDB_NOTFOUND) {
        qCDebug(ENGINE) << "IdFilenameDB::del" << mdb_strerror(rc);
    }
}

QMap<quint64, IdFilenameDB::FilePath> IdFilenameDB::toTestMap() const
{
    MDB_cursor* cursor;
    mdb_cursor_open(m_txn, m_dbi, &cursor);

    MDB_val key = {0, nullptr};
    MDB_val val;

    QMap<quint64, FilePath> map;
    while (1) {
        int rc = mdb_cursor_get(cursor, &key, &val, MDB_NEXT);
        if (rc) {
            qCDebug(ENGINE) << "IdFilenameDB::toTestMap" << mdb_strerror(rc);
            break;
        }

        const quint64 id = *(static_cast<quint64*>(key.mv_data));

        FilePath path;
        path.parentId = static_cast<quint64*>(val.mv_data)[0];
        path.name = QByteArray(static_cast<char*>(val.mv_data) + 8, val.mv_size - 8);

        map.insert(id, path);
    }

    mdb_cursor_close(cursor);
    return map;
}
