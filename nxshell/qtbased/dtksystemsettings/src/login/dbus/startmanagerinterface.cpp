// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "startmanagerinterface.h"

#include "ddbusinterface.h"

#include <qdbusconnection.h>

DLOGIN_BEGIN_NAMESPACE
StartManagerInterface::StartManagerInterface(const QString &service,
                                             const QString &path,
                                             QDBusConnection connection,
                                             QObject *parent)
    : QObject(parent)
    , m_interface(new DDBusInterface(service, path, staticInterfaceName(), connection, this))
{
    connection.connect(service,
                       path,
                       staticInterfaceName(),
                       "AutostartChanged",
                       this,
                       SIGNAL(autostartChanged(const QString &, const QString &)));
}

QDBusPendingReply<bool> StartManagerInterface::addAutostart(const QString &fileName)
{
    QDBusPendingReply<bool> result =
            m_interface->asyncCallWithArgumentList(QLatin1String("AddAutostart"),
                                                   { QVariant::fromValue(fileName) });
    return result;
}

QDBusPendingReply<bool> StartManagerInterface::removeAutostart(const QString &fileName)
{
    QDBusPendingReply<bool> result =
            m_interface->asyncCallWithArgumentList(QLatin1String("RemoveAutostart"),
                                                   { QVariant::fromValue(fileName) });
    return result;
}

DLOGIN_END_NAMESPACE
