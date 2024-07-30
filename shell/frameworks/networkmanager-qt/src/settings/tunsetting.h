/*
    SPDX-FileCopyrightText: 2016 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_TUN_SETTING_H
#define NETWORKMANAGERQT_TUN_SETTING_H

#include "setting.h"
#include <networkmanagerqt/networkmanagerqt_export.h>

#include <QString>

namespace NetworkManager
{
class TunSettingPrivate;

/**
 * Represents generic setting
 */
class NETWORKMANAGERQT_EXPORT TunSetting : public Setting
{
public:
    typedef QSharedPointer<TunSetting> Ptr;
    typedef QList<Ptr> List;
    enum Mode {
        Tun,
        Tap,
    };

    TunSetting();
    explicit TunSetting(const Ptr &other);
    ~TunSetting() override;

    QString name() const override;

    void setGroup(const QString &group);
    QString group() const;

    void setMode(Mode mode);
    Mode mode() const;

    void setMultiQueue(bool multiQueue);
    bool multiQueue() const;

    void setOwner(const QString &owner);
    QString owner() const;

    void setPi(bool pi);
    bool pi() const;

    void setVnetHdr(bool vnetHdr);
    bool vnetHdr() const;

    void fromMap(const QVariantMap &setting) override;

    QVariantMap toMap() const override;

protected:
    TunSettingPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(TunSetting)
};

NETWORKMANAGERQT_EXPORT QDebug operator<<(QDebug dbg, const TunSetting &setting);

}

#endif // NETWORKMANAGERQT_TUN_SETTING_H
