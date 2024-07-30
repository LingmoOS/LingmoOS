/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_TRANSACTION_H
#define BALOO_TRANSACTION_H

#include "databasedbis.h"
#include "mtimedb.h"
#include "postingdb.h"
#include "writetransaction.h"
#include "documenttimedb.h"
#include <functional>
#include <memory>

#include <lmdb.h>

namespace Baloo {

class Database;
class Document;
class PostingIterator;
class EngineQuery;
class DatabaseSize;
class DBState;

class BALOO_ENGINE_EXPORT Transaction
{
public:
    enum TransactionType {
        ReadOnly,
        ReadWrite,
    };
    Transaction(const Database& db, TransactionType type);
    Transaction(Database* db, TransactionType type);
    ~Transaction();

    //
    // Getters
    //
    bool hasDocument(quint64 id) const;
    bool inPhaseOne(quint64 id) const;
    bool hasFailed(quint64 id) const;
    QVector<quint64> failedIds(quint64 limit) const;
    QByteArray documentUrl(quint64 id) const;

    /**
     * This method is not cheap, and does not stat the filesystem in order to convert the path
     * \p path into an id.
     */
    quint64 documentId(const QByteArray& path) const;
    QByteArray documentData(quint64 id) const;

    DocumentTimeDB::TimeInfo documentTimeInfo(quint64 id) const;

    PostingIterator* postingIterator(const EngineQuery& query) const;
    PostingIterator* postingCompIterator(const QByteArray& prefix, qlonglong value, PostingDB::Comparator com) const;
    PostingIterator* postingCompIterator(const QByteArray& prefix, double value, PostingDB::Comparator com) const;
    PostingIterator* postingCompIterator(const QByteArray& prefix, const QByteArray& value, PostingDB::Comparator com) const;
    PostingIterator* mTimeRangeIter(quint32 beginTime, quint32 endTime) const;
    PostingIterator* docUrlIter(quint64 id) const;

    QVector<quint64> fetchPhaseOneIds(int size) const;
    uint phaseOneSize() const;
    uint size() const;

    QVector<QByteArray> fetchTermsStartingWith(const QByteArray& term) const;

    //
    // Introspecing document data
    //
    QVector<QByteArray> documentTerms(quint64 docId) const;
    QVector<QByteArray> documentFileNameTerms(quint64 docId) const;
    QVector<QByteArray> documentXattrTerms(quint64 docId) const;

    DatabaseSize dbSize();

    //
    // Transaction handling
    //
    bool commit();
    void abort();
    void reset(TransactionType type);

    //
    // Write Methods
    //
    void addDocument(const Document& doc);
    void removeDocument(quint64 id);
    void removeRecursively(quint64 parentId);
    void addFailed(quint64 id);

    bool removeRecursively(quint64 parentId, std::function<bool(quint64)> shouldDelete)
    {
        Q_ASSERT(m_txn);
        Q_ASSERT(m_writeTrans);

        return m_writeTrans->removeRecursively(parentId, shouldDelete);
    }

    void replaceDocument(const Document& doc, DocumentOperations operations);
    void setPhaseOne(quint64 id);
    void removePhaseOne(quint64 id);

    // Debugging
    void checkFsTree();
    void checkTermsDbinPostingDb();
    void checkPostingDbinTermsDb();

private:
    Transaction(const Transaction& rhs) = delete;
    void init(TransactionType type);

    const DatabaseDbis& m_dbis;
    MDB_txn *m_txn = nullptr;
    MDB_env *m_env = nullptr;
    std::unique_ptr<WriteTransaction> m_writeTrans;

    friend class DatabaseSanitizerImpl;
    friend class DBState; // for testing
};
}

#endif // BALOO_TRANSACTION_H
