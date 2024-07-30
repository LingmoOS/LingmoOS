/*
    SPDX-FileCopyrightText: 2014 Lukáš Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_TEAM_SETTING_H
#define NETWORKMANAGERQT_TEAM_SETTING_H

#include "setting.h"
#include <networkmanagerqt/networkmanagerqt_export.h>

#include <QString>

namespace NetworkManager
{
class TeamSettingPrivate;

/**
 * Represents team setting
 */
class NETWORKMANAGERQT_EXPORT TeamSetting : public Setting
{
public:
    typedef QSharedPointer<TeamSetting> Ptr;
    typedef QList<Ptr> List;
    TeamSetting();
    explicit TeamSetting(const Ptr &other);
    ~TeamSetting() override;

    QString name() const override;

    /// @deprecated in favor of connection.interface-name in NM 1.0.0
    void setInterfaceName(const QString &name);
    QString interfaceName() const;

    QString config() const;
    void setConfig(const QString &config);

    void fromMap(const QVariantMap &setting) override;

    QVariantMap toMap() const override;

protected:
    TeamSettingPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(TeamSetting)
};

NETWORKMANAGERQT_EXPORT QDebug operator<<(QDebug dbg, const TeamSetting &setting);

}

#endif // NETWORKMANAGERQT_TEAM_SETTING_H
