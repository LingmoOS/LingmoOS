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

#ifndef LINGMO_PANEL_START_MENU_BUTTON_H
#define LINGMO_PANEL_START_MENU_BUTTON_H

#include <QObject>

class QAction;

class StartMenuButton : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QList<QAction *> userActions MEMBER m_userActions CONSTANT)
    Q_PROPERTY(QList<QAction *> powerActions MEMBER m_powerActions CONSTANT)
public:
    explicit StartMenuButton(QObject *parent=nullptr);

    Q_INVOKABLE void openStartMenu();
    Q_INVOKABLE void showDesktop();

public Q_SLOTS:
    void execSessionAction(const QString &action);

    // TODO: Reload Config
    // void reloadActions();

private:
    void loadUserAction();
    void loadPowerAction();

private:
    QList<QAction *> m_userActions;
    QList<QAction *> m_powerActions;
};

#endif //LINGMO_PANEL_START_MENU_BUTTON_H
