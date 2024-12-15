// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @copyright 2020-2020 Uniontech Technology Co., Ltd.
 *
 * @file urlthread.h
 *
 * @brief 解析任务线程类
 *
 * @date 2020-08-28 14:42
 *
 * Author: bulongwei  <bulongwei@uniontech.com>
 *
 * Maintainer: bulongwei  <bulongwei@uniontech.com>
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
 */
#ifndef URLTHREAD_H
#define URLTHREAD_H

#include "analysisurl.h"
#include <QObject>
//class LinkInfo;
class QThread;
class QSettings;
class QNetworkReply;

static QSettings *m_iniFile = nullptr;
class UrlThread : public QObject
{
    Q_OBJECT

public:
    explicit UrlThread(QObject *parent = nullptr);
    ~UrlThread(){}

    /**
     * @brief start 获取任务
     * @param urlInfo urlInfo 任务信息
     */
    void getLinkInfo(LinkInfo);
    void start(LinkInfo &urlInfo);

    void stop();

private slots:
    /**
     * @brief begin 开始解析url
     */
    void begin();

    /**
     * @brief onHttpRequest 获取解析后的数据
     */
    void onHttpRequest(QNetworkReply *reply);

private:
    /**
     * @brief getUrlType 获取解析后的数据
     * @param url 地址
     * @return url 类型
     */
    QString getUrlType(QString url);
    /**
     * @brief getUrlSize 获取解析后的数据
     * @param url 地址
     * @return url 大小
     */
    QString getUrlSize(QString url);

    QString getType(QString contentType);

    QString getNoContentType();

signals:
    /**
     * @brief sendFinishedUrl 发送解析后的数据
     * @param link 数据
     */
    void sendFinishedUrl(LinkInfo link);

    void sendTrueUrl(LinkInfo link);

private:
    LinkInfo m_linkInfo; //当前url信息
};

#endif // URLTHREAD_H
