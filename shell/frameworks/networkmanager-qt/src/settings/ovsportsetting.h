/*
    SPDX-FileCopyrightText: 2018 Pranav Gade <pranavgade20@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQTOVS_PORT_SETTING_H
#define NETWORKMANAGERQTOVS_PORT_SETTING_H

#include "setting.h"
#include <networkmanagerqt/networkmanagerqt_export.h>

namespace NetworkManager
{
class OvsPortSettingPrivate;

/**
 * Represents OvsPort setting
 */
class NETWORKMANAGERQT_EXPORT OvsPortSetting : public Setting
{
public:
    typedef QSharedPointer<OvsPortSetting> Ptr;
    typedef QList<Ptr> List;

    OvsPortSetting();
    explicit OvsPortSetting(const Ptr &other);
    ~OvsPortSetting() override;

    QString name() const override;

    void setBondDowndelay(quint32 delay);
    quint32 bondDowndelay() const;

    void setBondUpdelay(quint32 delay);
    quint32 bondUpdelay() const;

    void setTag(quint32 tag);
    quint32 tag() const;

    void setBondMode(const QString &mode);
    QString bondMode() const;

    void setLacp(const QString &lacp);
    QString lacp() const;

    void setVlanMode(const QString &mode);
    QString vlanMode() const;

    void fromMap(const QVariantMap &setting) override;

    QVariantMap toMap() const override;

protected:
    OvsPortSettingPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(OvsPortSetting)
};

NETWORKMANAGERQT_EXPORT QDebug operator<<(QDebug dbg, const OvsPortSetting &setting);

}

#endif // NETWORKMANAGERQT_OVS_PORT_SETTING_H
