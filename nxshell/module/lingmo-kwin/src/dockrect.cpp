// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dockrect.h"

/*
 * Implementation of interface class OrgLingmoDdeDaemonDock1Interface
 */

OrgLingmoDdeDaemonDock1Interface::OrgLingmoDdeDaemonDock1Interface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
}

OrgLingmoDdeDaemonDock1Interface::~OrgLingmoDdeDaemonDock1Interface()
{
}
