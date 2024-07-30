/*
 *   SPDX-FileCopyrightText: 2010 Alejandro Fiestas Olivares <alex@eyeos.org>
 *   SPDX-FileCopyrightText: 2010 Eduardo Robles Elvira <edulix@gmail.com>
 *   SPDX-FileCopyrightText: 2010 UFO Coders <info@ufocoders.com>
 *   SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "requestconfirmation.h"
#include "bluedevil_kded.h"

#include <KLocalizedString>
#include <KNotification>

RequestConfirmation::RequestConfirmation(BluezQt::DevicePtr device, const QString &pin, QObject *parent)
    : QObject(parent)
    , m_device(device)
    , m_pin(pin)
{
    KNotification *notification = new KNotification(QStringLiteral("RequestConfirmation"), KNotification::Persistent, this);

    notification->setComponentName(QStringLiteral("bluedevil"));
    notification->setTitle(QStringLiteral("%1 (%2)").arg(m_device->name().toHtmlEscaped(), m_device->address()));
    notification->setText(
        i18nc("The text is shown in a notification to know if the PIN is correct,"
              "%1 is the remote bluetooth device and %2 is the pin",
              "%1 is asking if the PIN is correct: %2",
              m_device->name().toHtmlEscaped(),
              m_pin));

    auto correctAction = notification->addAction(i18nc("Notification button to know if the pin is correct or not", "PIN correct"));
    auto incorrectAction = notification->addAction(i18nc("Notification button to say that the PIN is wrong", "PIN incorrect"));

    connect(correctAction, &KNotificationAction::activated, this, &RequestConfirmation::pinCorrect);
    connect(incorrectAction, &KNotificationAction::activated, this, &RequestConfirmation::pinWrong);
    connect(notification, &KNotification::closed, this, &RequestConfirmation::pinWrong);
    connect(notification, &KNotification::ignored, this, &RequestConfirmation::pinWrong);
    connect(parent, SIGNAL(agentCanceled()), this, SLOT(pinWrong()));

    notification->sendEvent();
}

void RequestConfirmation::pinCorrect()
{
    qCDebug(BLUEDEVIL_KDED_LOG) << "PIN correct:" << m_device->name() << m_device->address();

    deleteLater();
    Q_EMIT done(Accept);
}

void RequestConfirmation::pinWrong()
{
    qCDebug(BLUEDEVIL_KDED_LOG) << "PIN wrong:" << m_device->name() << m_device->address();

    deleteLater();
    Q_EMIT done(Deny);
}

#include "moc_requestconfirmation.cpp"
