/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2024 KylinSoft Co., Ltd.
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

#include <QDebug>
#include <QDBusError>
#include <QDBusConnection>
#include <QCoreApplication>

#include "test.h"

/*
 * 测试情况：
 * getEyeCare(): 通过
 * getNightMode():通过
 * getPrimaryBrightness():通过
 * getScreenMode():通过
 * getScreensParam()::通过
 * increaseBrightness():通过
 * openDisplaySetting()::通过
 * reduceBrightness()::通过
 * setAllScreenSameBrightness():通过
 * setColorTemperature():未通过
 * setEyeCare():通过
 * setNightMode():通过
 * setPrimaryBrightness()：通过，建议异步调用
 * setScreenBrightness()：通过，
 * setScreenMode()：通过，
 * setScreensParam()：通过，
 *
*/
int main(int argc, char *argv[])
{
    QCoreApplication app(argc,argv);
    Screen *m_screen = new Screen();
    QDBusConnection sessionBug = QDBusConnection::sessionBus();
    if (sessionBug.registerService("org.lingmo.SettingsDaemon1")) {
        sessionBug.registerObject("/screen", m_screen,
                                 QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals
                                 );
    }
    app.exec();
    return 0;
}
