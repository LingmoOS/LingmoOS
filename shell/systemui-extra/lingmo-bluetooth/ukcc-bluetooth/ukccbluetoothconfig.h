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

#ifndef UKCCBLUETOOTHCONFIG_H
#define UKCCBLUETOOTHCONFIG_H

#include <QFile>
#include <QDebug>
#include <QPixmap>
#include <QProcess>
#include <QGSettings>
#include <lingmo-log4qt.h>


#ifdef UKCC_BLUETOOTH_DATA_BURIAL_POINT
    #include <ukcc/interface/ukcccommon.h>
#endif

#include "lingmosdk/lingmosdk-system/libkysysinfo.h"

#include "config.h"

class ukccbluetoothconfig
{
public:
    ukccbluetoothconfig();
    ~ukccbluetoothconfig();
    enum PixmapColor {
            WHITE = 0,
            BLACK,
            GRAY,
            BLUE,
            DEFAULT
    };
    static QGSettings *ukccGsetting;

    static bool ukccBtBuriedSettings(QString pluginName, QString settingsName, QString action, QString value = nullptr);
    static void launchBluetoothServiceStart(const QString &processName);
    static void killAppProcess(const quint64 &processId);
    static bool checkProcessRunning(const QString &processName, QList<quint64> &listProcessId);
    static void setEnvPCValue();
    static const QPixmap loadSvg(const QPixmap &source, const PixmapColor &cgColor);
    static const QImage loadSvgImage(const QPixmap &source, const PixmapColor &cgColor);

};

#endif // UKCCBLUETOOTHCONFIG_H
