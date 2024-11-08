/*
 * Copyright (C) 2024 LingmoOS Team.
 *
 * Author:     lingmoos <lingmoos@foxmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef BRIGHTNESS_H
#define BRIGHTNESS_H

#include <QObject>
#include <QDBusInterface>

class Brightness : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int value READ value NOTIFY valueChanged)
    Q_PROPERTY(bool enabled READ enabled CONSTANT)

public:
    explicit Brightness(QObject *parent = nullptr);

    Q_INVOKABLE void setValue(int value);

    int value() const;
    bool enabled() const;

signals:
    void valueChanged();

private:
    QDBusConnection m_dbusConnection;
    QDBusInterface m_iface;
    int m_value;
    bool m_enabled;
};

#endif // BRIGHTNESS_H
