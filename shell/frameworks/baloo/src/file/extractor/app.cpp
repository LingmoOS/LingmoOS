/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2013-2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "app.h"
#include "basicindexingjob.h"
#include "result.h"
#include "idutils.h"
#include "transaction.h"
#include "baloodebug.h"
#include "global.h"

#include <QCoreApplication>

#include <QTimer>
#include <QFileInfo>

#include <KFileMetaData/Extractor>
#include <KFileMetaData/MimeUtils>
#include <KIdleTime>

#include <unistd.h> //for STDIN_FILENO
#include <iostream>

using namespace Baloo;

App::App(QObject* parent)
    : QObject(parent)
    , m_notifyNewData(STDIN_FILENO, QSocketNotifier::Read)
    , m_input()
    , m_output()
    , m_workerPipe(&m_input, &m_output)
    , m_tr(nullptr)
{
    m_input.open(STDIN_FILENO, QIODevice::ReadOnly | QIODevice::Unbuffered );
    m_output.open(STDOUT_FILENO, QIODevice::WriteOnly | QIODevice::Unbuffered );

    static int s_idleTimeout = 1000 * 60 * 1; // 1 min
    m_idleTime = KIdleTime::instance();
    m_idleTime->addIdleTimeout(s_idleTimeout);
    connect(m_idleTime, &KIdleTime::resumingFromIdle, this, [this]() {
        qCInfo(BALOO) << "Busy, paced indexing";
        m_isBusy = true;
    });
    connect(m_idleTime, qOverload<int, int>(&KIdleTime::timeoutReached), this, [this]() {
        qCInfo(BALOO) << "Not busy, fast indexing";
        m_isBusy = false;
    });

    using WorkerPipe = Baloo::Private::WorkerPipe;
    connect(&m_notifyNewData, &QSocketNotifier::activated, &m_workerPipe, &WorkerPipe::processIdData);
    connect(&m_workerPipe, &WorkerPipe::newDocumentIds, this, &App::slotNewBatch);
    connect(&m_workerPipe, &WorkerPipe::inputEnd, this, &QCoreApplication::quit);
}

App::~App()
{
    if (m_tr) {
        // Abort the transaction in case the parent process exited
        m_tr->abort();
        m_tr.reset();
    }
}

void App::slotNewBatch(const QVector<quint64>& ids)
{
    m_ids = ids;

    Database *db = globalDatabaseInstance();
    if (!db->open(Database::ReadWriteDatabase)) {
        qCCritical(BALOO) << "Failed to open the database";
        exit(1);
    }

    Q_ASSERT(m_tr == nullptr);

    if (!m_isBusy) {
        m_idleTime->catchNextResumeEvent();
    }

    QTimer::singleShot((m_isBusy ? 500 : 0), this, [this, db] () {
        // FIXME: The transaction is open for way too long. We should just open it for when we're
        //        committing the data not during the extraction.
        m_tr = std::make_unique<Transaction>(db, Transaction::ReadWrite);
        processNextFile();
    });

    /**
     * A Single Batch seems to be triggering the SocketNotifier more than once
     * so we disable it till the batch is done.
     */
    m_notifyNewData.setEnabled(false);
}

void App::processNextFile()
{
    if (!m_ids.isEmpty()) {
        quint64 id = m_ids.takeFirst();

        QString url = QFile::decodeName(m_tr->documentUrl(id));
        if (url.isEmpty() || !QFile::exists(url)) {
            m_tr->removeDocument(id);
            QTimer::singleShot(0, this, &App::processNextFile);
            return;
        }

        bool indexed = index(m_tr.get(), url, id);

        int delay = (m_isBusy && indexed) ? 10 : 0;
        QTimer::singleShot(delay, this, &App::processNextFile);

    } else {
        bool ok = m_tr->commit();
        if (!ok) {
            exit(2);
        }
        m_tr.reset();

        // Enable the SocketNotifier for the next batch
        m_notifyNewData.setEnabled(true);
        m_workerPipe.batchFinished();
    }
}

bool App::index(Transaction* tr, const QString& url, quint64 id)
{
    if (!m_config.shouldBeIndexed(url)) {
        // This apparently happens when the config has changed after the document
        // was added to the content indexing db
        qCDebug(BALOO) << "Found" << url << "in the ContentIndexingDB, although it should be skipped";
        tr->removeDocument(id);
        m_workerPipe.urlFailed(url);
        return false;
    }

    // The initial BasicIndexingJob run has been supplied with the file extension
    // mimetype only, skip based on the "real" mimetype
    QString mimetype = KFileMetaData::MimeUtils::strictMimeType(url, m_mimeDb).name();
    if (!m_config.shouldMimeTypeBeIndexed(mimetype)) {
        qCDebug(BALOO) << "Skipping" << url << "- mimetype:" << mimetype;
        // FIXME: in case the extension based and content based mimetype differ
        // we should update it.
        tr->removePhaseOne(id);
        m_workerPipe.urlFailed(url);
        return false;
    }

    // HACK: Also, we're ignoring ttext files which are greater tha 10 Mb as we
    // have trouble processing them
    //
    if (mimetype.startsWith(QLatin1String("text/"))) {
        QFileInfo fileInfo(url);
        if (fileInfo.size() >= 10 * 1024 * 1024) {
            qCDebug(BALOO) << "Skipping large " << url << "- mimetype:" << mimetype;
            tr->removePhaseOne(id);
            m_workerPipe.urlFailed(url);
            return false;
        }
    }
    qCDebug(BALOO) << "Indexing" << id << url << mimetype;
    m_workerPipe.urlStarted(url);

    // We always run the basic indexing again. This is mostly so that the proper
    // mimetype is set and we get proper type information.
    // The mimetype fetched in the BasicIndexingJob is fast but not accurate
    BasicIndexingJob basicIndexer(url, mimetype, BasicIndexingJob::NoLevel);
    basicIndexer.index();

    Baloo::Document doc = basicIndexer.document();

    Result result(url, mimetype, KFileMetaData::ExtractionResult::ExtractMetaData | KFileMetaData::ExtractionResult::ExtractPlainText);
    result.setDocument(doc);

    const QList<KFileMetaData::Extractor*> exList = m_extractorCollection.fetchExtractors(mimetype);

    for (KFileMetaData::Extractor* ex : exList) {
        ex->extract(&result);
    }

    result.finish();
    if (doc.id() != id) {
        qCWarning(BALOO) << url << "id seems to have changed. Perhaps baloo was not running, and this file was deleted + re-created";
        tr->removeDocument(id);
        if (!tr->hasDocument(doc.id())) {
            tr->addDocument(result.document());
        } else {
            tr->replaceDocument(result.document(), DocumentTerms | DocumentData);
        }
    } else {
        tr->replaceDocument(result.document(), DocumentTerms | DocumentData);
    }
    tr->removePhaseOne(doc.id());
    m_workerPipe.urlFinished(url);
    return true;
}

#include "moc_app.cpp"
