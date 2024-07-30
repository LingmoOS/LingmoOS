/*
 * SPDX-FileCopyrightText: 2018 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "mediainterface.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>

MediaInterface::MediaInterface(const QDBusObjectPath &path, QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    setName(QStringLiteral("org.bluez.Media1"));
    setPath(path);
}

void MediaInterface::runAction(const QString &actionName, const QVariantMap &properties)
{
    if (actionName == QLatin1String("set-configuration")) {
        runSetConfigurationAction(properties);
    } else if (actionName == QLatin1String("select-configuration")) {
        runSelectConfigurationAction(properties);
    } else if (actionName == QLatin1String("clear-configuration")) {
        runClearConfigurationAction(properties);
    } else if (actionName == QLatin1String("release")) {
        runReleaseAction();
    }
}

void MediaInterface::RegisterEndpoint(const QDBusObjectPath &path, const QVariantMap &properties, const QDBusMessage &msg)
{
    m_endpoint = path;
    m_service = msg.service();
    m_properties = properties;
}

void MediaInterface::UnregisterEndpoint(const QDBusObjectPath &path, const QDBusMessage &msg)
{
    if (m_endpoint == path && m_service == msg.service()) {
        m_endpoint = QDBusObjectPath();
        m_service.clear();
        m_properties.clear();
    }
}

void MediaInterface::runSetConfigurationAction(const QVariantMap &properties)
{
    QDBusMessage call =
        QDBusMessage::createMethodCall(m_service, m_endpoint.path(), QStringLiteral("org.bluez.MediaEndpoint1"), QStringLiteral("SetConfiguration"));
    call << QVariant::fromValue(properties.value(QStringLiteral("Transport")).value<QDBusObjectPath>());
    call << properties.value(QStringLiteral("Properties"));
    QDBusConnection::sessionBus().asyncCall(call);
}

void MediaInterface::runSelectConfigurationAction(const QVariantMap &properties)
{
    QDBusMessage call =
        QDBusMessage::createMethodCall(m_service, m_endpoint.path(), QStringLiteral("org.bluez.MediaEndpoint1"), QStringLiteral("SelectConfiguration"));
    call << properties.value(QStringLiteral("Capabilities"));
    QDBusConnection::sessionBus().asyncCall(call);
}

void MediaInterface::runClearConfigurationAction(const QVariantMap &properties)
{
    QDBusMessage call =
        QDBusMessage::createMethodCall(m_service, m_endpoint.path(), QStringLiteral("org.bluez.MediaEndpoint1"), QStringLiteral("ClearConfiguration"));
    call << QVariant::fromValue(properties.value(QStringLiteral("Transport")).value<QDBusObjectPath>());
    QDBusConnection::sessionBus().asyncCall(call);
}

void MediaInterface::runReleaseAction()
{
    QDBusMessage call = QDBusMessage::createMethodCall(m_service, m_endpoint.path(), QStringLiteral("org.bluez.MediaEndpoint1"), QStringLiteral("Release"));
    QDBusConnection::sessionBus().asyncCall(call);
}

#include "moc_mediainterface.cpp"
