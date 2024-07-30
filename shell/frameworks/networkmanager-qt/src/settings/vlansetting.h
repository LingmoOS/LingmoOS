/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_VLAN_SETTING_H
#define NETWORKMANAGERQT_VLAN_SETTING_H

#include "setting.h"
#include <networkmanagerqt/networkmanagerqt_export.h>

#include <QString>

namespace NetworkManager
{
class VlanSettingPrivate;

/**
 * Represents vlan setting
 */
class NETWORKMANAGERQT_EXPORT VlanSetting : public Setting
{
public:
    typedef QSharedPointer<VlanSetting> Ptr;
    typedef QList<Ptr> List;
    enum Flag {
        None = 0,
        ReorderHeaders = 0x1,
        Gvrp = 0x2,
        LooseBinding = 0x4,
    };
    Q_DECLARE_FLAGS(Flags, Flag)

    VlanSetting();
    explicit VlanSetting(const Ptr &other);
    ~VlanSetting() override;

    QString name() const override;

    /// @deprecated in favor of connection.interface-name in NM 1.0.0
    void setInterfaceName(const QString &name);
    QString interfaceName() const;

    void setParent(const QString &parent);
    QString parent() const;

    void setId(quint32 id);
    quint32 id() const;

    void setFlags(Flags flags);
    Flags flags() const;

    void setIngressPriorityMap(const QStringList &map);
    QStringList ingressPriorityMap() const;

    void setEgressPriorityMap(const QStringList &map);
    QStringList egressPriorityMap() const;

    void fromMap(const QVariantMap &setting) override;

    QVariantMap toMap() const override;

protected:
    VlanSettingPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(VlanSetting)
};
Q_DECLARE_OPERATORS_FOR_FLAGS(VlanSetting::Flags)

NETWORKMANAGERQT_EXPORT QDebug operator<<(QDebug dbg, const VlanSetting &setting);

}

#endif // NETWORKMANAGERQT_VLAN_SETTING_H
