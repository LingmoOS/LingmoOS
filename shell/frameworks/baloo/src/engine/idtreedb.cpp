/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "idtreedb.h"
#include "enginedebug.h"
#include "postingiterator.h"

#include <algorithm>

using namespace Baloo;

IdTreeDB::IdTreeDB(MDB_dbi dbi, MDB_txn* txn)
    : m_txn(txn)
    , m_dbi(dbi)
{
    Q_ASSERT(txn != nullptr);
    Q_ASSERT(dbi != 0);
}

MDB_dbi IdTreeDB::create(MDB_txn* txn)
{
    MDB_dbi dbi = 0;
    int rc = mdb_dbi_open(txn, "idtree", MDB_CREATE | MDB_INTEGERKEY, &dbi);
    if (rc) {
        qCWarning(ENGINE) << "IdTreeDB::create" << mdb_strerror(rc);
        return 0;
    }

    return dbi;
}

MDB_dbi IdTreeDB::open(MDB_txn* txn)
{
    MDB_dbi dbi = 0;
    int rc = mdb_dbi_open(txn, "idtree", MDB_INTEGERKEY, &dbi);
    if (rc) {
        qCWarning(ENGINE) << "IdTreeDB::open" << mdb_strerror(rc);
        return 0;
    }

    return dbi;
}

void IdTreeDB::set(quint64 docId, const QVector<quint64> &subDocIds)
{
    Q_ASSERT(!subDocIds.contains(0));

    MDB_val key;
    key.mv_size = sizeof(quint64);
    key.mv_data = static_cast<void*>(&docId);

    int rc;
    if (subDocIds.empty()) {
        rc = mdb_del(m_txn, m_dbi, &key, nullptr);
        if (rc == MDB_NOTFOUND) {
            rc = 0;
        }
    } else {
        MDB_val val;
        val.mv_size = subDocIds.size() * sizeof(quint64);
        val.mv_data = static_cast<void*>(const_cast<quint64*>(subDocIds.constData()));

        rc = mdb_put(m_txn, m_dbi, &key, &val, 0);
    }
    if (rc) {
        qCWarning(ENGINE) << "IdTreeDB::set" << mdb_strerror(rc);
    }
}

QVector<quint64> IdTreeDB::get(quint64 docId)
{
    MDB_val key;
    key.mv_size = sizeof(quint64);
    key.mv_data = static_cast<void*>(&docId);

    MDB_val val{0, nullptr};
    int rc = mdb_get(m_txn, m_dbi, &key, &val);
    if (rc) {
        if (rc != MDB_NOTFOUND) {
            qCDebug(ENGINE) << "IdTreeDB::get" << docId << mdb_strerror(rc);
        }
        return QVector<quint64>();
    }

    // FIXME: This still makes a copy of the data. Perhaps we can avoid that?
    QVector<quint64> list(val.mv_size / sizeof(quint64));
    memcpy(list.data(), val.mv_data, val.mv_size);

    return list;
}

//
// Iter
//
class IdTreePostingIterator : public PostingIterator {
public:
    IdTreePostingIterator(const IdTreeDB& db, const QVector<quint64> &list)
        : m_db(db), m_pos(-1), m_idList(list) {}

    quint64 docId() const override {
        if (m_pos >= 0 && m_pos < m_resultList.size()) {
            return m_resultList[m_pos];
        }
        return 0;
    }

    quint64 next() override {
        if (m_resultList.isEmpty() && m_idList.isEmpty()) {
            return 0;
        }

        if (m_resultList.isEmpty()) {
            while (!m_idList.isEmpty()) {
                quint64 id = m_idList.takeLast();
                m_idList << m_db.get(id);
                m_resultList << id;
            }
            std::sort(m_resultList.begin(), m_resultList.end());
            m_pos = 0;
        }
        else {
            if (m_pos < m_resultList.size()) {
                m_pos++;
            } else {
                m_resultList.clear();
            }
        }

        if (m_pos < m_resultList.size()) {
            return m_resultList[m_pos];
        } else {
            return 0;
        }
    }

private:
    IdTreeDB m_db;
    int m_pos;
    QVector<quint64> m_idList;
    QVector<quint64> m_resultList;
};

PostingIterator* IdTreeDB::iter(quint64 docId)
{
    Q_ASSERT(docId > 0);

    QVector<quint64> list = {docId};
    return new IdTreePostingIterator(*this, list);
}

QMap<quint64, QVector<quint64>> IdTreeDB::toTestMap() const
{
    MDB_cursor* cursor;
    mdb_cursor_open(m_txn, m_dbi, &cursor);

    MDB_val key = {0, nullptr};
    MDB_val val;

    QMap<quint64, QVector<quint64>> map;
    while (1) {
        int rc = mdb_cursor_get(cursor, &key, &val, MDB_NEXT);
        if (rc == MDB_NOTFOUND) {
            break;
        }
        if (rc) {
            qCDebug(ENGINE) << "IdTreeDB::toTestMap" << mdb_strerror(rc);
            break;
        }

        const quint64 id = *(static_cast<quint64*>(key.mv_data));

        QVector<quint64> list(val.mv_size / sizeof(quint64));
        memcpy(list.data(), val.mv_data, val.mv_size);

        map.insert(id, list);
    }

    mdb_cursor_close(cursor);
    return map;
}
