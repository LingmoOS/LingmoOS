/*
    SPDX-FileCopyrightText: 2018 Billy Laws <blaws05@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_VXLAN_SETTING_H
#define NETWORKMANAGERQT_VXLAN_SETTING_H

#include "setting.h"
#include <networkmanagerqt/networkmanagerqt_export.h>

#include <QString>

namespace NetworkManager
{
class VxlanSettingPrivate;

/**
 * Represents vxlan setting
 */
class NETWORKMANAGERQT_EXPORT VxlanSetting : public Setting
{
public:
    typedef QSharedPointer<VxlanSetting> Ptr;
    typedef QList<Ptr> List;
    VxlanSetting();
    explicit VxlanSetting(const Ptr &other);
    ~VxlanSetting() override;

    QString name() const override;

    void setAgeing(quint32 ageing);
    quint32 ageing() const;

    void setDestinationPort(quint32 port);
    quint32 destinationPort() const;

    void setId(quint32 id);
    quint32 id() const;

    void setL2Miss(bool enable);
    bool l2Miss() const;

    void setL3Miss(bool enable);
    bool l3Miss() const;

    void setLearning(bool enable);
    bool learning() const;

    void setLimit(quint32 limit);
    quint32 limit() const;

    void setLocal(QString local);
    QString local() const;

    void setParent(QString parent);
    QString parent() const;

    void setProxy(bool enable);
    bool proxy() const;

    void setRemote(QString remote);
    QString remote() const;

    void setRsc(bool enable);
    bool rsc() const;

    void setSourcePortMax(quint32 maxPort);
    quint32 sourcePortMax() const;

    void setSourcePortMin(quint32 minPort);
    quint32 sourcePortMin() const;

    void setTos(quint32 tos);
    quint32 tos() const;

    void setTtl(quint32 ttl);
    quint32 ttl() const;

    void fromMap(const QVariantMap &setting) override;

    QVariantMap toMap() const override;

protected:
    VxlanSettingPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(VxlanSetting)
};

NETWORKMANAGERQT_EXPORT QDebug operator<<(QDebug dbg, const VxlanSetting &setting);

}

#endif // NETWORKMANAGERQT_VXLAN_SETTING_H
