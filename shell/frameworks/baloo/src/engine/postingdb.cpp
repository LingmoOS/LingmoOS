/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "enginedebug.h"
#include "postingdb.h"
#include "orpostingiterator.h"
#include "postingcodec.h"

using namespace Baloo;

PostingDB::PostingDB(MDB_dbi dbi, MDB_txn* txn)
    : m_txn(txn)
    , m_dbi(dbi)
{
    Q_ASSERT(txn != nullptr);
    Q_ASSERT(dbi != 0);
}

PostingDB::~PostingDB()
{
}

MDB_dbi PostingDB::create(MDB_txn* txn)
{
    MDB_dbi dbi = 0;
    int rc = mdb_dbi_open(txn, "postingdb", MDB_CREATE, &dbi);
    if (rc) {
        qCWarning(ENGINE) << "PostingDB::create" << mdb_strerror(rc);
        return 0;
    }

    return dbi;
}

MDB_dbi PostingDB::open(MDB_txn* txn)
{
    MDB_dbi dbi = 0;
    int rc = mdb_dbi_open(txn, "postingdb", 0, &dbi);
    if (rc) {
        qCWarning(ENGINE) << "PostingDB::open" << mdb_strerror(rc);
        return 0;
    }

    return dbi;
}

void PostingDB::put(const QByteArray& term, const PostingList& list)
{
    Q_ASSERT(!term.isEmpty());
    Q_ASSERT(!list.isEmpty());

    MDB_val key;
    key.mv_size = term.size();
    key.mv_data = static_cast<void*>(const_cast<char*>(term.constData()));

    QByteArray arr = PostingCodec::encode(list);

    MDB_val val;
    val.mv_size = arr.size();
    val.mv_data = static_cast<void*>(arr.data());

    int rc = mdb_put(m_txn, m_dbi, &key, &val, 0);
    if (rc) {
        qCWarning(ENGINE) << "PostingDB::put" << mdb_strerror(rc);
    }
}

PostingList PostingDB::get(const QByteArray& term)
{
    Q_ASSERT(!term.isEmpty());

    MDB_val key;
    key.mv_size = term.size();
    key.mv_data = static_cast<void*>(const_cast<char*>(term.constData()));

    MDB_val val{0, nullptr};
    int rc = mdb_get(m_txn, m_dbi, &key, &val);
    if (rc) {
        if (rc != MDB_NOTFOUND) {
            qCDebug(ENGINE) << "PostingDB::get" << term << mdb_strerror(rc);
        }
        return PostingList();
    }

    QByteArray arr = QByteArray::fromRawData(static_cast<char*>(val.mv_data), val.mv_size);

    return PostingCodec::decode(arr);
}

void PostingDB::del(const QByteArray& term)
{
    Q_ASSERT(!term.isEmpty());

    MDB_val key;
    key.mv_size = term.size();
    key.mv_data = static_cast<void*>(const_cast<char*>(term.constData()));

    int rc = mdb_del(m_txn, m_dbi, &key, nullptr);
    if (rc != 0 && rc != MDB_NOTFOUND) {
        qCDebug(ENGINE) << "PostingDB::del" << term << mdb_strerror(rc);
    }
}

QVector< QByteArray > PostingDB::fetchTermsStartingWith(const QByteArray& term)
{
    MDB_val key;
    key.mv_size = term.size();
    key.mv_data = static_cast<void*>(const_cast<char*>(term.constData()));

    MDB_cursor* cursor;
    int rc = mdb_cursor_open(m_txn, m_dbi, &cursor);
    if (rc) {
        qCWarning(ENGINE) << "PostingDB::fetchTermsStartingWith" << mdb_strerror(rc);
        return {};
    }

    QVector<QByteArray> terms;
    rc = mdb_cursor_get(cursor, &key, nullptr, MDB_SET_RANGE);
    while (rc == 0) {
        const QByteArray arr(static_cast<char*>(key.mv_data), key.mv_size);
        if (!arr.startsWith(term)) {
            break;
        }
        terms << arr;
        rc = mdb_cursor_get(cursor, &key, nullptr, MDB_NEXT);
    }
    if (rc != MDB_NOTFOUND) {
        qCDebug(ENGINE) << "PostingDB::fetchTermsStartingWith" << mdb_strerror(rc);
    }

    mdb_cursor_close(cursor);
    return terms;
}

class DBPostingIterator : public PostingIterator {
public:
    DBPostingIterator(void* data, uint size);
    quint64 docId() const override;
    quint64 next() override;

private:
    const QVector<quint64> m_vec;
    int m_pos;
};

PostingIterator* PostingDB::iter(const QByteArray& term)
{
    MDB_val key;
    key.mv_size = term.size();
    key.mv_data = static_cast<void*>(const_cast<char*>(term.constData()));

    MDB_val val;
    int rc = mdb_get(m_txn, m_dbi, &key, &val);
    if (rc) {
        qCDebug(ENGINE) << "PostingDB::iter" << term << mdb_strerror(rc);
        return nullptr;
    }

    return new DBPostingIterator(val.mv_data, val.mv_size);
}

//
// Posting Iterator
//
DBPostingIterator::DBPostingIterator(void* data, uint size)
    : m_vec(PostingCodec().decode(QByteArray(static_cast<char*>(data), size)))
    , m_pos(-1)
{
}

quint64 DBPostingIterator::docId() const
{
    if (m_pos < 0 || m_pos >= m_vec.size()) {
        return 0;
    }

    return m_vec[m_pos];
}

quint64 DBPostingIterator::next()
{
    if (m_pos >= m_vec.size() - 1) {
        m_pos = m_vec.size();
        return 0;
    }

    m_pos++;
    return m_vec[m_pos];
}

template <typename Validator>
PostingIterator* PostingDB::iter(const QByteArray& prefix, Validator validate)
{
    Q_ASSERT(!prefix.isEmpty());

    MDB_val key;
    key.mv_size = prefix.size();
    key.mv_data = static_cast<void*>(const_cast<char*>(prefix.constData()));

    MDB_cursor* cursor;
    int rc = mdb_cursor_open(m_txn, m_dbi, &cursor);

    if (rc) {
        qCWarning(ENGINE) << "PostingDB::regexpIter" << mdb_strerror(rc);
        return nullptr;
    }

    QVector<PostingIterator*> termIterators;

    MDB_val val;
    rc = mdb_cursor_get(cursor, &key, &val, MDB_SET_RANGE);
    while (rc == 0) {
        const QByteArray arr(static_cast<char*>(key.mv_data), key.mv_size);
        if (!arr.startsWith(prefix)) {
            break;
        }
        if (validate(arr)) {
            termIterators << new DBPostingIterator(val.mv_data, val.mv_size);
        }
        rc = mdb_cursor_get(cursor, &key, &val, MDB_NEXT);
    }

    if (rc != 0 && rc != MDB_NOTFOUND) {
        qCWarning(ENGINE) << "PostingDB::regexpIter" << mdb_strerror(rc);
    }

    mdb_cursor_close(cursor);
    if (termIterators.isEmpty()) {
        return nullptr;
    }
    return new OrPostingIterator(termIterators);
}

PostingIterator* PostingDB::prefixIter(const QByteArray& prefix)
{
    auto validate = [] (const QByteArray& arr) {
        Q_UNUSED(arr);
        return true;
    };
    return iter(prefix, validate);
}

PostingIterator* PostingDB::regexpIter(const QRegularExpression& regexp, const QByteArray& prefix)
{
    int prefixLen = prefix.length();
    auto validate = [&regexp, prefixLen] (const QByteArray& arr) {
        QString term = QString::fromUtf8(arr.mid(prefixLen));
        return regexp.match(term).hasMatch();
    };

    return iter(prefix, validate);
}

PostingIterator* PostingDB::compIter(const QByteArray& prefix, qlonglong comVal, PostingDB::Comparator com)
{
    int prefixLen = prefix.length();
    auto validate = [prefixLen, comVal, com] (const QByteArray& arr) {
        bool ok = false;
        auto val = QByteArray::fromRawData(arr.constData() + prefixLen, arr.length() - prefixLen).toLongLong(&ok);
        return ok && ((com == LessEqual && val <= comVal) || (com == GreaterEqual && val >= comVal));
    };
    return iter(prefix, validate);
}

PostingIterator* PostingDB::compIter(const QByteArray& prefix, double comVal, PostingDB::Comparator com)
{
    int prefixLen = prefix.length();
    auto validate = [prefixLen, comVal, com] (const QByteArray& arr) {
        bool ok = false;
        auto val = QByteArray::fromRawData(arr.constData() + prefixLen, arr.length() - prefixLen).toDouble(&ok);
        return ok && ((com == LessEqual && val <= comVal) ||
                      (com == GreaterEqual && val >= comVal));
    };
    return iter(prefix, validate);
}

PostingIterator* PostingDB::compIter(const QByteArray& prefix, const QByteArray& comVal, PostingDB::Comparator com)
{
    int prefixLen = prefix.length();
    auto validate = [prefixLen, comVal, com] (const QByteArray& arr) {
        auto val = QByteArray::fromRawData(arr.constData() + prefixLen, arr.length() - prefixLen);
        return ((com == LessEqual && val <= comVal) ||
                (com == GreaterEqual && val >= comVal));
    };
    return iter(prefix, validate);
}

QMap<QByteArray, PostingList> PostingDB::toTestMap() const
{
    MDB_cursor* cursor;
    mdb_cursor_open(m_txn, m_dbi, &cursor);

    MDB_val key = {0, nullptr};
    MDB_val val;

    QMap<QByteArray, PostingList> map;
    while (1) {
        int rc = mdb_cursor_get(cursor, &key, &val, MDB_NEXT);
        if (rc == MDB_NOTFOUND) {
            break;
        }
        if (rc) {
            qCDebug(ENGINE) << "PostingDB::toTestMap" << mdb_strerror(rc);
            break;
        }

        const QByteArray ba(static_cast<char*>(key.mv_data), key.mv_size);
        const PostingList plist = PostingCodec::decode(QByteArray(static_cast<char*>(val.mv_data), val.mv_size));
        map.insert(ba, plist);
    }

    mdb_cursor_close(cursor);
    return map;
}
