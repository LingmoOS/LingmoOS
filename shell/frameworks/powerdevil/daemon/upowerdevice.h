/*
    SPDX-FileCopyrightText: 2023 Stefan Brüns <stefan.bruens@rwth-aachen.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef POWERDEVIL_UPOWERDEVICE_H
#define POWERDEVIL_UPOWERDEVICE_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>

class UPowerDevice : public QObject
{
    Q_OBJECT

public:
    explicit UPowerDevice(const QString &dbusObjectPath);
    UPowerDevice() = delete;
    ~UPowerDevice() = default;

    enum class State {
        Unknown = 0,
        Charging = 1,
        Discharging = 2,
        FullyCharged = 4,
    };
    State state() const
    {
        return m_state;
    }

    enum class Type {
        Unknown = 0,
        Battery = 2,
        Ups = 3,
    };
    Type type() const
    {
        return m_type;
    }
    bool isPowerSupply() const
    {
        return m_isPowerSupply;
    }
    bool isPresent() const
    {
        return m_isPresent;
    }

    double energy() const
    {
        return m_energy;
    }
    double energyFull() const
    {
        return m_energyFull;
    }

    /**
     * Amount of energy being drained from the source, measured
     * in W. If positive, the source is being discharged, if
     * negative it's being charged.
     * @return positive value if discharging, negative value if charging, or 0 if unknown
     * @see org.freedesktop.UPower.Device.xml
     */
    double energyRate() const
    {
        return m_energyRate;
    }
    qulonglong updateTime() const
    {
        return m_updateTime;
    }

private:
    State m_state = State::Unknown;
    Type m_type = Type::Unknown;
    bool m_isPowerSupply = false;
    bool m_isPresent = false;

    double m_energy = 0.0;
    double m_energyFull = 0.0;
    double m_energyRate = 0.0;
    qulonglong m_updateTime = 0;

private Q_SLOTS:
    void onPropertiesChanged(const QString &ifaceName, const QVariantMap &changedProps, const QStringList &invalidatedProps);
    bool updateProperties(const QVariantMap &properties);

Q_SIGNALS:
    void propertiesChanged();
};

#endif
