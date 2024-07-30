/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2015 Pinak Ahuja <pinak.ahuja@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

/**
 * WARNING: make sure baloo_file is not running before running this test
 * otherwise we'll have two baloo_file_extractor processes running and that will
 * cause problems when registering the monitor.
 */

#include "database.h"
#include "firstrunindexer.h"
#include "filecontentindexerprovider.h"
#include "filecontentindexer.h"
#include "fileindexerconfig.h"
#include "timeestimator.h"
#include "extractor_interface.h"

#include <QCoreApplication>
#include <QThreadPool>
#include <QTemporaryDir>
#include <QObject>
#include <QDBusConnection>
#include <QTextStream>
#include <QString>
#include <QDBusServiceWatcher>

#include <iostream>

namespace org {
    namespace kde {
        namespace baloo {
            typedef OrgKdeBalooExtractorInterface extractorInterface;
        }
    }
}

class Scheduler : public QObject
{
    Q_OBJECT

public:
    explicit Scheduler(QObject* parent = 0);
    void startIndexing();

private Q_SLOTS:
    void startContentIndexer();
    void printTime();
    void finished();
    void registerMonitor(const QString& service);

private:
    QTemporaryDir m_dir;

    Baloo::Database m_db;
    Baloo::FileContentIndexerProvider m_provider;
    Baloo::FileIndexerConfig* m_config;
    Baloo::FileContentIndexer* m_contentRunnable;
    org::kde::baloo::extractorInterface* m_extractorInterface;

    QThreadPool m_pool;

    int m_count;
    int m_batchSize;

    QTextStream m_out;
};

Scheduler::Scheduler(QObject* parent)
    : QObject(parent)
    , m_db(m_dir.path())
    , m_provider(&m_db)
    , m_contentRunnable(0)
    , m_count(0)
    , m_batchSize(40)
    , m_out(stdout)
{
    m_db.open(Baloo::Database::CreateDatabase);
    m_pool.setMaxThreadCount(1);

    QString extractorService = QStringLiteral("org.kde.baloo.extractor");
    m_extractorInterface = new org::kde::baloo::extractorInterface(extractorService,
                                                            QStringLiteral("/extractor"),
                                                            QDBusConnection::sessionBus(),
                                                            this);

    connect(m_extractorInterface, &org::kde::baloo::extractorInterface::currentUrlChanged,
            this, &Scheduler::printTime);

    QDBusServiceWatcher* extractorWatcher = new QDBusServiceWatcher(extractorService,
                                                            QDBusConnection::sessionBus(),
                                                            QDBusServiceWatcher::WatchForRegistration,
                                                            this);

    connect(extractorWatcher, &QDBusServiceWatcher::serviceRegistered, this, &Scheduler::registerMonitor);

    // Set test path
    qputenv("BALOO_DB_PATH", m_dir.path().toUtf8());

    QStandardPaths::setTestModeEnabled(true);
    QString testConfigPath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) +
                                                                            QStringLiteral("baloofilerc");
    // Remove config from previous runs
    QFile::remove(testConfigPath);

    m_config = new Baloo::FileIndexerConfig(this);
    m_config->setInitialRun(true);
}

void Scheduler::startIndexing()
{
    auto firstRunnable = new Baloo::FirstRunIndexer(&m_db, m_config, m_config->includeFolders());
    connect(firstRunnable, &Baloo::FirstRunIndexer::done, this, &Scheduler::startContentIndexer);
    m_pool.start(firstRunnable);
}

void Scheduler::startContentIndexer()
{
    m_contentRunnable = new Baloo::FileContentIndexer(&m_provider);
    connect(m_contentRunnable, &Baloo::FileContentIndexer::done, this, &Scheduler::finished);
    m_pool.start(m_contentRunnable);
}

void Scheduler::printTime()
{
    Q_ASSERT(m_contentRunnable != 0);

    if (++m_count == 10 * m_batchSize) {
        Baloo::TimeEstimator estimator;
        estimator.setBatchTimings(m_contentRunnable->batchTimings());
        estimator.setFilesLeft(m_provider.size());
        // print Remaining time after every 10 batches
        m_out <<  "Remaining Time: " << estimator.calculateTimeLeft() << endl;
        m_count = 0;
    }
}

void Scheduler::finished()
{
    m_out << "done!" << endl;
    QCoreApplication::exit();
}

void Scheduler::registerMonitor(const QString& service)
{
    Q_UNUSED(service);
    m_extractorInterface->registerMonitor();
}

int main (int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    Scheduler sched;
    sched.startIndexing();
    app.exec();
}

#include "remainingtimetest.moc"
