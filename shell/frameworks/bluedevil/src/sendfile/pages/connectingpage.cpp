/*
 * This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2010-2011 Alejandro Fiestas Olivares <afiestas@kde.org>
 * SPDX-FileCopyrightText: 2010-2011 UFO Coders <info@ufocoders.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "connectingpage.h"
#include "../sendfilewizard.h"
#include "bluedevil_sendfile.h"

#include <QDBusObjectPath>

#include <KLocalizedString>

#include <BluezQt/Device>
#include <BluezQt/InitObexManagerJob>
#include <BluezQt/PendingCall>

ConnectingPage::ConnectingPage(SendFileWizard *wizard)
    : QWizardPage(wizard)
    , m_wizard(wizard)
{
    setupUi(this);
}

void ConnectingPage::initializePage()
{
    m_device = m_wizard->device();
    connLabel->setText(i18nc("Connecting to a Bluetooth device", "Connecting to %1…", m_device->name()));

    m_wizard->setWindowTitle(QString());
    m_wizard->setButtonLayout(wizardButtonsLayout());

    // Init BluezQt
    BluezQt::ObexManager *manager = new BluezQt::ObexManager(this);
    BluezQt::InitObexManagerJob *job = manager->init();
    job->start();
    connect(job, &BluezQt::InitObexManagerJob::result, this, &ConnectingPage::initJobResult);
}

bool ConnectingPage::isComplete() const
{
    return false;
}

void ConnectingPage::initJobResult(BluezQt::InitObexManagerJob *job)
{
    if (job->error()) {
        qCWarning(BLUEDEVIL_SENDFILE_LOG) << "Error initializing obex manager" << job->errorText();
        m_wizard->next();
        return;
    }

    // Create ObjectPush session
    QVariantMap map;
    map[QStringLiteral("Target")] = QStringLiteral("opp");
    BluezQt::PendingCall *call = job->manager()->createSession(m_device->address(), map);
    connect(call, &BluezQt::PendingCall::finished, this, &ConnectingPage::createSessionFinished);
}

void ConnectingPage::createSessionFinished(BluezQt::PendingCall *call)
{
    if (call->error()) {
        qCWarning(BLUEDEVIL_SENDFILE_LOG) << "Error creating session" << call->errorText();
        m_wizard->setErrorMessage(call->errorText());
        m_wizard->next();
        return;
    }

    m_wizard->startTransfer(call->value().value<QDBusObjectPath>());
}

QList<QWizard::WizardButton> ConnectingPage::wizardButtonsLayout() const
{
    QList<QWizard::WizardButton> list;
    list << QWizard::Stretch;
    list << QWizard::CancelButton;
    return list;
}

#include "moc_connectingpage.cpp"
