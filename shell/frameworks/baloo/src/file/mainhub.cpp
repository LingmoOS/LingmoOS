/*
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "mainhub.h"
#include "fileindexerconfig.h"

#include <QDBusConnection>
#include <QCoreApplication>
#include <QTimer>

using namespace Baloo;

MainHub::MainHub(Database* db, FileIndexerConfig* config, bool firstRun)
    : m_db(db)
    , m_config(config)
    , m_fileWatcher(db, config, this)
    , m_fileIndexScheduler(db, config, firstRun, this)
{
    Q_ASSERT(db);
    Q_ASSERT(config);

    connect(&m_fileWatcher, &FileWatch::indexNewFile, &m_fileIndexScheduler, &FileIndexScheduler::indexNewFile);
    connect(&m_fileWatcher, &FileWatch::indexModifiedFile, &m_fileIndexScheduler, &FileIndexScheduler::indexModifiedFile);
    connect(&m_fileWatcher, &FileWatch::indexXAttr, &m_fileIndexScheduler, &FileIndexScheduler::indexXAttrFile);
    connect(&m_fileWatcher, &FileWatch::fileRemoved, &m_fileIndexScheduler, &FileIndexScheduler::handleFileRemoved);

    connect(&m_fileWatcher, &FileWatch::installedWatches, &m_fileIndexScheduler, &FileIndexScheduler::scheduleIndexing);

    QDBusConnection bus = QDBusConnection::sessionBus();
    bus.registerObject(QStringLiteral("/"), this, QDBusConnection::ExportAllSlots |
                        QDBusConnection::ExportScriptableSignals | QDBusConnection::ExportAdaptors);

    if (firstRun) {
        QTimer::singleShot(5000, this, [this] {
            m_fileIndexScheduler.startupFinished();
        });
    } else {
        // Delay these checks so we don't end up consuming excessive resources on login
        QTimer::singleShot(5000, this, [this] {
            m_fileIndexScheduler.checkUnindexedFiles();
            m_fileIndexScheduler.checkStaleIndexEntries();
            m_fileIndexScheduler.startupFinished();
        });
    }
    QTimer::singleShot(0, &m_fileWatcher, &FileWatch::updateIndexedFoldersWatches);
}

void MainHub::quit() const
{
    QCoreApplication::instance()->quit();
}

void MainHub::updateConfig()
{
    m_config->forceConfigUpdate();
    m_fileWatcher.updateIndexedFoldersWatches();
    m_fileIndexScheduler.updateConfig();
}

#include "moc_mainhub.cpp"
