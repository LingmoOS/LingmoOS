// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dnmsetting_p.h"

DNETWORKMANAGER_BEGIN_NAMESPACE

DNMSettingPrivate::DNMSettingPrivate()
    : m_type(DNMSetting::SettingType::Generic)
    , m_init(false)
{
}

DNMSetting::DNMSetting(SettingType type)
    : d_ptr(new DNMSettingPrivate())
{
    setType(type);
}

DNMSetting::DNMSetting(const QSharedPointer<DNMSetting> &other)
    : d_ptr(new DNMSettingPrivate())
{
    setInit(!other->isInit());
    setType(other->type());
}

DNMSetting::~DNMSetting() = default;

QString DNMSetting::typeToString(const SettingType type)
{
    switch (type) {
        case SettingType::Adsl:
            return QLatin1String(NM_SETTING_ADSL_SETTING_NAME);
        case SettingType::Ipv4:
            return QLatin1String(NM_SETTING_IP4_CONFIG_SETTING_NAME);
        case SettingType::Ipv6:
            return QLatin1String(NM_SETTING_IP6_CONFIG_SETTING_NAME);
        case SettingType::Security8021x:
            return QLatin1String(NM_SETTING_802_1X_SETTING_NAME);
        case SettingType::Vpn:
            return QLatin1String(NM_SETTING_VPN_SETTING_NAME);
        case SettingType::Wired:
            return QLatin1String(NM_SETTING_WIRED_SETTING_NAME);
        case SettingType::Wireless:
            return QLatin1String(NM_SETTING_WIRELESS_SETTING_NAME);
        case SettingType::WirelessSecurity:
            return QLatin1String(NM_SETTING_WIRELESS_SECURITY_SETTING_NAME);
        case SettingType::Generic:
            return QLatin1String(NM_SETTING_GENERIC_SETTING_NAME);
        case SettingType::Tun:
            return QLatin1String(NM_SETTING_TUN_SETTING_NAME);
        default:
            return "Unknown or Unsupported";
    }
    Q_UNREACHABLE();
}

DNMSetting::SettingType DNMSetting::stringToType(const QString &str)
{
    static QMap<QString, SettingType> map{{NM_SETTING_ADSL_SETTING_NAME, SettingType::Adsl},
                                          {NM_SETTING_IP4_CONFIG_SETTING_NAME, SettingType::Ipv4},
                                          {NM_SETTING_IP6_CONFIG_SETTING_NAME, SettingType::Ipv6},
                                          {NM_SETTING_802_1X_SETTING_NAME, SettingType::Security8021x},
                                          {NM_SETTING_VPN_SETTING_NAME, SettingType::Vpn},
                                          {NM_SETTING_WIRED_SETTING_NAME, SettingType::Wired},
                                          {NM_SETTING_WIRELESS_SETTING_NAME, SettingType::Wireless},
                                          {NM_SETTING_WIRELESS_SECURITY_SETTING_NAME, SettingType::WirelessSecurity},
                                          {NM_SETTING_GENERIC_SETTING_NAME, SettingType::Generic},
                                          {NM_SETTING_TUN_SETTING_NAME, SettingType::Tun}};
    if (const auto &elem = map.find(str); elem != map.end())
        return elem.value();
    return DNMSetting::SettingType::Unknown;
}

void DNMSetting::setType(SettingType type)
{
    Q_D(DNMSetting);
    d->m_type = type;
}

DNMSetting::SettingType DNMSetting::type() const
{
    Q_D(const DNMSetting);
    return d->m_type;
}

bool DNMSetting::isInit() const
{
    Q_D(const DNMSetting);
    return d->m_init;
}

void DNMSetting::setInit(bool init)
{
    Q_D(DNMSetting);
    d->m_init = init;
}

QStringList DNMSetting::needSecrets(bool request) const
{
    Q_UNUSED(request)
    return {};
}

QVariantMap DNMSetting::secretsToMap() const
{
    return {};
}

void DNMSetting::mapToSecrets(const QVariantMap &secrets)
{
    Q_UNUSED(secrets)
}

void DNMSetting::stringMapTosecrets(const QMap<QString, QString> &map)
{
    QVariantMap secretsMap;
    QMap<QString, QString>::ConstIterator i = map.constBegin();
    while (i != map.constEnd()) {
        secretsMap.insert(i.key(), i.value());
        ++i;
    }
    mapToSecrets(secretsMap);
}

QMap<QString, QString> DNMSetting::secretsToStringMap() const
{
    QMap<QString, QString> ret;
    QVariantMap secretsMap = secretsToMap();
    QVariantMap::ConstIterator i = secretsMap.constBegin();
    while (i != secretsMap.constEnd()) {
        ret.insert(i.key(), i.value().toString());
        ++i;
    }

    return ret;
}

DNETWORKMANAGER_END_NAMESPACE
