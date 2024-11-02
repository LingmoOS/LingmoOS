/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/

#ifndef CONFIG_H
#define CONFIG_H

#include <QString>

#define BluetoothServiceExePath "/usr/bin/bluetoothService -o"
#define BluetoothTrayExePath    "/usr/bin/lingmo-bluetooth"
#define BluetoothServiceName    "bluetoothService"
#define BluetoothTrayName       "lingmo-bluetooth"

#define SYSTEMSTYLESCHEMA "org.lingmo.style"
#define SYSTEMSTYLENAME "styleName"
#define SYSTEMFONTSIZE  "systemFontSize"
#define SYSTEMFONT      "systemFont"

#define MAX_DEVICE_CONECTIONS_TIMES 3

#define DELAYED_SCANNING_TIME_S                 (2*1000)
#define DEVICE_CONNECTION_TIMEOUT_S             (30*1000)
#define LOADING_ICON_TIMEOUT_INTERVAL_MS        (110)
#define NO_DEV_ADD_SIGNAL_DETECTION_INTERVAL    (5*1000)

#define TRANSPARENCY_SETTINGS       "org.lingmo.control-center.personalise"
#define TRANSPARENCY_KEY            "transparency"

#define GSETTING_LINGMO_STYLE "org.lingmo.style"
#define GSETTING_SCHEMA_UKCC "org.lingmo.control-center.plugins"
#define GSETTING_PACH_UKCC "/org/lingmo/control-center/plugins/Bluetooth/"

#define DBUSSERVICE     "com.lingmo.bluetooth"
#define DBUSPATH        "/com/lingmo/bluetooth"
#define DBUSINTERFACE   "com.lingmo.bluetooth"

#define TITLE_ICON_BLUETOOTH "bluetooth"


const QString SERVICE   = "com.lingmo.bluetooth";
const QString PATH      = "/com/lingmo/bluetooth";
const QString INTERFACE = "com.lingmo.bluetooth";

enum Environment
{
    NOMAL = 0,
    HUAWEI,
    LAIKA,
    MAVIS
};

extern Environment envPC;
extern bool global_rightToleft;

struct struct_pos
{
    struct_pos() {}
    struct_pos(int _x, int _y, int _width, int _high){
        x = _x;
        y = _y;
        width = _width;
        high = _high;
    }
    int x = 0;
    int y = 0;
    int width = 0;
    int high = 0;
};

static struct_pos getWidgetPos(int ax, int ay, int aw, int ah, int width)
{
    struct_pos t(ax, ay, aw, ah);
    //从右到左布局，需要重新定位x坐标
    if (global_rightToleft) {
        t.x = width - ax - aw;
    }
    return t;
}

#endif // CONFIG_H
