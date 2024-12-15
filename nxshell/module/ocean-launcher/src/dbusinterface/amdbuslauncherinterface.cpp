// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "amdbuslauncherinterface.h"

/*
 * Implementation of interface class AMDBusLauncherInter
 */

AMDBusLauncherInter::AMDBusLauncherInter(QObject *parent)
    : QDBusAbstractInterface(INTERFACE_NAME, SERVICE_PATH, staticInterfaceName(), QDBusConnection::sessionBus(), parent)
{
    CategoryInfo::registerMetaType();
    FrequencyInfo::registerMetaType();
    ItemInfo_v2::registerMetaType();
    InstalledTimeInfo::registerMetaType();

    QDBusConnection::sessionBus().connect(service(), path(), "org.freedesktop.DBus.Properties",  "PropertiesChanged","sa{sv}as", this, SLOT(__propertyChanged__(const QDBusMessage&)));
}

AMDBusLauncherInter::~AMDBusLauncherInter()
{
    QDBusConnection::sessionBus().disconnect(service(), path(), "org.freedesktop.DBus.Properties",  "PropertiesChanged",  "sa{sv}as", this, SLOT(__propertyChanged__(const QDBusMessage&)));
}

