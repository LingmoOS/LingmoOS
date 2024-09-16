// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef NETWORKDBUSPROXY_H
#define NETWORKDBUSPROXY_H
#include <QDBusObjectPath>
#include <QObject>
class QDBusInterface;
class QDBusMessage;

namespace dde {
namespace network {

class DCCDBusInterface;

class NetworkDBusProxy : public QObject
{
    Q_OBJECT
public:
    explicit NetworkDBusProxy(QObject *parent = nullptr);
    // networkInter
    Q_PROPERTY(QString ActiveConnections READ activeConnections NOTIFY ActiveConnectionsChanged)
    QString activeConnections();
    Q_PROPERTY(QString Connections READ connections NOTIFY ConnectionsChanged)
    QString connections();
    Q_PROPERTY(uint Connectivity READ connectivity NOTIFY ConnectivityChanged)
    uint connectivity();
    Q_PROPERTY(QString Devices READ devices NOTIFY DevicesChanged)
    QString devices();
    Q_PROPERTY(bool NetworkingEnabled READ networkingEnabled WRITE setNetworkingEnabled NOTIFY NetworkingEnabledChanged)
    bool networkingEnabled();
    void setNetworkingEnabled(bool value);
    Q_PROPERTY(uint State READ state NOTIFY StateChanged)
    uint state();
    Q_PROPERTY(bool VpnEnabled READ vpnEnabled WRITE setVpnEnabled NOTIFY VpnEnabledChanged)
    bool vpnEnabled();
    void setVpnEnabled(bool value);
    Q_PROPERTY(QString WirelessAccessPoints READ wirelessAccessPoints NOTIFY WirelessAccessPointsChanged)
    QString wirelessAccessPoints();
    // proxyChains
    Q_PROPERTY(QString IP READ iP NOTIFY IPChanged)
    QString iP();
    Q_PROPERTY(QString Password READ password NOTIFY PasswordChanged)
    QString password();
    Q_PROPERTY(uint Port READ port NOTIFY PortChanged)
    uint port();
    Q_PROPERTY(QString Type READ type NOTIFY TypeChanged)
    QString type();
    Q_PROPERTY(QString User READ user NOTIFY UserChanged)
    QString user();
    // airplaneMode
    Q_PROPERTY(bool Enabled READ enabled NOTIFY EnabledChanged)
    bool enabled();

    static void ShowPage(const QString &url);

Q_SIGNALS: // SIGNALS
    // networkInter
    void AccessPointAdded(const QString &in0, const QString &in1);
    void AccessPointPropertiesChanged(const QString &in0, const QString &in1);
    void AccessPointRemoved(const QString &in0, const QString &in1);
    void ActiveConnectionInfoChanged();
    void DeviceEnabled(const QString &device, bool enabled);
    void IPConflict(const QString &in0, const QString &in1);
    void ProxyMethodChanged(const QString &method);
    // begin property changed signals
    void ActiveConnectionsChanged(const QString &value) const;
    void ConnectionsChanged(const QString &value) const;
    void ConnectivityChanged(uint value) const;
    void DevicesChanged(const QString &value) const;
    void NetworkingEnabledChanged(bool value) const;
    void StateChanged(uint value) const;
    void VpnEnabledChanged(bool value) const;
    void WirelessAccessPointsChanged(const QString &value) const;
    // proxyChains
    // begin property changed signals
    void IPChanged(const QString &value) const;
    void PasswordChanged(const QString &value) const;
    void PortChanged(uint value) const;
    void TypeChanged(const QString &value) const;
    void UserChanged(const QString &value) const;
    // airplaneMode
    // begin property changed signals
    void BluetoothEnabledChanged(bool value) const;
    void EnabledChanged(bool value) const;
    void WifiEnabledChanged(bool value) const;

public Q_SLOTS:
    // networkInter
    void EnableDevice(const QDBusObjectPath &devPath, bool enabled);
    QString GetProxyMethod();
    void SetProxyMethod(const QString &proxyMode);
    void SetProxyMethod(const QString &proxyMode, QObject *receiver, const char *member);
    QString GetProxyIgnoreHosts();
    void SetProxyIgnoreHosts(const QString &ignoreHosts);
    void SetProxyIgnoreHosts(const QString &ignoreHosts, QObject *receiver, const char *member);
    QString GetAutoProxy();
    void SetAutoProxy(const QString &proxyAuto);
    void SetAutoProxy(const QString &proxyAuto, QObject *receiver, const char *member);
    QStringList GetProxy(const QString &proxyType); // 返回值待处理
    void SetProxy(const QString &proxyType, const QString &host, const QString &port);
    void SetProxy(const QString &proxyType, const QString &host, const QString &port, QObject *receiver, const char *member);
    QString GetActiveConnectionInfo();
    QDBusObjectPath ActivateConnection(const QString &uuid, const QDBusObjectPath &devicePath);
    QDBusObjectPath ActivateAccessPoint(const QString &uuid, const QDBusObjectPath &apPath, const QDBusObjectPath &devPath);
    bool ActivateAccessPoint(const QString &uuid, const QDBusObjectPath &apPath, const QDBusObjectPath &devPath, QObject *receiver, const char *member, const char *errorSlot);
    void DisconnectDevice(const QDBusObjectPath &devPath);
    void RequestIPConflictCheck(const QString &ip, const QString &ifc);
    bool IsDeviceEnabled(const QDBusObjectPath &devPath);
    void RequestWirelessScan();

    // proxyChains
    void Set(const QString &type0, const QString &ip, uint port, const QString &user, const QString &password);
    // Notifications
    static uint Notify(const QString &in0, uint in1, const QString &in2, const QString &in3, const QString &in4, const QStringList &in5, const QVariantMap &in6, int in7);

private:
    DCCDBusInterface *m_networkInter;
    DCCDBusInterface *m_proxyChainsInter;
    DCCDBusInterface *m_airplaneModeInter;
};

}
}
#endif // NETWORKDBUSPROXY_H
