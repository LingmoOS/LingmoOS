/*
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "leadvertisingmanagerinterface.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>

LEAdvertisingManagerInterface::LEAdvertisingManagerInterface(const QDBusObjectPath &path, QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    setName(QStringLiteral("org.bluez.LEAdvertisingManager1"));
    setPath(path);
}

void LEAdvertisingManagerInterface::runAction(const QString &actionName, const QVariantMap & /*properties*/)
{
    if (actionName == QLatin1String("release")) {
        runReleaseAction();
    }
}

void LEAdvertisingManagerInterface::RegisterAdvertisement(const QDBusObjectPath &path, const QVariantMap & /*options*/, const QDBusMessage &msg)
{
    m_advertisement = path;
    m_service = msg.service();
}

void LEAdvertisingManagerInterface::UnregisterAdvertisement(const QDBusObjectPath &path, const QDBusMessage &msg)
{
    if (m_advertisement == path && m_service == msg.service()) {
        m_advertisement = QDBusObjectPath();
        m_service.clear();
    }
}

void LEAdvertisingManagerInterface::runReleaseAction()
{
    QDBusMessage call =
        QDBusMessage::createMethodCall(m_service, m_advertisement.path(), QStringLiteral("org.bluez.LEAdvertisement1"), QStringLiteral("Release"));
    QDBusConnection::sessionBus().asyncCall(call);
}

#include "moc_leadvertisingmanagerinterface.cpp"
