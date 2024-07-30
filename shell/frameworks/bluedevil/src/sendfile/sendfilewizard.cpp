/*
 * This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2010-2011 Alejandro Fiestas Olivares <afiestas@kde.org>
 * SPDX-FileCopyrightText: 2010-2011 UFO Coders <info@ufocoders.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "sendfilewizard.h"
#include "bluedevil_sendfile.h"
#include "sendfilesjob.h"

#include "pages/connectingpage.h"
#include "pages/failpage.h"
#include "pages/selectdeviceandfilespage.h"
#include "pages/selectdevicepage.h"
#include "pages/selectfilespage.h"

#include <QApplication>
#include <QPushButton>
#include <QUrl>

#include <KIO/JobTracker>
#include <KLocalizedString>
#include <KStatusBarJobTracker>

#include <BluezQt/Adapter>
#include <BluezQt/Device>
#include <BluezQt/InitManagerJob>
#include <BluezQt/ObexManager>
#include <BluezQt/PendingCall>

SendFileWizard::SendFileWizard(const QString &device, const QStringList &files)
    : QWizard()
    , m_deviceUrl(device)
    , m_files(files)
    , m_job(nullptr)
{
    setOption(NoCancelButton, false);
    setButton(QWizard::NextButton, new QPushButton(QIcon::fromTheme(QStringLiteral("document-export")), i18n("Send Files")));
    setButton(QWizard::CancelButton, new QPushButton(QIcon::fromTheme(QStringLiteral("dialog-cancel")), i18n("Cancel")));
    setOption(QWizard::DisabledBackButtonOnLastPage);
    setOption(QWizard::NoBackButtonOnStartPage);

    qCDebug(BLUEDEVIL_SENDFILE_LOG) << "Device" << m_deviceUrl;
    qCDebug(BLUEDEVIL_SENDFILE_LOG) << "Files" << m_files;

    // Initialize BluezQt
    m_manager = new BluezQt::Manager(this);
    BluezQt::InitManagerJob *initJob = m_manager->init();
    initJob->start();
    connect(initJob, &BluezQt::InitManagerJob::result, this, &SendFileWizard::initJobResult);

    // Make sure that obexd is running
    BluezQt::PendingCall *servicePendingCall = BluezQt::ObexManager::startService();
    connect(servicePendingCall, &BluezQt::PendingCall::finished, this, &SendFileWizard::slotServicePendingCallFinished);
}

SendFileWizard::~SendFileWizard()
{
    if (m_job) {
        m_job->doKill();
    }
}

void SendFileWizard::done(int result)
{
    qCDebug(BLUEDEVIL_SENDFILE_LOG) << "Wizard done: " << result;

    QWizard::done(result);

    if (!m_job) {
        qApp->quit();
    }
}

BluezQt::Manager *SendFileWizard::manager() const
{
    return m_manager;
}

QStringList SendFileWizard::files() const
{
    return m_files;
}

void SendFileWizard::setFiles(const QStringList &files)
{
    m_files = files;
}

BluezQt::DevicePtr SendFileWizard::device() const
{
    return m_device;
}

void SendFileWizard::setDevice(BluezQt::DevicePtr device)
{
    m_device = device;
}

void SendFileWizard::startTransfer(const QDBusObjectPath &session)
{
    if (m_files.isEmpty()) {
        qCDebug(BLUEDEVIL_SENDFILE_LOG) << "No files to send";
        return;
    }

    if (!m_device) {
        qCDebug(BLUEDEVIL_SENDFILE_LOG) << "No device selected";
        return;
    }

    m_job = new SendFilesJob(m_files, m_device, session);
    connect(m_job, &SendFilesJob::destroyed, qApp, &QCoreApplication::quit);

    KIO::getJobTracker()->registerJob(m_job);
    m_job->start();

    done(1);
}

void SendFileWizard::initJobResult(BluezQt::InitManagerJob *job)
{
    if (job->error()) {
        qCWarning(BLUEDEVIL_SENDFILE_LOG) << "Error initializing manager:" << job->errorText();
        qApp->exit();
        return;
    }

    qCDebug(BLUEDEVIL_SENDFILE_LOG) << "Manager initialized";

    if (!m_obexServiceInitialized) {
        // org.bluez.obex is not ready yet, so init the wizard later
        connect(this, &SendFileWizard::obexServiceInitialized, this, &SendFileWizard::initWizard);
        return;
    }

    initWizard();
}

void SendFileWizard::slotServicePendingCallFinished(BluezQt::PendingCall *call)
{
    m_obexServiceInitialized = true;
    m_obexdServiceError = static_cast<BluezQt::PendingCall::Error>(call->error());

    if (m_obexdServiceError != BluezQt::PendingCall::NoError) {
        setErrorMessage(i18nc("@info:status %1 error message", "Failed to start org.bluez.obex service: %1", call->errorText()));
    }

    Q_EMIT obexServiceInitialized();
}

void SendFileWizard::initWizard()
{
    if (m_obexdServiceError != BluezQt::PendingCall::NoError) {
        // Error message is set in slotServicePendingCallFinished
        auto earlyFailPage = new FailPage(this);
        earlyFailPage->setErrorMessage(errorMessage());
        addPage(earlyFailPage);
        show();
        return;
    }

    // KIO address: bluetooth://40-87-2b-1a-39-28/00001105-0000-1000-8000-00805F9B34FB
    if (m_deviceUrl.startsWith(QLatin1String("bluetooth"))) {
        QString address = QUrl(m_deviceUrl).host();
        address.replace(QLatin1Char('-'), QLatin1Char(':'));
        m_device = m_manager->deviceForAddress(address.toUpper());
    } else {
        m_device = m_manager->deviceForUbi(m_deviceUrl);
    }

    // If the device's adapter is powered off, sending file would fail.
    // Let the user know about it!
    if (m_device && !m_device->adapter()->isPowered()) {
        m_device.clear();
    }

    if (m_device) {
        if (m_files.isEmpty()) {
            addPage(new SelectFilesPage(this));
        }
    } else {
        if (m_files.isEmpty()) {
            addPage(new SelectDeviceAndFilesPage(this));
        } else {
            addPage(new SelectDevicePage(this));
        }
    }

    addPage(new ConnectingPage(this));
    addPage(new FailPage(this));

    // Only show wizard after init is completed
    show();
}

QString SendFileWizard::errorMessage() const
{
    return m_errorMessage;
}

void SendFileWizard::setErrorMessage(const QString &message)
{
    m_errorMessage = message;
}

#include "moc_sendfilewizard.cpp"
