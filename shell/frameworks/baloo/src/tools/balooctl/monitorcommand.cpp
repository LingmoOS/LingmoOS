/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2015 Pinak Ahuja <pinak.ahuja@gmail.com>
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "monitorcommand.h"
#include "indexerstate.h"

#include <QDBusConnection>
#include <QDBusServiceWatcher>

using namespace Baloo;

MonitorCommand::MonitorCommand(QObject *parent)
    : QObject(parent)
    , m_out(stdout)
    , m_err(stderr)
    , m_indexerDBusInterface(nullptr)
    , m_schedulerDBusInterface(nullptr)
    , m_dbusServiceWatcher(nullptr)

{
    m_dbusServiceWatcher = new QDBusServiceWatcher(
        QStringLiteral("org.kde.baloo"), QDBusConnection::sessionBus(),
        QDBusServiceWatcher::WatchForOwnerChange, this
    );
    connect(m_dbusServiceWatcher, &QDBusServiceWatcher::serviceRegistered,
            this, &MonitorCommand::balooIsAvailable);
    connect(m_dbusServiceWatcher, &QDBusServiceWatcher::serviceUnregistered,
            this, &MonitorCommand::balooIsNotAvailable);

    m_indexerDBusInterface = new org::kde::baloo::fileindexer(QStringLiteral("org.kde.baloo"),
        QStringLiteral("/fileindexer"),
        QDBusConnection::sessionBus(),
        this
    );
    connect(m_indexerDBusInterface, &org::kde::baloo::fileindexer::startedIndexingFile,
        this, &MonitorCommand::startedIndexingFile);
    connect(m_indexerDBusInterface, &org::kde::baloo::fileindexer::finishedIndexingFile,
        this, &MonitorCommand::finishedIndexingFile);

    m_schedulerDBusInterface = new org::kde::baloo::scheduler(QStringLiteral("org.kde.baloo"),
        QStringLiteral("/scheduler"),
        QDBusConnection::sessionBus(),
        this
    );
    connect(m_schedulerDBusInterface, &org::kde::baloo::scheduler::stateChanged,
        this, &MonitorCommand::stateChanged);

    if (m_indexerDBusInterface->isValid() && m_schedulerDBusInterface->isValid()) {
        m_err << i18n("Press ctrl+c to stop monitoring\n");
        m_err.flush();
        balooIsAvailable();
        stateChanged(m_schedulerDBusInterface->state());
        const QString currentFile = m_indexerDBusInterface->currentFile();
        if (!currentFile.isEmpty()) {
            startedIndexingFile(currentFile);
        }
    } else {
        balooIsNotAvailable();
    }
}

void MonitorCommand::balooIsNotAvailable()
{
    m_indexerDBusInterface->unregisterMonitor();
    m_err << i18n("Waiting for file indexer to start\n");
    m_err << i18n("Press Ctrl+C to stop monitoring\n");
    m_err.flush();
}

void MonitorCommand::balooIsAvailable()
{
    m_indexerDBusInterface->registerMonitor();
    m_err << i18n("File indexer is running\n");
    m_err.flush();
}

int MonitorCommand::exec(const QCommandLineParser& parser)
{
    Q_UNUSED(parser);
    return QCoreApplication::instance()->exec();
}

void MonitorCommand::startedIndexingFile(const QString& filePath)
{
    if (!m_currentFile.isEmpty()) {
	m_out << '\n';
    }
    m_currentFile = filePath;
    m_out << i18nc("currently indexed file", "Indexing: %1", filePath);
    m_out.flush();
}

void MonitorCommand::finishedIndexingFile(const QString& filePath)
{
    Q_UNUSED(filePath);

    m_currentFile.clear();
    m_out << i18n(": Ok\n");
    m_out.flush();
}

void MonitorCommand::stateChanged(int state)
{
    m_out << stateString(state) << '\n';
    m_out.flush();
}

#include "moc_monitorcommand.cpp"
