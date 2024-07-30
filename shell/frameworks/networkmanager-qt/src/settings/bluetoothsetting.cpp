/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "bluetoothsetting.h"
#include "bluetoothsetting_p.h"

#include <QDebug>

NetworkManager::BluetoothSettingPrivate::BluetoothSettingPrivate()
    : name(NM_SETTING_BLUETOOTH_SETTING_NAME)
    , profileType(BluetoothSetting::Unknown)
{
}

NetworkManager::BluetoothSetting::BluetoothSetting()
    : Setting(Setting::Bluetooth)
    , d_ptr(new BluetoothSettingPrivate())
{
}

NetworkManager::BluetoothSetting::BluetoothSetting(const Ptr &other)
    : Setting(other)
    , d_ptr(new BluetoothSettingPrivate())
{
    setBluetoothAddress(other->bluetoothAddress());
    setProfileType(other->profileType());
}

NetworkManager::BluetoothSetting::~BluetoothSetting()
{
    delete d_ptr;
}

QString NetworkManager::BluetoothSetting::name() const
{
    Q_D(const BluetoothSetting);

    return d->name;
}

void NetworkManager::BluetoothSetting::setBluetoothAddress(const QByteArray &address)
{
    Q_D(BluetoothSetting);

    d->bdaddr = address;
}

QByteArray NetworkManager::BluetoothSetting::bluetoothAddress() const
{
    Q_D(const BluetoothSetting);

    return d->bdaddr;
}

void NetworkManager::BluetoothSetting::setProfileType(NetworkManager::BluetoothSetting::ProfileType type)
{
    Q_D(BluetoothSetting);

    d->profileType = type;
}

NetworkManager::BluetoothSetting::ProfileType NetworkManager::BluetoothSetting::profileType() const
{
    Q_D(const BluetoothSetting);

    return d->profileType;
}

void NetworkManager::BluetoothSetting::fromMap(const QVariantMap &setting)
{
    if (setting.contains(QLatin1String(NM_SETTING_BLUETOOTH_BDADDR))) {
        setBluetoothAddress(setting.value(QLatin1String(NM_SETTING_BLUETOOTH_BDADDR)).toByteArray());
    }

    if (setting.contains(QLatin1String(NM_SETTING_BLUETOOTH_TYPE))) {
        const QString type = setting.value(QLatin1String(NM_SETTING_BLUETOOTH_TYPE)).toString();

        if (type == QLatin1String(NM_SETTING_BLUETOOTH_TYPE_DUN)) {
            setProfileType(Dun);
        } else if (type == QLatin1String(NM_SETTING_BLUETOOTH_TYPE_PANU)) {
            setProfileType(Panu);
        }
    }
}

QVariantMap NetworkManager::BluetoothSetting::toMap() const
{
    QVariantMap setting;

    if (!bluetoothAddress().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_BLUETOOTH_BDADDR), bluetoothAddress());
    }

    switch (profileType()) {
    case Dun:
        setting.insert(QLatin1String(NM_SETTING_BLUETOOTH_TYPE), QLatin1String(NM_SETTING_BLUETOOTH_TYPE_DUN));
        break;
    case Panu:
        setting.insert(QLatin1String(NM_SETTING_BLUETOOTH_TYPE), QLatin1String(NM_SETTING_BLUETOOTH_TYPE_PANU));
        break;
    case Unknown:
        break;
    }

    return setting;
}

QDebug NetworkManager::operator<<(QDebug dbg, const NetworkManager::BluetoothSetting &setting)
{
    dbg.nospace() << "type: " << setting.typeAsString(setting.type()) << '\n';
    dbg.nospace() << "initialized: " << !setting.isNull() << '\n';

    dbg.nospace() << NM_SETTING_BLUETOOTH_BDADDR << ": " << setting.bluetoothAddress() << '\n';
    dbg.nospace() << NM_SETTING_BLUETOOTH_TYPE << ": " << setting.profileType() << '\n';

    return dbg.maybeSpace();
}
