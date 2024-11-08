/*
 *
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Shengjie Ji <jishengjie@kylinos.cn>
 *
 */

#include <QCoreApplication>
#include <QDebug>
#include <QObject>

#include "gsettingmonitor.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "系统字号 : " << kdk::GsettingMonitor::getSystemFontSize();
    qDebug() << "系统透明度 : " << kdk::GsettingMonitor::getSystemTransparency();
    qDebug() << "系统主题" << kdk::GsettingMonitor::getSystemTheme();

    QObject::connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemFontSizeChange, [=]() {
        qDebug() << "系统字号改变 : " << kdk::GsettingMonitor::getSystemFontSize();
    });

    QObject::connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemThemeChange, [=]() {
        qDebug() << "系统主题改变 : " << kdk::GsettingMonitor::getSystemTheme();
    });

    QObject::connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemTransparencyChange, [=]() {
        qDebug() << "系统透明度改变 : " << kdk::GsettingMonitor::getSystemTransparency();
    });

    return app.exec();
}
