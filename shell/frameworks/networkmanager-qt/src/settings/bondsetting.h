/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_BOND_SETTING_H
#define NETWORKMANAGERQT_BOND_SETTING_H

#include "setting.h"
#include <networkmanagerqt/networkmanagerqt_export.h>

#include <QString>

namespace NetworkManager
{
class BondSettingPrivate;

/**
 * Represents bond setting
 */
class NETWORKMANAGERQT_EXPORT BondSetting : public Setting
{
public:
    typedef QSharedPointer<BondSetting> Ptr;
    typedef QList<Ptr> List;
    BondSetting();
    explicit BondSetting(const Ptr &other);
    ~BondSetting() override;

    QString name() const override;

    /// @deprecated in favor of connection.interface-name in NM 1.0.0
    void setInterfaceName(const QString &name);
    QString interfaceName() const;

    void addOption(const QString &option, const QString &value);
    void setOptions(const NMStringMap &options);
    NMStringMap options() const;

    void fromMap(const QVariantMap &setting) override;

    QVariantMap toMap() const override;

protected:
    BondSettingPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(BondSetting)
};

NETWORKMANAGERQT_EXPORT QDebug operator<<(QDebug dbg, const BondSetting &setting);

}

#endif // NETWORKMANAGERQT_BOND_SETTING_H
