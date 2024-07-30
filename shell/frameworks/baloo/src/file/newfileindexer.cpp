/*
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "newfileindexer.h"
#include "basicindexingjob.h"
#include "fileindexerconfig.h"

#include "database.h"
#include "transaction.h"

#include <QMimeDatabase>
#include <QFileInfo>

using namespace Baloo;

NewFileIndexer::NewFileIndexer(Database* db, const FileIndexerConfig* config, const QStringList& newFiles)
    : m_db(db)
    , m_config(config)
    , m_files(newFiles)
{
    Q_ASSERT(m_db);
    Q_ASSERT(m_config);
    Q_ASSERT(!m_files.isEmpty());
}

void NewFileIndexer::run()
{
    QMimeDatabase mimeDb;
    BasicIndexingJob::IndexingLevel level = m_config->onlyBasicIndexing() ? BasicIndexingJob::NoLevel
        : BasicIndexingJob::MarkForContentIndexing;

    Transaction tr(m_db, Transaction::ReadWrite);

    for (const QString &path : m_files) {
        auto filePath = path;
        if (filePath.endsWith(QLatin1Char('/'))) {
            filePath.chop(1);
        }

        QString mimetype;
        QFileInfo fileInfo(filePath);

        if (fileInfo.isSymLink()) {
            continue;
        }

        if (fileInfo.isDir()) {
            if (!m_config->shouldFolderBeIndexed(filePath)) {
                continue;
            }
            mimetype = QStringLiteral("inode/directory");

        } else {
            QString fileName = filePath.mid(filePath.lastIndexOf(QLatin1Char('/')) + 1);
            if (!m_config->shouldFileBeIndexed(fileName)) {
                continue;
            }
            mimetype = mimeDb.mimeTypeForFile(filePath, QMimeDatabase::MatchExtension).name();
        }

        BasicIndexingJob job(filePath, mimetype, level);
        if (!job.index()) {
            continue;
        }

        // The same file can be sent twice though it shouldn't be.
        // Lets just silently ignore it instead of crashing
        if (tr.hasDocument(job.document().id())) {
            continue;
        }
        tr.addDocument(job.document());
    }

    tr.commit();
    Q_EMIT done();
}

#include "moc_newfileindexer.cpp"
