// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @copyright 2021-2021 Uniontech Technology Co., Ltd.
 *
 * @file websockethandle.cpp
 *
 * @brief Websocket服务处理类
 *
 * @date 2021-06-29 16:01
 *
 * Author: zhaoyue  <zhaoyue@uniontech.com>
 *
 * Maintainer: zhaoyue  <zhaoyue@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
**/


#include "websockethandle.h"

#include <QSettings>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QDBusConnection>

Websockethandle::Websockethandle(QObject *parent)
    : QObject(parent)
{
    QDBusConnection::sessionBus().unregisterService("com.dlmExtensionService.service");
    QDBusConnection::sessionBus().registerService("com.dlmExtensionService.service");
    QDBusConnection::sessionBus().registerObject("/dlmExtensionService/path", this, QDBusConnection ::ExportAllSlots | QDBusConnection ::ExportAllSignals);
}

Websockethandle::~Websockethandle()
{
}

bool Websockethandle::isControlBrowser()
{
    QString iniConfigPath = QString("%1/%2/%3/config.conf")
                                .arg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation))
                                .arg("uos")
                                .arg("downloader");
    QSettings set(iniConfigPath, QSettings::IniFormat);

    bool b = set.value("Monitoring.MonitoringObject.Browser/value").toBool();
    return b;
}

void Websockethandle::sendTextToClient(bool b)
{
    sendText(QString().number(b));
}

void Websockethandle::receiveText(const QString &text)
{
    if (text.contains("init")) { //初始化
        bool b = isControlBrowser();
        sendText(QString().number(b));
        return;
    }
    sendWebText(text);  //发送链接到下载器
}
