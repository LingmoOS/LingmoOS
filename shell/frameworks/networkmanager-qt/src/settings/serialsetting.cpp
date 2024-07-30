/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "serialsetting.h"
#include "serialsetting_p.h"

#include <QDebug>

NetworkManager::SerialSettingPrivate::SerialSettingPrivate()
    : name(NM_SETTING_SERIAL_SETTING_NAME)
    , baud(57600)
    , bits(8)
    , parity(SerialSetting::NoParity)
    , stopbits(1)
    , sendDelay(0)
{
}

NetworkManager::SerialSetting::SerialSetting()
    : Setting(Setting::Serial)
    , d_ptr(new SerialSettingPrivate())
{
}

NetworkManager::SerialSetting::SerialSetting(const Ptr &other)
    : Setting(other)
    , d_ptr(new SerialSettingPrivate())
{
    setBaud(other->baud());
    setBits(other->bits());
    setParity(other->parity());
    setStopbits(other->stopbits());
    setSendDelay(other->sendDelay());
}

NetworkManager::SerialSetting::~SerialSetting()
{
    delete d_ptr;
}

QString NetworkManager::SerialSetting::name() const
{
    Q_D(const SerialSetting);

    return d->name;
}

void NetworkManager::SerialSetting::setBaud(quint32 speed)
{
    Q_D(SerialSetting);

    d->baud = speed;
}

quint32 NetworkManager::SerialSetting::baud() const
{
    Q_D(const SerialSetting);

    return d->baud;
}

void NetworkManager::SerialSetting::setBits(quint32 byteWidgh)
{
    Q_D(SerialSetting);

    d->bits = byteWidgh;
}

quint32 NetworkManager::SerialSetting::bits() const
{
    Q_D(const SerialSetting);

    return d->bits;
}

void NetworkManager::SerialSetting::setParity(NetworkManager::SerialSetting::Parity paritysetting)
{
    Q_D(SerialSetting);

    d->parity = paritysetting;
}

NetworkManager::SerialSetting::Parity NetworkManager::SerialSetting::parity() const
{
    Q_D(const SerialSetting);

    return d->parity;
}

void NetworkManager::SerialSetting::setStopbits(quint32 number)
{
    Q_D(SerialSetting);

    d->stopbits = number;
}

quint32 NetworkManager::SerialSetting::stopbits() const
{
    Q_D(const SerialSetting);

    return d->stopbits;
}

void NetworkManager::SerialSetting::setSendDelay(quint64 delay)
{
    Q_D(SerialSetting);

    d->sendDelay = delay;
}

quint64 NetworkManager::SerialSetting::sendDelay() const
{
    Q_D(const SerialSetting);

    return d->sendDelay;
}

void NetworkManager::SerialSetting::fromMap(const QVariantMap &setting)
{
    if (setting.contains(QLatin1String(NM_SETTING_SERIAL_BAUD))) {
        setBaud(setting.value(QLatin1String(NM_SETTING_SERIAL_BAUD)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_SERIAL_BITS))) {
        setBits(setting.value(QLatin1String(NM_SETTING_SERIAL_BITS)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_SERIAL_PARITY))) {
        QChar character = setting.value(QLatin1String(NM_SETTING_SERIAL_PARITY)).toChar();

        if (character == 'n') {
            setParity(NoParity);
        } else if (character == 'E') {
            setParity(EvenParity);
        } else if (character == 'o') {
            setParity(OddParity);
        }
    }

    if (setting.contains(QLatin1String(NM_SETTING_SERIAL_STOPBITS))) {
        setStopbits(setting.value(QLatin1String(NM_SETTING_SERIAL_STOPBITS)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_SERIAL_SEND_DELAY))) {
        setSendDelay((Setting::SecretFlagType)setting.value(QLatin1String(NM_SETTING_SERIAL_SEND_DELAY)).toULongLong());
    }
}

QVariantMap NetworkManager::SerialSetting::toMap() const
{
    QVariantMap setting;

    if (baud() != 57600) {
        setting.insert(QLatin1String(NM_SETTING_SERIAL_BAUD), baud());
    }

    if (bits() != 8) {
        setting.insert(QLatin1String(NM_SETTING_SERIAL_BITS), bits());
    }

    if (parity() != NoParity) {
        if (parity() == EvenParity) {
            setting.insert(QLatin1String(NM_SETTING_SERIAL_PARITY), QChar('E'));
        } else if (parity() == OddParity) {
            setting.insert(QLatin1String(NM_SETTING_SERIAL_PARITY), QChar('o'));
        }
    }

    if (stopbits() != 1) {
        setting.insert(QLatin1String(NM_SETTING_SERIAL_STOPBITS), stopbits());
    }

    if (sendDelay()) {
        setting.insert(QLatin1String(NM_SETTING_SERIAL_SEND_DELAY), sendDelay());
    }

    return setting;
}

QDebug NetworkManager::operator<<(QDebug dbg, const NetworkManager::SerialSetting &setting)
{
    dbg.nospace() << "type: " << setting.typeAsString(setting.type()) << '\n';
    dbg.nospace() << "initialized: " << !setting.isNull() << '\n';

    dbg.nospace() << NM_SETTING_SERIAL_BAUD << ": " << setting.baud() << '\n';
    dbg.nospace() << NM_SETTING_SERIAL_BITS << ": " << setting.bits() << '\n';
    dbg.nospace() << NM_SETTING_SERIAL_PARITY << ": " << setting.parity() << '\n';
    dbg.nospace() << NM_SETTING_SERIAL_STOPBITS << ": " << setting.stopbits() << '\n';
    dbg.nospace() << NM_SETTING_SERIAL_SEND_DELAY << ": " << setting.sendDelay() << '\n';

    return dbg.maybeSpace();
}
