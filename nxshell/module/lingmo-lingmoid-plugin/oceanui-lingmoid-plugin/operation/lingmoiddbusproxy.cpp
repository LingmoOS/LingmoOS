// SPDX-FileCopyrightText: 2018-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "lingmoiddbusproxy.h"

const QString LINGMO_ID_SERVICE = QStringLiteral("com.lingmo.lingmoid");
const QString LINGMO_ID_PATH = QStringLiteral("/com/lingmo/lingmoid");
const QString LINGMO_ID_INTERFACE = QStringLiteral("com.lingmo.lingmoid");

LingmoIdProxy::LingmoIdProxy(QObject *parent)
    : QObject(parent)
    , m_lingmoId(new DDBusInterface(LINGMO_ID_SERVICE,
                                    LINGMO_ID_PATH,
                                    LINGMO_ID_INTERFACE,
                                    QDBusConnection::sessionBus(),
                                    this))
{
}

QVariantMap LingmoIdProxy::userInfo()
{
    return m_lingmoId->property("UserInfo").toMap();
}

void LingmoIdProxy::setDBusBlockSignals(bool status)
{
    m_lingmoId->blockSignals(status);
}

QDBusReply<QString> LingmoIdProxy::LocalBindCheck(const QString &uuid)
{
    return m_lingmoId->call(QDBus::BlockWithGui, "LocalBindCheck", uuid);
}

void LingmoIdProxy::Logout()
{
    m_lingmoId->asyncCall("Logout");
}

void LingmoIdProxy::Login()
{
    m_lingmoId->asyncCall("Login");
}
