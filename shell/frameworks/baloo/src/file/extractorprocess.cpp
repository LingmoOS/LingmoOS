/*
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>
    SPDX-FileCopyrightText: 2015 Pinak Ahuja <pinak.ahuja@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "baloodebug.h"
#include "extractorprocess.h"

using namespace Baloo;

ExtractorProcess::ExtractorProcess(const QString& extractorPath, QObject* parent)
    : QObject(parent)
    , m_extractorPath(extractorPath)
    , m_extractorProcess(this)
    , m_controller(&m_extractorProcess, &m_extractorProcess)
{
    using ControllerPipe = Baloo::Private::ControllerPipe;

    connect(&m_extractorProcess, &QProcess::readyRead, &m_controller, &ControllerPipe::processStatusData);
    connect(&m_controller, &ControllerPipe::urlStarted, this, &ExtractorProcess::startedIndexingFile);
    connect(&m_controller, &ControllerPipe::urlFinished, this, [this](const QString& url) {
        Q_EMIT finishedIndexingFile(url, true);
    });
    connect(&m_controller, &ControllerPipe::urlFailed, this, [this](const QString& url) {
        Q_EMIT finishedIndexingFile(url, false);
    });
    connect(&m_controller, &ControllerPipe::batchFinished, this, [this]() {
        qCDebug(BALOO) << "Batch finished";
        Q_EMIT done();
    });

    connect(&m_extractorProcess, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), [=](int exitCode, QProcess::ExitStatus exitStatus) {
        if (exitStatus == QProcess::CrashExit) {
            qCWarning(BALOO) << "Extractor crashed";
            Q_EMIT failed();
        } else if (exitCode == 1) {
            // DB open error
            Q_EMIT failed();
        } else if (exitCode == 2) {
            // DB transaction commit error
            Q_EMIT failed();
        } else if (exitCode == 253) {
            // DrKonqi mangles signals depending on the core_pattern
            // and does a regular exit with status 253 instead
            qCWarning(BALOO) << "Extractor probably crashed";
            Q_EMIT failed();
        } else if (exitCode != 0) {
            qCWarning(BALOO) << "Unexpected exit code:" << exitCode;
            Q_EMIT failed();
        }
    });

    m_extractorProcess.setProgram(m_extractorPath);
    m_extractorProcess.setProcessChannelMode(QProcess::ForwardedErrorChannel);
    m_extractorProcess.start();
}

ExtractorProcess::~ExtractorProcess()
{
    m_extractorProcess.closeWriteChannel();
    m_extractorProcess.waitForFinished();
    m_extractorProcess.close();
}

void ExtractorProcess::start()
{
    m_extractorProcess.start(QIODevice::Unbuffered | QIODevice::ReadWrite);
    m_extractorProcess.waitForStarted();
    m_extractorProcess.setReadChannel(QProcess::StandardOutput);
}

void ExtractorProcess::index(const QVector<quint64>& fileIds)
{
    Q_ASSERT(!fileIds.isEmpty());
    m_controller.processIds(fileIds);
}

#include "moc_extractorprocess.cpp"
