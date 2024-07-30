/*
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_FILEINDEXSCHEDULER_H
#define BALOO_FILEINDEXSCHEDULER_H

#include <QObject>
#include <QStringList>
#include <QThreadPool>
#include <QTimer>

#include "filecontentindexerprovider.h"
#include "powerstatemonitor.h"
#include "indexerstate.h"
#include "timeestimator.h"

namespace Baloo {

class Database;
class FileIndexerConfig;
class FileContentIndexer;

class FileIndexScheduler : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.baloo.scheduler")

    Q_PROPERTY(int state READ state NOTIFY stateChanged)
public:
    FileIndexScheduler(Database* db, FileIndexerConfig* config, bool firstRun, QObject* parent = nullptr);
    ~FileIndexScheduler() override;
    int state() const { return m_indexerState; }

Q_SIGNALS:
    Q_SCRIPTABLE void stateChanged(int state);

public Q_SLOTS:
    void indexNewFile(const QString& file) {
        if (!m_newFiles.contains(file)) {
            m_newFiles << file;
            if (isIndexerIdle()) {
                QTimer::singleShot(0, this, &FileIndexScheduler::scheduleIndexing);
            }
        }
    }

    void indexModifiedFile(const QString& file) {
        if (!m_modifiedFiles.contains(file)) {
            m_modifiedFiles << file;
            if (isIndexerIdle()) {
                QTimer::singleShot(0, this, &FileIndexScheduler::scheduleIndexing);
            }
        }
    }

    void indexXAttrFile(const QString& file) {
        if (!m_xattrFiles.contains(file)) {
            m_xattrFiles << file;
            if (isIndexerIdle()) {
                QTimer::singleShot(0, this, &FileIndexScheduler::scheduleIndexing);
            }
        }
    }

    void runnerFinished() {
        m_isGoingIdle = true;
        QTimer::singleShot(0, this, &FileIndexScheduler::scheduleIndexing);
    }

    void handleFileRemoved(const QString& file);

    void updateConfig();
    void scheduleIndexing();
    void scheduleCheckUnindexedFiles();
    void scheduleCheckStaleIndexEntries();
    void startupFinished();

    Q_SCRIPTABLE void suspend() { setSuspend(true); }
    Q_SCRIPTABLE void resume() { setSuspend(false); }
    Q_SCRIPTABLE uint getRemainingTime();
    Q_SCRIPTABLE void checkUnindexedFiles();
    Q_SCRIPTABLE void checkStaleIndexEntries();
    Q_SCRIPTABLE uint getBatchSize();

private Q_SLOTS:
    void powerManagementStatusChanged(bool isOnBattery);

private:
    void setSuspend(bool suspend);
    bool isIndexerIdle() {
        return m_isGoingIdle ||
               (m_indexerState == Suspended) ||
               (m_indexerState == Startup) ||
               (m_indexerState == Idle) ||
               (m_indexerState == LowPowerIdle);
    }

    Database* m_db;
    FileIndexerConfig* m_config;

    QStringList m_newFiles;
    QStringList m_modifiedFiles;
    QStringList m_xattrFiles;

    QThreadPool m_threadPool;

    FileContentIndexerProvider m_provider;
    FileContentIndexer* m_contentIndexer;

    PowerStateMonitor m_powerMonitor;

    IndexerState m_indexerState;
    TimeEstimator m_timeEstimator;
    uint m_indexPendingFiles = 0;
    uint m_indexFinishedFiles = 0;

    bool m_checkUnindexedFiles;
    bool m_checkStaleIndexEntries;
    bool m_isGoingIdle;
    bool m_isSuspended;
    bool m_isFirstRun;
    bool m_inStartup;
};

}

#endif // BALOO_FILEINDEXSCHEDULER_H
