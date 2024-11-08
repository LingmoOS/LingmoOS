/*
 * liblingmosdk-base's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Shengjie Ji <jishengjie@kylinos.cn>
 *
 */

#include <QDBusConnection>
#include <QDBusMessage>

#include "dbus.hpp"
#include "log.hpp"

namespace kdk
{
namespace kabase
{

DBus::DBus() = default;

DBus::~DBus() = default;

bool DBus::registerService(QString serviceName)
{
    if (serviceName.isEmpty()) {
        error << "kabase : service is empty!";
    }

    QDBusConnection connection = QDBusConnection::sessionBus();

    if (!connection.registerService(serviceName)) {
        error << "kabase : dbus register service fail " << connection.lastError().message();
        return false;
    }

    return true;
}

bool DBus::registerObject(QString objectPath, QObject *object)
{
    if (objectPath.isEmpty() || object == nullptr) {
        error << "kabase : args error!";
        return false;
    }

    QDBusConnection connection = QDBusConnection::sessionBus();

    if (!connection.registerObject(objectPath, object, QDBusConnection::ExportAllSlots)) {
        error << "kabase : dbus register object fail " << connection.lastError().message();
        return false;
    }

    return true;
}

QList<QVariant> DBus::callMethod(QString serviceName, QString objectPath, QString interfaceName, QString methodName,
                                 QList<QVariant> args)
{
    QList<QVariant> ret;
    ret.clear();

    if (serviceName.isEmpty() || objectPath.isEmpty() || interfaceName.isEmpty() || methodName.isEmpty()) {
        error << "kabase : args error!";
        return ret;
    }

    QDBusMessage message = QDBusMessage::createMethodCall(serviceName, objectPath, interfaceName, methodName);
    if (!args.isEmpty()) {
        message.setArguments(args);
    }

    QDBusMessage reply = QDBusConnection::sessionBus().call(message);
    if (reply.type() == QDBusMessage::ReplyMessage) {
        ret = reply.arguments();
        return ret;
    } else {
        error << "kabase : dbus call method fail " << reply.errorMessage();
    }

    return ret;
}

} /* namespace kabase */
} /* namespace kdk */
