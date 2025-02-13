/*
 * Copyright (C) 2024 LingmoOS Team.
 *
 * Author:     Lingmo OS Team <team@lingmo.org>
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

#ifndef VERSION_H
#define VERSION_H

#include <QObject>
#include <QString>
#include <QSysInfo>
#include <qqml.h>

class SysVersion : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString version READ version CONSTANT)

public:
    explicit SysVersion(QObject *parent = nullptr);

    QString version();

private:
    qlonglong calculateTotalRam() const;
};

#endif // VERSION_H