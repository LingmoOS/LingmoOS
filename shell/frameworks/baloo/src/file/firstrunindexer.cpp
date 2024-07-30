/*
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "firstrunindexer.h"
#include "basicindexingjob.h"
#include "fileindexerconfig.h"
#include "filtereddiriterator.h"

#include "baloodebug.h"
#include "database.h"
#include "transaction.h"

#include <QMimeDatabase>

using namespace Baloo;

FirstRunIndexer::FirstRunIndexer(Database* db, FileIndexerConfig* config, const QStringList& folders)
    : m_db(db)
    , m_config(config)
    , m_folders(folders)
{
    Q_ASSERT(m_db);
    Q_ASSERT(m_config);
    Q_ASSERT(!m_folders.isEmpty());
}

void FirstRunIndexer::run()
{
    QMimeDatabase mimeDb;
    BasicIndexingJob::IndexingLevel level = m_config->onlyBasicIndexing() ? BasicIndexingJob::NoLevel
        : BasicIndexingJob::MarkForContentIndexing;

    for (const QString& folder : std::as_const(m_folders)) {
        Transaction tr(m_db, Transaction::ReadWrite);
        int transactionDocumentCount = 0;

        FilteredDirIterator it(m_config, folder);
        while (!it.next().isEmpty()) {
            QString mimetype;
            if (it.fileInfo().isDir()) {
                mimetype = QStringLiteral("inode/directory");
            } else {
                mimetype = mimeDb.mimeTypeForFile(it.filePath(), QMimeDatabase::MatchExtension).name();
            }

            BasicIndexingJob job(it.filePath(), mimetype, level);
            if (!job.index()) {
                continue;
            }

            // Even though this is the first run, because 2 hard links will resolve to the same id,
            // we land up crashing (due to the asserts in addDocument).
            // Hence we are checking before.
            // FIXME: Silently ignore hard links!
            //
            if (tr.hasDocument(job.document().id())) {
                continue;
            }
            tr.addDocument(job.document());

            transactionDocumentCount++;
            if (transactionDocumentCount > 20000) {
                qCDebug(BALOO) << "Commit";
                tr.commit();
                tr.reset(Transaction::ReadWrite);
                transactionDocumentCount = 0;
            }
        }

        // FIXME: This would consume too much memory. We should make some more commits
        //        based on how much memory we consume
        tr.commit();
    }

    Q_EMIT done();
}

#include "moc_firstrunindexer.cpp"
