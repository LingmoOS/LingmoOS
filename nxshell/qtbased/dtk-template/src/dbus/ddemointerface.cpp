// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ddemointerface.h"
#include "ddbusinterface.h"

DDEMO_BEGIN_NAMESPACE

DDemoInterface::DDemoInterface(QObject *parent)
    : QObject(parent)
{
#ifdef USE_FAKE_INTERFACE
    const QString &Service = QStringLiteral("org.freedesktop.fakelogin");
    const QString &Path = QStringLiteral("/org/freedesktop/fakelogin");
    const QString &Interface = QStringLiteral("org.freedesktop.fakelogin.Manager");
    auto Connection = QDBusConnection::sessionBus();
#else
    const QString &Service = QStringLiteral("org.freedesktop.login1");
    const QString &Path = QStringLiteral("/org/freedesktop/login1");
    const QString &Interface = QStringLiteral("org.freedesktop.login1.Manager");
    auto Connection = QDBusConnection::systemBus();
#endif
    // 自定义类型必须进行元对象类型注册才能正常使用。
    UserPath_p::registerMetaType();

    // 连接DBus 对象已存在的对象。
    Connection.connect(Service, Path, Interface, "UserNew", this, SLOT(UserNew(const uint, const QDBusObjectPath &)));
    m_interface = new DDBusInterface(Service, Path, Interface, Connection, this);
}

bool DDemoInterface::Docked() const
{
    // 注意这里必须使用qdbus_cast，否则自定义类型无法进行转换。
    return qdbus_cast<bool>(m_interface->property("Docked"));
}

// 此处演示了使用自定义类型的DBus方法，属性也是一样
QDBusPendingReply<UserPathList_p> DDemoInterface::ListUsers()
{
    return m_interface->asyncCall(QStringLiteral("ListUsers"));
}

DDEMO_END_NAMESPACE
