// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "sessionmanagerinterface.h"

#include "ddbusinterface.h"

#include <qdbusconnection.h>
#include <qdebug.h>
DLOGIN_BEGIN_NAMESPACE

SessionManagerInterface::SessionManagerInterface(const QString &service,
                                                 const QString &path,
                                                 QDBusConnection connection,
                                                 QObject *parent)
    : QObject(parent)
    , m_interface(new DDBusInterface(service, path, staticInterfaceName(), connection, this))
{
}

bool SessionManagerInterface::Locked()
{
    return qdbus_cast<bool>(m_interface->property("Locked"));
}

QDBusPendingReply<> SessionManagerInterface::SetLocked(const bool locked)
{
    QDBusPendingReply<> reply =
            m_interface->asyncCallWithArgumentList("SetLocked", { QVariant::fromValue(locked) });
    return reply;
}

DLOGIN_END_NAMESPACE