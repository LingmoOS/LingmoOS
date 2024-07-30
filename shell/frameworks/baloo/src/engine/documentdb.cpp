/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "documentdb.h"
#include "doctermscodec.h"
#include "enginedebug.h"

using namespace Baloo;

DocumentDB::DocumentDB(MDB_dbi dbi, MDB_txn* txn)
    : m_txn(txn)
    , m_dbi(dbi)
{
    Q_ASSERT(txn != nullptr);
    Q_ASSERT(dbi != 0);
}

DocumentDB::~DocumentDB()
{
}

MDB_dbi DocumentDB::create(const char* name, MDB_txn* txn)
{
    MDB_dbi dbi = 0;
    const int rc = mdb_dbi_open(txn, name, MDB_CREATE | MDB_INTEGERKEY, &dbi);
    if (rc) {
        qCWarning(ENGINE) << "DocumentDB::create" << name << mdb_strerror(rc);
        return 0;
    }

    return dbi;
}

MDB_dbi DocumentDB::open(const char* name, MDB_txn* txn)
{
    MDB_dbi dbi = 0;
    const int rc = mdb_dbi_open(txn, name, MDB_INTEGERKEY, &dbi);
    if (rc) {
        qCWarning(ENGINE) << "DocumentDB::open" << name << mdb_strerror(rc);
        return 0;
    }

    return dbi;
}

void DocumentDB::put(quint64 docId, const QVector<QByteArray>& list)
{
    Q_ASSERT(docId > 0);
    Q_ASSERT(!list.isEmpty());

    MDB_val key;
    key.mv_size = sizeof(quint64);
    key.mv_data = static_cast<void*>(&docId);

    QByteArray arr = DocTermsCodec::encode(list);

    MDB_val val;
    val.mv_size = arr.size();
    val.mv_data = static_cast<void*>(arr.data());

    int rc = mdb_put(m_txn, m_dbi, &key, &val, 0);
    if (rc) {
        qCWarning(ENGINE) << "DocumentDB::put" << mdb_strerror(rc);
    }
}

QVector<QByteArray> DocumentDB::get(quint64 docId)
{
    Q_ASSERT(docId > 0);

    MDB_val key;
    key.mv_size = sizeof(quint64);
    key.mv_data = static_cast<void*>(&docId);

    MDB_val val{0, nullptr};
    int rc = mdb_get(m_txn, m_dbi, &key, &val);
    if (rc) {
        qCDebug(ENGINE) << "DocumentDB::get" << docId << mdb_strerror(rc);
        return QVector<QByteArray>();
    }

    QByteArray arr = QByteArray::fromRawData(static_cast<char*>(val.mv_data), val.mv_size);

    auto result = DocTermsCodec::decode(arr);
    if (result.isEmpty()) {
        qCDebug(ENGINE) << "Document Terms DB contains corrupt data for " << docId;
    }
    return result;
}

void DocumentDB::del(quint64 docId)
{
    Q_ASSERT(docId > 0);

    MDB_val key;
    key.mv_size = sizeof(quint64);
    key.mv_data = static_cast<void*>(&docId);

    int rc = mdb_del(m_txn, m_dbi, &key, nullptr);
    if (rc != 0 && rc != MDB_NOTFOUND) {
        qCDebug(ENGINE) << "DocumentDB::del" << docId << mdb_strerror(rc);
    }
}

bool DocumentDB::contains(quint64 docId)
{
    Q_ASSERT(docId > 0);

    MDB_val key;
    key.mv_size = sizeof(quint64);
    key.mv_data = static_cast<void*>(&docId);

    MDB_val val;
    int rc = mdb_get(m_txn, m_dbi, &key, &val);
    if (rc) {
        if (rc != MDB_NOTFOUND) {
            qCDebug(ENGINE) << "DocumentDB::contains" << docId << mdb_strerror(rc);
        }
        return false;
    }

    return true;
}

uint DocumentDB::size()
{
    MDB_stat stat;
    int rc = mdb_stat(m_txn, m_dbi, &stat);
    if (rc) {
        qCDebug(ENGINE) << "DocumentDB::size" << mdb_strerror(rc);
        return 0;
    }

    return stat.ms_entries;
}

QMap<quint64, QVector<QByteArray>> DocumentDB::toTestMap() const
{
    MDB_cursor* cursor;
    mdb_cursor_open(m_txn, m_dbi, &cursor);

    MDB_val key = {0, nullptr};
    MDB_val val;

    QMap<quint64, QVector<QByteArray>> map;
    while (1) {
        int rc = mdb_cursor_get(cursor, &key, &val, MDB_NEXT);
        if (rc == MDB_NOTFOUND) {
            break;
        }
        if (rc) {
            qCWarning(ENGINE) << "DocumentDB::toTestMap" << mdb_strerror(rc);
            break;
        }

        const quint64 id = *(static_cast<quint64*>(key.mv_data));
        const QVector<QByteArray> vec = DocTermsCodec::decode(QByteArray(static_cast<char*>(val.mv_data), val.mv_size));
        map.insert(id, vec);
    }

    mdb_cursor_close(cursor);
    return map;
}
