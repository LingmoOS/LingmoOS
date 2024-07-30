/*
    SPDX-FileCopyrightText: 2015 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "processjob.h"
#include "cmake-paths.h"
#include "purpose_external_process_debug.h"
#include <QCborValue>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QLibrary>
#include <QMetaMethod>
#include <QRandomGenerator>
#include <QStandardPaths>

using namespace Purpose;

ProcessJob::ProcessJob(const QString &pluginPath, const QString &pluginType, const QJsonObject &data, QObject *parent)
    : Job(parent)
    , m_process(new QProcess(this))
    , m_pluginPath(pluginPath)
    , m_pluginType(pluginType)
    , m_data(data)
    , m_localSocket(nullptr)
{
    if (QLibrary::isLibrary(pluginPath)) {
        QString exec = QStandardPaths::findExecutable(QStringLiteral("purposeprocess"), QStringList(QStringLiteral(KDE_INSTALL_FULL_LIBEXECDIR_KF)));
        Q_ASSERT(!exec.isEmpty());
        m_process->setProgram(exec);
    } else {
        Q_ASSERT(QFile::exists(pluginPath));
        Q_ASSERT(QFileInfo(pluginPath).permission(QFile::ExeOther | QFile::ExeGroup | QFile::ExeUser));
        m_process->setProgram(pluginPath);
    }
    m_process->setProcessChannelMode(QProcess::ForwardedChannels);

    connect(static_cast<QProcess *>(m_process), &QProcess::errorOccurred, this, [](QProcess::ProcessError error) {
        qCWarning(PURPOSE_EXTERNAL_PROCESS_LOG) << "error!" << error;
    });
    connect(static_cast<QProcess *>(m_process), &QProcess::stateChanged, this, &ProcessJob::processStateChanged);

    m_socket.setMaxPendingConnections(1);
    m_socket.setSocketOptions(QLocalServer::UserAccessOption);
    bool b = m_socket.listen(QStringLiteral("randomname-%1").arg(QRandomGenerator::global()->generate()));
    Q_ASSERT(b);
    connect(&m_socket, &QLocalServer::newConnection, this, &ProcessJob::writeSocket);
}

ProcessJob::~ProcessJob()
{
    m_process->kill();
    delete m_process;
}

void ProcessJob::writeSocket()
{
    m_localSocket = m_socket.nextPendingConnection();
    connect(static_cast<QIODevice *>(m_localSocket), &QIODevice::readyRead, this, &ProcessJob::readSocket);

    m_socket.removeServer(m_socket.serverName());

    const QByteArray data = QCborValue::fromJsonValue(m_data).toCbor();
    m_localSocket->write(QByteArray::number(data.size()) + '\n');
    const auto ret = m_localSocket->write(data);
    Q_ASSERT(ret == data.size());
    m_localSocket->flush();
}

void ProcessJob::readSocket()
{
    QJsonParseError error;
    while (m_localSocket && m_localSocket->canReadLine()) {
        const QByteArray json = m_localSocket->readLine();

        const QJsonObject object = QJsonDocument::fromJson(json, &error).object();
        if (error.error != QJsonParseError::NoError) {
            qCWarning(PURPOSE_EXTERNAL_PROCESS_LOG) << "error!" << error.errorString() << json;
            continue;
        }

        for (auto it = object.constBegin(), itEnd = object.constEnd(); it != itEnd; ++it) {
            const QByteArray propName = it.key().toLatin1();
            if (propName == "percent") {
                setPercent(it->toInt());
            } else if (propName == "error") {
                setError(it->toInt());
            } else if (propName == "errorText") {
                setErrorText(it->toString());
            } else if (propName == "output") {
                setOutput(it->toObject());
            }
        }
    }
}

void ProcessJob::start()
{
    m_process->setArguments(
        {QStringLiteral("--server"), m_socket.fullServerName(), QStringLiteral("--pluginType"), m_pluginType, QStringLiteral("--pluginPath"), m_pluginPath});

    qCDebug(PURPOSE_EXTERNAL_PROCESS_LOG) << "launching..." << m_process->program() << m_process->arguments().join(QLatin1Char(' ')).constData();

    m_process->start();
}

void Purpose::ProcessJob::processStateChanged(QProcess::ProcessState state)
{
    if (state == QProcess::NotRunning) {
        Q_ASSERT(m_process->exitCode() != 0 || m_localSocket);
        if (m_process->exitCode() != 0) {
            qCWarning(PURPOSE_EXTERNAL_PROCESS_LOG) << "process exited with code:" << m_process->exitCode();
        }

        do {
            readSocket();
        } while (m_localSocket->waitForReadyRead());
        emitResult();
    }
}

#include "moc_processjob.cpp"
