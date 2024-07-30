/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_POSTINGDB_H
#define BALOO_POSTINGDB_H

#include "postingiterator.h"

#include <QByteArray>
#include <QVector>
#include <QRegularExpression>

#include <lmdb.h>

namespace Baloo {

typedef QVector<quint64> PostingList;

/**
 * The PostingDB is the main database that maps <term> -> <id1> <id2> <id2> ...
 * This is used to lookup ids when searching for a <term>.
 */
class BALOO_ENGINE_EXPORT PostingDB
{
public:
    PostingDB(MDB_dbi, MDB_txn* txn);
    ~PostingDB();

    static MDB_dbi create(MDB_txn* txn);
    static MDB_dbi open(MDB_txn* txn);

    void put(const QByteArray& term, const PostingList& list);
    PostingList get(const QByteArray& term);
    void del(const QByteArray& term);

    PostingIterator* iter(const QByteArray& term);
    PostingIterator* prefixIter(const QByteArray& term);
    PostingIterator* regexpIter(const QRegularExpression& regexp, const QByteArray& prefix);

    enum Comparator {
        LessEqual,
        GreaterEqual,
    };
    // For integral types only:
    template<typename T>
    typename std::enable_if<std::is_integral<T>::value, PostingIterator*>::type
    compIter(const QByteArray& prefix, T val, Comparator com) {
        qlonglong l = val;
        return compIter(prefix, l, com);
    }
    PostingIterator* compIter(const QByteArray& prefix, qlonglong val, Comparator com);
    PostingIterator* compIter(const QByteArray& prefix, double val, Comparator com);
    PostingIterator* compIter(const QByteArray& prefix, const QByteArray& val, Comparator com);

    QVector<QByteArray> fetchTermsStartingWith(const QByteArray& term);

    QMap<QByteArray, PostingList> toTestMap() const;
private:
    template <typename Validator>
    PostingIterator* iter(const QByteArray& prefix, Validator validate);

    MDB_txn* m_txn;
    MDB_dbi m_dbi;
};

}

#endif // BALOO_POSTINGDB_H
