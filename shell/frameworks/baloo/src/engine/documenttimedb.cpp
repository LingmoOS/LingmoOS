/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "documenttimedb.h"
#include "enginedebug.h"

using namespace Baloo;

DocumentTimeDB::DocumentTimeDB(MDB_dbi dbi, MDB_txn* txn)
    : m_txn(txn)
    , m_dbi(dbi)
{
    Q_ASSERT(txn != nullptr);
    Q_ASSERT(dbi != 0);
}

DocumentTimeDB::~DocumentTimeDB()
{
}

MDB_dbi DocumentTimeDB::create(MDB_txn* txn)
{
    MDB_dbi dbi = 0;
    int rc = mdb_dbi_open(txn, "documenttimedb", MDB_CREATE | MDB_INTEGERKEY, &dbi);
    if (rc) {
        qCWarning(ENGINE) << "DocumentTimeDB::create" << mdb_strerror(rc);
        return 0;
    }

    return dbi;
}

MDB_dbi DocumentTimeDB::open(MDB_txn* txn)
{
    MDB_dbi dbi = 0;
    int rc = mdb_dbi_open(txn, "documenttimedb", MDB_INTEGERKEY, &dbi);
    if (rc) {
        qCWarning(ENGINE) << "DocumentTimeDB::open" << mdb_strerror(rc);
        return 0;
    }

    return dbi;
}

void DocumentTimeDB::put(quint64 docId, const TimeInfo& info)
{
    Q_ASSERT(docId > 0);

    MDB_val key;
    key.mv_size = sizeof(quint64);
    key.mv_data = &docId;

    MDB_val val;
    val.mv_size = sizeof(TimeInfo);
    val.mv_data = static_cast<void*>(const_cast<TimeInfo*>(&info));

    int rc = mdb_put(m_txn, m_dbi, &key, &val, 0);
    if (rc) {
        qCWarning(ENGINE) << "DocumentTimeDB::put" << docId << mdb_strerror(rc);
    }
}

DocumentTimeDB::TimeInfo DocumentTimeDB::get(quint64 docId)
{
    Q_ASSERT(docId > 0);

    MDB_val key;
    key.mv_size = sizeof(quint64);
    key.mv_data = &docId;

    MDB_val val{0, nullptr};
    int rc = mdb_get(m_txn, m_dbi, &key, &val);
    if (rc) {
        if (rc != MDB_NOTFOUND) {
            qCDebug(ENGINE) << "DocumentTimeDB::get" << docId << mdb_strerror(rc);
        }
        return TimeInfo();
    }

    return *(static_cast<TimeInfo*>(val.mv_data));
}

void DocumentTimeDB::del(quint64 docId)
{
    Q_ASSERT(docId > 0);

    MDB_val key;
    key.mv_size = sizeof(quint64);
    key.mv_data = static_cast<void*>(&docId);

    int rc = mdb_del(m_txn, m_dbi, &key, nullptr);
    if (rc != 0 && rc != MDB_NOTFOUND) {
        qCDebug(ENGINE) << "DocumentTimeDB::del" << docId << mdb_strerror(rc);
    }
}

bool DocumentTimeDB::contains(quint64 docId)
{
    Q_ASSERT(docId > 0);

    MDB_val key;
    key.mv_size = sizeof(quint64);
    key.mv_data = static_cast<void*>(&docId);

    MDB_val val;
    int rc = mdb_get(m_txn, m_dbi, &key, &val);
    if (rc) {
        if (rc != MDB_NOTFOUND) {
            qCDebug(ENGINE) << "DocumentTimeDB::contains" << docId << mdb_strerror(rc);
        }
        return false;
    }

    return true;
}

QMap<quint64, DocumentTimeDB::TimeInfo> DocumentTimeDB::toTestMap() const
{
    MDB_cursor* cursor;
    mdb_cursor_open(m_txn, m_dbi, &cursor);

    MDB_val key = {0, nullptr};
    MDB_val val;

    QMap<quint64, TimeInfo> map;
    while (1) {
        int rc = mdb_cursor_get(cursor, &key, &val, MDB_NEXT);
        if (rc) {
            qCDebug(ENGINE) << "DocumentTimeDB::toTestMap" << mdb_strerror(rc);
            break;
        }

        const quint64 id = *(static_cast<quint64*>(key.mv_data));
        const TimeInfo ti = *(static_cast<TimeInfo*>(val.mv_data));
        map.insert(id, ti);
    }

    mdb_cursor_close(cursor);
    return map;
}
