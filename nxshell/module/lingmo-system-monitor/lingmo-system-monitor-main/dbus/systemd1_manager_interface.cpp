// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "systemd1_manager_interface.h"

/*
 * Implementation of interface class Systemd1ManagerInterface
 */

// Constructor
Systemd1ManagerInterface::Systemd1ManagerInterface(const QString &service,
                                                   const QString &path,
                                                   const QDBusConnection &connection,
                                                   QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
}

// Destructor
Systemd1ManagerInterface::~Systemd1ManagerInterface()
{
}

