// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "nmnetworkmanager.h"

#include <QDBusConnection>

namespace NetworkManager {

QDBusPendingReply<QDBusObjectPath> activateConnection2(const QString &connectionUni, const QString &interfaceUni, const QString &connectionParameter, const QVariantMap &options)
{
    QString extra_interface_parameter = interfaceUni;
    QString extra_connection_parameter = connectionParameter;
    if (extra_interface_parameter.isEmpty()) {
        extra_interface_parameter = QLatin1String("/");
    }
    if (extra_connection_parameter.isEmpty()) {
        extra_connection_parameter = QLatin1String("/");
    }
    // TODO store error code
    QDBusObjectPath connPath(connectionUni);
    QDBusObjectPath interfacePath(interfaceUni);

    QDBusMessage msg = QDBusMessage::createMethodCall("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager", "org.freedesktop.NetworkManager", "ActivateConnection2");
    msg << connPath << QDBusObjectPath(extra_interface_parameter) << QDBusObjectPath(extra_connection_parameter) << options;
    return QDBusConnection::systemBus().asyncCall(msg);
}
} // namespace NetworkManager
