/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

#ifndef LINGMO_PANEL_SHOW_DESKTOP_H
#define LINGMO_PANEL_SHOW_DESKTOP_H

#include <QObject>

namespace LingmoUIPanel {

class ShowDesktop : public QObject
{
    Q_OBJECT
public:
    explicit ShowDesktop(QObject *parent = nullptr);

    Q_INVOKABLE void showDesktop();
};

} // LingmoUIPanel

#endif //LINGMO_PANEL_SHOW_DESKTOP_H
