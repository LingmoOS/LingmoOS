/*
    This file is part of the KDE Project
    SPDX-FileCopyrightText: 2009-2011 Sebastian Trueg <trueg@kde.org>
    SPDX-FileCopyrightText: 2013-2014 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "metadatamover.h"
#include "database.h"
#include "termgenerator.h"
#include "transaction.h"
#include "baloodebug.h"

#include <QFile>

using namespace Baloo;

MetadataMover::MetadataMover(Database* db, QObject* parent)
    : QObject(parent)
    , m_db(db)
{
}

MetadataMover::~MetadataMover()
{
}

void MetadataMover::moveFileMetadata(const QString& from, const QString& to)
{
//    qCDebug(BALOO) << from << to;
    Q_ASSERT(!from.isEmpty() && from != QLatin1String("/"));
    Q_ASSERT(!to.isEmpty() && to != QLatin1String("/"));

    Transaction tr(m_db, Transaction::ReadWrite);

    // We do NOT get deleted messages for overwritten files! Thus, we
    // have to remove all metadata for overwritten files first.
    removeMetadata(&tr, to);

    // and finally update the old statements
    updateMetadata(&tr, from, to);

    tr.commit();
}

void MetadataMover::removeFileMetadata(const QString& file)
{
    Q_ASSERT(!file.isEmpty() && file != QLatin1String("/"));

    Transaction tr(m_db, Transaction::ReadWrite);
    removeMetadata(&tr, file);
    tr.commit();
}

void MetadataMover::removeMetadata(Transaction* tr, const QString& url)
{
    Q_ASSERT(!url.isEmpty());

    quint64 id = tr->documentId(QFile::encodeName(url));
    if (!id) {
        Q_EMIT fileRemoved(url);
        return;
    }

    bool isDir = url.endsWith(QLatin1Char('/'));
    if (!isDir) {
        tr->removeDocument(id);
    } else {
        tr->removeRecursively(id);
    }

    Q_EMIT fileRemoved(url);
}

void MetadataMover::updateMetadata(Transaction* tr, const QString& from, const QString& to)
{
    qCDebug(BALOO) << from << "->" << to;
    Q_ASSERT(!from.isEmpty() && !to.isEmpty());
    Q_ASSERT(to[to.size()-1] != QLatin1Char('/'));

    // directory case
    auto normalizedFrom = from;
    if (normalizedFrom.endsWith(QLatin1Char('/'))) {
        normalizedFrom.chop(1);
    }

    const QByteArray fromPath = QFile::encodeName(normalizedFrom);
    quint64 id = tr->documentId(fromPath);
    if (!id) {
        qCDebug(BALOO) << "Document not (yet) known, signaling newFile" << to;
        Q_EMIT movedWithoutData(to);
        return;
    }

    const QByteArray toPath = QFile::encodeName(to);
    auto lastSlash = toPath.lastIndexOf('/');
    const QByteArray parentPath = toPath.left(lastSlash + 1);

    quint64 parentId = tr->documentId(parentPath);
    if (!parentId) {
        qCDebug(BALOO) << "Parent directory not (yet) known, signaling newFile" << to;
        Q_EMIT movedWithoutData(QFile::decodeName(parentPath));
        return;
    }

    Document doc;

    const QByteArray fileName = toPath.mid(lastSlash + 1);
    TermGenerator tg(doc);
    tg.indexFileNameText(QFile::decodeName(fileName));

    doc.setId(id);
    doc.setParentId(parentId);
    doc.setUrl(toPath);
    tr->replaceDocument(doc, DocumentUrl | FileNameTerms);

    // Possible scenarios
    // 1. file moves to the same device - id is preserved
    // 2. file moves to a different device - id is not preserved
}

#include "moc_metadatamover.cpp"
