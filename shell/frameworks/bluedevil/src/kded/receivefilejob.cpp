/*
 *  SPDX-FileCopyrightText: 2013 Alejandro Fiestas Fiestas <afiestas@kde.org>
 *  SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "receivefilejob.h"
#include "bluedevil_kded.h"
#include "filereceiversettings.h"
#include "obexagent.h"

#include <QDir>
#include <QIcon>
#include <QTemporaryFile>
#include <QTimer>

#include <KIO/CopyJob>
#include <KIO/JobTracker>
#include <KJobTrackerInterface>
#include <KLocalizedString>
#include <KNotification>

#include <BluezQt/Adapter>
#include <BluezQt/Device>
#include <BluezQt/Manager>
#include <BluezQt/ObexSession>

ReceiveFileJob::ReceiveFileJob(const BluezQt::Request<QString> &req, BluezQt::ObexTransferPtr transfer, BluezQt::ObexSessionPtr session, ObexAgent *parent)
    : KJob(parent)
    , m_speedBytes(0)
    , m_agent(parent)
    , m_transfer(transfer)
    , m_session(session)
    , m_request(req)
{
    setCapabilities(Killable);
    // Pretend to be BlueDevil's file transfer agent even though we're run inside kded
    setProperty("desktopFileName", QStringLiteral("org.kde.bluedevilsendfile"));
    setProperty("immediateProgressReporting", true);
}

QString ReceiveFileJob::deviceAddress() const
{
    return m_deviceAddress;
}

void ReceiveFileJob::start()
{
    QMetaObject::invokeMethod(this, "init", Qt::QueuedConnection);
}

bool ReceiveFileJob::doKill()
{
    qCDebug(BLUEDEVIL_KDED_LOG) << "ReceiveFileJob-Kill";
    m_transfer->cancel();
    return true;
}

void ReceiveFileJob::init()
{
    qCDebug(BLUEDEVIL_KDED_LOG) << "ReceiveFileJob:";
    qCDebug(BLUEDEVIL_KDED_LOG) << "\tName:" << m_transfer->name();
    qCDebug(BLUEDEVIL_KDED_LOG) << "\tFilename:" << m_transfer->fileName();
    qCDebug(BLUEDEVIL_KDED_LOG) << "\tStatus:" << m_transfer->status();
    qCDebug(BLUEDEVIL_KDED_LOG) << "\tType:" << m_transfer->type();
    qCDebug(BLUEDEVIL_KDED_LOG) << "\tSize:" << m_transfer->size();
    qCDebug(BLUEDEVIL_KDED_LOG) << "\tTransferred:" << m_transfer->transferred();

    qCDebug(BLUEDEVIL_KDED_LOG) << "ObexSession:";
    qCDebug(BLUEDEVIL_KDED_LOG) << "\tSource:" << m_session->source();
    qCDebug(BLUEDEVIL_KDED_LOG) << "\tDestination:" << m_session->destination();

    connect(m_transfer.data(), &BluezQt::ObexTransfer::statusChanged, this, &ReceiveFileJob::statusChanged);
    connect(m_transfer.data(), &BluezQt::ObexTransfer::transferredChanged, this, &ReceiveFileJob::transferredChanged);

    m_deviceName = m_session->destination();

    BluezQt::AdapterPtr adapter = m_agent->manager()->adapterForAddress(m_session->source());
    if (!adapter) {
        qCDebug(BLUEDEVIL_KDED_LOG) << "No adapter for" << m_session->source();
        showNotification();
        return;
    }

    BluezQt::DevicePtr device = adapter->deviceForAddress(m_session->destination());
    if (!device) {
        qCDebug(BLUEDEVIL_KDED_LOG) << "No device for" << m_session->destination();
        showNotification();
        return;
    }

    m_deviceName = device->name();
    m_deviceAddress = device->address();

    if (m_agent->shouldAutoAcceptTransfer(m_deviceAddress)) {
        slotAccept();
        return;
    }

    FileReceiverSettings::self()->load();
    switch (FileReceiverSettings::self()->autoAccept()) {
    case 0: // Never auto-accept transfers
        showNotification();
        break;

    case 1: // Auto-accept only from trusted devices
        if (device->isTrusted()) {
            qCDebug(BLUEDEVIL_KDED_LOG) << "Auto-accepting transfer for trusted device";
            slotAccept();
        } else {
            showNotification();
        }
        break;

    case 2: // Auto-accept all transfers
        qCDebug(BLUEDEVIL_KDED_LOG) << "Auto-accepting transfers for all devices";
        slotAccept();
        break;

    default: // Unknown
        showNotification();
        break;
    }
}

void ReceiveFileJob::showNotification()
{
    KNotification *notification = new KNotification(QStringLiteral("IncomingFile"), KNotification::Persistent, this);

    notification->setTitle(QStringLiteral("%1 (%2)").arg(m_deviceName.toHtmlEscaped(), m_deviceAddress));
    notification->setText(i18nc("Show a notification asking to authorize or deny an incoming file transfer to this computer from a Bluetooth device.",
                                "%1 is sending you the file %2",
                                m_deviceName.toHtmlEscaped(),
                                m_transfer->name()));

    auto acceptAction =
        notification->addAction(i18nc("Button to accept the incoming file transfer and download it in the default download directory", "Accept"));
    auto cancelAction = notification->addAction(i18nc("Deny the incoming file transfer", "Cancel"));

    connect(acceptAction, &KNotificationAction::activated, this, &ReceiveFileJob::slotAccept);
    connect(cancelAction, &KNotificationAction::activated, this, &ReceiveFileJob::slotCancel);
    connect(notification, &KNotification::closed, this, &ReceiveFileJob::slotCancel);

    notification->setComponentName(QStringLiteral("bluedevil"));

    notification->sendEvent();
}

void ReceiveFileJob::slotAccept()
{
    qCDebug(BLUEDEVIL_KDED_LOG) << "ReceiveFileJob-Accept";

    KIO::getJobTracker()->registerJob(this);

    FileReceiverSettings::self()->load();
    m_targetPath = FileReceiverSettings::self()->saveUrl().adjusted(QUrl::StripTrailingSlash);
    m_targetPath.setPath(m_targetPath.path() + QLatin1Char('/') + m_transfer->name());

    setTotalAmount(Files, 1);

    Q_EMIT description(this,
                       i18nc("@title job", "Receiving file"),
                       QPair<QString, QString>(i18nc("File transfer origin", "From"), m_deviceName),
                       QPair<QString, QString>(i18nc("File transfer destination", "To"), m_targetPath.toDisplayString()));

    m_tempPath = createTempPath(m_transfer->name());
    qCDebug(BLUEDEVIL_KDED_LOG) << "TempPath" << m_tempPath;

    m_accepted = true;
    m_request.accept(m_tempPath);
}

void ReceiveFileJob::slotCancel()
{
    if (!m_accepted && m_transfer->status() == BluezQt::ObexTransfer::Queued) {
        qCDebug(BLUEDEVIL_KDED_LOG) << "Cancel Push";
        m_request.reject();
        setError(KJob::UserDefinedError);
        emitResult();
    }
}

void ReceiveFileJob::moveFinished(KJob *job)
{
    if (job->error()) {
        qCDebug(BLUEDEVIL_KDED_LOG) << job->error();
        qCDebug(BLUEDEVIL_KDED_LOG) << job->errorText();
        setError(job->error());
        setErrorText(i18n("Saving file failed"));

        QFile::remove(m_tempPath);
    }

    setProcessedAmount(Files, 1);

    emitResult();
}

void ReceiveFileJob::statusChanged(BluezQt::ObexTransfer::Status status)
{
    switch (status) {
    case BluezQt::ObexTransfer::Active:
        qCDebug(BLUEDEVIL_KDED_LOG) << "ReceiveFileJob-Transfer Active";
        setTotalAmount(Bytes, m_transfer->size());
        setProcessedAmount(Bytes, 0);
        m_time = QTime::currentTime();
        break;

    case BluezQt::ObexTransfer::Complete: {
        qCDebug(BLUEDEVIL_KDED_LOG) << "ReceiveFileJob-Transfer Complete";
        KIO::CopyJob *job = KIO::move(QUrl::fromLocalFile(m_tempPath), m_targetPath, KIO::HideProgressInfo);
        job->setUiDelegate(nullptr);
        connect(job, &KIO::CopyJob::finished, this, &ReceiveFileJob::moveFinished);
        break;
    }

    case BluezQt::ObexTransfer::Error:
        qCDebug(BLUEDEVIL_KDED_LOG) << "ReceiveFileJob-Transfer Error";
        setError(KJob::UserDefinedError);
        setErrorText(i18n("Bluetooth transfer failed"));

        // Delay emitResult to make sure notification is displayed even
        // when transfer errors right after accepting it
        QTimer::singleShot(500, this, [this]() {
            emitResult();
        });
        break;

    default:
        qCDebug(BLUEDEVIL_KDED_LOG) << "Not implemented status: " << status;
        break;
    }
}

void ReceiveFileJob::transferredChanged(quint64 transferred)
{
    // qCDebug(BLUEDEVIL_KDED_LOG) << "ReceiveFileJob-Transferred" << transferred;

    // If at least 1 second has passed since last update
    int secondsSinceLastTime = m_time.secsTo(QTime::currentTime());
    if (secondsSinceLastTime > 0) {
        unsigned long speed = (transferred - m_speedBytes) / secondsSinceLastTime;
        emitSpeed(speed);

        m_time = QTime::currentTime();
        m_speedBytes = transferred;
    }

    setProcessedAmount(Bytes, transferred);
}

QString ReceiveFileJob::createTempPath(const QString &fileName) const
{
    QString xdgCacheHome = QFile::decodeName(qgetenv("XDG_CACHE_HOME"));
    if (xdgCacheHome.isEmpty()) {
        xdgCacheHome = QDir::homePath() + QStringLiteral("/.cache");
    }

    xdgCacheHome.append(QLatin1String("/obexd/"));
    QString path = xdgCacheHome + fileName;

    int i = 0;
    while (QFile::exists(path)) {
        path = xdgCacheHome + fileName + QString::number(i);
        i++;
    }

    return path;
}

#include "moc_receivefilejob.cpp"
