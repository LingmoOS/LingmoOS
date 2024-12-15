// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#include "dmdbusinterface.h"

/*
 * Implementation of interface class DMDBusInterface
 */

DMDBusInterface::DMDBusInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
}

DMDBusInterface::~DMDBusInterface()
{
}
