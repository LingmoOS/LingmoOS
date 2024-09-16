// SPDX-FileCopyrightText: 2015 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "displayinterface.h"

/*
 * Implementation of interface class DisplayInterface
 */

DisplayInterface::DisplayInterface(QObject *parent)
    : QDBusAbstractInterface(staticServiceName(), staticObjectPath(), staticInterfaceName(), QDBusConnection::sessionBus(), parent)
{
    qDBusRegisterMetaType<BrightnessMap>();
    qDBusRegisterMetaType<DisplayRect>();

    QDBusConnection::sessionBus().connect(this->service(), this->path(), "org.freedesktop.DBus.Properties",  "PropertiesChanged","sa{sv}as", this, SLOT(__propertyChanged__(QDBusMessage)));
}

DisplayInterface::~DisplayInterface()
{
    QDBusConnection::sessionBus().disconnect(service(), path(), "org.freedesktop.DBus.Properties",  "PropertiesChanged",  "sa{sv}as", this, SLOT(propertyChanged(QDBusMessage)));
}


QDBusArgument &operator<<(QDBusArgument &argument, const DisplayRect &rect)
{
    argument.beginStructure();
    argument << rect.x << rect.y << rect.width << rect.height;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, DisplayRect &rect)
{
    argument.beginStructure();
    argument >> rect.x >> rect.y >> rect.width >> rect.height;
    argument.endStructure();
    return argument;
}

QDebug operator<<(QDebug deg, const DisplayRect &rect)
{
    qDebug() << "x:" << rect.x << "y:" << rect.y << "width:" << rect.width << "height:" << rect.height;

    return deg;
}
