// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dsecretagentinterface.h"
#include <QDBusObjectPath>
#include <QDBusMetaType>

DNETWORKMANAGER_BEGIN_NAMESPACE

DSecretAgentInterface::DSecretAgentInterface(QObject *parent)
    : QObject(parent)
{
#ifdef USE_FAKE_INTERFACE
    const QString &Service = QStringLiteral("com.deepin.FakeNetworkManager");
    const QString &Interface = QStringLiteral("com.deepin.FakeNetworkManager.SecretAgent");
    const QString &Path = QStringLiteral("/com/deepin/FakeNetworkManager/SecretAgent");
    QDBusConnection Connection = QDBusConnection::sessionBus();
#else
    const QString &Service = QStringLiteral("org.freedesktop.NetworkManager");
    const QString &Interface = QStringLiteral("org.freedesktop.NetworkManager.SecretAgent");
    const QString &Path = QStringLiteral("/org/freedesktop/NetworkManager/SecretAgent");
    QDBusConnection Connection = QDBusConnection::systemBus();
#endif
    m_inter = new DDBusInterface(Service, Path, Interface, Connection, this);
    qDBusRegisterMetaType<SettingDesc>();
    qRegisterMetaType<SettingDesc>("SettingDesc");
}

QDBusPendingReply<SettingDesc> DSecretAgentInterface::getSecrets(const SettingDesc &connection,
                                                                 const QByteArray &connectionPath,
                                                                 const QByteArray &settingName,
                                                                 const QList<QByteArray> &hints,
                                                                 const quint32 flag) const
{
    return m_inter->asyncCallWithArgumentList("GetSecrets",
                                              {QVariant::fromValue<SettingDesc>(connection),
                                               QVariant::fromValue(QDBusObjectPath(QString(connectionPath))),
                                               QVariant::fromValue<QString>(settingName),
                                               QVariant::fromValue([&hints]() {
                                                   QStringList ret;
                                                   for (int i = 0; i < hints.size(); ++i)
                                                       ret.append(hints[i]);
                                                   return ret;
                                               }()),
                                               QVariant::fromValue(flag)});
}

QDBusPendingReply<void> DSecretAgentInterface::cancelGetSecrets(const QByteArray &connectionPath,
                                                                const QByteArray &settingName) const
{
    return m_inter->asyncCallWithArgumentList(
        "CancelGetSecrets",
        {QVariant::fromValue(QDBusObjectPath(QString(connectionPath))), QVariant::fromValue<QString>(settingName)});
}

QDBusPendingReply<void> DSecretAgentInterface::saveSecrets(const SettingDesc &connection, const QByteArray &connectionPath) const
{
    return m_inter->asyncCallWithArgumentList(
        "SaveSecrets",
        {QVariant::fromValue<SettingDesc>(connection), QVariant::fromValue(QDBusObjectPath(QString(connectionPath)))});
}

QDBusPendingReply<void> DSecretAgentInterface::deleteSecrets(const SettingDesc &connection,
                                                             const QByteArray &connectionPath) const
{
    return m_inter->asyncCallWithArgumentList(
        "DeleteSecrets",
        {QVariant::fromValue<SettingDesc>(connection), QVariant::fromValue(QDBusObjectPath(QString(connectionPath)))});
}

DNETWORKMANAGER_END_NAMESPACE
