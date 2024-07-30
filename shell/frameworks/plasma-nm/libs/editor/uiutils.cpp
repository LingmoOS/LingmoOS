/*
    SPDX-FileCopyrightText: 2008-2010 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2013-2014 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

// Own
#include "uiutils.h"

#include "configuration.h"
#include "plasma_nm_editor.h"

// KDE
#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>
#include <KUser>
#include <KWallet>

#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/Security8021xSetting>

#include <ModemManagerQt/Manager>
#include <ModemManagerQt/Modem3Gpp>
#include <ModemManagerQt/ModemCdma>
#include <ModemManagerQt/ModemDevice>

// Qt
#include <QFile>
#include <QHostAddress>
#include <QSizeF>
#include <QString>

#include <optional>

using namespace NetworkManager;

UiUtils::SortedConnectionType UiUtils::connectionTypeToSortedType(NetworkManager::ConnectionSettings::ConnectionType type)
{
    switch (type) {
    case NetworkManager::ConnectionSettings::Unknown:
        return UiUtils::Unknown;
        break;
    case NetworkManager::ConnectionSettings::Adsl:
        return UiUtils::Adsl;
        break;
    case NetworkManager::ConnectionSettings::Bluetooth:
        return UiUtils::Bluetooth;
        break;
    case NetworkManager::ConnectionSettings::Bond:
        return UiUtils::Bond;
        break;
    case NetworkManager::ConnectionSettings::Bridge:
        return UiUtils::Bridge;
        break;
    case NetworkManager::ConnectionSettings::Cdma:
        return UiUtils::Cdma;
        break;
    case NetworkManager::ConnectionSettings::Gsm:
        return UiUtils::Gsm;
        break;
    case NetworkManager::ConnectionSettings::Infiniband:
        return UiUtils::Infiniband;
        break;
    case NetworkManager::ConnectionSettings::OLPCMesh:
        return UiUtils::OLPCMesh;
        break;
    case NetworkManager::ConnectionSettings::Pppoe:
        return UiUtils::Pppoe;
        break;
    case NetworkManager::ConnectionSettings::Team:
        return UiUtils::Team;
        break;
    case NetworkManager::ConnectionSettings::Vlan:
        return UiUtils::Vlan;
        break;
    case NetworkManager::ConnectionSettings::Vpn:
        return UiUtils::Vpn;
        break;
    case NetworkManager::ConnectionSettings::Wired:
        return UiUtils::Wired;
        break;
    case NetworkManager::ConnectionSettings::Wireless:
        return UiUtils::Wireless;
        break;
    case NetworkManager::ConnectionSettings::WireGuard:
        return UiUtils::Wireguard;
        break;
    default:
        return UiUtils::Unknown;
        break;
    }
}

bool UiUtils::isConnectionTypeSupported(NetworkManager::ConnectionSettings::ConnectionType type)
{
    if (type == NetworkManager::ConnectionSettings::Generic || type == NetworkManager::ConnectionSettings::Tun) {
        return false;
    }

    if (isConnectionTypeVirtual(type)) {
        return Configuration::self().manageVirtualConnections();
    }

    return true;
}

bool UiUtils::isConnectionTypeVirtual(NetworkManager::ConnectionSettings::ConnectionType type)
{
    if (type == NetworkManager::ConnectionSettings::Bond //
        || type == NetworkManager::ConnectionSettings::Bridge //
        || type == NetworkManager::ConnectionSettings::Infiniband //
        || type == NetworkManager::ConnectionSettings::Team //
        || type == NetworkManager::ConnectionSettings::Vlan //
        || type == NetworkManager::ConnectionSettings::Loopback) {
        return true;
    }

    return false;
}

QString UiUtils::interfaceTypeLabel(const NetworkManager::Device::Type type, const NetworkManager::Device::Ptr iface)
{
    QString deviceText;
    switch (type) {
    case NetworkManager::Device::Wifi:
        deviceText = i18nc("title of the interface widget in nm's popup", "Wi-Fi");
        break;
    case NetworkManager::Device::Bluetooth:
        deviceText = i18nc("title of the interface widget in nm's popup", "Bluetooth");
        break;
    case NetworkManager::Device::InfiniBand:
        deviceText = i18nc("title of the interface widget in nm's popup", "Infiniband");
        break;
    case NetworkManager::Device::Adsl:
        deviceText = i18nc("title of the interface widget in nm's popup", "ADSL");
        break;
    case NetworkManager::Device::Bond:
        deviceText = i18nc("title of the interface widget in nm's popup", "Virtual (bond)");
        break;
    case NetworkManager::Device::Bridge:
        deviceText = i18nc("title of the interface widget in nm's popup", "Virtual (bridge)");
        break;
    case NetworkManager::Device::Vlan:
        deviceText = i18nc("title of the interface widget in nm's popup", "Virtual (vlan)");
        break;
    case NetworkManager::Device::Team:
        deviceText = i18nc("title of the interface widget in nm's popup", "Virtual (team)");
        break;
    case NetworkManager::Device::Modem: {
        const NetworkManager::ModemDevice::Ptr nmModemIface = iface.objectCast<NetworkManager::ModemDevice>();
        if (nmModemIface) {
            switch (modemSubType(nmModemIface->currentCapabilities())) {
            case NetworkManager::ModemDevice::Pots:
                deviceText = i18nc("title of the interface widget in nm's popup", "Serial Modem");
                break;
            case NetworkManager::ModemDevice::GsmUmts:
            case NetworkManager::ModemDevice::CdmaEvdo:
            case NetworkManager::ModemDevice::Lte:
                deviceText = i18nc("title of the interface widget in nm's popup", "Mobile Broadband");
                break;
            case NetworkManager::ModemDevice::NoCapability:
                qCWarning(PLASMA_NM_EDITOR_LOG) << "Unhandled modem sub type: NetworkManager::ModemDevice::NoCapability";
                break;
            }
        }
    } break;
    case NetworkManager::Device::Loopback:
        deviceText = i18nc("title of the interface widget in nm's popup", "Loopback");
        break;
    case NetworkManager::Device::Ethernet:
    default:
        deviceText = i18nc("title of the interface widget in nm's popup", "Wired Ethernet");
        break;
    }
    return deviceText;
}

QString UiUtils::iconAndTitleForConnectionSettingsType(NetworkManager::ConnectionSettings::ConnectionType type, QString &title)
{
    QString text;
    QString icon = QStringLiteral("network-wired");
    switch (type) {
    case ConnectionSettings::Adsl:
        text = i18n("ADSL");
        icon = QStringLiteral("network-modem");
        break;
    case ConnectionSettings::Pppoe:
        text = i18n("DSL");
        icon = QStringLiteral("network-modem");
        break;
    case ConnectionSettings::Bluetooth:
        text = i18n("Bluetooth");
        icon = QStringLiteral("network-wireless-bluetooth-symbolic");
        break;
    case ConnectionSettings::Bond:
        text = i18n("Bond");
        break;
    case ConnectionSettings::Bridge:
        text = i18n("Bridge");
        break;
    case ConnectionSettings::Gsm:
    case ConnectionSettings::Cdma:
        text = i18n("Mobile broadband");
        icon = QStringLiteral("smartphone");
        break;
    case ConnectionSettings::Infiniband:
        text = i18n("Infiniband");
        break;
    case ConnectionSettings::OLPCMesh:
        text = i18n("Olpc mesh");
        break;
    case ConnectionSettings::Vlan:
        text = i18n("VLAN");
        break;
    case ConnectionSettings::Vpn:
        text = i18n("VPN");
        icon = QStringLiteral("network-vpn");
        break;
    case ConnectionSettings::Wired:
        text = i18n("Wired Ethernet");
        icon = QStringLiteral("network-wired");
        break;
    case ConnectionSettings::Wireless:
        text = i18n("Wi-Fi");
        icon = QStringLiteral("network-wireless");
        break;
    case ConnectionSettings::Team:
        text = i18n("Team");
        break;
    case ConnectionSettings::WireGuard:
        text = i18n("WireGuard VPN");
        icon = QStringLiteral("network-vpn");
        break;
    case ConnectionSettings::Loopback:
        text = i18n("Loopback");
        break;
    default:
        text = i18n("Unknown connection type");
        break;
    }
    title = text;
    return icon;
}

QString UiUtils::prettyInterfaceName(NetworkManager::Device::Type type, const QString &interfaceName)
{
    QString ret;
    switch (type) {
    case NetworkManager::Device::Wifi:
        ret = i18n("Wireless Interface (%1)", interfaceName);
        break;
    case NetworkManager::Device::Ethernet:
        ret = i18n("Wired Interface (%1)", interfaceName);
        break;
    case NetworkManager::Device::Bluetooth:
        ret = i18n("Bluetooth (%1)", interfaceName);
        break;
    case NetworkManager::Device::Modem:
        ret = i18n("Modem (%1)", interfaceName);
        break;
    case NetworkManager::Device::Adsl:
        ret = i18n("ADSL (%1)", interfaceName);
        break;
    case NetworkManager::Device::Vlan:
        ret = i18n("VLan (%1)", interfaceName);
        break;
    case NetworkManager::Device::Bridge:
        ret = i18n("Bridge (%1)", interfaceName);
        break;
    case NetworkManager::Device::Loopback:
        ret = i18n("Loopback Interface (%1)", interfaceName);
        break;
    default:
        ret = interfaceName;
    }
    return ret;
}

QString UiUtils::connectionStateToString(NetworkManager::Device::State state, const QString &connectionName)
{
    QString stateString;
    switch (state) {
    case NetworkManager::Device::UnknownState:
        stateString = i18nc("description of unknown network interface state", "Unknown");
        break;
    case NetworkManager::Device::Unmanaged:
        stateString = i18nc("description of unmanaged network interface state", "Unmanaged");
        break;
    case NetworkManager::Device::Unavailable:
        stateString = i18nc("description of unavailable network interface state", "Unavailable");
        break;
    case NetworkManager::Device::Disconnected:
        stateString = i18nc("description of unconnected network interface state", "Not connected");
        break;
    case NetworkManager::Device::Preparing:
        stateString = i18nc("description of preparing to connect network interface state", "Preparing to connect");
        break;
    case NetworkManager::Device::ConfiguringHardware:
        stateString = i18nc("description of configuring hardware network interface state", "Configuring interface");
        break;
    case NetworkManager::Device::NeedAuth:
        stateString = i18nc("description of waiting for authentication network interface state", "Waiting for authorization");
        break;
    case NetworkManager::Device::ConfiguringIp:
        stateString = i18nc("network interface doing dhcp request in most cases", "Setting network address");
        break;
    case NetworkManager::Device::CheckingIp:
        stateString = i18nc("is other action required to fully connect? captive portals, etc.", "Checking further connectivity");
        break;
    case NetworkManager::Device::WaitingForSecondaries:
        stateString = i18nc("a secondary connection (e.g. VPN) has to be activated first to continue", "Waiting for a secondary connection");
        break;
    case NetworkManager::Device::Activated:
        if (connectionName.isEmpty()) {
            stateString = i18nc("network interface connected state label", "Connected");
        } else {
            stateString = i18nc("network interface connected state label", "Connected to %1", connectionName);
        }
        break;
    case NetworkManager::Device::Deactivating:
        stateString = i18nc("network interface disconnecting state label", "Deactivating connection");
        break;
    case NetworkManager::Device::Failed:
        stateString = i18nc("network interface connection failed state label", "Connection Failed");
        break;
    default:
        stateString = i18nc("interface state", "Error: Invalid state");
    }
    return stateString;
}

QString UiUtils::vpnConnectionStateToString(VpnConnection::State state)
{
    QString stateString;
    switch (state) {
    case VpnConnection::Unknown:
        stateString = i18nc("The state of the VPN connection is unknown", "Unknown");
        break;
    case VpnConnection::Prepare:
        stateString = i18nc("The VPN connection is preparing to connect", "Preparing to connect");
        break;
    case VpnConnection::NeedAuth:
        stateString = i18nc("The VPN connection needs authorization credentials", "Needs authorization");
        break;
    case VpnConnection::Connecting:
        stateString = i18nc("The VPN connection is being established", "Connecting");
        break;
    case VpnConnection::GettingIpConfig:
        stateString = i18nc("The VPN connection is getting an IP address", "Setting network address");
        break;
    case VpnConnection::Activated:
        stateString = i18nc("The VPN connection is active", "Activated");
        break;
    case VpnConnection::Failed:
        stateString = i18nc("The VPN connection failed", "Failed");
        break;
    case VpnConnection::Disconnected:
        stateString = i18nc("The VPN connection is disconnected", "Failed");
        break;
    default:
        stateString = i18nc("interface state", "Error: Invalid state");
    }
    return stateString;
}

QString UiUtils::operationModeToString(NetworkManager::WirelessDevice::OperationMode mode)
{
    QString modeString;
    switch (mode) {
    case NetworkManager::WirelessDevice::WirelessDevice::Unknown:
        modeString = i18nc("wireless network operation mode", "Unknown");
        break;
    case NetworkManager::WirelessDevice::Adhoc:
        modeString = i18nc("wireless network operation mode", "Adhoc");
        break;
    case NetworkManager::WirelessDevice::WirelessDevice::Infra:
        modeString = i18nc("wireless network operation mode", "Infrastructure");
        break;
    case NetworkManager::WirelessDevice::WirelessDevice::ApMode:
        modeString = i18nc("wireless network operation mode", "Access point");
        break;
    default:
        modeString = QStringLiteral("INCORRECT MODE FIX ME");
    }
    return modeString;
}

QStringList UiUtils::wpaFlagsToStringList(NetworkManager::AccessPoint::WpaFlags flags)
{
    /* for testing purposes
    flags = NetworkManager::AccessPoint::PairWep40
            | NetworkManager::AccessPoint::PairWep104
            | NetworkManager::AccessPoint::PairTkip
            | NetworkManager::AccessPoint::PairCcmp
            | NetworkManager::AccessPoint::GroupWep40
            | NetworkManager::AccessPoint::GroupWep104
            | NetworkManager::AccessPoint::GroupTkip
            | NetworkManager::AccessPoint::GroupCcmp
            | NetworkManager::AccessPoint::KeyMgmtPsk
            | NetworkManager::AccessPoint::KeyMgmt8021x; */

    QStringList flagList;

    if (flags.testFlag(NetworkManager::AccessPoint::PairWep40))
        flagList.append(i18nc("wireless network cipher", "Pairwise WEP40"));
    if (flags.testFlag(NetworkManager::AccessPoint::PairWep104))
        flagList.append(i18nc("wireless network cipher", "Pairwise WEP104"));
    if (flags.testFlag(NetworkManager::AccessPoint::PairTkip))
        flagList.append(i18nc("wireless network cipher", "Pairwise TKIP"));
    if (flags.testFlag(NetworkManager::AccessPoint::PairCcmp))
        flagList.append(i18nc("wireless network cipher", "Pairwise CCMP"));
    if (flags.testFlag(NetworkManager::AccessPoint::GroupWep40))
        flagList.append(i18nc("wireless network cipher", "Group WEP40"));
    if (flags.testFlag(NetworkManager::AccessPoint::GroupWep104))
        flagList.append(i18nc("wireless network cipher", "Group WEP104"));
    if (flags.testFlag(NetworkManager::AccessPoint::GroupTkip))
        flagList.append(i18nc("wireless network cipher", "Group TKIP"));
    if (flags.testFlag(NetworkManager::AccessPoint::GroupCcmp))
        flagList.append(i18nc("wireless network cipher", "Group CCMP"));
    if (flags.testFlag(NetworkManager::AccessPoint::KeyMgmtPsk))
        flagList.append(i18nc("wireless network cipher", "PSK"));
    if (flags.testFlag(NetworkManager::AccessPoint::KeyMgmt8021x))
        flagList.append(i18nc("wireless network cipher", "802.1x"));

    return flagList;
}

QString UiUtils::connectionSpeed(double bitrate)
{
    QString out;
    if (bitrate < 1000) {
        out = i18nc("connection speed", "%1 Bit/s", bitrate);
    } else if (bitrate < 1000000) {
        out = i18nc("connection speed", "%1 MBit/s", bitrate / 1000);
    } else {
        out = i18nc("connection speed", "%1 GBit/s", bitrate / 1000000);
    }
    return out;
}

QString UiUtils::wirelessBandToString(NetworkManager::WirelessSetting::FrequencyBand band)
{
    switch (band) {
    case NetworkManager::WirelessSetting::Automatic:
        return QStringLiteral("automatic");
        break;
    case NetworkManager::WirelessSetting::A:
        return QStringLiteral("a");
        break;
    case NetworkManager::WirelessSetting::Bg:
        return QStringLiteral("b/g");
        break;
    }

    return {};
}

QString UiUtils::wirelessFrequencyToString(uint frequency)
{
    return i18nc("Wireless connection frequency", "%1 GHz", frequency / 1000.0);
}

QString UiUtils::convertAllowedModeToString(ModemManager::Modem::ModemModes modes)
{
    if (modes.testFlag(MM_MODEM_MODE_4G)) {
        return i18nc("Gsm modes (2G/3G/any)", "LTE");
    } else if (modes.testFlag(MM_MODEM_MODE_3G)) {
        return i18nc("Gsm modes (2G/3G/any)", "UMTS/HSxPA");
    } else if (modes.testFlag(MM_MODEM_MODE_2G)) {
        return i18nc("Gsm modes (2G/3G/any)", "GPRS/EDGE");
    } else if (modes.testFlag(MM_MODEM_MODE_CS)) {
        return i18nc("Gsm modes (2G/3G/any)", "GSM");
    } else if (modes.testFlag(MM_MODEM_MODE_ANY)) {
        return i18nc("Gsm modes (2G/3G/any)", "Any");
    }

    return i18nc("Gsm modes (2G/3G/any)", "Any");
}

QString UiUtils::convertAccessTechnologyToString(ModemManager::Modem::AccessTechnologies tech)
{
#if MM_CHECK_VERSION(1, 14, 0)
    if (tech.testFlag(MM_MODEM_ACCESS_TECHNOLOGY_5GNR)) {
        return i18nc("Cellular access technology", "5G NR");
    } else
#endif
        if (tech.testFlag(MM_MODEM_ACCESS_TECHNOLOGY_LTE)) {
        return i18nc("Cellular access technology", "LTE");
    } else if (tech.testFlag(MM_MODEM_ACCESS_TECHNOLOGY_EVDOB)) {
        return i18nc("Cellular access technology", "CDMA2000 EVDO revision B");
    } else if (tech.testFlag(MM_MODEM_ACCESS_TECHNOLOGY_EVDOA)) {
        return i18nc("Cellular access technology", "CDMA2000 EVDO revision A");
    } else if (tech.testFlag(MM_MODEM_ACCESS_TECHNOLOGY_EVDO0)) {
        return i18nc("Cellular access technology", "CDMA2000 EVDO revision 0");
    } else if (tech.testFlag(MM_MODEM_ACCESS_TECHNOLOGY_1XRTT)) {
        return i18nc("Cellular access technology", "CDMA2000 1xRTT");
    } else if (tech.testFlag(MM_MODEM_ACCESS_TECHNOLOGY_HSPA_PLUS)) {
        return i18nc("Cellular access technology", "HSPA+");
    } else if (tech.testFlag(MM_MODEM_ACCESS_TECHNOLOGY_HSPA)) {
        return i18nc("Cellular access technology", "HSPA");
    } else if (tech.testFlag(MM_MODEM_ACCESS_TECHNOLOGY_HSUPA)) {
        return i18nc("Cellular access technology", "HSUPA");
    } else if (tech.testFlag(MM_MODEM_ACCESS_TECHNOLOGY_HSDPA)) {
        return i18nc("Cellular access technology", "HSDPA");
    } else if (tech.testFlag(MM_MODEM_ACCESS_TECHNOLOGY_UMTS)) {
        return i18nc("Cellular access technology", "UMTS");
    } else if (tech.testFlag(MM_MODEM_ACCESS_TECHNOLOGY_EDGE)) {
        return i18nc("Cellular access technology", "EDGE");
    } else if (tech.testFlag(MM_MODEM_ACCESS_TECHNOLOGY_GPRS)) {
        return i18nc("Cellular access technology", "GPRS");
    } else if (tech.testFlag(MM_MODEM_ACCESS_TECHNOLOGY_GSM_COMPACT)) {
        return i18nc("Cellular access technology", "Compact GSM");
    } else if (tech.testFlag(MM_MODEM_ACCESS_TECHNOLOGY_GSM)) {
        return i18nc("Cellular access technology", "GSM");
    } else if (tech.testFlag(MM_MODEM_ACCESS_TECHNOLOGY_POTS)) {
        return i18nc("Analog wireline modem", "Analog");
    } else if (tech.testFlag(MM_MODEM_ACCESS_TECHNOLOGY_UNKNOWN)) {
        return i18nc("Unknown cellular access technology", "Unknown");
    } else if (tech.testFlag(MM_MODEM_ACCESS_TECHNOLOGY_ANY)) {
        return i18nc("Any cellular access technology", "Any");
    }

    return i18nc("Unknown cellular access technology", "Unknown");
}

QString UiUtils::convertLockReasonToString(MMModemLock reason)
{
    switch (reason) {
    case MM_MODEM_LOCK_NONE:
        return i18nc("possible SIM lock reason", "Modem is unlocked.");
    case MM_MODEM_LOCK_SIM_PIN:
        return i18nc("possible SIM lock reason", "SIM requires the PIN code.");
    case MM_MODEM_LOCK_SIM_PIN2:
        return i18nc("possible SIM lock reason", "SIM requires the PIN2 code.");
    case MM_MODEM_LOCK_SIM_PUK:
        return i18nc("possible SIM lock reason", "SIM requires the PUK code.");
    case MM_MODEM_LOCK_SIM_PUK2:
        return i18nc("possible SIM lock reason", "SIM requires the PUK2 code.");
    case MM_MODEM_LOCK_PH_SP_PIN:
        return i18nc("possible SIM lock reason", "Modem requires the service provider PIN code.");
    case MM_MODEM_LOCK_PH_SP_PUK:
        return i18nc("possible SIM lock reason", "Modem requires the service provider PUK code.");
    case MM_MODEM_LOCK_PH_NET_PIN:
        return i18nc("possible SIM lock reason", "Modem requires the network PIN code.");
    case MM_MODEM_LOCK_PH_NET_PUK:
        return i18nc("possible SIM lock reason", "Modem requires the network PUK code.");
    case MM_MODEM_LOCK_PH_SIM_PIN:
        return i18nc("possible SIM lock reason", "Modem requires the PIN code.");
    case MM_MODEM_LOCK_PH_CORP_PIN:
        return i18nc("possible SIM lock reason", "Modem requires the corporate PIN code.");
    case MM_MODEM_LOCK_PH_CORP_PUK:
        return i18nc("possible SIM lock reason", "Modem requires the corporate PUK code.");
    case MM_MODEM_LOCK_PH_FSIM_PIN:
        return i18nc("possible SIM lock reason", "Modem requires the PH-FSIM PIN code.");
    case MM_MODEM_LOCK_PH_FSIM_PUK:
        return i18nc("possible SIM lock reason", "Modem requires the PH-FSIM PUK code.");
    case MM_MODEM_LOCK_PH_NETSUB_PIN:
        return i18nc("possible SIM lock reason", "Modem requires the network subset PIN code.");
    case MM_MODEM_LOCK_PH_NETSUB_PUK:
        return i18nc("possible SIM lock reason", "Modem requires the network subset PUK code.");
    case MM_MODEM_LOCK_UNKNOWN:
    default:
        return i18nc("possible SIM lock reason", "Lock reason unknown.");
    }
}

NetworkManager::ModemDevice::Capability UiUtils::modemSubType(NetworkManager::ModemDevice::Capabilities modemCaps)
{
    if (modemCaps & NetworkManager::ModemDevice::Lte) {
        return NetworkManager::ModemDevice::Lte;
    } else if (modemCaps & NetworkManager::ModemDevice::CdmaEvdo) {
        return NetworkManager::ModemDevice::CdmaEvdo;
    } else if (modemCaps & NetworkManager::ModemDevice::GsmUmts) {
        return NetworkManager::ModemDevice::GsmUmts;
    } else if (modemCaps & NetworkManager::ModemDevice::Pots) {
        return NetworkManager::ModemDevice::Pots;
    }
    return NetworkManager::ModemDevice::NoCapability;
}

QString UiUtils::labelFromWirelessSecurity(NetworkManager::WirelessSecurityType type)
{
    QString tip;
    switch (type) {
    case NetworkManager::NoneSecurity:
        tip = i18nc("@label no security", "Insecure");
        break;
    case NetworkManager::StaticWep:
        tip = i18nc("@label WEP security", "WEP");
        break;
    case NetworkManager::Leap:
        tip = i18nc("@label LEAP security", "LEAP");
        break;
    case NetworkManager::DynamicWep:
        tip = i18nc("@label Dynamic WEP security", "Dynamic WEP");
        break;
    case NetworkManager::WpaPsk:
        tip = i18nc("@label WPA-PSK security", "WPA-PSK");
        break;
    case NetworkManager::WpaEap:
        tip = i18nc("@label WPA-EAP security", "WPA-EAP");
        break;
    case NetworkManager::Wpa2Psk:
        tip = i18nc("@label WPA2-PSK security", "WPA2-PSK");
        break;
    case NetworkManager::Wpa2Eap:
        tip = i18nc("@label WPA2-EAP security", "WPA2-EAP");
        break;
    case NetworkManager::SAE:
        tip = i18nc("@label WPA3-SAE security", "WPA3-SAE");
        break;
    case NetworkManager::Wpa3SuiteB192:
        tip = i18nc("@label WPA3-EAP-SUITE-B-192 security", "WPA3-EAP-SUITE-B-192");
        break;
    case NetworkManager::OWE:
        tip = i18nc("@label OWE security", "Enhanced Open (OWE)");
        break;
    default:
        tip = i18nc("@label unknown security", "Unknown security type");
        break;
    }
    return tip;
}

QString UiUtils::formatDateRelative(const QDateTime &lastUsed)
{
    QString lastUsedText;
    if (lastUsed.isValid()) {
        const QDateTime now = QDateTime::currentDateTime();
        if (lastUsed.daysTo(now) == 0) {
            const int secondsAgo = lastUsed.secsTo(now);
            if (secondsAgo < (60 * 60)) {
                const int minutesAgo = secondsAgo / 60;
                lastUsedText = i18ncp("Label for last used time for a network connection used in the last hour, as the number of minutes since usage",
                                      "One minute ago",
                                      "%1 minutes ago",
                                      minutesAgo);
            } else {
                const int hoursAgo = secondsAgo / (60 * 60);
                lastUsedText = i18ncp("Label for last used time for a network connection used in the last day, as the number of hours since usage",
                                      "One hour ago",
                                      "%1 hours ago",
                                      hoursAgo);
            }
        } else if (lastUsed.daysTo(now) == 1) {
            lastUsedText = i18nc("Label for last used time for a network connection used the previous day", "Yesterday");
        } else {
            lastUsedText = QLocale().toString(lastUsed.date(), QLocale::ShortFormat);
        }
    } else {
        lastUsedText = i18nc(
            "Label for last used time for a "
            "network connection that has never been used",
            "Never");
    }
    return lastUsedText;
}

QString UiUtils::formatLastUsedDateRelative(const QDateTime &lastUsed)
{
    QString lastUsedText;
    if (lastUsed.isValid()) {
        const QDateTime now = QDateTime::currentDateTime();
        if (lastUsed.daysTo(now) == 0) {
            const int secondsAgo = lastUsed.secsTo(now);
            if (secondsAgo < (60 * 60)) {
                const int minutesAgo = secondsAgo / 60;
                lastUsedText = i18ncp("Label for last used time for a network connection used in the last hour, as the number of minutes since usage",
                                      "Last used one minute ago",
                                      "Last used %1 minutes ago",
                                      minutesAgo);
            } else {
                const int hoursAgo = secondsAgo / (60 * 60);
                lastUsedText = i18ncp("Label for last used time for a network connection used in the last day, as the number of hours since usage",
                                      "Last used one hour ago",
                                      "Last used %1 hours ago",
                                      hoursAgo);
            }
        } else if (lastUsed.daysTo(now) == 1) {
            lastUsedText = i18nc("Label for last used time for a network connection used the previous day", "Last used yesterday");
        } else {
            lastUsedText = i18n("Last used on %1", QLocale().toString(lastUsed.date(), QLocale::ShortFormat));
        }
    } else {
        lastUsedText = i18nc(
            "Label for last used time for a "
            "network connection that has never been used",
            "Never used");
    }
    return lastUsedText;
}

bool UiUtils::isLiveImage()
{
    static std::optional<bool> liveImage = std::nullopt;
    if (liveImage.has_value()) {
        return liveImage.value();
    }

    QFile cmdFile(QStringLiteral("/proc/cmdline"));
    cmdFile.open(QIODevice::ReadOnly);

    if (!cmdFile.isOpen()) {
        return false;
    }

    const QString cmdFileOutput = cmdFile.readAll();
    cmdFile.close();

    if (cmdFileOutput.contains(QStringLiteral("rd.live.image"))) {
        liveImage = true;
        return true;
    }

    liveImage = false;
    return false;
}

void UiUtils::setConnectionDefaultPermissions(NetworkManager::ConnectionSettings::Ptr &settings)
{
    auto wifiSecurity = settings->setting(NetworkManager::Setting::WirelessSecurity).dynamicCast<NetworkManager::WirelessSecuritySetting>();
    auto security8021x = settings->setting(NetworkManager::Setting::Security8021x).dynamicCast<NetworkManager::Security8021xSetting>();

    if (!wifiSecurity || !security8021x) {
        return;
    }

    if (Configuration::self().systemConnectionsByDefault() || !KWallet::Wallet::isEnabled() || isLiveImage()) {
        auto modifySystem = NetworkManager::permissions().value(QStringLiteral("org.freedesktop.NetworkManager.settings.modify.system"));
        if (modifySystem == QLatin1String("yes")) {
            wifiSecurity->setLeapPasswordFlags(NetworkManager::Setting::SecretFlagType::None);
            wifiSecurity->setPskFlags(NetworkManager::Setting::SecretFlagType::None);
            wifiSecurity->setWepKeyFlags(NetworkManager::Setting::SecretFlagType::None);
            security8021x->setPasswordFlags(NetworkManager::Setting::SecretFlagType::None);
            return;
        }
    }

    settings->addToPermissions(KUser().loginName(), QString());
    wifiSecurity->setLeapPasswordFlags(NetworkManager::Setting::SecretFlagType::AgentOwned);
    wifiSecurity->setPskFlags(NetworkManager::Setting::SecretFlagType::AgentOwned);
    wifiSecurity->setWepKeyFlags(NetworkManager::Setting::SecretFlagType::AgentOwned);
    security8021x->setPasswordFlags(NetworkManager::Setting::SecretFlagType::AgentOwned);
}
