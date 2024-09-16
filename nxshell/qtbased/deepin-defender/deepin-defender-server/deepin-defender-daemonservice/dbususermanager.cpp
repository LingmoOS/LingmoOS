// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbususermanager.h"

DbususerManager::DbususerManager(QString path,QObject *parent)
    : QDBusAbstractInterface("com.deepin.daemon.Accounts", path, staticInterfaceName(), QDBusConnection::systemBus(), parent)
{
    QDBusConnection::systemBus().connect(this->service(), this->path(), "org.freedesktop.DBus.Properties",
                 "PropertiesChanged","sa{sv}as", this, SLOT(__propertyChanged__(QDBusMessage)));}

DbususerManager::~DbususerManager()
{
//    QDBusConnection::systemBus().disconnect(service(), path(), "org.freedesktop.DBus.Properties",  "PropertiesChanged",  "i", this, SLOT(propertyChanged(QDBusMessage)));
}
