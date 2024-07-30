/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_BLUETOOTH_SETTING_H
#define NETWORKMANAGERQT_BLUETOOTH_SETTING_H

#include "setting.h"
#include <networkmanagerqt/networkmanagerqt_export.h>

#include <QString>

namespace NetworkManager
{
class BluetoothSettingPrivate;

/**
 * Represents bluetooth setting
 */
class NETWORKMANAGERQT_EXPORT BluetoothSetting : public Setting
{
public:
    typedef QSharedPointer<BluetoothSetting> Ptr;
    typedef QList<Ptr> List;
    enum ProfileType {
        Unknown = 0,
        Dun,
        Panu,
    };

    BluetoothSetting();
    explicit BluetoothSetting(const Ptr &other);
    ~BluetoothSetting() override;

    QString name() const override;

    void setBluetoothAddress(const QByteArray &address);
    QByteArray bluetoothAddress() const;

    void setProfileType(ProfileType type);
    ProfileType profileType() const;

    void fromMap(const QVariantMap &setting) override;

    QVariantMap toMap() const override;

protected:
    BluetoothSettingPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(BluetoothSetting)
};

NETWORKMANAGERQT_EXPORT QDebug operator<<(QDebug dbg, const BluetoothSetting &setting);

}

#endif // NETWORKMANAGERQT_SETTINGS_BLUETOOTH_H
