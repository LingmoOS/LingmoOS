/*
    SPDX-FileCopyrightText: 2018 Pranav Gade <pranavgade20@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_MATCH_SETTING_H
#define NETWORKMANAGERQT_MATCH_SETTING_H

#include "setting.h"
#include <networkmanagerqt/networkmanagerqt_export.h>

namespace NetworkManager
{
class MatchSettingPrivate;

/**
 * Represents Match setting
 */
class NETWORKMANAGERQT_EXPORT MatchSetting : public Setting
{
public:
    typedef QSharedPointer<MatchSetting> Ptr;
    typedef QList<Ptr> List;

    MatchSetting();
    explicit MatchSetting(const Ptr &other);
    ~MatchSetting() override;

    QString name() const override;

    void setInterfaceName(const QStringList &name);
    QStringList interfaceName() const;

    void fromMap(const QVariantMap &setting) override;

    QVariantMap toMap() const override;

protected:
    MatchSettingPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(MatchSetting)
};

NETWORKMANAGERQT_EXPORT QDebug operator<<(QDebug dbg, const MatchSetting &setting);

}

#endif // NETWORKMANAGERQT_MATCH_SETTING_H
