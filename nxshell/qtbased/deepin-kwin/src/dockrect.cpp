// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dockrect.h"

/*
 * Implementation of interface class OrgDeepinDdeDaemonDock1Interface
 */

OrgDeepinDdeDaemonDock1Interface::OrgDeepinDdeDaemonDock1Interface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
}

OrgDeepinDdeDaemonDock1Interface::~OrgDeepinDdeDaemonDock1Interface()
{
}
