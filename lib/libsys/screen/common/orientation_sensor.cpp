/********************************************************************
Copyright © 2019 Roman Gilg <subdiff@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/
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
