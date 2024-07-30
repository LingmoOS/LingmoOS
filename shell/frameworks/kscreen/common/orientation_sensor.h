/*
    SPDX-FileCopyrightText: 2019 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
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
