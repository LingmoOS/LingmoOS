/*
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "SensorProperty.h"

#ifdef Q_OS_FREEBSD
#include <sys/sysctl.h>
#include <sys/types.h>
#endif

namespace KSysGuard
{
/**
 * Convenience subclass of SensorProperty that reads a value of type T from FreeBSD's sysctl interface
 */
template<typename T>
class SysctlSensor : public SensorProperty
{
public:
    /**
     * Contstrucor.
     * @param sysctlName The name of the sysctl entry
     */
    SysctlSensor(const QString &id, const QByteArray &sysctlName, SensorObject *parent);
    SysctlSensor(const QString &id, const QByteArray &sysctlName, const QVariant &initialValue, SensorObject *parent);
    SysctlSensor(const QString &id, const QString &name, const QByteArray &sysctlName, SensorObject *parent);
    SysctlSensor(const QString &id, const QString &name, const QByteArray &sysctlName, const QVariant &initialValue, SensorObject *parent);

    /**
     * Update this sensor.
     *
     * This will cause the sensor to call syscctl and update the value from that.
     * It should be called periodically so values are updated properly.
     */
    void update() override;
    /**
     * Set the function used to convert the data from sysctl to the value of this sensor.
     *
     * This accepts a function that takes a reference to an object of type T and converts that to a
     * QVariant. Use this if you need to convert the value into another unit or do a calculation to
     * arrive at the sensor value. By default the value is stored as is in a QVariant.
     */
    void setConversionFunction(const std::function<QVariant(const T &)> &function)
    {
        m_conversionFunction = function;
    }

private:
    const QByteArray m_sysctlName;
    std::function<QVariant(const T &)> m_conversionFunction = [](const T &t) {
        return QVariant::fromValue(t);
    };
};

#ifdef Q_OS_FREEBSD

template<typename T>
SysctlSensor<T>::SysctlSensor(const QString &id, const QByteArray &sysctlName, SensorObject *parent)
    : SysctlSensor(id, id, sysctlName, QVariant{}, parent)
{
}

template<typename T>
SysctlSensor<T>::SysctlSensor(const QString &id, const QByteArray &sysctlName, const QVariant &initialValue, SensorObject *parent)
    : SysctlSensor(id, id, sysctlName, initialValue, parent)
{
}

template<typename T>
SysctlSensor<T>::SysctlSensor(const QString &id, const QString &name, const QByteArray &sysctlName, SensorObject *parent)
    : SysctlSensor(id, name, sysctlName, QVariant{}, parent)
{
}

template<typename T>
SysctlSensor<T>::SysctlSensor(const QString &id, const QString &name, const QByteArray &sysctlName, const QVariant &initialValue, SensorObject *parent)
    : SensorProperty(id, name, initialValue, parent)
    , m_sysctlName(sysctlName)
{
}

template<typename T>
void SysctlSensor<T>::update()
{
    if (!isSubscribed()) {
        return;
    }
    T value{};
    size_t size = sizeof(T);
    if (sysctlbyname(m_sysctlName.constData(), &value, &size, nullptr, 0) != -1) {
        setValue(m_conversionFunction(value));
    }
}
#endif

} // namespace KSysGuard
