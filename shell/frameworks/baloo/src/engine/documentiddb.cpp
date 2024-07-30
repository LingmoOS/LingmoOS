/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "documentiddb.h"
#include "enginedebug.h"

using namespace Baloo;

DocumentIdDB::DocumentIdDB(MDB_dbi dbi, MDB_txn* txn)
    : m_txn(txn)
    , m_dbi(dbi)
{
    Q_ASSERT(txn != nullptr);
    Q_ASSERT(dbi != 0);
}

DocumentIdDB::~DocumentIdDB()
{
}

MDB_dbi DocumentIdDB::create(const char* name, MDB_txn* txn)
{
    MDB_dbi dbi = 0;
    int rc = mdb_dbi_open(txn, name, MDB_CREATE | MDB_INTEGERKEY, &dbi);
    if (rc) {
        qCWarning(ENGINE) << "DocumentIdDB::create" << name << mdb_strerror(rc);
        return 0;
    }

    return dbi;
}

MDB_dbi DocumentIdDB::open(const char* name, MDB_txn* txn)
{
    MDB_dbi dbi = 0;
    int rc = mdb_dbi_open(txn, name, MDB_INTEGERKEY, &dbi);
    if (rc) {
        qCWarning(ENGINE) << "DocumentIdDB::open" << name << mdb_strerror(rc);
        return 0;
    }

    return dbi;
}

void DocumentIdDB::put(quint64 docId)
{
    Q_ASSERT(docId > 0);

    MDB_val key;
    key.mv_size = sizeof(quint64);
    key.mv_data = static_cast<void*>(&docId);

    MDB_val val;
    val.mv_size = 0;
    val.mv_data = nullptr;

    int rc = mdb_put(m_txn, m_dbi, &key, &val, 0);
    if (rc) {
        qCWarning(ENGINE) << "DocumentIdDB::put" << mdb_strerror(rc);
    }
}

bool DocumentIdDB::contains(quint64 docId)
{
    Q_ASSERT(docId > 0);

    MDB_val key;
    key.mv_size = sizeof(quint64);
    key.mv_data = static_cast<void*>(&docId);

    MDB_val val{0, nullptr};
    int rc = mdb_get(m_txn, m_dbi, &key, &val);
    if (rc) {
        if (rc != MDB_NOTFOUND) {
            qCDebug(ENGINE) << "DocumentIdDB::contains" << docId << mdb_strerror(rc);
        }
        return false;
    }

    return true;
}

void DocumentIdDB::del(quint64 docId)
{
    Q_ASSERT(docId > 0);

    MDB_val key;
    key.mv_size = sizeof(quint64);
    key.mv_data = static_cast<void*>(&docId);

    int rc = mdb_del(m_txn, m_dbi, &key, nullptr);
    if (rc != 0 && rc != MDB_NOTFOUND) {
        qCDebug(ENGINE) << "DocumentIdDB::del" << docId << mdb_strerror(rc);
    }
}

QVector<quint64> DocumentIdDB::fetchItems(int size)
{
    Q_ASSERT(size > 0);

    MDB_cursor* cursor;
    mdb_cursor_open(m_txn, m_dbi, &cursor);

    QVector<quint64> vec;
    vec.reserve(size);

    for (int i = 0; i < size; i++) {
        MDB_val key{0, nullptr};
        int rc = mdb_cursor_get(cursor, &key, nullptr, MDB_NEXT);
        if (rc) {
            if (rc != MDB_NOTFOUND) {
                qCWarning(ENGINE) << "DocumentIdDB::fetchItems" << size << mdb_strerror(rc);
            }
            break;
        }

        quint64 id = *(static_cast<quint64*>(key.mv_data));
        vec << id;
    }
    mdb_cursor_close(cursor);

    return vec;
}

uint DocumentIdDB::size()
{
    MDB_stat stat;
    int rc = mdb_stat(m_txn, m_dbi, &stat);
    if (rc) {
        qCDebug(ENGINE) << "DocumentIdDB::size" << mdb_strerror(rc);
        return 0;
    }

    return stat.ms_entries;
}

QVector<quint64> DocumentIdDB::toTestVector() const
{
    MDB_cursor* cursor;
    mdb_cursor_open(m_txn, m_dbi, &cursor);

    MDB_val key{0, nullptr};
    MDB_val val{0, nullptr};

    QVector<quint64> vec;
    while (1) {
        int rc = mdb_cursor_get(cursor, &key, &val, MDB_NEXT);
        if (rc) {
            qCDebug(ENGINE) << "DocumentTimeDB::toTestMap" << mdb_strerror(rc);
            break;
        }

        const quint64 id = *(static_cast<quint64*>(key.mv_data));
        vec << id;
    }

    mdb_cursor_close(cursor);
    return vec;
}
