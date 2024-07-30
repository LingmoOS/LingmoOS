/*
    SPDX-FileCopyrightText: 2018 Pranav Gade <pranavgade20@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_TEAM_PORT_SETTING_H
#define NETWORKMANAGERQT_TEAM_PORT_SETTING_H

#include "setting.h"
#include <networkmanagerqt/networkmanagerqt_export.h>

#include <QString>

namespace NetworkManager
{
class TeamPortSettingPrivate;

/**
 * Represents TeamPort setting
 */
class NETWORKMANAGERQT_EXPORT TeamPortSetting : public Setting
{
public:
    typedef QSharedPointer<TeamPortSetting> Ptr;
    typedef QList<Ptr> List;

    TeamPortSetting();
    explicit TeamPortSetting(const Ptr &other);
    ~TeamPortSetting() override;

    QString name() const override;

    void config(const QString &config);
    QString config() const;

    void lacpKey(qint32 key);
    qint32 lacpKey() const;

    void lacpPrio(qint32 priority);
    qint32 lacpPrio() const;

    void prio(qint32 prio);
    qint32 prio() const;

    void queueId(qint32 id);
    qint32 queueId() const;

    void sticky(bool sticky);
    bool sticky() const;

    void setLinkWatchers(const NMVariantMapList &linkWatchers);
    NMVariantMapList linkWatchers() const;

    void fromMap(const QVariantMap &setting) override;

    QVariantMap toMap() const override;

protected:
    TeamPortSettingPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(TeamPortSetting)
};

NETWORKMANAGERQT_EXPORT QDebug operator<<(QDebug dbg, const TeamPortSetting &setting);

}

#endif // NETWORKMANAGERQT_TEAM_PORT_SETTING_H
