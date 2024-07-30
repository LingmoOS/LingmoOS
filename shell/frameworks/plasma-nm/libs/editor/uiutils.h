/*
    SPDX-FileCopyrightText: 2008-2010 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2013-2014 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef PLASMA_NM_UIUTILS_H
#define PLASMA_NM_UIUTILS_H

#include "plasmanm_editor_export.h"

#include <NetworkManagerQt/AccessPoint>
#include <NetworkManagerQt/BluetoothDevice>
#include <NetworkManagerQt/Device>
#include <NetworkManagerQt/ModemDevice>
#include <NetworkManagerQt/Utils>
#include <NetworkManagerQt/VpnConnection>
#include <NetworkManagerQt/VpnSetting>
#include <NetworkManagerQt/WiredDevice>
#include <NetworkManagerQt/WirelessDevice>
#include <NetworkManagerQt/WirelessSetting>

#include <ModemManager/ModemManager.h>
#include <ModemManagerQt/Modem>

class PLASMANM_EDITOR_EXPORT UiUtils
{
public:
    enum SortedConnectionType {
        Wired,
        Wireless,
        Gsm,
        Cdma,
        Pppoe,
        Adsl,
        Infiniband,
        OLPCMesh,
        Bluetooth,
        Wireguard,
        Vpn,
        Vlan,
        Bridge,
        Bond,
        Team,
        Unknown,
    };

    /*
     * @return sorted connection type used to prioritize specific connection types
     */
    static SortedConnectionType connectionTypeToSortedType(NetworkManager::ConnectionSettings::ConnectionType type);

    /*
     * @return whether given connection type is supported
     * Currently ignored connection types: Bond, Bridge, Generic, Infiniband, Team, Vlan, Tun
     */
    static bool isConnectionTypeSupported(NetworkManager::ConnectionSettings::ConnectionType type);

    /**
     * @return true if the connection is virtual.
     * @param type Type of the network connection
     */
    static bool isConnectionTypeVirtual(NetworkManager::ConnectionSettings::ConnectionType type);

    /**
     * @return a human-readable description for the network interface type for use as label
     * @param type the type of the network interface
     */
    static QString interfaceTypeLabel(const NetworkManager::Device::Type type, const NetworkManager::Device::Ptr iface);

    /**
     * @return a human-readable name for a given network interface according to the configured
     * naming style
     * @param type type of the network interface
     * @param interfaceName name of the network interface (eg eth0)
     */
    static QString prettyInterfaceName(NetworkManager::Device::Type type, const QString &interfaceName);

    /**
     * @return a human-readable description of the connection state of a given network interface
     * @param state The connection state
     */
    static QString connectionStateToString(NetworkManager::Device::State state, const QString &connectionName = QString());

    static QString vpnConnectionStateToString(NetworkManager::VpnConnection::State state);

    static QString iconAndTitleForConnectionSettingsType(NetworkManager::ConnectionSettings::ConnectionType type, QString &title);
    /**
     * @return a human-readable description of operation mode.
     * @param mode the operation mode
     */
    static QString operationModeToString(NetworkManager::WirelessDevice::OperationMode mode);

    /**
     * @return string list with a human-readable description of wpa flags.
     * @param flags the wpa flags
     */
    static QStringList wpaFlagsToStringList(NetworkManager::AccessPoint::WpaFlags flags);

    /**
     * @return localized string showing a human-readable connection speed. 1000 is used as base.
     * @param bitrate bitrate of the connection per second
     */
    static QString connectionSpeed(double bitrate);

    /**
     * @param band The band of a wireless network. The value corresponds to the type enum in Knm::WirelessSetting::EnumBand
     * @return A string representation
     */
    static QString wirelessBandToString(NetworkManager::WirelessSetting::FrequencyBand band);

    /**
     * @param frequency The frequency of a wireless network access point in MHz
     * @return A string representation
     */
    static QString wirelessFrequencyToString(uint frequency);

    /**
     * Check whether we're running in a live image session.
     */
    static bool isLiveImage();

    /**
     * Setup default permissions for a new connection.
     *
     * This determines whether we should use system connections or user
     * connections by default and sets the default permissions accordingly.
     * System connections are preferred if either the preference for that is
     * set, KWallet is not enabled or we're running in a live image. However, if
     * we do not have permissions to create system connections, we fall back to
     * user connections.
     *
     * @param settings The connection settings to setup.
     * @param liveSession Whether we are running a session from a live disk image.
     */
    static void setConnectionDefaultPermissions(NetworkManager::ConnectionSettings::Ptr &settings);

    static QString convertAllowedModeToString(ModemManager::Modem::ModemModes mode);
    static QString convertAccessTechnologyToString(ModemManager::Modem::AccessTechnologies tech);
    static QString convertLockReasonToString(MMModemLock reason);
    static NetworkManager::ModemDevice::Capability modemSubType(NetworkManager::ModemDevice::Capabilities modemCaps);
    static QString labelFromWirelessSecurity(NetworkManager::WirelessSecurityType type);

    static QString formatDateRelative(const QDateTime &lastUsed);
    static QString formatLastUsedDateRelative(const QDateTime &lastUsed);
};
#endif // UIUTILS_H
