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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 */
#ifndef LINGMOSEARCHGUI_H
#define LINGMOSEARCHGUI_H

#include <QObject>

#include "qtsingleapplication.h"
#include "mainwindow.h"
#include "lingmo-search-dbus-service.h"
#undef Bool
namespace LingmoUISearch {
class LingmoUISearchGui : public QtSingleApplication
{
    Q_OBJECT
public:
    LingmoUISearchGui(int &argc, char *argv[] , const QString &display, const QString &sessionType, const QString &applicationName = "lingmo-search-gui");
    ~LingmoUISearchGui();

protected Q_SLOTS:
    void parseCmd(QString msg, bool isPrimary);

private:
    MainWindow *m_mainWindow = nullptr;
    LingmoUISearchDbusServices *m_dbusService = nullptr;
};
}

#endif // LINGMOSEARCHGUI_H
