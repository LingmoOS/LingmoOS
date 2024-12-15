// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DWIRELESSSECURITYSETTING_H
#define DWIRELESSSECURITYSETTING_H

#include "dnmsetting.h"
#include <QStringList>

DNETWORKMANAGER_BEGIN_NAMESPACE

class DWirelessSecuritySettingPrivate;

class DWirelessSecuritySetting : public DNMSetting
{
public:
    enum class KeyMgmt {
        Unknown = -1,
        Wep,
        Ieee8021x,
        WpaNone,
        WpaPsk,
        WpaEap,
        SAE,
        WpaEapSuiteB192,
    };
    enum class AuthAlg {
        None,
        Open,
        Shared,
        Leap,
    };
    enum class WpaProtocolVersion {
        Wpa,
        Rsn,
    };
    enum class WpaEncryptionCapabilities {
        Wep40,
        Wep104,
        Tkip,
        Ccmp,
    };
    enum class WepKeyType {
        NotSpecified,
        Hex,
        Passphrase,
    };
    enum class Pmf {
        Default,
        Disable,
        Optional,
        Required,
    };

    DWirelessSecuritySetting();
    explicit DWirelessSecuritySetting(const QSharedPointer<DWirelessSecuritySetting> &other);
    ~DWirelessSecuritySetting() override = default;

    QString name() const override;

    void setKeyMgmt(KeyMgmt mgmt);
    KeyMgmt keyMgmt() const;

    void setWepTxKeyindex(quint32 index);
    quint32 wepTxKeyindex() const;

    void setAuthAlg(AuthAlg alg);
    AuthAlg authAlg() const;

    void setProto(const QList<WpaProtocolVersion> &list);
    QList<WpaProtocolVersion> proto() const;

    void setPairwise(const QList<WpaEncryptionCapabilities> &list);
    QList<WpaEncryptionCapabilities> pairwise() const;

    void setGroup(const QList<WpaEncryptionCapabilities> &list);
    QList<WpaEncryptionCapabilities> group() const;

    void setLeapUsername(const QString &username);
    QString leapUsername() const;

    void setWepKey0(const QString &key);
    QString wepKey0() const;

    void setWepKey1(const QString &key);
    QString wepKey1() const;

    void setWepKey2(const QString &key);
    QString wepKey2() const;

    void setWepKey3(const QString &key);
    QString wepKey3() const;

    void setWepKeyFlags(SecretFlags type);
    SecretFlags wepKeyFlags() const;

    void setWepKeyType(WepKeyType type);
    WepKeyType wepKeyType() const;

    void setPsk(const QString &key);
    QString psk() const;

    void setPskFlags(SecretFlags type);
    SecretFlags pskFlags() const;

    void setLeapPassword(const QString &password);
    QString leapPassword() const;

    void setLeapPasswordFlags(SecretFlags type);
    SecretFlags leapPasswordFlags() const;

    void setPmf(Pmf pmf);
    Pmf pmf() const;

    void mapToSecrets(const QVariantMap &secrets) override;

    QVariantMap secretsToMap() const override;

    QStringList needSecrets(bool requestNew = false) const override;

    void fromMap(const QVariantMap &map) override;

    QVariantMap toMap() const override;

protected:
    QScopedPointer<DWirelessSecuritySettingPrivate> d_ptr;

private:
    Q_DECLARE_PRIVATE(DWirelessSecuritySetting)
};

DNETWORKMANAGER_END_NAMESPACE

Q_DECLARE_METATYPE(DTK_DNETWORKMANAGER_NAMESPACE::DWirelessSecuritySetting::WepKeyType)

#endif
