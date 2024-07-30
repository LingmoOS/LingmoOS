/*
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "gattmanagerinterface.h"
#include "objectmanager.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDBusPendingReply>

GattManagerInterface::GattManagerInterface(const QDBusObjectPath &path, QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    setName(QStringLiteral("org.bluez.GattManager1"));
    setPath(path);
}

void GattManagerInterface::runAction(const QString &actionName, const QVariantMap &properties)
{
    if (actionName == QLatin1String("get-objects")) {
        runGetObjectsAction();
    } else if (actionName == QLatin1String("read-charc")) {
        runReadCharcAction(properties);
    } else if (actionName == QLatin1String("write-charc")) {
        runWriteCharcAction(properties);
    }
}

void GattManagerInterface::RegisterApplication(const QDBusObjectPath &path, const QVariantMap & /*options*/, const QDBusMessage &msg)
{
    m_application = path;
    m_service = msg.service();
}

void GattManagerInterface::UnregisterApplication(const QDBusObjectPath &path, const QDBusMessage &msg)
{
    if (m_application == path && m_service == msg.service()) {
        m_application = QDBusObjectPath();
        m_service.clear();
    }
}

void GattManagerInterface::runGetObjectsAction()
{
    QDBusMessage call = QDBusMessage::createMethodCall(m_service,
                                                       m_application.path(),
                                                       QStringLiteral("org.freedesktop.DBus.ObjectManager"),
                                                       QStringLiteral("GetManagedObjects"));

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(QDBusConnection::sessionBus().asyncCall(call));
    connect(watcher, &QDBusPendingCallWatcher::finished, [this](QDBusPendingCallWatcher *watcher) {
        const QDBusPendingReply<DBusManagerStruct> &reply = *watcher;
        watcher->deleteLater();
        if (reply.isError()) {
            return;
        }

        DBusManagerStruct objects = reply.value();
        for (const auto &object : objects.keys()) {
            if (object.path().contains(QLatin1String("char"))) {
                m_characteristic = object;
                break;
            }
        }
    });
}

void GattManagerInterface::runReadCharcAction(const QVariantMap &properties)
{
    QDBusMessage call =
        QDBusMessage::createMethodCall(m_service, m_characteristic.path(), QStringLiteral("org.bluez.GattCharacteristic1"), QStringLiteral("ReadValue"));
    call << properties.value(QStringLiteral("Options"));
    QDBusConnection::sessionBus().asyncCall(call);
}

void GattManagerInterface::runWriteCharcAction(const QVariantMap &properties)
{
    QDBusMessage call =
        QDBusMessage::createMethodCall(m_service, m_characteristic.path(), QStringLiteral("org.bluez.GattCharacteristic1"), QStringLiteral("WriteValue"));
    call << properties.value(QStringLiteral("Value"));
    call << properties.value(QStringLiteral("Options"));
    QDBusConnection::sessionBus().asyncCall(call);
}

#include "moc_gattmanagerinterface.cpp"
