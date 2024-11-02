/*
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
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
 *
 */
#ifndef SEPARATIONLINE_H
#define SEPARATIONLINE_H

#include <QObject>
#include <QFrame>

class SeparationLine : public QFrame
{
    Q_OBJECT
public:
    SeparationLine(QWidget *parent = nullptr);
    ~SeparationLine() = default;
public Q_SLOTS:
    void setLineStyle();
};

#endif // SEPARATIONLINE_H
