/*
 * liblingmosdk-ukenv's Library
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

#include <unistd.h>

#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusReply>
#include <QDebug>

#include "usermanual.h"

namespace kdk
{

namespace
{
constexpr char dbusObjectPath[] = "/";
constexpr char dbusInterfaceName[] = "com.guide.hotel";
constexpr char dbusCallUserManualMethod[] = "showGuide";
} // namespace

UserManual::UserManual() = default;

UserManual::~UserManual() = default;

bool UserManual::callUserManual(QString appName)
{
    if (appName.isEmpty()) {
        return false;
    }

    const QString dbusServiceName = QString("com.lingmoUserGuide.hotel") + QString("_") + QString::number(getuid());

    QDBusMessage message =
        QDBusMessage::createMethodCall(dbusServiceName, dbusObjectPath, dbusInterfaceName, dbusCallUserManualMethod);

    QList<QVariant> args;
    args << appName;
    message.setArguments(args);

    QDBusMessage ret = QDBusConnection::sessionBus().call(message);
    if (ret.type() == QDBusMessage::InvalidMessage || ret.type() == QDBusMessage::ErrorMessage) {
        qCritical() << "kdk : user manual d-bus call fail!";
        return false;
    }

    return true;
}

} // namespace kdk
