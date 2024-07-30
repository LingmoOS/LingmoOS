/*
 * This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2010 Alejandro Fiestas Olivares <afiestas@kde.org>
 * SPDX-FileCopyrightText: 2010-2011 UFO Coders <info@ufocoders.com>
 * SPDX-FileCopyrightText: 2014 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "success.h"
#include "../bluewizard.h"
#include "bluedevil_wizard.h"

#include <QIcon>

#include <BluezQt/Device>

#include <KNotification>

SuccessPage::SuccessPage(BlueWizard *parent)
    : QWizardPage(parent)
    , m_wizard(parent)
{
}

int SuccessPage::nextId() const
{
    return -1;
}

void SuccessPage::initializePage()
{
    qCDebug(BLUEDEVIL_WIZARD_LOG) << "Sending Success notification";

    BluezQt::DevicePtr device = m_wizard->device();

    KNotification *notification = new KNotification(QStringLiteral("SetupFinished"), KNotification::CloseOnTimeout, this);
    notification->setComponentName(QStringLiteral("bluedevil"));
    notification->setTitle(i18n("Setup Finished"));
    if (device->name().isEmpty()) {
        notification->setText(i18n("The device has been set up and can now be used."));
    } else {
        notification->setText(i18nc("Placeholder is device name", "The device '%1' has been set up and can now be used.", device->name()));
    }
    // Mark as response to explicit user action ("pairing the device")
    notification->setHint(QStringLiteral("x-kde-user-action-feedback"), true);
    notification->sendEvent();

    setFinalPage(true);
}

#include "moc_success.cpp"
