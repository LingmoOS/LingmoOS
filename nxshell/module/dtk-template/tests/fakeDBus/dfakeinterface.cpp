// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dfakeinterface.h"

#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDebug>

DFakeInterface::DFakeInterface(QObject *parent)
    : QObject(parent)
{
    registerService();
}

DFakeInterface::~DFakeInterface() {}

bool DFakeInterface::registerService()
{
    Dtk::Demo::UserPath_p::registerMetaType();
    const QString &service = QLatin1String("org.freedesktop.fakelogin");
    const QString &path = QLatin1String("/org/freedesktop/fakelogin");
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.registerService(service)) {
        QString errorMsg = bus.lastError().message();
        if (errorMsg.isEmpty())
            errorMsg = "maybe it's running";

        qWarning() << QString("Can't register the %1 service, %2.").arg(service).arg(errorMsg);
        return false;
    }
    if (!bus.registerObject(path, this, QDBusConnection::ExportAllContents)) {
        qWarning() << QString("Can't register %1 the D-Bus object.").arg(path);
        return false;
    }

    return true;
}

void DFakeInterface::unRegisterService()
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    bus.unregisterObject(QLatin1String("/org/freedesktop/fakelogin"));
    bus.unregisterService(QLatin1String("org.freedesktop.fakelogin"));
}

bool DFakeInterface::Docked() const
{
    return m_docked;
}

void DFakeInterface::setDocked(bool docked)
{
    if (docked == m_docked)
        return;

    m_docked = docked;
    Q_EMIT DockedChanged(docked);
}

Dtk::Demo::UserPathList_p DFakeInterface::ListUsers()
{
    return m_users;
}

void DFakeInterface::setListUsers(const Dtk::Demo::UserPathList_p &list)
{
    if (list == m_users)
        return;

    for (auto user : list) {
        if (m_users.indexOf(user) < 0)
            Q_EMIT UserNew(user.user_id, user.path);
    }

    m_users = list;
}
