/*
    SPDX-FileCopyrightText: 2018 Pranav Gade <pranavgade20@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_OVS_BRIDGE_SETTING_H
#define NETWORKMANAGERQT_OVS_BRIDGE_SETTING_H

#include "setting.h"
#include <networkmanagerqt/networkmanagerqt_export.h>

#include <QString>

namespace NetworkManager
{
class OvsBridgeSettingPrivate;

/**
 * Represents ovs-bridge setting
 */
class NETWORKMANAGERQT_EXPORT OvsBridgeSetting : public Setting
{
public:
    typedef QSharedPointer<OvsBridgeSetting> Ptr;
    typedef QList<Ptr> List;

    OvsBridgeSetting();
    explicit OvsBridgeSetting(const Ptr &other);
    ~OvsBridgeSetting() override;

    QString name() const override;

    void setMcastSnoopingEnable(bool mcastSnoopingEnable);
    bool mcastSnoopingEnable() const;

    void setRstpEnable(bool rstpEnable);
    bool rstpEnable() const;

    void setStpEnable(bool stpEnable);
    bool stpEnable() const;

    void setFailMode(const QString &mode);
    QString failMode() const;

    void fromMap(const QVariantMap &setting) override;

    QVariantMap toMap() const override;

protected:
    OvsBridgeSettingPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(OvsBridgeSetting)
};

NETWORKMANAGERQT_EXPORT QDebug operator<<(QDebug dbg, const OvsBridgeSetting &setting);

}

#endif // NETWORKMANAGERQT_OVS_BRIDGE_SETTING_H
