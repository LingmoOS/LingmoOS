/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_GSM_SETTING_H
#define NETWORKMANAGERQT_GSM_SETTING_H

#include "setting.h"
#include <networkmanagerqt/networkmanagerqt_export.h>

#include <QString>

namespace NetworkManager
{
class GsmSettingPrivate;

/**
 * Represents gsm setting
 */
class NETWORKMANAGERQT_EXPORT GsmSetting : public Setting
{
public:
    typedef QSharedPointer<GsmSetting> Ptr;
    typedef QList<Ptr> List;
    enum NetworkType {
        Any = -1,
        Only3G,
        GprsEdgeOnly,
        Prefer3G,
        Prefer2G,
        Prefer4GLte,
        Only4GLte,
    };

    GsmSetting();
    explicit GsmSetting(const Ptr &other);
    ~GsmSetting() override;

    QString name() const override;

    void setNumber(const QString &number);
    QString number() const;

    void setUsername(const QString &username);
    QString username() const;

    void setPassword(const QString &password);
    QString password() const;

    void setPasswordFlags(SecretFlags flags);
    SecretFlags passwordFlags() const;

    void setApn(const QString &apn);
    QString apn() const;

    void setNetworkId(const QString &id);
    QString networkId() const;

    /// @deprecated since NM 1.0.0
    void setNetworkType(NetworkType type);
    NetworkType networkType() const;

    void setPin(const QString &pin);
    QString pin() const;

    void setPinFlags(SecretFlags flags);
    SecretFlags pinFlags() const;

    /// @deprecated since NM 1.0.0
    void setAllowedBand(quint32 band);
    quint32 allowedBand() const;

    void setHomeOnly(bool homeOnly);
    bool homeOnly() const;

    void setDeviceId(const QString &id);
    QString deviceId() const;

    void setSimId(const QString &id);
    QString simId() const;

    void setSimOperatorId(const QString &id);
    QString simOperatorId() const;

    void secretsFromMap(const QVariantMap &secrets) override;

    QVariantMap secretsToMap() const override;

    QStringList needSecrets(bool requestNew = false) const override;

    void fromMap(const QVariantMap &setting) override;

    QVariantMap toMap() const override;

protected:
    GsmSettingPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(GsmSetting)
};

NETWORKMANAGERQT_EXPORT QDebug operator<<(QDebug dbg, const GsmSetting &setting);

}

#endif // NETWORKMANAGERQT_GSM_SETTING_H
