/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>
    SPDX-FileCopyrightText: 2013 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_CONNECTION_SETTINGS_H
#define NETWORKMANAGERQT_CONNECTION_SETTINGS_H

#include "setting.h"
#include <networkmanagerqt/networkmanagerqt_export.h>

#undef signals
#include <libnm/NetworkManager.h>
#define signals Q_SIGNALS

#include <QDateTime>
#include <QHash>
#include <QString>

namespace NetworkManager
{
class ConnectionSettingsPrivate;

/**
 * Represents collection of all connection settings
 */
class NETWORKMANAGERQT_EXPORT ConnectionSettings
{
    Q_ENUMS(ConnectionType)
public:
    typedef QSharedPointer<ConnectionSettings> Ptr;
    typedef QList<Ptr> List;
    enum ConnectionType {
        Unknown = 0,
        Adsl,
        Bluetooth,
        Bond,
        Bridge,
        Cdma,
        Gsm,
        Infiniband,
        OLPCMesh,
        Pppoe,
        Vlan,
        Vpn,
        Wimax,
        Wired,
        Wireless,
        Team,
        Generic,
        Tun,
        IpTunnel,
        WireGuard,
        Loopback,
    };

    enum AutoconnectSlaves {
        SlavesDefault = -1,
        DoNotConnectSlaves = 0,
        ConnectAllSlaves = 1,
    };

    enum Lldp {
        LldpDefault = -1,
        LldpDisable = 0,
        LldpEnableRx = 1,
    };

    // FIXME same enum as in device.h, unfortunately it's not possible to use that one
    // maybe in future move all enums into one header so they can be used across all classes
    enum Metered {
        MeteredUnknown = 0,
        MeteredYes = 1,
        MeteredNo = 2,
        MeteredGuessYes = 3,
        MeteredGuessNo = 4,
    };

    enum Mdns {
        MdnsDefault = -1,
        MdnsNo = 0,
        MdnsResolve = 1,
        MdnsResolveAndRespond = 2
    };

    static ConnectionType typeFromString(const QString &typeString);
    static QString typeAsString(const ConnectionType type);
    static QString createNewUuid();

    ConnectionSettings();
    explicit ConnectionSettings(ConnectionType type, NMBluetoothCapabilities bt_cap = NM_BT_CAPABILITY_DUN);
    explicit ConnectionSettings(const ConnectionSettings::Ptr &other);
    explicit ConnectionSettings(const NMVariantMapMap &map);
    virtual ~ConnectionSettings();

    QString name() const;

    void fromMap(const NMVariantMapMap &map);
    void fromMeCard(const QVariantMap &map);

    NMVariantMapMap toMap() const;

    void setId(const QString &id);
    QString id() const;

    void setUuid(const QString &uuid);
    QString uuid() const;

    void setInterfaceName(const QString &interfaceName);
    QString interfaceName() const;

    void setConnectionType(ConnectionType type, NMBluetoothCapabilities bt_cap = NM_BT_CAPABILITY_DUN);
    ConnectionType connectionType() const;

    void addToPermissions(const QString &user, const QString &type);
    void setPermissions(const QHash<QString, QString> &perm);
    QHash<QString, QString> permissions() const;

    void setAutoconnect(bool autoconnect);
    bool autoconnect() const;

    void setAutoconnectPriority(int priority);
    int autoconnectPriority() const;

    void setTimestamp(const QDateTime &timestamp);
    QDateTime timestamp() const;

    void setReadOnly(bool readonly);
    bool readOnly() const;

    void setZone(const QString &zone);
    QString zone() const;

    bool isSlave() const;

    void setMaster(const QString &master);
    QString master() const;

    void setSlaveType(const QString &type);
    QString slaveType() const;

    void setSecondaries(const QStringList &secondaries);
    QStringList secondaries() const;

    void setGatewayPingTimeout(quint32 timeout);
    quint32 gatewayPingTimeout() const;

    void setAutoconnectRetries(int retries);
    int autoconnectRetries() const;

    void setAutoconnectSlaves(AutoconnectSlaves autoconnectSlaves);
    AutoconnectSlaves autoconnectSlaves() const;

    void setLldp(Lldp lldp);
    Lldp lldp() const;

    void setMetered(Metered metered);
    Metered metered() const;

    void setMdns(Mdns mdns);
    Mdns mdns() const;

    void setStableId(const QString &stableId);
    QString stableId() const;

    Setting::Ptr setting(Setting::SettingType type) const;
    Setting::Ptr setting(const QString &type) const;

    Setting::List settings() const;

protected:
    ConnectionSettingsPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(ConnectionSettings)
};

NETWORKMANAGERQT_EXPORT QDebug operator<<(QDebug dbg, const ConnectionSettings &setting);

}

#endif // NETWORKMANAGERQT_CONNECTION_SETTINGS_H
