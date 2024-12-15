// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lingmoiddbusproxy.h"
#include "utils.h"

LingmoidDBusProxy::LingmoidDBusProxy(QObject *parent)
    : QObject(parent)
    , m_lingmoId(new DDBusInterface(LINGMOID_SERVICE, LINGMOID_PATH, LINGMOID_INTERFACE, QDBusConnection::sessionBus(), this))
{
}

void LingmoidDBusProxy::login() const
{
    m_lingmoId->asyncCall("Login");
}

void LingmoidDBusProxy::logout() const
{
    m_lingmoId->asyncCall("Logout");
}

QDBusReply<QString> LingmoidDBusProxy::localBindCheck(const QString &uuid)
{
    return m_lingmoId->call(QDBus::BlockWithGui, "LocalBindCheck", uuid);
}

QVariantMap LingmoidDBusProxy::userInfo()
{
    return m_lingmoId->property("UserInfo").toMap();
}
