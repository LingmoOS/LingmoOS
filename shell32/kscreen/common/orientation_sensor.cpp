/*
    SPDX-FileCopyrightText: 2019 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "orientation_sensor.h"

#include <QOrientationSensor>

OrientationSensor::OrientationSensor(QObject *parent)
    : QObject(parent)
    , m_sensor(new QOrientationSensor(this))
{
    connect(m_sensor, &QOrientationSensor::activeChanged, this, &OrientationSensor::refresh);
}

OrientationSensor::~OrientationSensor() = default;

void OrientationSensor::updateState()
{
    const auto orientation = m_sensor->reading()->orientation();
    if (m_value != orientation) {
        m_value = orientation;
        Q_EMIT valueChanged(orientation);
    }
}

void OrientationSensor::refresh()
{
    if (m_sensor->isActive()) {
        if (m_enabled) {
            updateState();
        }
        Q_EMIT availableChanged(true);
    } else {
        Q_EMIT availableChanged(false);
    }
}

QOrientationReading::Orientation OrientationSensor::value() const
{
    return m_value;
}

bool OrientationSensor::available() const
{
    return m_sensor->connectToBackend();
}

bool OrientationSensor::enabled() const
{
    return m_sensor->isActive();
}

void OrientationSensor::setEnabled(bool enable)
{
    if (m_enabled == enable) {
        return;
    }
    m_enabled = enable;

    if (enable) {
        connect(m_sensor, &QOrientationSensor::readingChanged, this, &OrientationSensor::updateState);
        m_sensor->start();
    } else {
        disconnect(m_sensor, &QOrientationSensor::readingChanged, this, &OrientationSensor::updateState);
        m_value = QOrientationReading::Undefined;
    }
    Q_EMIT enabledChanged(enable);
}
