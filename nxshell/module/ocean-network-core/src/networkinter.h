// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef NETWORKINTER_H
#define NETWORKINTER_H

#include <ddbusinterface.h>

#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

class NetworkInter : public Dtk::Core::DDBusInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName() { return "com.lingmo.daemon.Network"; }

public:
    explicit NetworkInter(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0);
    void setSync(bool sync);
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

public Q_SLOTS: // METHODS

    inline QDBusPendingReply<QDBusObjectPath> ActivateAccessPoint(const QString &in0, const QDBusObjectPath &in1, const QDBusObjectPath &in2)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0) << QVariant::fromValue(in1) << QVariant::fromValue(in2);
        return asyncCallWithArgumentList(QStringLiteral("ActivateAccessPoint"), argumentList);
    }

    inline QDBusPendingReply<QDBusObjectPath> ActivateConnection(const QString &in0, const QDBusObjectPath &in1)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0) << QVariant::fromValue(in1);
        return asyncCallWithArgumentList(QStringLiteral("ActivateConnection"), argumentList);
    }

    inline QDBusPendingReply<> CancelSecret(const QString &in0, const QString &in1)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0) << QVariant::fromValue(in1);
        return asyncCallWithArgumentList(QStringLiteral("CancelSecret"), argumentList);
    }

    inline void CancelSecretQueued(const QString &in0, const QString &in1)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0) << QVariant::fromValue(in1);

        CallQueued(QStringLiteral("CancelSecret"), argumentList);
    }

    inline QDBusPendingReply<QDBusObjectPath> CreateConnection(const QString &in0, const QDBusObjectPath &in1)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0) << QVariant::fromValue(in1);
        return asyncCallWithArgumentList(QStringLiteral("CreateConnection"), argumentList);
    }

    inline QDBusPendingReply<QDBusObjectPath> CreateConnectionForAccessPoint(const QDBusObjectPath &in0, const QDBusObjectPath &in1)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0) << QVariant::fromValue(in1);
        return asyncCallWithArgumentList(QStringLiteral("CreateConnectionForAccessPoint"), argumentList);
    }

    inline QDBusPendingReply<> DeactivateConnection(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("DeactivateConnection"), argumentList);
    }

    inline void DeactivateConnectionQueued(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);

        CallQueued(QStringLiteral("DeactivateConnection"), argumentList);
    }

    inline QDBusPendingReply<> DeleteConnection(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("DeleteConnection"), argumentList);
    }

    inline void DeleteConnectionQueued(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);

        CallQueued(QStringLiteral("DeleteConnection"), argumentList);
    }

    inline QDBusPendingReply<> DisableWirelessHotspotMode(const QDBusObjectPath &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("DisableWirelessHotspotMode"), argumentList);
    }

    inline void DisableWirelessHotspotModeQueued(const QDBusObjectPath &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);

        CallQueued(QStringLiteral("DisableWirelessHotspotMode"), argumentList);
    }

    inline QDBusPendingReply<> DisconnectDevice(const QDBusObjectPath &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("DisconnectDevice"), argumentList);
    }

    inline void DisconnectDeviceQueued(const QDBusObjectPath &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);

        CallQueued(QStringLiteral("DisconnectDevice"), argumentList);
    }

    inline QDBusPendingReply<QDBusObjectPath> EditConnection(const QString &in0, const QDBusObjectPath &in1)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0) << QVariant::fromValue(in1);
        return asyncCallWithArgumentList(QStringLiteral("EditConnection"), argumentList);
    }

    inline QDBusPendingReply<> EnableDevice(const QDBusObjectPath &in0, bool in1)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0) << QVariant::fromValue(in1);
        return asyncCallWithArgumentList(QStringLiteral("EnableDevice"), argumentList);
    }

    inline void EnableDeviceQueued(const QDBusObjectPath &in0, bool in1)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0) << QVariant::fromValue(in1);

        CallQueued(QStringLiteral("EnableDevice"), argumentList);
    }

    inline QDBusPendingReply<> EnableWirelessHotspotMode(const QDBusObjectPath &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("EnableWirelessHotspotMode"), argumentList);
    }

    inline void EnableWirelessHotspotModeQueued(const QDBusObjectPath &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);

        CallQueued(QStringLiteral("EnableWirelessHotspotMode"), argumentList);
    }

    inline QDBusPendingReply<> FeedSecret(const QString &in0, const QString &in1, const QString &in2, bool in3)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0) << QVariant::fromValue(in1) << QVariant::fromValue(in2) << QVariant::fromValue(in3);
        return asyncCallWithArgumentList(QStringLiteral("FeedSecret"), argumentList);
    }

    inline void FeedSecretQueued(const QString &in0, const QString &in1, const QString &in2, bool in3)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0) << QVariant::fromValue(in1) << QVariant::fromValue(in2) << QVariant::fromValue(in3);

        CallQueued(QStringLiteral("FeedSecret"), argumentList);
    }

    inline QDBusPendingReply<QString> GetAccessPoints(const QDBusObjectPath &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("GetAccessPoints"), argumentList);
    }

    inline QDBusPendingReply<QString> GetActiveConnectionInfo()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("GetActiveConnectionInfo"), argumentList);
    }

    inline QDBusPendingReply<QString> GetAutoProxy()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("GetAutoProxy"), argumentList);
    }

    inline QDBusPendingReply<QString, QString> GetProxy(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("GetProxy"), argumentList);
    }

    inline QDBusReply<QString> GetProxy(const QString &in0, QString &out1)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        QDBusMessage reply = callWithArgumentList(QDBus::Block, QStringLiteral("GetProxy"), argumentList);
        if (reply.type() == QDBusMessage::ReplyMessage && reply.arguments().count() == 2) {
            out1 = qdbus_cast<QString>(reply.arguments().at(1));
        }
        return reply;
    }

    inline QDBusPendingReply<QString> GetProxyIgnoreHosts()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("GetProxyIgnoreHosts"), argumentList);
    }

    inline QDBusPendingReply<QString> GetProxyMethod()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("GetProxyMethod"), argumentList);
    }

    inline QDBusPendingReply<QStringList> GetSupportedConnectionTypes()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("GetSupportedConnectionTypes"), argumentList);
    }

    inline QDBusPendingReply<QString> GetWiredConnectionUuid(const QDBusObjectPath &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("GetWiredConnectionUuid"), argumentList);
    }

    inline QDBusPendingReply<bool> IsDeviceEnabled(const QDBusObjectPath &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("IsDeviceEnabled"), argumentList);
    }

    inline QDBusPendingReply<bool> IsPasswordValid(const QString &in0, const QString &in1)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0) << QVariant::fromValue(in1);
        return asyncCallWithArgumentList(QStringLiteral("IsPasswordValid"), argumentList);
    }

    inline QDBusPendingReply<bool> IsWirelessHotspotModeEnabled(const QDBusObjectPath &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("IsWirelessHotspotModeEnabled"), argumentList);
    }

    inline QDBusPendingReply<QList<QDBusObjectPath> > ListDeviceConnections(const QDBusObjectPath &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("ListDeviceConnections"), argumentList);
    }

    inline QDBusPendingReply<> RegisterSecretReceiver()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("RegisterSecretReceiver"), argumentList);
    }

    inline void RegisterSecretReceiverQueued()
    {
        QList<QVariant> argumentList;

        CallQueued(QStringLiteral("RegisterSecretReceiver"), argumentList);
    }

    inline QDBusPendingReply<> RequestIPConflictCheck(const QString &in0, const QString &in1)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0) << QVariant::fromValue(in1);
        return asyncCallWithArgumentList(QStringLiteral("RequestIPConflictCheck"), argumentList);
    }

    inline void RequestIPConflictCheckQueued(const QString &in0, const QString &in1)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0) << QVariant::fromValue(in1);

        CallQueued(QStringLiteral("RequestIPConflictCheck"), argumentList);
    }

    inline QDBusPendingReply<> RequestWirelessScan()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("RequestWirelessScan"), argumentList);
    }

    inline void RequestWirelessScanQueued()
    {
        QList<QVariant> argumentList;

        CallQueued(QStringLiteral("RequestWirelessScan"), argumentList);
    }

    inline QDBusPendingReply<> SetAutoProxy(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("SetAutoProxy"), argumentList);
    }

    inline void SetAutoProxyQueued(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);

        CallQueued(QStringLiteral("SetAutoProxy"), argumentList);
    }

    inline QDBusPendingReply<> SetDeviceManaged(const QString &in0, bool in1)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0) << QVariant::fromValue(in1);
        return asyncCallWithArgumentList(QStringLiteral("SetDeviceManaged"), argumentList);
    }

    inline void SetDeviceManagedQueued(const QString &in0, bool in1)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0) << QVariant::fromValue(in1);

        CallQueued(QStringLiteral("SetDeviceManaged"), argumentList);
    }

    inline QDBusPendingReply<> SetProxy(const QString &in0, const QString &in1, const QString &in2)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0) << QVariant::fromValue(in1) << QVariant::fromValue(in2);
        return asyncCallWithArgumentList(QStringLiteral("SetProxy"), argumentList);
    }

    inline void SetProxyQueued(const QString &in0, const QString &in1, const QString &in2)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0) << QVariant::fromValue(in1) << QVariant::fromValue(in2);

        CallQueued(QStringLiteral("SetProxy"), argumentList);
    }

    inline QDBusPendingReply<> SetProxyIgnoreHosts(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("SetProxyIgnoreHosts"), argumentList);
    }

    inline void SetProxyIgnoreHostsQueued(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);

        CallQueued(QStringLiteral("SetProxyIgnoreHosts"), argumentList);
    }

    inline QDBusPendingReply<> SetProxyMethod(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("SetProxyMethod"), argumentList);
    }

    inline void SetProxyMethodQueued(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);

        CallQueued(QStringLiteral("SetProxyMethod"), argumentList);
    }

Q_SIGNALS: // SIGNALS
    void AccessPointAoceand(const QString &in0, const QString &in1);
    void AccessPointPropertiesChanged(const QString &in0, const QString &in1);
    void AccessPointRemoved(const QString &in0, const QString &in1);
    void ActiveConnectionInfoChanged();
    void DeviceEnabled(const QString &in0, bool in1);
    void IPConflict(const QString &in0, const QString &in1);
    void NeedSecrets(const QString &in0);
    void NeedSecretsFinished(const QString &in0, const QString &in1);
    // begin property changed signals
    void ActiveConnectionsChanged(const QString &value) const;
    void ConnectionsChanged(const QString &value) const;
    void ConnectivityChanged(uint value) const;
    void DevicesChanged(const QString &value) const;
    void NetworkingEnabledChanged(bool value) const;
    void StateChanged(uint value) const;
    void VpnEnabledChanged(bool value) const;
    void WirelessAccessPointsChanged(const QString &value) const;

public Q_SLOTS:
    void CallQueued(const QString &callName, const QList<QVariant> &args);

    // private Q_SLOTS:
    //     void onPendingCallFinished(QDBusPendingCallWatcher *w);
    //     void onPropertyChanged(const QString &propName, const QVariant &value);
};

#endif // NETWORKINTER_H
