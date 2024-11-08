/*
 * Qt5-LINGMO
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "mainwindow.h"

#include <QApplication>

#include <QVector4D>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w3;

    w3.setWindowFlag(Qt::FramelessWindowHint);
    w3.setProperty("useCustomShadow", true);
    w3.setProperty("customShadowDarkness", 1.0);
    w3.setProperty("customShadowWidth", 20);
    w3.setProperty("customShadowRadius", QVector4D(1, 1, 1, 1));
    w3.setProperty("customShadowMargins", QVector4D(20, 20, 20, 20));

    w3.show();
    return a.exec();
}
