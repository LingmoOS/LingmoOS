/*
    SPDX-FileCopyrightText: 2018 Pranav Gade <pranavgade20@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_OVS_INTERFACE_SETTING_H
#define NETWORKMANAGERQT_OVS_INTERFACE_SETTING_H

#include "setting.h"
#include <networkmanagerqt/networkmanagerqt_export.h>

#include <QString>

namespace NetworkManager
{
class OvsInterfaceSettingPrivate;

/**
 * Represents ovs-interface setting
 */
class NETWORKMANAGERQT_EXPORT OvsInterfaceSetting : public Setting
{
public:
    typedef QSharedPointer<OvsInterfaceSetting> Ptr;
    typedef QList<Ptr> List;

    OvsInterfaceSetting();
    explicit OvsInterfaceSetting(const Ptr &other);
    ~OvsInterfaceSetting() override;

    QString name() const override;

    void setInterfaceType(const QString &type);
    QString interfaceType() const;

    void fromMap(const QVariantMap &setting) override;

    QVariantMap toMap() const override;

protected:
    OvsInterfaceSettingPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(OvsInterfaceSetting)
};

NETWORKMANAGERQT_EXPORT QDebug operator<<(QDebug dbg, const OvsInterfaceSetting &setting);

}

#endif // NETWORKMANAGERQT_OVS_INTERFACE_SETTING_H
