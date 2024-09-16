// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dwirelesssecuritysetting_p.h"

DNETWORKMANAGER_BEGIN_NAMESPACE

DWirelessSecuritySettingPrivate::DWirelessSecuritySettingPrivate()
    : m_wepTxKeyidx(0)
    , m_keyMgmt(DWirelessSecuritySetting::KeyMgmt::Unknown)
    , m_authAlg(DWirelessSecuritySetting::AuthAlg::None)
    , m_wepKeyFlags(DNMSetting::SecretFlagType::None)
    , m_wepKeyType(DWirelessSecuritySetting::WepKeyType::NotSpecified)
    , m_pskFlags(DNMSetting::SecretFlagType::None)
    , m_leapPasswordFlags(DNMSetting::SecretFlagType::None)
    , m_pmf(DWirelessSecuritySetting::Pmf::Default)
    , m_name(NM_SETTING_WIRELESS_SECURITY_SETTING_NAME)
{
}

DWirelessSecuritySetting::DWirelessSecuritySetting()
    : DNMSetting(DNMSetting::SettingType::WirelessSecurity)
    , d_ptr(new DWirelessSecuritySettingPrivate())
{
}

DWirelessSecuritySetting::DWirelessSecuritySetting(const QSharedPointer<DWirelessSecuritySetting> &other)
    : DNMSetting(other)
    , d_ptr(new DWirelessSecuritySettingPrivate())
{
    setKeyMgmt(other->keyMgmt());
    setWepTxKeyindex(other->wepTxKeyindex());
    setAuthAlg(other->authAlg());
    setProto(other->proto());
    setPairwise(other->pairwise());
    setGroup(other->group());
    setWepKey0(other->wepKey0());
    setWepKey1(other->wepKey1());
    setWepKey2(other->wepKey2());
    setWepKey3(other->wepKey3());
    setWepKeyFlags(other->wepKeyFlags());
    setWepKeyType(other->wepKeyType());
    setPsk(other->psk());
    setPskFlags(other->pskFlags());
    setLeapPassword(other->leapPassword());
    setLeapPasswordFlags(other->leapPasswordFlags());
    setPmf(other->pmf());
}

QString DWirelessSecuritySetting::name() const
{
    Q_D(const DWirelessSecuritySetting);

    return d->m_name;
}

void DWirelessSecuritySetting::setKeyMgmt(DWirelessSecuritySetting::KeyMgmt mgmt)
{
    Q_D(DWirelessSecuritySetting);

    d->m_keyMgmt = mgmt;
}

DWirelessSecuritySetting::KeyMgmt DWirelessSecuritySetting::keyMgmt() const
{
    Q_D(const DWirelessSecuritySetting);

    return d->m_keyMgmt;
}

void DWirelessSecuritySetting::setWepTxKeyindex(quint32 index)
{
    Q_D(DWirelessSecuritySetting);

    d->m_wepTxKeyidx = index;
}

quint32 DWirelessSecuritySetting::wepTxKeyindex() const
{
    Q_D(const DWirelessSecuritySetting);

    return d->m_wepTxKeyidx;
}

void DWirelessSecuritySetting::setAuthAlg(DWirelessSecuritySetting::AuthAlg alg)
{
    Q_D(DWirelessSecuritySetting);

    d->m_authAlg = alg;
}

DWirelessSecuritySetting::AuthAlg DWirelessSecuritySetting::authAlg() const
{
    Q_D(const DWirelessSecuritySetting);

    return d->m_authAlg;
}

void DWirelessSecuritySetting::setProto(const QList<DWirelessSecuritySetting::WpaProtocolVersion> &list)
{
    Q_D(DWirelessSecuritySetting);

    d->m_proto = list;
}

QList<DWirelessSecuritySetting::WpaProtocolVersion> DWirelessSecuritySetting::proto() const
{
    Q_D(const DWirelessSecuritySetting);

    return d->m_proto;
}

void DWirelessSecuritySetting::setPairwise(const QList<DWirelessSecuritySetting::WpaEncryptionCapabilities> &list)
{
    Q_D(DWirelessSecuritySetting);

    d->m_pairwise = list;
}

QList<DWirelessSecuritySetting::WpaEncryptionCapabilities> DWirelessSecuritySetting::pairwise() const
{
    Q_D(const DWirelessSecuritySetting);

    return d->m_pairwise;
}

void DWirelessSecuritySetting::setGroup(const QList<DWirelessSecuritySetting::WpaEncryptionCapabilities> &list)
{
    Q_D(DWirelessSecuritySetting);

    d->m_group = list;
}

QList<DWirelessSecuritySetting::WpaEncryptionCapabilities> DWirelessSecuritySetting::group() const
{
    Q_D(const DWirelessSecuritySetting);

    return d->m_group;
}

void DWirelessSecuritySetting::setLeapUsername(const QString &username)
{
    Q_D(DWirelessSecuritySetting);

    d->m_leapUsername = username;
}

QString DWirelessSecuritySetting::leapUsername() const
{
    Q_D(const DWirelessSecuritySetting);

    return d->m_leapUsername;
}

void DWirelessSecuritySetting::setWepKey0(const QString &key)
{
    Q_D(DWirelessSecuritySetting);

    d->m_wepKey0 = key;
}

QString DWirelessSecuritySetting::wepKey0() const
{
    Q_D(const DWirelessSecuritySetting);

    return d->m_wepKey0;
}

void DWirelessSecuritySetting::setWepKey1(const QString &key)
{
    Q_D(DWirelessSecuritySetting);

    d->m_wepKey1 = key;
}

QString DWirelessSecuritySetting::wepKey1() const
{
    Q_D(const DWirelessSecuritySetting);

    return d->m_wepKey1;
}

void DWirelessSecuritySetting::setWepKey2(const QString &key)
{
    Q_D(DWirelessSecuritySetting);

    d->m_wepKey2 = key;
}

QString DWirelessSecuritySetting::wepKey2() const
{
    Q_D(const DWirelessSecuritySetting);

    return d->m_wepKey2;
}

void DWirelessSecuritySetting::setWepKey3(const QString &key)
{
    Q_D(DWirelessSecuritySetting);

    d->m_wepKey3 = key;
}

QString DWirelessSecuritySetting::wepKey3() const
{
    Q_D(const DWirelessSecuritySetting);

    return d->m_wepKey3;
}

void DWirelessSecuritySetting::setWepKeyFlags(DNMSetting::SecretFlags type)
{
    Q_D(DWirelessSecuritySetting);

    d->m_wepKeyFlags = type;
}

DNMSetting::SecretFlags DWirelessSecuritySetting::wepKeyFlags() const
{
    Q_D(const DWirelessSecuritySetting);

    return d->m_wepKeyFlags;
}

void DWirelessSecuritySetting::setWepKeyType(DWirelessSecuritySetting::WepKeyType type)
{
    Q_D(DWirelessSecuritySetting);

    d->m_wepKeyType = type;
}

DWirelessSecuritySetting::WepKeyType DWirelessSecuritySetting::wepKeyType() const
{
    Q_D(const DWirelessSecuritySetting);

    return d->m_wepKeyType;
}

void DWirelessSecuritySetting::setPsk(const QString &key)
{
    Q_D(DWirelessSecuritySetting);

    d->m_psk = key;
}

QString DWirelessSecuritySetting::psk() const
{
    Q_D(const DWirelessSecuritySetting);

    return d->m_psk;
}

void DWirelessSecuritySetting::setPskFlags(DNMSetting::SecretFlags type)
{
    Q_D(DWirelessSecuritySetting);

    d->m_pskFlags = type;
}

DNMSetting::SecretFlags DWirelessSecuritySetting::pskFlags() const
{
    Q_D(const DWirelessSecuritySetting);

    return d->m_pskFlags;
}

void DWirelessSecuritySetting::setLeapPassword(const QString &password)
{
    Q_D(DWirelessSecuritySetting);

    d->m_leapPassword = password;
}

QString DWirelessSecuritySetting::leapPassword() const
{
    Q_D(const DWirelessSecuritySetting);

    return d->m_leapPassword;
}

void DWirelessSecuritySetting::setLeapPasswordFlags(DNMSetting::SecretFlags type)
{
    Q_D(DWirelessSecuritySetting);

    d->m_leapPasswordFlags = type;
}

DNMSetting::SecretFlags DWirelessSecuritySetting::leapPasswordFlags() const
{
    Q_D(const DWirelessSecuritySetting);

    return d->m_leapPasswordFlags;
}

void DWirelessSecuritySetting::setPmf(DWirelessSecuritySetting::Pmf pmf)
{
    Q_D(DWirelessSecuritySetting);

    d->m_pmf = pmf;
}

DWirelessSecuritySetting::Pmf DWirelessSecuritySetting::pmf() const
{
    Q_D(const DWirelessSecuritySetting);

    return d->m_pmf;
}

void DWirelessSecuritySetting::mapToSecrets(const QVariantMap &secrets)
{
    if (secrets.contains(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY0))) {
        setWepKey0(secrets.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY0)).toString());
    }

    if (secrets.contains(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY1))) {
        setWepKey1(secrets.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY1)).toString());
    }

    if (secrets.contains(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY2))) {
        setWepKey2(secrets.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY2)).toString());
    }

    if (secrets.contains(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY3))) {
        setWepKey3(secrets.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY3)).toString());
    }

    if (secrets.contains(QLatin1String(NM_SETTING_WIRELESS_SECURITY_PSK))) {
        setPsk(secrets.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_PSK)).toString());
    }

    if (secrets.contains(QLatin1String(NM_SETTING_WIRELESS_SECURITY_LEAP_PASSWORD))) {
        setLeapPassword(secrets.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_LEAP_PASSWORD)).toString());
    }
}

QVariantMap DWirelessSecuritySetting::secretsToMap() const
{
    QVariantMap secrets;

    if (!wepKey0().isEmpty()) {
        secrets.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY0), wepKey0());
    }

    if (!wepKey1().isEmpty()) {
        secrets.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY1), wepKey1());
    }

    if (!wepKey2().isEmpty()) {
        secrets.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY2), wepKey2());
    }

    if (!wepKey3().isEmpty()) {
        secrets.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY3), wepKey3());
    }

    if (!psk().isEmpty()) {
        secrets.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_PSK), psk());
    }

    if (!leapPassword().isEmpty()) {
        secrets.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_LEAP_PASSWORD), leapPassword());
    }

    return secrets;
}

QStringList DWirelessSecuritySetting::needSecrets(bool requestNew) const
{
    QStringList secrets;

    if (keyMgmt() == KeyMgmt::Wep) {
        if (!wepKeyFlags().testFlag(DNMSetting::SecretFlagType::NotRequired)) {
            switch (wepTxKeyindex()) {
                case 0:
                    if (wepKey0().isEmpty() or requestNew) {
                        secrets << QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY0);
                        return secrets;
                    }
                    break;
                case 1:
                    if (wepKey1().isEmpty() or requestNew) {
                        secrets << QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY1);
                        return secrets;
                    }
                    break;
                case 2:
                    if (wepKey2().isEmpty() or requestNew) {
                        secrets << QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY2);
                        return secrets;
                    }
                    break;
                case 3:
                    if (wepKey3().isEmpty() or requestNew) {
                        secrets << QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY3);
                        return secrets;
                    }
                    break;
            }
        }
    }

    if (keyMgmt() == KeyMgmt::WpaNone or keyMgmt() == KeyMgmt::WpaPsk or keyMgmt() == KeyMgmt::SAE) {
        if (!pskFlags().testFlag(DNMSetting::SecretFlagType::NotRequired)) {
            if (psk().isEmpty() or requestNew) {
                secrets << QLatin1String(NM_SETTING_WIRELESS_SECURITY_PSK);
                return secrets;
            }
        }
    }

    if (authAlg() == AuthAlg::Leap and keyMgmt() == KeyMgmt::Ieee8021x) {
        if (!leapPasswordFlags().testFlag(DNMSetting::SecretFlagType::NotRequired)) {
            if (leapPassword().isEmpty() or requestNew) {
                secrets << QLatin1String(NM_SETTING_WIRELESS_SECURITY_LEAP_PASSWORD);
                return secrets;
            }
        }
    }

    return secrets;
}

void DWirelessSecuritySetting::fromMap(const QVariantMap &map)
{
    if (map.contains(QLatin1String(NM_SETTING_WIRELESS_SECURITY_KEY_MGMT))) {
        const QString key = map.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_KEY_MGMT)).toString();
        if (key == "none") {
            setKeyMgmt(KeyMgmt::Wep);
        } else if (key == "ieee8021x") {
            setKeyMgmt(KeyMgmt::Ieee8021x);
        } else if (key == "wpa-none") {
            setKeyMgmt(KeyMgmt::WpaNone);
        } else if (key == "wpa-psk") {
            setKeyMgmt(KeyMgmt::WpaPsk);
        } else if (key == "wpa-eap") {
            setKeyMgmt(KeyMgmt::WpaEap);
        } else if (key == "sae") {
            setKeyMgmt(KeyMgmt::SAE);
        } else if (key == "wpa-eap-suite-b-192") {
            setKeyMgmt(KeyMgmt::WpaEapSuiteB192);
        }
    }

    if (map.contains(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_TX_KEYIDX))) {
        setWepTxKeyindex(map.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_TX_KEYIDX)).toUInt());
    }

    if (map.contains(QLatin1String(NM_SETTING_WIRELESS_SECURITY_AUTH_ALG))) {
        const QString alg = map.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_AUTH_ALG)).toString();
        if (alg == "open") {
            setAuthAlg(AuthAlg::Open);
        } else if (alg == "shared") {
            setAuthAlg(AuthAlg::Shared);
        } else if (alg == "leap") {
            setAuthAlg(AuthAlg::Leap);
        }
    }

    if (map.contains(QLatin1String(NM_SETTING_WIRELESS_SECURITY_PROTO))) {
        const QStringList strList = map.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_PROTO)).toStringList();
        QList<WpaProtocolVersion> list;
        for (const QString &str : strList) {
            if (str == "wpa") {
                list.push_back(WpaProtocolVersion::Wpa);
            } else if (str == "rsn") {
                list.push_back(WpaProtocolVersion::Rsn);
            }
        }
        setProto(list);
    }

    if (map.contains(QLatin1String(NM_SETTING_WIRELESS_SECURITY_PAIRWISE))) {
        const QStringList strList = map.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_PAIRWISE)).toStringList();
        QList<WpaEncryptionCapabilities> list;
        for (const QString &str : strList) {
            if (str == "wep40") {
                list.push_back(WpaEncryptionCapabilities::Wep40);
            } else if (str == "wep104") {
                list.push_back(WpaEncryptionCapabilities::Wep104);
            } else if (str == "tkip") {
                list.push_back(WpaEncryptionCapabilities::Tkip);
            } else if (str == "ccmp") {
                list.push_back(WpaEncryptionCapabilities::Ccmp);
            }
        }
        setPairwise(list);
    }

    if (map.contains(QLatin1String(NM_SETTING_WIRELESS_SECURITY_GROUP))) {
        const QStringList strList = map.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_GROUP)).toStringList();
        QList<WpaEncryptionCapabilities> list;
        for (const QString &str : strList) {
            if (str == "wep40") {
                list.push_back(WpaEncryptionCapabilities::Wep40);
            } else if (str == "wep104") {
                list.push_back(WpaEncryptionCapabilities::Wep104);
            } else if (str == "tkip") {
                list.push_back(WpaEncryptionCapabilities::Tkip);
            } else if (str == "ccmp") {
                list.push_back(WpaEncryptionCapabilities::Ccmp);
            }
        }
        setGroup(list);
    }

    if (map.contains(QLatin1String(NM_SETTING_WIRELESS_SECURITY_LEAP_USERNAME))) {
        setLeapUsername(map.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_LEAP_USERNAME)).toString());
    }

    if (map.contains(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY0))) {
        setWepKey0(map.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY0)).toString());
    }

    if (map.contains(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY1))) {
        setWepKey1(map.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY1)).toString());
    }

    if (map.contains(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY2))) {
        setWepKey2(map.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY2)).toString());
    }

    if (map.contains(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY3))) {
        setWepKey3(map.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY3)).toString());
    }

    if (map.contains(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY_FLAGS))) {
        setWepKeyFlags((SecretFlags)map.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY_FLAGS)).toInt());
    }

    if (map.contains(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY_TYPE))) {
        setWepKeyType((WepKeyType)map.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY_TYPE)).toUInt());
    }

    if (map.contains(QLatin1String(NM_SETTING_WIRELESS_SECURITY_PSK))) {
        setPsk(map.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_PSK)).toString());
    }

    if (map.contains(QLatin1String(NM_SETTING_WIRELESS_SECURITY_PSK_FLAGS))) {
        setPskFlags((SecretFlags)map.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_PSK_FLAGS)).toInt());
    }

    if (map.contains(QLatin1String(NM_SETTING_WIRELESS_SECURITY_LEAP_PASSWORD))) {
        setLeapPassword(map.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_LEAP_PASSWORD)).toString());
    }

    if (map.contains(QLatin1String(NM_SETTING_WIRELESS_SECURITY_LEAP_PASSWORD_FLAGS))) {
        setLeapPasswordFlags((SecretFlags)map.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_LEAP_PASSWORD_FLAGS)).toInt());
    }

    if (map.contains(QLatin1String(NM_SETTING_WIRELESS_SECURITY_PMF))) {
        setPmf((Pmf)map.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_PMF)).toInt());
    }
}

QVariantMap DWirelessSecuritySetting::toMap() const
{
    QVariantMap setting;

    if (keyMgmt() == KeyMgmt::Wep) {
        setting.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_KEY_MGMT), "none");
    } else if (keyMgmt() == KeyMgmt::Ieee8021x) {
        setting.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_KEY_MGMT), "ieee8021x");
    } else if (keyMgmt() == KeyMgmt::WpaNone) {
        setting.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_KEY_MGMT), "wpa-none");
    } else if (keyMgmt() == KeyMgmt::WpaPsk) {
        setting.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_KEY_MGMT), "wpa-psk");
    } else if (keyMgmt() == KeyMgmt::WpaEap) {
        setting.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_KEY_MGMT), "wpa-eap");
    } else if (keyMgmt() == KeyMgmt::SAE) {
        setting.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_KEY_MGMT), "sae");
    } else if (keyMgmt() == KeyMgmt::WpaEapSuiteB192) {
        setting.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_KEY_MGMT), "wpa-eap-suite-b-192");
    }

    if (wepTxKeyindex()) {
        setting.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_TX_KEYIDX), wepTxKeyindex());
    }

    if (authAlg() != AuthAlg::None) {
        if (authAlg() == AuthAlg::Open) {
            setting.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_AUTH_ALG), "open");
        } else if (authAlg() == AuthAlg::Shared) {
            setting.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_AUTH_ALG), "shared");
        } else if (authAlg() == AuthAlg::Leap) {
            setting.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_AUTH_ALG), "leap");
        }
    }

    if (!proto().isEmpty()) {
        QStringList strList;
        const auto protoList = proto();
        for (const WpaProtocolVersion &version : protoList) {
            if (version == WpaProtocolVersion::Wpa) {
                strList << "wpa";
            } else if (version == WpaProtocolVersion::Rsn) {
                strList << "rsn";
            }
        }
        setting.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_PROTO), strList);
    }

    if (!pairwise().isEmpty()) {
        QStringList strList;
        const auto encryptions = pairwise();
        for (const WpaEncryptionCapabilities &capability : encryptions) {
            if (capability == WpaEncryptionCapabilities::Wep40) {
                strList << "wep40";
            } else if (capability == WpaEncryptionCapabilities::Wep104) {
                strList << "wep104";
            } else if (capability == WpaEncryptionCapabilities::Tkip) {
                strList << "tkip";
            } else if (capability == WpaEncryptionCapabilities::Ccmp) {
                strList << "ccmp";
            }
        }
        setting.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_PAIRWISE), strList);
    }

    if (!group().isEmpty()) {
        QStringList strList;
        const auto encryptions = group();
        for (const WpaEncryptionCapabilities &capability : encryptions) {
            if (capability == WpaEncryptionCapabilities::Wep40) {
                strList << "wep40";
            } else if (capability == WpaEncryptionCapabilities::Wep104) {
                strList << "wep104";
            } else if (capability == WpaEncryptionCapabilities::Tkip) {
                strList << "tkip";
            } else if (capability == WpaEncryptionCapabilities::Ccmp) {
                strList << "ccmp";
            }
        }
        setting.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_GROUP), strList);
    }

    if (!leapUsername().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_LEAP_USERNAME), leapUsername());
    }

    if (!wepKey0().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY0), wepKey0());
    }

    if (!wepKey1().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY1), wepKey1());
    }

    if (!wepKey2().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY2), wepKey2());
    }

    if (!wepKey3().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY3), wepKey3());
    }

    if (wepKeyFlags()) {
        setting.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY_FLAGS), static_cast<int>(wepKeyFlags()));
    }

    if (wepKeyType() != WepKeyType::NotSpecified) {
        setting.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY_TYPE), QVariant::fromValue(wepKeyType()));
    }

    if (!psk().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_PSK), psk());
    }

    if (pskFlags()) {
        setting.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_PSK_FLAGS), static_cast<int>(pskFlags()));
    }

    if (!leapPassword().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_LEAP_PASSWORD), leapPassword());
    }

    if (leapPasswordFlags()) {
        setting.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_LEAP_PASSWORD_FLAGS), static_cast<int>(leapPasswordFlags()));
    }

    if (pmf() != Pmf::Default) {
        setting.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_PMF), static_cast<int>(pmf()));
    }

    return setting;
}

DNETWORKMANAGER_END_NAMESPACE
