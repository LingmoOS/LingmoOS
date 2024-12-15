// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @copyright 2021-2021 Uniontech Technology Co., Ltd.
 *
 * @file extensionservice.h
 *
 * @brief Websocket服务
 *
 * @date 2021-06-29 16:00
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


#ifndef EXTENSIONSERVICE_H
#define EXTENSIONSERVICE_H

#include <QObject>
class QWebSocketServer;
class QTimer;

/**
 * @class extensionService
 * @brief Websocket服务
 */
class extensionService : public QObject
{
    Q_OBJECT
public:
    extensionService();
    ~extensionService();

private:
    /**
     * @brief 初始化websocket
     */
    void initWebsokcet();

    /**
     * @brief 向下载器发送下载链接
     * @param url 下载器链接
     */
    void sendUrlToDownloader(const QString& url);

private slots:
    /**
     * @brief 检查websocket连接，连接全部关闭时，进程退出
     */
    void checkConnection();

private:
    QWebSocketServer* m_server;
    QTimer *m_timer;
};

#endif // EXTENSIONSERVICE_H
