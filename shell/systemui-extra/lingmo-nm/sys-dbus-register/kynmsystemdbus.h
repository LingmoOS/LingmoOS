/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 *  This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef KYNMSYSTEMDBUS_H
#define KYNMSYSTEMDBUS_H

#include <QObject>

class KynmSystemDbus : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.lingmo.network.interface")

public:
    explicit KynmSystemDbus(QObject *parent = nullptr);
    ~KynmSystemDbus();

public Q_SLOTS:
    Q_SCRIPTABLE bool checkIpv4IsConflict(const QString devName, const QString ipv4Address, QStringList macList);
    Q_SCRIPTABLE bool checkIpv6IsConflict(const QString devName, const QString ipv6Address);
};

#endif // KYNMSYSTEMDBUS_H
