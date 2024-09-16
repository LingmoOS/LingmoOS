// SPDX-FileCopyrightText: 2018-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "deepiniddbusproxy.h"

const QString DEEPIN_ID_SERVICE = QStringLiteral("com.deepin.deepinid");
const QString DEEPIN_ID_PATH = QStringLiteral("/com/deepin/deepinid");
const QString DEEPIN_ID_INTERFACE = QStringLiteral("com.deepin.deepinid");

DeepinIdProxy::DeepinIdProxy(QObject *parent)
    : QObject(parent)
    , m_deepinId(new DDBusInterface(DEEPIN_ID_SERVICE,
                                    DEEPIN_ID_PATH,
                                    DEEPIN_ID_INTERFACE,
                                    QDBusConnection::sessionBus(),
                                    this))
{
}

QVariantMap DeepinIdProxy::userInfo()
{
    return m_deepinId->property("UserInfo").toMap();
}

void DeepinIdProxy::setDBusBlockSignals(bool status)
{
    m_deepinId->blockSignals(status);
}

QDBusReply<QString> DeepinIdProxy::LocalBindCheck(const QString &uuid)
{
    return m_deepinId->call(QDBus::BlockWithGui, "LocalBindCheck", uuid);
}

void DeepinIdProxy::Logout()
{
    m_deepinId->asyncCall("Logout");
}

void DeepinIdProxy::Login()
{
    m_deepinId->asyncCall("Login");
}
