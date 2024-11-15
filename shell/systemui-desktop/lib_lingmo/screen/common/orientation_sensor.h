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
#pragma once

#include <QObject>
#include <QOrientationReading>

class OrientationSensor final : public QObject
{
    Q_OBJECT
public:
    explicit OrientationSensor(QObject *parent = nullptr);
    ~OrientationSensor() override final;

    QOrientationReading::Orientation value() const;
    bool available() const;
    bool enabled() const;

    void setEnabled(bool enable);

Q_SIGNALS:
    void valueChanged(QOrientationReading::Orientation orientation);
    void availableChanged(bool available);
    void enabledChanged(bool enabled);

private:
    void refresh();
    void updateState();

    QOrientationSensor *m_sensor;
    QOrientationReading::Orientation m_value = QOrientationReading::Undefined;
    bool m_enabled = false;
};
