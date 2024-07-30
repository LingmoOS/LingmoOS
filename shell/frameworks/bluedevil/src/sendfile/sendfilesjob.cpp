/*
 * This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2010-2011 Alejandro Fiestas Olivares <afiestas@kde.org>
 * SPDX-FileCopyrightText: 2010-2011 UFO Coders <info@ufocoders.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "sendfilesjob.h"
#include "bluedevil_sendfile.h"

#include <QDBusObjectPath>
#include <QFile>
#include <QUrl>

#include <KLocalizedString>

#include <BluezQt/Device>
#include <BluezQt/InitObexManagerJob>
#include <BluezQt/ObexManager>
#include <BluezQt/ObexObjectPush>
#include <BluezQt/PendingCall>

SendFilesJob::SendFilesJob(const QStringList &files, BluezQt::DevicePtr device, const QDBusObjectPath &session, QObject *parent)
    : KJob(parent)
    , m_files(files)
    , m_progress(0)
    , m_totalSize(0)
    , m_speedBytes(0)
    , m_currentFileSize(0)
    , m_currentFileProgress(0)
    , m_device(device)
{
    qCDebug(BLUEDEVIL_SENDFILE_LOG) << "SendFilesJob:" << files;

    for (const QString &filePath : files) {
        QFile file(filePath);
        m_filesSizes << file.size();
        m_totalSize += file.size();
    }

    setCapabilities(Killable);

    m_objectPush = new BluezQt::ObexObjectPush(session, this);
}

void SendFilesJob::start()
{
    QMetaObject::invokeMethod(this, "doStart", Qt::QueuedConnection);
}

bool SendFilesJob::doKill()
{
    if (m_transfer) {
        m_transfer->cancel();
    }
    return true;
}

void SendFilesJob::doStart()
{
    qCDebug(BLUEDEVIL_SENDFILE_LOG) << "SendFilesJob-DoStart";

    setTotalAmount(Files, m_files.count());
    setTotalAmount(Bytes, m_totalSize);
    setProcessedAmount(Bytes, 0);

    nextJob();
}

void SendFilesJob::nextJob()
{
    qCDebug(BLUEDEVIL_SENDFILE_LOG) << "SendFilesJob-NextJob";

    m_transfer.clear();
    m_currentFile = m_files.takeFirst();
    m_currentFileSize = m_filesSizes.takeFirst();
    // Starts at 1 file.
    setProcessedAmount(Files, totalAmount(Files) - m_files.count());

    Q_EMIT description(this,
                       i18n("Sending file over Bluetooth"),
                       QPair<QString, QString>(i18nc("File transfer origin", "From"), m_currentFile),
                       QPair<QString, QString>(i18nc("File transfer destination", "To"), m_device->name()));

    BluezQt::PendingCall *call = m_objectPush->sendFile(m_currentFile);
    connect(call, &BluezQt::PendingCall::finished, this, &SendFilesJob::sendFileFinished);
}

void SendFilesJob::sendFileFinished(BluezQt::PendingCall *call)
{
    if (call->error()) {
        qCWarning(BLUEDEVIL_SENDFILE_LOG) << "Error sending file" << call->errorText();
        setError(UserDefinedError);
        setErrorText(call->errorText());
        emitResult();
        return;
    }

    m_transfer = call->value().value<BluezQt::ObexTransferPtr>();
    connect(m_transfer.data(), &BluezQt::ObexTransfer::statusChanged, this, &SendFilesJob::statusChanged);
    connect(m_transfer.data(), &BluezQt::ObexTransfer::transferredChanged, this, &SendFilesJob::transferredChanged);
}

void SendFilesJob::jobDone()
{
    qCDebug(BLUEDEVIL_SENDFILE_LOG) << "SendFilesJob-JobDone";

    m_speedBytes = 0;
    m_currentFileSize = 0;
    m_currentFileProgress = 0;

    if (!m_files.isEmpty()) {
        nextJob();
        return;
    }

    emitResult();
}

void SendFilesJob::transferredChanged(quint64 transferred)
{
    // qCDebug(SENDFILE) << "SendFilesJob-Transferred" << transferred;

    // If at least 1 second has passed since last update
    int secondsSinceLastTime = m_time.secsTo(QTime::currentTime());
    if (secondsSinceLastTime > 0) {
        float speed = (transferred - m_speedBytes) / secondsSinceLastTime;
        emitSpeed(speed);

        m_time = QTime::currentTime();
        m_speedBytes = transferred;
    }

    progress(transferred);
}

void SendFilesJob::statusChanged(BluezQt::ObexTransfer::Status status)
{
    switch (status) {
    case BluezQt::ObexTransfer::Active:
        qCDebug(BLUEDEVIL_SENDFILE_LOG) << "SendFilesJob-Transfer Active";
        m_time = QTime::currentTime();
        break;

    case BluezQt::ObexTransfer::Complete:
        qCDebug(BLUEDEVIL_SENDFILE_LOG) << "SendFilesJob-Transfer Complete";
        jobDone();
        break;

    case BluezQt::ObexTransfer::Error:
        qCDebug(BLUEDEVIL_SENDFILE_LOG) << "SendFilesJob-Transfer Error";
        setError(UserDefinedError);
        setErrorText(i18n("Bluetooth transfer failed"));
        emitResult();
        break;

    default:
        qCWarning(BLUEDEVIL_SENDFILE_LOG) << "Not implemented status: " << status;
        break;
    }
}

void SendFilesJob::progress(quint64 transferBytes)
{
    quint64 toAdd = transferBytes - m_currentFileProgress;
    m_currentFileProgress = transferBytes;
    m_progress += toAdd;
    setProcessedAmount(Bytes, m_progress);
}

#include "moc_sendfilesjob.cpp"
