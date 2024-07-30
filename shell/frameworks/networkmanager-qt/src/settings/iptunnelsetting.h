/*
    SPDX-FileCopyrightText: 2018 Pranav Gade <pranavgade20@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_IPTUNNEL_SETTING_H
#define NETWORKMANAGERQT_IPTUNNEL_SETTING_H

#include "setting.h"
#include <networkmanagerqt/networkmanagerqt_export.h>

namespace NetworkManager
{
class IpTunnelSettingPrivate;

/**
 * Represents IpTunnel setting
 */
class NETWORKMANAGERQT_EXPORT IpTunnelSetting : public Setting
{
public:
    typedef QSharedPointer<IpTunnelSetting> Ptr;
    typedef QList<Ptr> List;
    enum Mode {
        Unknown = NM_IP_TUNNEL_MODE_UNKNOWN,
        Ipip = NM_IP_TUNNEL_MODE_IPIP,
        Gre = NM_IP_TUNNEL_MODE_GRE,
        Sit = NM_IP_TUNNEL_MODE_SIT,
        Isatap = NM_IP_TUNNEL_MODE_ISATAP,
        Vti = NM_IP_TUNNEL_MODE_VTI,
        Ip6ip6 = NM_IP_TUNNEL_MODE_IP6IP6,
        Ipip6 = NM_IP_TUNNEL_MODE_IPIP6,
        Ip6gre = NM_IP_TUNNEL_MODE_IP6GRE,
        Vti6 = NM_IP_TUNNEL_MODE_VTI,
    };

    enum Flag {
        None = 0x0,
        Ip6IgnEncapLimit = 0x1,
        Ip6UseOrigTclass = 0x2,
        Ip6UseOrigFlowlabel = 0x4,
        Ip6Mip6Dev = 0x8,
        Ip6RcvDscpCopy = 0x10,
        Ip6UseOrigFwmark = 0x20,
    };
    Q_DECLARE_FLAGS(Flags, Flag)

    IpTunnelSetting();
    explicit IpTunnelSetting(const Ptr &other);
    ~IpTunnelSetting() override;

    QString name() const override;

    void setMode(Mode mode);
    Mode mode() const;

    void setPathMtuDiscovery(bool discovery);
    bool pathMtuDiscovery() const;

    void setEncapsulationLimit(quint32 limit);
    quint32 encapsulationLimit() const;

    void setFlags(Flags flags);
    Flags flags() const;

    void setFlowLabel(quint32 label);
    quint32 flowLabel() const;

    void setMtu(quint32 mtu);
    quint32 mtu() const;

    void setTos(quint32 tos);
    quint32 tos() const;

    void setTtl(quint32 ttl);
    quint32 ttl() const;

    void setInputKey(const QString &key);
    QString inputKey() const;

    void setLocal(const QString &local);
    QString local() const;

    void setParent(const QString &parent);
    QString parent() const;

    void setOutputKey(const QString &key);
    QString outputKey() const;

    void setRemote(const QString &remote);
    QString remote() const;

    void fromMap(const QVariantMap &setting) override;

    QVariantMap toMap() const override;

protected:
    IpTunnelSettingPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(IpTunnelSetting)
};
Q_DECLARE_OPERATORS_FOR_FLAGS(IpTunnelSetting::Flags)

NETWORKMANAGERQT_EXPORT QDebug operator<<(QDebug dbg, const IpTunnelSetting &setting);

}

#endif // NETWORKMANAGERQT_IP_TUNNEL_SETTING_H
