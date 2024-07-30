/*
    SPDX-FileCopyrightText: 2018 Pranav Gade <pranavgade20@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_USER_SETTING_H
#define NETWORKMANAGERQT_USER_SETTING_H

#include "setting.h"
#include <networkmanagerqt/networkmanagerqt_export.h>

#include <QString>

namespace NetworkManager
{
class UserSettingPrivate;

/**
 * Represents user setting
 */
class NETWORKMANAGERQT_EXPORT UserSetting : public Setting
{
public:
    typedef QSharedPointer<UserSetting> Ptr;
    typedef QList<Ptr> List;

    UserSetting();
    explicit UserSetting(const Ptr &other);
    ~UserSetting() override;

    QString name() const override;

    void addData(const QString &data, const QString &value);
    void setData(const NMStringMap &data);
    NMStringMap data() const;

    void fromMap(const QVariantMap &setting) override;

    QVariantMap toMap() const override;

protected:
    UserSettingPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(UserSetting)
};

NETWORKMANAGERQT_EXPORT QDebug operator<<(QDebug dbg, const UserSetting &setting);

}

#endif // NETWORKMANAGERQT_USER_SETTING_H
