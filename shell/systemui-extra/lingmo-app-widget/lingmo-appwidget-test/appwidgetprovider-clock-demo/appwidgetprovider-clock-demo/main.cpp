/*
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
 * Authors: wangyan <wangyan@kylinos.cn>
 *
 */

#include "mainwindow.h"
#include <QtSingleApplication>
#include <QApplication>

int main(int argc, char *argv[])
{
    QtSingleApplication a("appwidgetprovider-clock-demo", argc, argv);
    if(a.isRunning())
    {
        a.sendMessage("raise_window_noop");
        return EXIT_SUCCESS;
    }
    MainWindow w;
    a.setActivationWindow(&w);//单例
    w.show();

    return a.exec();

}
