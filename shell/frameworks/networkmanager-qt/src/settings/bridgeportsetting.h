/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_BRIDGEPORT_SETTING_H
#define NETWORKMANAGERQT_BRIDGEPORT_SETTING_H

#include "setting.h"
#include <networkmanagerqt/networkmanagerqt_export.h>

#include <QString>

namespace NetworkManager
{
class BridgePortSettingPrivate;

/**
 * Represents bridge port setting
 */
class NETWORKMANAGERQT_EXPORT BridgePortSetting : public Setting
{
public:
    typedef QSharedPointer<BridgePortSetting> Ptr;
    typedef QList<Ptr> List;
    BridgePortSetting();
    explicit BridgePortSetting(const Ptr &other);
    ~BridgePortSetting() override;

    QString name() const override;

    void setPriority(quint32 priority);
    quint32 priority() const;

    void setPathCost(quint32 cost);
    quint32 pathCost() const;

    void setHairpinMode(bool enable);
    bool hairpinMode() const;

    void fromMap(const QVariantMap &setting) override;

    QVariantMap toMap() const override;

protected:
    BridgePortSettingPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(BridgePortSetting)
};

NETWORKMANAGERQT_EXPORT QDebug operator<<(QDebug dbg, const BridgePortSetting &setting);

}

#endif // NETWORKMANAGERQT_BRIDGEPORT_SETTING_H
