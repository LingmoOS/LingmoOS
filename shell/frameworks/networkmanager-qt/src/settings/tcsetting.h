/*
    SPDX-FileCopyrightText: 2018 Pranav Gade <pranavgade20@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_TC_SETTING_H
#define NETWORKMANAGERQT_TC_SETTING_H

#include "setting.h"
#include <networkmanagerqt/networkmanagerqt_export.h>

namespace NetworkManager
{
class TcSettingPrivate;

/**
 * Represents Tc setting
 */
class NETWORKMANAGERQT_EXPORT TcSetting : public Setting
{
public:
    typedef QSharedPointer<TcSetting> Ptr;
    typedef QList<Ptr> List;

    TcSetting();
    explicit TcSetting(const Ptr &other);
    ~TcSetting() override;

    QString name() const override;

    void setQdiscs(const NMVariantMapList &qdiscs);
    NMVariantMapList qdiscs() const;

    void setTfilters(const NMVariantMapList &tfilters);
    NMVariantMapList tfilters() const;

    void fromMap(const QVariantMap &setting) override;

    QVariantMap toMap() const override;

protected:
    TcSettingPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(TcSetting)
};

NETWORKMANAGERQT_EXPORT QDebug operator<<(QDebug dbg, const TcSetting &setting);

}

#endif // NETWORKMANAGERQT_TC_SETTING_H
