/*
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "modifiedfileindexer.h"
#include "basicindexingjob.h"
#include "fileindexerconfig.h"
#include "idutils.h"

#include "database.h"
#include "transaction.h"

#include <QMimeDatabase>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>

using namespace Baloo;

ModifiedFileIndexer::ModifiedFileIndexer(Database* db, const FileIndexerConfig* config, const QStringList& files)
    : m_db(db)
    , m_config(config)
    , m_files(files)
{
    Q_ASSERT(m_db);
    Q_ASSERT(m_config);
    Q_ASSERT(!m_files.isEmpty());
}

void ModifiedFileIndexer::run()
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

        QString fileName = filePath.mid(filePath.lastIndexOf(QLatin1Char('/')) + 1);
        if (!m_config->shouldFileBeIndexed(fileName)) {
            continue;
        }

        quint64 fileId = filePathToId(QFile::encodeName(filePath));
        if (!fileId) {
            continue;
        }

        // FIXME: Using QFileInfo over here is quite expensive!
        QFileInfo fileInfo(filePath);
        if (fileInfo.isSymLink()) {
            continue;
        }

        bool mTimeChanged;
        bool cTimeChanged;
        const bool isKnownFile = tr.hasDocument(fileId);
        if (isKnownFile) {
            DocumentTimeDB::TimeInfo timeInfo = tr.documentTimeInfo(fileId);
            mTimeChanged = timeInfo.mTime != fileInfo.lastModified().toSecsSinceEpoch();
            cTimeChanged = timeInfo.cTime != fileInfo.metadataChangeTime().toSecsSinceEpoch();
        } else {
            mTimeChanged = cTimeChanged = true;
        }

        if (!mTimeChanged && !cTimeChanged) {
            continue;
        }

        QString mimetype;
        if (fileInfo.isDir()) {
            // The folder ctime changes when the folder is created, when the folder is
            // renamed, or when the xattrs (tags, comments, ...) change
            if (!cTimeChanged) {
                continue;
            }
            mimetype = QStringLiteral("inode/directory");

        } else {
            mimetype = mimeDb.mimeTypeForFile(filePath, QMimeDatabase::MatchExtension).name();
        }

        // Only mTime changed
        if (!cTimeChanged) {
            Document doc;
            doc.setId(fileId);
            doc.setMTime(fileInfo.lastModified().toSecsSinceEpoch());
            doc.setCTime(fileInfo.metadataChangeTime().toSecsSinceEpoch());
            if (level == BasicIndexingJob::MarkForContentIndexing) {
                doc.setContentIndexing(true);
            }

            tr.replaceDocument(doc, DocumentTime);
            continue;
        }

        BasicIndexingJob job(filePath, mimetype, level);
        if (!job.index()) {
            continue;
        }

        // We can get modified events for files which do not yet exist in the database
        // because Baloo was not running and missed the creation events
        if (isKnownFile && (job.document().id() == fileId)) {
            tr.replaceDocument(job.document(), XAttrTerms | DocumentTime | FileNameTerms | DocumentUrl);
        } else {
            tr.addDocument(job.document());
        }
    }

    tr.commit();
    Q_EMIT done();
}

#include "moc_modifiedfileindexer.cpp"
