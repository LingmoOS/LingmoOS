/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_WRITETRANSACTION_H
#define BALOO_WRITETRANSACTION_H

#include "positioninfo.h"
#include "document.h"
#include "documentoperations.h"
#include "databasedbis.h"
#include "documenturldb.h"
#include <functional>

namespace Baloo {

class BALOO_ENGINE_EXPORT WriteTransaction
{
public:
    WriteTransaction(DatabaseDbis dbis, MDB_txn* txn)
        : m_txn(txn)
        , m_dbis(dbis)
    {}

    void addDocument(const Document& doc);
    void removeDocument(quint64 id);

    /**
     * Remove the document with id \p parentId and all its children.
     */
    void removeRecursively(quint64 parentId);

    /**
     * Goes through every document in the database, and remove the ones for which \p shouldDelete
     * returns false. It starts searching from \p parentId, which can be 0 to search
     * through everything.
     *
     * \arg shouldDelete takes a quint64 as a parameter
     * \ret true if the document (and all its children) has been removed
     *
     * This function should typically be called when there are no other ReadTransaction in process
     * as that would otherwise balloon the size of the database.
     */
    bool removeRecursively(quint64 parentId, const std::function<bool(quint64)> &shouldDelete);

    void replaceDocument(const Document& doc, DocumentOperations operations);
    void commit();

    enum OperationType {
        AddId,
        RemoveId,
    };
    struct Operation {
        OperationType type;
        PositionInfo data;
    };

private:
    /*
     * Adds an 'addId' operation to the pending queue for each term.
     * Returns the list of all the terms.
     */
    BALOO_ENGINE_NO_EXPORT QVector<QByteArray> addTerms(quint64 id, const QMap<QByteArray, Document::TermData>& terms);
    BALOO_ENGINE_NO_EXPORT QVector<QByteArray> replaceTerms(quint64 id, const QVector<QByteArray>& prevTerms,
                                     const QMap<QByteArray, Document::TermData>& terms);
    BALOO_ENGINE_NO_EXPORT void removeTerms(quint64 id, const QVector<QByteArray>& terms);

    QHash<QByteArray, QVector<Operation> > m_pendingOperations;

    MDB_txn* m_txn;
    DatabaseDbis m_dbis;
};
}

Q_DECLARE_TYPEINFO(Baloo::WriteTransaction::Operation, Q_RELOCATABLE_TYPE);

#endif // BALOO_WRITETRANSACTION_H
