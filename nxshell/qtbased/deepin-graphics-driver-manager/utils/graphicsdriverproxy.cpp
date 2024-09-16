// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "graphicsdriverproxy.h"

/*
 * Implementation of interface class ComDeepinDaemonGraphicsDriverInterface
 */

ComDeepinDaemonGraphicsDriverInterface::ComDeepinDaemonGraphicsDriverInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
}

ComDeepinDaemonGraphicsDriverInterface::~ComDeepinDaemonGraphicsDriverInterface()
{
}

