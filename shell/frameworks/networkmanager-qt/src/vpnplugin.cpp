/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "vpnplugin.h"

#include "manager_p.h"
#include "vpnplugininterface.h"

class NetworkManager::VpnPluginPrivate
{
public:
    VpnPluginPrivate(const QString &path);

    VpnConnection::State state;
    OrgFreedesktopNetworkManagerVPNPluginInterface iface;
};

NetworkManager::VpnPluginPrivate::VpnPluginPrivate(const QString &path)
#ifdef NMQT_STATIC
    : iface(NetworkManagerPrivate::DBUS_SERVICE, path, QDBusConnection::sessionBus())
#else
    : iface(NetworkManagerPrivate::DBUS_SERVICE, path, QDBusConnection::systemBus())
#endif
{
}

NetworkManager::VpnPlugin::VpnPlugin(const QString &path, QObject *parent)
    : QObject(parent)
    , d_ptr(new VpnPluginPrivate(path))
{
    Q_D(VpnPlugin);
    d->state = (NetworkManager::VpnConnection::State)d->iface.state();

    QObject::connect(&d->iface, SIGNAL(Config(QVariantMap)), this, SLOT(setConfig(QVariantMap)));
    QObject::connect(&d->iface, SIGNAL(Failure(uint)), this, SLOT(setFailure(QString)));
    QObject::connect(&d->iface, SIGNAL(Ip4Config(QVariantMap)), this, SLOT(setIp4Config(QVariantMap)));
    QObject::connect(&d->iface, SIGNAL(Ip6Config(QVariantMap)), this, SLOT(setIp6Config(QVariantMap)));
    // QObject::connect(&d->iface, SIGNAL(LoginBanner(QString)),
    //        this, SLOT(onLoginBanner(QString)));
    QObject::connect(&d->iface, SIGNAL(StateChanged(uint)), this, SLOT(onStateChanged(uint)));
}

NetworkManager::VpnPlugin::~VpnPlugin()
{
    delete d_ptr;
}

void NetworkManager::VpnPlugin::connect(const NMVariantMapMap &connection)
{
    Q_D(VpnPlugin);

    QDBusPendingReply<> reply = d->iface.Connect(connection);
}

void NetworkManager::VpnPlugin::disconnect()
{
    Q_D(VpnPlugin);

    QDBusPendingReply<> reply = d->iface.Disconnect();
}

QString NetworkManager::VpnPlugin::needSecrets(const NMVariantMapMap &connection)
{
    Q_D(VpnPlugin);

    QDBusPendingReply<QString> reply = d->iface.NeedSecrets(connection);

    return reply.value();
}

void NetworkManager::VpnPlugin::setConfig(const QVariantMap &configuration)
{
    Q_D(VpnPlugin);

    QDBusPendingReply<QString> reply = d->iface.SetConfig(configuration);

    Q_EMIT configChanged(configuration);
}

void NetworkManager::VpnPlugin::setFailure(const QString &reason)
{
    Q_D(VpnPlugin);

    QDBusPendingReply<QString> reply = d->iface.SetFailure(reason);

    // TODO
    // Q_EMIT failureChanged(reason);
}

void NetworkManager::VpnPlugin::setIp4Config(const QVariantMap &config)
{
    Q_D(VpnPlugin);

    QDBusPendingReply<> reply = d->iface.SetIp4Config(config);

    Q_EMIT ip4ConfigChanged(config);
}

void NetworkManager::VpnPlugin::setIp6Config(const QVariantMap &config)
{
    Q_D(VpnPlugin);

    QDBusPendingReply<> reply = d->iface.SetIp6Config(config);

    Q_EMIT ip6ConfigChanged(config);
}

void NetworkManager::VpnPlugin::onStateChanged(uint state)
{
    Q_D(VpnPlugin);

    d->state = (VpnConnection::State)state;

    Q_EMIT stateChanged(d->state);
}

#include "moc_vpnplugin.cpp"
