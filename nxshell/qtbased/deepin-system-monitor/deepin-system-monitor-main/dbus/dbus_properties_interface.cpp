// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbus_properties_interface.h"

/*
 * Implementation of interface class DBusPropertiesInterface
 */

// Constructor
DBusPropertiesInterface::DBusPropertiesInterface(const QString &service,
                                                 const QString &path,
                                                 const QDBusConnection &connection,
                                                 QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
}

// Destructor
DBusPropertiesInterface::~DBusPropertiesInterface()
{
}

