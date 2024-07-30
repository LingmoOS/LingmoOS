/*
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "baloodebug.h"
#include "config.h"
#include "filecontentindexer.h"
#include "filecontentindexerprovider.h"
#include "extractorprocess.h"

#include <QEventLoop>
#include <QElapsedTimer>
#include <QDBusConnection>

using namespace Baloo;

namespace {
    // TODO KF6 -- remove/combine with started/finished DBus signal
    void sendChangedSignal(const QStringList& updatedFiles)
    {
        auto message = QDBusMessage::createSignal(QStringLiteral("/files"),
                                                  QStringLiteral("org.kde"),
                                                  QStringLiteral("changed"));
        message.setArguments({updatedFiles});
        QDBusConnection::sessionBus().send(message);
    }
}

FileContentIndexer::FileContentIndexer(uint batchSize,
        FileContentIndexerProvider* provider,
        uint& finishedCount, QObject* parent)
    : QObject(parent)
    , m_batchSize(batchSize)
    , m_provider(provider)
    , m_finishedCount(finishedCount)
    , m_stop(0)
    , m_extractorPath(QStringLiteral(KDE_INSTALL_FULL_LIBEXECDIR_KF "/baloo_file_extractor"))
{
    Q_ASSERT(provider);

    QDBusConnection bus = QDBusConnection::sessionBus();
    m_monitorWatcher.setConnection(bus);
    m_monitorWatcher.setWatchMode(QDBusServiceWatcher::WatchForUnregistration);
    connect(&m_monitorWatcher, &QDBusServiceWatcher::serviceUnregistered, this,
            &FileContentIndexer::monitorClosed);

    bus.registerObject(QStringLiteral("/fileindexer"),
                        this, QDBusConnection::ExportScriptableContents);
}

void FileContentIndexer::run()
{
    ExtractorProcess process{m_extractorPath};
    connect(&process, &ExtractorProcess::startedIndexingFile, this, &FileContentIndexer::slotStartedIndexingFile);
    connect(&process, &ExtractorProcess::finishedIndexingFile, this, &FileContentIndexer::slotFinishedIndexingFile);
    m_stop.storeRelaxed(false);
    auto batchSize = m_batchSize;
    while (true) {
        //
        // WARNING: This will go mad, if the Extractor does not commit after N=m_batchSize files
        // cause then we will keep fetching the same N files again and again.
        //
        QVector<quint64> idList = m_provider->fetch(batchSize);
        if (idList.isEmpty() || m_stop.loadRelaxed()) {
            break;
        }
        QEventLoop loop;
        connect(&process, &ExtractorProcess::done, &loop, &QEventLoop::quit);

        bool hadErrors = false;
        connect(&process, &ExtractorProcess::failed, &loop, [&hadErrors, &loop]() { hadErrors = true; loop.quit(); });

        uint batchStartCount = m_finishedCount;
        connect(&process, &ExtractorProcess::finishedIndexingFile, &loop, [this]() { m_finishedCount++; });

        QElapsedTimer timer;
        timer.start();

        process.index(idList);
        loop.exec();
        batchSize = idList.size();

        if (hadErrors && !m_stop.loadRelaxed()) {
            if (batchSize == 1) {
                auto failedId = idList.first();
                auto ok = m_provider->markFailed(failedId);
                if (!ok) {
                    qCCritical(BALOO) << "Not able to commit to DB, DB likely is in a bad state. Exiting";
                    exit(1);
                }
                batchSize = m_batchSize;
            } else {
                batchSize /= 2;
            }
            m_updatedFiles.clear();
            // reset to old value - nothing committed
            m_finishedCount = batchStartCount;
            process.start();
        } else {
            // Notify some metadata may have changed
            sendChangedSignal(m_updatedFiles);
            m_updatedFiles.clear();

            // Update remaining time estimate
            auto elapsed = timer.elapsed();
            QMetaObject::invokeMethod(this,
                [this, elapsed, batchSize] { committedBatch(elapsed, batchSize); },
                Qt::QueuedConnection);
        }
    }
    QMetaObject::invokeMethod(this, &FileContentIndexer::done, Qt::QueuedConnection);
}

void FileContentIndexer::slotStartedIndexingFile(const QString& filePath)
{
    m_currentFile = filePath;
    if (!m_registeredMonitors.isEmpty()) {
        Q_EMIT startedIndexingFile(filePath);
    }
}

void FileContentIndexer::slotFinishedIndexingFile(const QString& filePath, bool fileUpdated)
{
    if (fileUpdated) {
        m_updatedFiles.append(filePath);
    }

    m_currentFile = QString();
    if (!m_registeredMonitors.isEmpty()) {
        Q_EMIT finishedIndexingFile(filePath);
    }
}

void FileContentIndexer::registerMonitor(const QDBusMessage& message)
{
    if (!m_registeredMonitors.contains(message.service())) {
        m_registeredMonitors << message.service();
        m_monitorWatcher.addWatchedService(message.service());
    }
}

void FileContentIndexer::unregisterMonitor(const QDBusMessage& message)
{
    m_registeredMonitors.removeAll(message.service());
    m_monitorWatcher.removeWatchedService(message.service());
}

void FileContentIndexer::monitorClosed(const QString& service)
{
    m_registeredMonitors.removeAll(service);
    m_monitorWatcher.removeWatchedService(service);
}

#include "moc_filecontentindexer.cpp"
