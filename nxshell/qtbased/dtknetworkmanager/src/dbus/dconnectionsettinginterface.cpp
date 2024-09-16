// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dconnectionsettinginterface.h"
#include <QDBusArgument>
#include <QDBusMetaType>

DNETWORKMANAGER_BEGIN_NAMESPACE

DConnectionSettingInterface::DConnectionSettingInterface(const QByteArray &path, QObject *parent)
    : QObject(parent)
{
#ifdef USE_FAKE_INTERFACE
    const QString &Service = QStringLiteral("com.deepin.FakeNetworkManager");
    const QString &Interface = QStringLiteral("com.deepin.FakeNetworkManager.Settings.Connection");
    QDBusConnection Connection = QDBusConnection::sessionBus();
#else
    const QString &Service = QStringLiteral("org.freedesktop.NetworkManager");
    const QString &Interface = QStringLiteral("org.freedesktop.NetworkManager.Settings.Connection");
    QDBusConnection Connection = QDBusConnection::systemBus();
    Connection.connect(Service, path, Interface, "Updated", this, SIGNAL(Updated()));
    Connection.connect(Service, path, Interface, "Removed", this, SIGNAL(Removed()));
#endif
    m_inter = new DDBusInterface(Service, path, Interface, Connection, this);
    qRegisterMetaType<SettingDesc>("SettingDesc");
    qDBusRegisterMetaType<SettingDesc>();
}

bool DConnectionSettingInterface::unsaved() const
{
    return qdbus_cast<bool>(m_inter->property("Unsaved"));
}

QString DConnectionSettingInterface::filename() const
{
    return "file:" + qdbus_cast<QString>(m_inter->property("Filename"));
}

quint32 DConnectionSettingInterface::flags() const
{
    return qdbus_cast<quint32>(m_inter->property("Flags"));
}

QDBusPendingReply<void> DConnectionSettingInterface::UpdateSetting(const SettingDesc &prop) const
{
    return m_inter->asyncCallWithArgumentList("Update", {QVariant::fromValue(prop)});
}

QDBusPendingReply<void> DConnectionSettingInterface::deleteSetting() const
{
    return m_inter->asyncCall("Delete");
}

QDBusPendingReply<SettingDesc> DConnectionSettingInterface::getSettigns() const
{
    return m_inter->asyncCall("GetSettings");
}

DNETWORKMANAGER_END_NAMESPACE
