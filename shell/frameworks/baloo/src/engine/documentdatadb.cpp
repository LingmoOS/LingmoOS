/*
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "documentdatadb.h"
#include "enginedebug.h"

using namespace Baloo;

DocumentDataDB::DocumentDataDB(MDB_dbi dbi, MDB_txn* txn)
    : m_txn(txn)
    , m_dbi(dbi)
{
    Q_ASSERT(txn != nullptr);
    Q_ASSERT(dbi != 0);
}

DocumentDataDB::~DocumentDataDB()
{
}

MDB_dbi DocumentDataDB::create(MDB_txn* txn)
{
    MDB_dbi dbi = 0;
    int rc = mdb_dbi_open(txn, "documentdatadb", MDB_CREATE | MDB_INTEGERKEY, &dbi);
    if (rc) {
        qCWarning(ENGINE) << "DocumentDataDB::create" << mdb_strerror(rc);
        return 0;
    }

    return dbi;
}

MDB_dbi DocumentDataDB::open(MDB_txn* txn)
{
    MDB_dbi dbi = 0;
    int rc = mdb_dbi_open(txn, "documentdatadb", MDB_INTEGERKEY, &dbi);
    if (rc) {
        qCWarning(ENGINE) << "DocumentDataDB::open" << mdb_strerror(rc);
        return 0;
    }

    return dbi;
}

void DocumentDataDB::put(quint64 docId, const QByteArray& url)
{
    Q_ASSERT(docId > 0);
    Q_ASSERT(!url.isEmpty());

    MDB_val key;
    key.mv_size = sizeof(quint64);
    key.mv_data = static_cast<void*>(&docId);

    MDB_val val;
    val.mv_size = url.size();
    val.mv_data = static_cast<void*>(const_cast<char*>(url.constData()));

    int rc = mdb_put(m_txn, m_dbi, &key, &val, 0);
    if (rc) {
        qCWarning(ENGINE) << "DocumentDataDB::put" << mdb_strerror(rc);
    }
}

QByteArray DocumentDataDB::get(quint64 docId)
{
    Q_ASSERT(docId > 0);

    MDB_val key;
    key.mv_size = sizeof(quint64);
    key.mv_data = static_cast<void*>(&docId);

    MDB_val val{0, nullptr};
    int rc = mdb_get(m_txn, m_dbi, &key, &val);
    if (rc) {
        if (rc != MDB_NOTFOUND) {
            qCDebug(ENGINE) << "DocumentDataDB::get" << docId << mdb_strerror(rc);
        }
        return QByteArray();
    }

    return QByteArray(static_cast<char*>(val.mv_data), val.mv_size);
}

void DocumentDataDB::del(quint64 docId)
{
    Q_ASSERT(docId > 0);

    MDB_val key;
    key.mv_size = sizeof(quint64);
    key.mv_data = static_cast<void*>(&docId);

    int rc = mdb_del(m_txn, m_dbi, &key, nullptr);
    if (rc != 0 && rc != MDB_NOTFOUND) {
        qCDebug(ENGINE) << "DocumentDataDB::del" << docId << mdb_strerror(rc);
    }
}

bool DocumentDataDB::contains(quint64 docId)
{
    Q_ASSERT(docId > 0);

    MDB_val key;
    key.mv_size = sizeof(quint64);
    key.mv_data = static_cast<void*>(&docId);

    MDB_val val{0, nullptr};
    int rc = mdb_get(m_txn, m_dbi, &key, &val);
    if (rc) {
        if (rc != MDB_NOTFOUND) {
            qCDebug(ENGINE) << "DocumentDataDB::contains" << docId << mdb_strerror(rc);
        }
        return false;
    }

    return true;
}

QMap<quint64, QByteArray> DocumentDataDB::toTestMap() const
{
    MDB_cursor* cursor;
    mdb_cursor_open(m_txn, m_dbi, &cursor);

    MDB_val key = {0, nullptr};
    MDB_val val;

    QMap<quint64, QByteArray> map;
    while (1) {
        int rc = mdb_cursor_get(cursor, &key, &val, MDB_NEXT);
        if (rc) {
            qCDebug(ENGINE) << "DocumentDataDB::toTestMap" << mdb_strerror(rc);
            break;
        }

        const quint64 id = *(static_cast<quint64*>(key.mv_data));
        const QByteArray ba(static_cast<char*>(val.mv_data), val.mv_size);
        map.insert(id, ba);
    }

    mdb_cursor_close(cursor);
    return map;
}
