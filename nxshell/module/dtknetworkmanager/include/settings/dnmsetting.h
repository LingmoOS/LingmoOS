// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DNMSETTING_H
#define DNMSETTING_H

#include "dnetworkmanager_global.h"
#include <QSharedPointer>
#include <QString>
#include <QVariant>

#undef signals  // avoiding gio signals(pointers to pointers) conflicts with Qt defined signals macro
#include <libnm/NetworkManager.h>
#define signals Q_SIGNALS

#if !NM_CHECK_VERSION(1, 16, 0)
#define NM_SETTING_WIREGUARD_SETTING_NAME "wireguard"
#endif

DNETWORKMANAGER_BEGIN_NAMESPACE

class DNMSettingPrivate;

class DNMSetting
{
public:
    enum class SettingType {
        Unknown,
        Adsl,
        Cdma,
        Gsm,
        Infiniband,
        Ipv4,
        Ipv6,
        Ppp,
        Pppoe,
        Security8021x,
        Serial,
        Vpn,
        Wired,
        Wireless,
        WirelessSecurity,
        Bluetooth,
        OlpcMesh,
        Vlan,
        Wimax,
        Bond,
        Bridge,
        BridgePort,
        Team,
        Generic,
        Tun,
        Vxlan,
        IpTunnel,
        Proxy,
        User,
        OvsBridge,
        OvsInterface,
        OvsPatch,
        OvsPort,
        Match,
        Tc,
        TeamPort,
        Macsec,
        Dcb,
        WireGuard
    };

    enum class SecretFlagType { None = 0, AgentOwned = 0x01, NotSaved = 0x02, NotRequired = 0x04 };
    Q_DECLARE_FLAGS(SecretFlags, SecretFlagType)

    static QString typeToString(const SettingType type);
    static SettingType stringToType(const QString &str);

    explicit DNMSetting() = default;
    explicit DNMSetting(SettingType type);
    explicit DNMSetting(const QSharedPointer<DNMSetting> &setting);
    virtual ~DNMSetting();

    virtual void fromMap(const QVariantMap &map) = 0;
    virtual QVariantMap toMap() const = 0;
    virtual QString name() const = 0;

    void setType(SettingType type);
    SettingType type() const;

    bool isInit() const;
    void setInit(bool init);

    virtual QStringList needSecrets(bool request = false) const;

    virtual QVariantMap secretsToMap() const;
    virtual void mapToSecrets(const QVariantMap &secrets);

    virtual void stringMapTosecrets(const QMap<QString, QString> &map);
    virtual QMap<QString, QString> secretsToStringMap() const;

protected:
    QScopedPointer<DNMSettingPrivate> d_ptr;

private:
    Q_DECLARE_PRIVATE(DNMSetting)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(DNMSetting::SecretFlags)

DNETWORKMANAGER_END_NAMESPACE

#endif
