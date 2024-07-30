/*
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "fileindexscheduler.h"

#include "baloodebug.h"
#include "firstrunindexer.h"
#include "newfileindexer.h"
#include "modifiedfileindexer.h"
#include "xattrindexer.h"
#include "filecontentindexer.h"
#include "unindexedfileindexer.h"
#include "indexcleaner.h"

#include "fileindexerconfig.h"

#include <memory>

#include <QDBusConnection>

using namespace Baloo;

FileIndexScheduler::FileIndexScheduler(Database* db, FileIndexerConfig* config, bool firstRun, QObject* parent)
    : QObject(parent)
    , m_db(db)
    , m_config(config)
    , m_provider(db)
    , m_contentIndexer(nullptr)
    , m_indexerState(Startup)
    , m_checkUnindexedFiles(false)
    , m_checkStaleIndexEntries(false)
    , m_isGoingIdle(false)
    , m_isSuspended(false)
    , m_isFirstRun(firstRun)
    , m_inStartup(true)
{
    Q_ASSERT(db);
    Q_ASSERT(config);

    m_threadPool.setMaxThreadCount(1);

    connect(&m_powerMonitor, &PowerStateMonitor::powerManagementStatusChanged,
            this, &FileIndexScheduler::powerManagementStatusChanged);

    if (m_powerMonitor.isOnBattery()) {
        m_indexerState = LowPowerIdle;
    }

    m_contentIndexer = new FileContentIndexer(m_config->maxUncomittedFiles(), &m_provider, m_indexFinishedFiles, this);
    m_contentIndexer->setAutoDelete(false);
    connect(m_contentIndexer, &FileContentIndexer::done, this,
            &FileIndexScheduler::runnerFinished);
    connect(m_contentIndexer, &FileContentIndexer::committedBatch, [this](uint time, uint batchSize) {
            this->m_timeEstimator.handleNewBatchTime(time, batchSize);
    });

    QDBusConnection::sessionBus().registerObject(QStringLiteral("/scheduler"),
                                                 this, QDBusConnection::ExportScriptableContents);
}

FileIndexScheduler::~FileIndexScheduler()
{
    m_contentIndexer->quit();
    m_threadPool.waitForDone(0); // wait 0 msecs
}

void FileIndexScheduler::startupFinished() {
    m_inStartup = false;
    QTimer::singleShot(0, this, &FileIndexScheduler::scheduleIndexing);
}

void FileIndexScheduler::scheduleIndexing()
{
    if (!isIndexerIdle()) {
        return;
    }
    m_isGoingIdle = false;

    if (m_isSuspended) {
        if (m_indexerState != Suspended) {
            m_indexerState = Suspended;
            Q_EMIT stateChanged(m_indexerState);
        }
        return;
    }

    if (m_isFirstRun) {
        if (m_inStartup) {
            return;
        }

        m_isFirstRun = false;
        auto runnable = new FirstRunIndexer(m_db, m_config, m_config->includeFolders());
        connect(runnable, &FirstRunIndexer::done, this, &FileIndexScheduler::runnerFinished);

        m_threadPool.start(runnable);
        m_indexerState = FirstRun;
        Q_EMIT stateChanged(m_indexerState);
        return;
    }

    if (!m_newFiles.isEmpty()) {
        auto runnable = new NewFileIndexer(m_db, m_config, m_newFiles);
        connect(runnable, &NewFileIndexer::done, this, &FileIndexScheduler::runnerFinished);

        m_threadPool.start(runnable);
        m_newFiles.clear();
        m_indexerState = NewFiles;
        Q_EMIT stateChanged(m_indexerState);
        return;
    }

    if (!m_modifiedFiles.isEmpty()) {
        auto runnable = new ModifiedFileIndexer(m_db, m_config, m_modifiedFiles);
        connect(runnable, &ModifiedFileIndexer::done, this, &FileIndexScheduler::runnerFinished);

        m_threadPool.start(runnable);
        m_modifiedFiles.clear();
        m_indexerState = ModifiedFiles;
        Q_EMIT stateChanged(m_indexerState);
        return;
    }

    if (!m_xattrFiles.isEmpty()) {
        auto runnable = new XAttrIndexer(m_db, m_config, m_xattrFiles);
        connect(runnable, &XAttrIndexer::done, this, &FileIndexScheduler::runnerFinished);

        m_threadPool.start(runnable);
        m_xattrFiles.clear();
        m_indexerState = XAttrFiles;
        Q_EMIT stateChanged(m_indexerState);
        return;
    }

    // No housekeeping, no content indexing
    if (m_powerMonitor.isOnBattery()) {
        if (m_indexerState != LowPowerIdle) {
            m_indexerState = LowPowerIdle;
            Q_EMIT stateChanged(m_indexerState);
        }
        return;
    }

    if (m_inStartup) {
        if (m_indexerState != Startup) {
            m_indexerState = Startup;
            Q_EMIT stateChanged(m_indexerState);
        }
        return;
    }

    // This has to be above content indexing, because there can be files that
    // should not be indexed in the DB (i.e. if config was changed)
    if (m_checkStaleIndexEntries) {
        auto runnable = new IndexCleaner(m_db, m_config);
        connect(runnable, &IndexCleaner::done, this, &FileIndexScheduler::runnerFinished);

        m_threadPool.start(runnable);
        m_checkStaleIndexEntries = false;
        m_indexerState = StaleIndexEntriesClean;
        Q_EMIT stateChanged(m_indexerState);
        return;
    }

    m_indexPendingFiles = m_provider.size();
    m_indexFinishedFiles = 0;
    if (m_indexPendingFiles) {
        m_threadPool.start(m_contentIndexer);
        m_indexerState = ContentIndexing;
        Q_EMIT stateChanged(m_indexerState);
        return;
    }

    if (m_checkUnindexedFiles) {
        auto runnable = new UnindexedFileIndexer(m_db, m_config);
        connect(runnable, &UnindexedFileIndexer::done, this, &FileIndexScheduler::runnerFinished);

        m_threadPool.start(runnable);
        m_checkUnindexedFiles = false;
        m_indexerState = UnindexedFileCheck;
        Q_EMIT stateChanged(m_indexerState);
        return;
    }

    if (m_indexerState != Idle) {
        m_indexerState = Idle;
        Q_EMIT stateChanged(m_indexerState);
    }
}

static void removeStartsWith(QStringList& list, const QString& dir)
{
    const auto tail = std::remove_if(list.begin(), list.end(),
        [&dir](const QString& file) {
            return file.startsWith(dir);
        });
    list.erase(tail, list.end());
}

static void removeShouldNotIndex(QStringList& list, FileIndexerConfig* config)
{
    const auto tail = std::remove_if(list.begin(), list.end(),
        [config](const QString& file) {
            return !config->shouldBeIndexed(file);
        });
    list.erase(tail, list.end());
}

void FileIndexScheduler::updateConfig()
{
    // Interrupt content indexer, to avoid indexing files that should
    // not be indexed (bug 373430)
    if (m_indexerState == ContentIndexing) {
        m_contentIndexer->quit();
    }
    removeShouldNotIndex(m_newFiles, m_config);
    removeShouldNotIndex(m_modifiedFiles, m_config);
    removeShouldNotIndex(m_xattrFiles, m_config);
    m_checkStaleIndexEntries = true;
    m_checkUnindexedFiles = true;
    scheduleIndexing();
}

void FileIndexScheduler::handleFileRemoved(const QString& file)
{
    if (!file.endsWith(QLatin1Char('/'))) {
        m_newFiles.removeOne(file);
        m_modifiedFiles.removeOne(file);
        m_xattrFiles.removeOne(file);
    }
    else {
        removeStartsWith(m_newFiles, file);
        removeStartsWith(m_modifiedFiles, file);
        removeStartsWith(m_xattrFiles, file);
    }
}

void FileIndexScheduler::powerManagementStatusChanged(bool isOnBattery)
{
    qCDebug(BALOO) << "Power state changed - onBattery:" << isOnBattery;
    if (isOnBattery && m_indexerState == ContentIndexing) {
        qCDebug(BALOO) << "On battery, stopping content indexer";
        m_contentIndexer->quit();
    } else {
        scheduleIndexing();
    }
}

void FileIndexScheduler::setSuspend(bool suspend)
{
    m_isSuspended = suspend;
    if (suspend) {
        qCDebug(BALOO) << "Suspending";
        if (m_indexerState == ContentIndexing) {
            m_contentIndexer->quit();
        } else {
            scheduleIndexing();
        }
    } else {
        qCDebug(BALOO) << "Resuming";
        // No need to emit here we'll be emitting in scheduling
        scheduleIndexing();
    }
}

uint FileIndexScheduler::getRemainingTime()
{
    if (m_indexerState != ContentIndexing) {
        return 0;
    }
    uint remainingFiles = m_indexPendingFiles - m_indexFinishedFiles;
    return m_timeEstimator.calculateTimeLeft(remainingFiles);
}

void FileIndexScheduler::scheduleCheckUnindexedFiles()
{
    m_checkUnindexedFiles = true;
}

void FileIndexScheduler::checkUnindexedFiles()
{
    m_checkUnindexedFiles = true;
    scheduleIndexing();
}

void FileIndexScheduler::scheduleCheckStaleIndexEntries()
{
    m_checkStaleIndexEntries = true;
}

void FileIndexScheduler::checkStaleIndexEntries()
{
    m_checkStaleIndexEntries = true;
    scheduleIndexing();
}

uint FileIndexScheduler::getBatchSize()
{
    return m_config->maxUncomittedFiles();
}

#include "moc_fileindexscheduler.cpp"
