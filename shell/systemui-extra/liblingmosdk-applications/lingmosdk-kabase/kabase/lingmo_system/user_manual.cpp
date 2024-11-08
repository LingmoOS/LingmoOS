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

#include <unistd.h>

#include <QDBusMessage>

#include "log.hpp"
#include "user_manual.hpp"
#include "dbus.hpp"

namespace kdk
{
namespace kabase
{
namespace
{
constexpr char dbusServiceNamePrefix[] = "com.lingmoUserGuide.hotel";
constexpr char dbusObjectPath[] = "/";
constexpr char dbusInterfaceName[] = "com.guide.hotel";
constexpr char dbusCallUserManualMethod[] = "showGuide";

const QString dbusServiceName = QString(dbusServiceNamePrefix) + QString("_") + QString::number(getuid());
} // namespace

UserManualManagement::UserManualManagement() = default;

UserManualManagement::~UserManualManagement() = default;

bool UserManualManagement::callUserManual(AppName appName)
{
    QString adapAppName = adapterAppName(Currency::getAppName(appName));
    if (adapAppName.isEmpty()) {
        error << "kabase : app name is empty!";
        return false;
    }

    QList<QVariant> args;
    args << QVariant(adapAppName);
    DBus::callMethod(dbusServiceName, dbusObjectPath, dbusInterfaceName, dbusCallUserManualMethod, args);

    return true;
}

bool UserManualManagement::callUserManual(QString appName)
{
    if (appName.isEmpty()) {
        error << "kabase : app name is empty!";
        return false;
    }

    QList<QVariant> args;
    args << QVariant(appName);
    DBus::callMethod(dbusServiceName, dbusObjectPath, dbusInterfaceName, dbusCallUserManualMethod, args);

    return true;
}

QString UserManualManagement::adapterAppName(QString appName)
{
    if (appName.isEmpty()) {
        return QString("");
    } else if (appName == QString("lingmo-ipmsg")) {
        return QString("messages");
    } else {
        return appName;
    }
}

} /* namespace kabase */
} /* namespace kdk */
