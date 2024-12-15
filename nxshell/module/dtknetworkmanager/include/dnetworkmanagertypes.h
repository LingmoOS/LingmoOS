// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DNETWORKMANAGERTYPES_H
#define DNETWORKMANAGERTYPES_H
#include "dnetworkmanager_global.h"
#include <QMap>
#include <QScopedPointer>
#include <QString>
#include <QVariant>

DNETWORKMANAGER_BEGIN_NAMESPACE

using Config = QMap<QString, QVariant>;
using SettingDesc = QMap<QString, Config>;

// custom enum

enum class NMState : quint8 {
    Unknown = 0,
    Asleep = 10,
    Disconnected = 20,
    Disconnecting = 30,
    Connecting = 40,
    ConnectedLocal = 50,
    ConnectedSite = 60,
    ConnectedGlobal = 70
};

enum class NMConnectivityState : quint8 { Unknown = 0, None, Portal, Limited, Full };

enum class NMActiveConnectionState : quint8 { Unknown = 0, Activating, Activated, Deactivating, Deactivated };

enum class NMActiveConnectionStateReason : quint8 {
    Unknown = 0,
    None,
    UserDisconnected,
    DeviceDisconnected,
    ServiceStopped,
    IPConfigInvalid,
    ConnectTimeout,
    ServiceStartTimeout,
    ServiceStartFailed,
    NoSecrets,
    LoginFailed,
    ConnectionRemoved,
    DependencyFailed,
    DeviceRealizeFailed,
    DeviceRemoved
};

enum class NMVpnConnectionState : quint8 {
    Unknown = 0,
    Prepare,
    NeedAuth,
    Connect,
    IPConfigGet,
    Activated,
    Failed,
    Disconnected
};

enum class NM80211ApFlags : quint8 { None = 0, Privacy, WPS, WPSPBC = 4, WPSPIN = 8 };

enum class NM80211ApSecurityFlags : quint16 {
    None = 0x0,
    PairWEP40,
    PairWEP104,
    PairTKIP = 0x4,
    PairCCMP = 0x8,
    GroupWEP40 = 0x10,
    GroupWEP104 = 0x20,
    GroupTKIP = 0x40,
    GroupCCMP = 0x80,
    KeyMgmtPsk = 0x100,
    KeyMgmt8021X = 0x200,
    KeyMgmtSAE = 0x400,
    KeyMgmtOWE = 0x800,
    KeyMgmtOWETM = 0x1000,
    KeyMgmtEAPSuiteB192 = 0x2000
};

enum class NMSettingsConnectionFlags : quint8 { None = 0, FlagUnsaved, NMGenerated, Volatile = 4, External = 8 };

enum class ProxyType : quint8 { HTTP = 0, Socks4, Socks5 };

enum class SystemProxyMethod : quint8 { None = 0, Manual, Auto };

enum class NMDeviceType : quint8 {
    Unknown = 0,
    Generic = 14,
    Ethernet = 1,
    WiFi = 2,
    Unused1 [[deprecated]] = 3,
    Unused2 [[deprecated]] = 4,
    BT = 5,
    OLPCMesh = 6,
    WiMAX = 7,
    Modem = 8,
    InfiniBand = 9,
    Bond = 10,
    VLAN = 11,
    ADSL = 12,
    Bridge = 13,
    Team = 15,
    Tun = 16,
    IPTunnel = 17,
    MACVLAN = 18,
    VXLAN = 19,
    VETH = 20,
    MACsec = 21,
    Dummy = 22,
    PPP = 23,
    OVSInterface = 24,
    OVSPort = 25,
    OVSBridge = 26,
    WPAN = 27,
    SixLoWPAN = 28,
    WireGuard = 29,
    WiFiP2P = 30,
    VRF = 31
};

enum class NMDeviceInterfaceFlags : quint32 { None = 0x0, UP, LowerUP, Carrier = 0x10000 };

enum class NMDeviceState : quint8 {
    Unknown = 0,
    Unmanaged = 10,
    Unavailable = 20,
    Disconnected = 30,
    Prepare = 40,
    Config = 50,
    NeedAuth = 60,
    IPConfig = 70,
    IPCheck = 80,
    Secondaries = 90,
    Activated = 100,
    Deactivated = 110,
    Failed = 120
};

enum class NMDeviceStateReason : quint8 {
    None = 0,
    Unknown,
    NowManaged,
    NowUnmanaged,
    ConfigFailed,
    IPConfigUnavailable,
    IPConfigExpired,
    NoSecret,
    SupplicantDisconnected,
    SupplicantConfigFailed,
    SupplicantFailed,
    SupplicantTimeout,
    PPPStartFailed,
    PPPDisconnect,
    PPPFailed,
    DHCPStartFailed,
    DHCPError,
    DHCPFailed,
    SharedStartFailed,
    SharedFailed,
    AutoIPStartFailed,
    AutoIPError,
    AutoIPFailed,
    ModemBusy,
    ModemNoDialTone,
    ModemNoCarrier,
    ModemDialTimeout,
    ModemDialFailed,
    ModemInitFailed,
    GSMAPNFailed,
    GSMRegistrationNotSearching,
    GSMRegistrationDenied,
    GSMRegistrationTimeout,
    GSMRegistrationFailed,
    GSMPINCheckFailed,
    FirmwareMissing,
    Removed,
    Sleeping,
    ConnectionRemoved,
    UserRequested,
    Carrier,
    ConnectionAssumed,
    SupplicantAvailable,
    ModemNotFound,
    BTFailed,
    GSMSIMNotInserted,
    GSMSIMPINRequired,
    GSMSIMPUKRequired,
    GSMSIMWrong,
    InfinibandMode,
    DependencyFailed,
    BR2684Failed,
    ModemManagerUnavailable,
    SSIDNotFound,
    SecondaryConnectionFailed,
    DCBFCoEFailed,
    TeamdControlFailed,
    ModemFailed,
    ModemAvailable,
    SIMPINIncorrect,
    NewActivation,
    ParentChanged,
    ParentManagedChanged,
    OVSDBFailed,
    IPAddressDuplicate,
    IPMethodUnsupported,
    SRIOVConfigurationFailed,
    PeerNotFound
};

enum class NM80211Mode : quint8 { Unknown = 0, AdHoc, Infra, AP, Mesh };

enum class NMDeviceWiFiCapabilities : quint16 {
    None = 0x0,
    CipherWEP40,
    CipherWEP104,
    CipherTKIP = 0x4,
    CipherCCMP = 0x8,
    WPA = 0x10,
    RSN = 0x20,
    AP = 0x40,
    AdHoc = 0x80,
    FreqValid = 0x100,
    Freq2GHz = 0x200,
    Freq5GHz = 0x400,
    Mesh = 0x1000,
    IBSSRsn = 0x2000
};

enum class NMSecretAgentCapabilities : quint8 { None = 0, VPNHints };

enum class NMSecretAgentGetSecretsFlags : quint32 {
    None = 0x0,
    AllowInteraction,
    RequestNew,
    UserRequested = 0x4,
    WPSPBCActive = 0x8,
    OnlySystem = 0x80000000,
    NoError = 0x40000000
};

enum class NMMetered { Unknown = 0, Yes, No, GuessYes, GuessNo };

enum class NMConfigMethod {
    Automatic = 0,
    LinkLocal,
    Manual,
    Disabled,
    Ipv4Shared,
    Ipv6Dhcp,
    Ipv6Ignored,
};

// custom structure

struct NewConn
{
    quint64 settingId;
    quint64 activeConnId;
    bool operator==(const NewConn &other) const
    {
        return (this->activeConnId == other.activeConnId) and (this->settingId == other.settingId);
    }
};

struct SystemProxyAddr
{
    quint32 port;
    QString host;
    bool operator==(const SystemProxyAddr &other) const { return (this->port == other.port) and (this->host == other.host); }
};

struct AppProxySet
{
    quint32 port;
    ProxyType type;
    QByteArray ip;
    QByteArray password;
    QString user;
    bool operator==(const AppProxySet &other) const
    {
        return (this->port == other.port) and (this->type == other.type) and (this->ip == other.ip) and
               (this->password == other.password) and (this->user == other.user);
    }
};

DNETWORKMANAGER_END_NAMESPACE

/**
 * @private
 */
Q_DECLARE_METATYPE(DTK_DNETWORKMANAGER_NAMESPACE::Config);

/**
 * @private
 */
Q_DECLARE_METATYPE(DTK_DNETWORKMANAGER_NAMESPACE::SettingDesc);

#endif
