/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_VPN_SETTING_H
#define NETWORKMANAGERQT_VPN_SETTING_H

#include "setting.h"
#include <networkmanagerqt/networkmanagerqt_export.h>

#include <QString>

namespace NetworkManager
{
class VpnSettingPrivate;

/**
 * Represents vpn setting
 */
class NETWORKMANAGERQT_EXPORT VpnSetting : public Setting
{
public:
    typedef QSharedPointer<VpnSetting> Ptr;
    typedef QList<Ptr> List;
    VpnSetting();
    explicit VpnSetting(const Ptr &other);
    ~VpnSetting() override;

    QString name() const override;

    bool persistent() const;
    void setPersistent(bool persistent);

    void setServiceType(const QString &type);
    QString serviceType() const;

    uint timeout() const;
    void setTimeout(uint timeout);

    void setUsername(const QString &username);
    QString username() const;

    void setData(const NMStringMap &data);
    NMStringMap data() const;

    void setSecrets(const NMStringMap &secrets);
    NMStringMap secrets() const;

    void secretsFromMap(const QVariantMap &secrets) override;
    QVariantMap secretsToMap() const override;

    void secretsFromStringMap(const NMStringMap &map) override;
    NMStringMap secretsToStringMap() const override;

    void fromMap(const QVariantMap &setting) override;

    QVariantMap toMap() const override;

protected:
    VpnSettingPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(VpnSetting)
};

NETWORKMANAGERQT_EXPORT QDebug operator<<(QDebug dbg, const VpnSetting &setting);

}

#endif // NETWORKMANAGERQT_VPN_SETTING_H
