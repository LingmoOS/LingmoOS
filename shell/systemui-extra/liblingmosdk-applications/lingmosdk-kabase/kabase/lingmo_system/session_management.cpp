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

#include <QVariant>
#include <QDebug>
#include <QList>

#include "log.hpp"
#include "lingmo_system/session_management.hpp"
#include "dbus.hpp"

namespace kdk
{
namespace kabase
{
namespace
{
constexpr char dbusServiceName[] = "org.gnome.SessionManager";
constexpr char dbusObjectPath[] = "/org/gnome/SessionManager";
constexpr char dbusInterfaceName[] = "org.gnome.SessionManager";
constexpr char dbusInhibitLockScreenMethod[] = "Inhibit";
constexpr char dbusUnInhibitLockScreenMethod[] = "Uninhibit";
} // namespace

SessionManagement::SessionManagement() = default;

SessionManagement::~SessionManagement() = default;

quint32 SessionManagement::setInhibitLockScreen(AppName appName, QString reason)
{
    if (Currency::getAppName(appName).isEmpty() || reason.isEmpty()) {
        error << "kabase : app name is empty or reason is empty!";
        return 0;
    }

    QList<QVariant> args;
    args << QVariant(Currency::getAppName(appName)) << QVariant(quint32(0)) << QVariant(reason) << QVariant(quint32(8));

    QList<QVariant> ret =
        DBus::callMethod(dbusServiceName, dbusObjectPath, dbusInterfaceName, dbusInhibitLockScreenMethod, args);
    if (ret.isEmpty()) {
        error << "kabase : dbus call fail!";
    } else {
        return ret.at(0).toUInt();
    }

    return 0;
}

bool SessionManagement::unInhibitLockScreen(quint32 flag)
{
    if (flag == 0) {
        error << "kabase : flag error";
        return false;
    }

    QList<QVariant> args;
    args << QVariant(flag);
    DBus::callMethod(dbusServiceName, dbusObjectPath, dbusInterfaceName, dbusUnInhibitLockScreenMethod, args);

    return true;
}

} /* namespace kabase */
} /* namespace kdk */
