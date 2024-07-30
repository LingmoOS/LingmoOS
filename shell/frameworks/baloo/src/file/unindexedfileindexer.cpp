/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2015 Pinak Ahuja <pinak.ahuja@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "unindexedfileindexer.h"

#include "unindexedfileiterator.h"
#include "transaction.h"
#include "fileindexerconfig.h"
#include "baloodebug.h"
#include "basicindexingjob.h"

using namespace Baloo;

UnindexedFileIndexer::UnindexedFileIndexer(Database* db, const FileIndexerConfig* config)
    : m_db(db)
    , m_config(config)
{
}

void UnindexedFileIndexer::run()
{
    const QStringList includeFolders = m_config->includeFolders();
    const BasicIndexingJob::IndexingLevel level = m_config->onlyBasicIndexing() ?
        BasicIndexingJob::NoLevel : BasicIndexingJob::MarkForContentIndexing;

    for (const QString& includeFolder : includeFolders) {
        Transaction tr(m_db, Transaction::ReadWrite);
        int transactionDocumentCount = 0;

        UnIndexedFileIterator it(m_config, &tr, includeFolder);
        while (!it.next().isEmpty()) {
            BasicIndexingJob job(it.filePath(), it.mimetype(), level);
            if (!job.index()) {
                continue;
            }

            if (it.mTimeChanged() && level == BasicIndexingJob::MarkForContentIndexing) {
                job.document().setContentIndexing(true);
            }

            // We handle modified files by simply updating the mTime and filename in the Db and marking them for ContentIndexing
            const quint64 id = job.document().id();
            if (tr.hasDocument(id)) {
                DocumentOperations ops = DocumentTime;
                if (it.cTimeChanged()) {
                    ops |= XAttrTerms;
                    if (QFile::decodeName(tr.documentUrl(id)) != it.filePath()) {
                        ops |= (FileNameTerms | DocumentUrl);
                    }
                }
                tr.replaceDocument(job.document(), ops);

            } else { // New file
                tr.addDocument(job.document());
            }

            transactionDocumentCount++;
            if (transactionDocumentCount > 20000) {
                qCDebug(BALOO) << "Commit";
                tr.commit();
                tr.reset(Transaction::ReadWrite);
                transactionDocumentCount = 0;
            }
        }
        tr.commit();
    }

    Q_EMIT done();
}

#include "moc_unindexedfileindexer.cpp"
