// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @copyright 2021-2021 Uniontech Technology Co., Ltd.
 *
 * @file websockethandle.h
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


#ifndef WEBSOCKETHANDLE_H
#define WEBSOCKETHANDLE_H

#include <QObject>

/**
 * @class Websockethandle
 * @brief Websocket服务处理类
 */
class Websockethandle : public QObject
{
    Q_OBJECT

public:
    Websockethandle(QObject *parent = nullptr);
    ~Websockethandle();

public slots:
    /**
     * @brief 向浏览器扩展发送数据
     * @param b 下载器是否接管
     */
    void sendTextToClient(bool b);
private:

    /**
     * @brief 获取下载器时候接管浏览器
     */
    bool isControlBrowser();

signals:
    void sendText(const QString &text);
    void sendWebText(const QString &text);
public slots:
    /**
     * @brief 接收浏览器扩展发送过来的数据
     */
    void receiveText(const QString &text);
};

#endif // WEBSOCKETHANDLE_H
