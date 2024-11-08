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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#ifndef LINGMO_PANEL_START_MENU_BUTTON_H
#define LINGMO_PANEL_START_MENU_BUTTON_H

#include <QObject>

class TaskViewButton : public QObject
{
    Q_OBJECT
public:
    explicit TaskViewButton(QObject *parent=nullptr);

    Q_INVOKABLE void showTaskView();
};

#endif //LINGMO_PANEL_START_MENU_BUTTON_H
