/*
    SPDX-FileCopyrightText: 2019 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_WIREGUARD_SETTING_H
#define NETWORKMANAGERQT_WIREGUARD_SETTING_H

#include "setting.h"
#include <networkmanagerqt/networkmanagerqt_export.h>

#include <QString>

namespace NetworkManager
{
class WireGuardSettingPrivate;

/**
 * Represents generic setting
 */
class NETWORKMANAGERQT_EXPORT WireGuardSetting : public Setting
{
public:
    typedef QSharedPointer<WireGuardSetting> Ptr;
    typedef QList<Ptr> List;
    WireGuardSetting();
    explicit WireGuardSetting(const Ptr &other);
    ~WireGuardSetting() override;

    QString name() const override;

    quint32 fwmark() const;
    void setFwmark(quint32 fwmark);

    quint32 listenPort() const;
    void setListenPort(quint32 port);

    quint32 mtu() const;
    void setMtu(quint32 mtu);

    bool peerRoutes() const;
    void setPeerRoutes(bool peerRoutes);

    NMVariantMapList peers() const;
    void setPeers(const NMVariantMapList &peers);

    QString privateKey() const;
    void setPrivateKey(const QString &key);

    SecretFlags privateKeyFlags() const;
    void setPrivateKeyFlags(SecretFlags flags);

    void secretsFromMap(const QVariantMap &secrets) override;

    QVariantMap secretsToMap() const override;

    void secretsFromStringMap(const NMStringMap &map) override;

    NMStringMap secretsToStringMap() const override;

    /*
     * Required "preshared-key" properties from "peers" property will be returned
     * as strings in format "peers.public-key.preshared-key".
     */
    QStringList needSecrets(bool requestNew = false) const override;

    void fromMap(const QVariantMap &setting) override;

    QVariantMap toMap() const override;

protected:
    WireGuardSettingPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(WireGuardSetting)
};

NETWORKMANAGERQT_EXPORT QDebug operator<<(QDebug dbg, const WireGuardSetting &setting);

}

#endif // NETWORKMANAGERQT_WIREGUARD_SETTING_H
