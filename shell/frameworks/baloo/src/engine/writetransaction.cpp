/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "writetransaction.h"
#include "transaction.h"

#include "postingdb.h"
#include "documentdb.h"
#include "documentiddb.h"
#include "positiondb.h"
#include "documenttimedb.h"
#include "documentdatadb.h"
#include "mtimedb.h"
#include "idutils.h"

using namespace Baloo;

void WriteTransaction::addDocument(const Document& doc)
{
    quint64 id = doc.id();

    DocumentDB documentTermsDB(m_dbis.docTermsDbi, m_txn);
    DocumentDB documentXattrTermsDB(m_dbis.docXattrTermsDbi, m_txn);
    DocumentDB documentFileNameTermsDB(m_dbis.docFilenameTermsDbi, m_txn);
    DocumentTimeDB docTimeDB(m_dbis.docTimeDbi, m_txn);
    DocumentDataDB docDataDB(m_dbis.docDataDbi, m_txn);
    DocumentIdDB contentIndexingDB(m_dbis.contentIndexingDbi, m_txn);
    MTimeDB mtimeDB(m_dbis.mtimeDbi, m_txn);
    DocumentUrlDB docUrlDB(m_dbis.idTreeDbi, m_dbis.idFilenameDbi, m_txn);

    Q_ASSERT(!documentTermsDB.contains(id));
    Q_ASSERT(!documentXattrTermsDB.contains(id));
    Q_ASSERT(!documentFileNameTermsDB.contains(id));
    Q_ASSERT(!docTimeDB.contains(id));
    Q_ASSERT(!docDataDB.contains(id));
    Q_ASSERT(!contentIndexingDB.contains(id));
    Q_ASSERT(doc.parentId());

    {
        auto url = doc.url();
        int pos = url.lastIndexOf('/');
        auto filename = url.mid(pos + 1);
        auto parentId = doc.parentId();

        if (pos > 0) {
            docUrlDB.addPath(url.left(pos));
        } else {
            parentId = 0;
        }

        if (!docUrlDB.put(id, parentId, filename)) {
            return;
        }
    }

    QVector<QByteArray> docTerms = addTerms(id, doc.m_terms);
    Q_ASSERT(!docTerms.empty());
    documentTermsDB.put(id, docTerms);

    QVector<QByteArray> docXattrTerms = addTerms(id, doc.m_xattrTerms);
    if (!docXattrTerms.isEmpty()) {
        documentXattrTermsDB.put(id, docXattrTerms);
    }

    QVector<QByteArray> docFileNameTerms = addTerms(id, doc.m_fileNameTerms);
    if (!docFileNameTerms.isEmpty()) {
        documentFileNameTermsDB.put(id, docFileNameTerms);
    }

    if (doc.contentIndexing()) {
        contentIndexingDB.put(doc.id());
    }

    DocumentTimeDB::TimeInfo info;
    info.mTime = doc.m_mTime;
    info.cTime = doc.m_cTime;

    docTimeDB.put(id, info);
    mtimeDB.put(doc.m_mTime, id);

    if (!doc.m_data.isEmpty()) {
        docDataDB.put(id, doc.m_data);
    }
}

QVector<QByteArray> WriteTransaction::addTerms(quint64 id, const QMap<QByteArray, Document::TermData>& terms)
{
    QVector<QByteArray> termList;
    termList.reserve(terms.size());
    m_pendingOperations.reserve(m_pendingOperations.size() + terms.size());

    for (auto it = terms.cbegin(), end = terms.cend(); it != end; ++it) {
        const QByteArray& term = it.key();
        termList.append(term);

        Operation op;
        op.type = AddId;
        op.data.docId = id;
        op.data.positions = it.value().positions;

        m_pendingOperations[term].append(op);
    }

    return termList;
}

void WriteTransaction::removeDocument(quint64 id)
{
    DocumentDB documentTermsDB(m_dbis.docTermsDbi, m_txn);
    DocumentDB documentXattrTermsDB(m_dbis.docXattrTermsDbi, m_txn);
    DocumentDB documentFileNameTermsDB(m_dbis.docFilenameTermsDbi, m_txn);
    DocumentTimeDB docTimeDB(m_dbis.docTimeDbi, m_txn);
    DocumentDataDB docDataDB(m_dbis.docDataDbi, m_txn);
    DocumentIdDB contentIndexingDB(m_dbis.contentIndexingDbi, m_txn);
    DocumentIdDB failedIndexingDB(m_dbis.failedIdDbi, m_txn);
    MTimeDB mtimeDB(m_dbis.mtimeDbi, m_txn);
    DocumentUrlDB docUrlDB(m_dbis.idTreeDbi, m_dbis.idFilenameDbi, m_txn);

    removeTerms(id, documentTermsDB.get(id));
    removeTerms(id, documentFileNameTermsDB.get(id));
    if (documentXattrTermsDB.contains(id)) {
        removeTerms(id, documentXattrTermsDB.get(id));
    }

    documentTermsDB.del(id);
    documentXattrTermsDB.del(id);
    documentFileNameTermsDB.del(id);

    docUrlDB.del(id);

    contentIndexingDB.del(id);
    failedIndexingDB.del(id);

    DocumentTimeDB::TimeInfo info = docTimeDB.get(id);
    docTimeDB.del(id);
    mtimeDB.del(info.mTime, id);

    docDataDB.del(id);
}

void WriteTransaction::removeTerms(quint64 id, const QVector<QByteArray>& terms)
{
    for (const QByteArray& term : terms) {
        Operation op;
        op.type = RemoveId;
        op.data.docId = id;

        m_pendingOperations[term].append(op);
    }
}

void WriteTransaction::removeRecursively(quint64 parentId)
{
    DocumentUrlDB docUrlDB(m_dbis.idTreeDbi, m_dbis.idFilenameDbi, m_txn);

    const QVector<quint64> children = docUrlDB.getChildren(parentId);
    for (quint64 id : children) {
        if (id) {
            removeRecursively(id);
        }
    }
    removeDocument(parentId);
}

bool WriteTransaction::removeRecursively(quint64 parentId, const std::function<bool(quint64)> &shouldDelete)
{
    DocumentUrlDB docUrlDB(m_dbis.idTreeDbi, m_dbis.idFilenameDbi, m_txn);

    if (parentId && !shouldDelete(parentId)) {
        return false;
    }

    bool isEmpty = true;
    const QVector<quint64> children = docUrlDB.getChildren(parentId);
    for (quint64 id : children) {
        isEmpty &= removeRecursively(id, shouldDelete);
    }
    // refetch
    if (isEmpty && docUrlDB.getChildren(parentId).isEmpty()) {
        removeDocument(parentId);
        return true;
    }
    return false;
}

void WriteTransaction::replaceDocument(const Document& doc, DocumentOperations operations)
{
    DocumentDB documentTermsDB(m_dbis.docTermsDbi, m_txn);
    DocumentDB documentXattrTermsDB(m_dbis.docXattrTermsDbi, m_txn);
    DocumentDB documentFileNameTermsDB(m_dbis.docFilenameTermsDbi, m_txn);
    DocumentTimeDB docTimeDB(m_dbis.docTimeDbi, m_txn);
    DocumentDataDB docDataDB(m_dbis.docDataDbi, m_txn);
    DocumentIdDB contentIndexingDB(m_dbis.contentIndexingDbi, m_txn);
    MTimeDB mtimeDB(m_dbis.mtimeDbi, m_txn);
    DocumentUrlDB docUrlDB(m_dbis.idTreeDbi, m_dbis.idFilenameDbi, m_txn);

    const quint64 id = doc.id();

    if (operations & DocumentTerms) {
        Q_ASSERT(!doc.m_terms.isEmpty());
        QVector<QByteArray> prevTerms = documentTermsDB.get(id);
        QVector<QByteArray> docTerms = replaceTerms(id, prevTerms, doc.m_terms);

        if (docTerms != prevTerms) {
            documentTermsDB.put(id, docTerms);
        }
    }

    if (operations & XAttrTerms) {
        QVector<QByteArray> prevTerms = documentXattrTermsDB.get(id);
        QVector<QByteArray> docXattrTerms = replaceTerms(id, prevTerms, doc.m_xattrTerms);

        if (docXattrTerms != prevTerms) {
            if (!docXattrTerms.isEmpty()) {
                documentXattrTermsDB.put(id, docXattrTerms);
            } else {
                documentXattrTermsDB.del(id);
            }
        }
    }

    if (operations & FileNameTerms) {
        QVector<QByteArray> prevTerms = documentFileNameTermsDB.get(id);
        QVector<QByteArray> docFileNameTerms = replaceTerms(id, prevTerms, doc.m_fileNameTerms);

        if (docFileNameTerms != prevTerms) {
            if (!docFileNameTerms.isEmpty()) {
                documentFileNameTermsDB.put(id, docFileNameTerms);
            } else {
                documentFileNameTermsDB.del(id);
            }
        }
    }

    if (doc.contentIndexing()) {
        contentIndexingDB.put(doc.id());
    }

    if (operations & DocumentTime) {
        DocumentTimeDB::TimeInfo info = docTimeDB.get(id);
        if (info.mTime != doc.m_mTime) {
            mtimeDB.del(info.mTime, id);
            mtimeDB.put(doc.m_mTime, id);
        }

        info.mTime = doc.m_mTime;
        info.cTime = doc.m_cTime;
        docTimeDB.put(id, info);
    }

    if (operations & DocumentData) {
        if (!doc.m_data.isEmpty()) {
            docDataDB.put(id, doc.m_data);
        } else {
            docDataDB.del(id);
        }
    }

    if (operations & DocumentUrl) {
        auto url = doc.url();
        int pos = url.lastIndexOf('/');
        auto newname = url.mid(pos + 1);
        docUrlDB.updateUrl(doc.id(), doc.parentId(), newname);
    }
}

QVector< QByteArray > WriteTransaction::replaceTerms(quint64 id, const QVector<QByteArray>& prevTerms,
                                                     const QMap<QByteArray, Document::TermData>& terms)
{
    m_pendingOperations.reserve(m_pendingOperations.size() + prevTerms.size() + terms.size());
    for (const QByteArray& term : prevTerms) {
        Operation op;
        op.type = RemoveId;
        op.data.docId = id;

        m_pendingOperations[term].append(op);
    }

    return addTerms(id, terms);
}

void WriteTransaction::commit()
{
    PostingDB postingDB(m_dbis.postingDbi, m_txn);
    PositionDB positionDB(m_dbis.positionDBi, m_txn);

    QHashIterator<QByteArray, QVector<Operation> > iter(m_pendingOperations);
    while (iter.hasNext()) {
        iter.next();

        const QByteArray& term = iter.key();
        const QVector<Operation> operations = iter.value();

        PostingList list = postingDB.get(term);

        bool fetchedPositionList = false;
        QVector<PositionInfo> positionList;

        for (const Operation& op : operations) {
            quint64 id = op.data.docId;

            if (op.type == AddId) {
                sortedIdInsert(list, id);

                if (!op.data.positions.isEmpty()) {
                    if (!fetchedPositionList) {
                        positionList = positionDB.get(term);
                        fetchedPositionList = true;
                    }
                    sortedIdInsert(positionList, op.data);
                }
            }
            else {
                sortedIdRemove(list, id);
                if (!fetchedPositionList) {
                    positionList = positionDB.get(term);
                    fetchedPositionList = true;
                }
                sortedIdRemove(positionList, PositionInfo(id));
            }
        }

        if (!list.isEmpty()) {
            postingDB.put(term, list);
        } else {
            postingDB.del(term);
        }

        if (fetchedPositionList) {
            if (!positionList.isEmpty()) {
                positionDB.put(term, positionList);
            } else {
                positionDB.del(term);
            }
        }
    }

    m_pendingOperations.clear();
}
