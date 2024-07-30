/*
    SPDX-FileCopyrightText: 2018 Pranav Gade <pranavgade20@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_OVS_PATCH_SETTING_H
#define NETWORKMANAGERQT_OVS_PATCH_SETTING_H

#include "setting.h"
#include <networkmanagerqt/networkmanagerqt_export.h>

#include <QString>

namespace NetworkManager
{
class OvsPatchSettingPrivate;

/**
 * Represents OvsPatch setting
 */
class NETWORKMANAGERQT_EXPORT OvsPatchSetting : public Setting
{
public:
    typedef QSharedPointer<OvsPatchSetting> Ptr;
    typedef QList<Ptr> List;

    OvsPatchSetting();
    explicit OvsPatchSetting(const Ptr &other);
    ~OvsPatchSetting() override;

    QString name() const override;

    void setPeer(const QString &peer);
    QString peer() const;

    void fromMap(const QVariantMap &setting) override;

    QVariantMap toMap() const override;

protected:
    OvsPatchSettingPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(OvsPatchSetting)
};

NETWORKMANAGERQT_EXPORT QDebug operator<<(QDebug dbg, const OvsPatchSetting &setting);

}

#endif // NETWORKMANAGERQT_OVS_PATCH_SETTING_H
