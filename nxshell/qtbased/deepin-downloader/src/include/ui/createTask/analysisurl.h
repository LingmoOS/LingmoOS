// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @copyright 2020-2020 Uniontech Technology Co., Ltd.
 *
 * @file analysisurl.h
 *
 * @brief 管理解析url线程
 *
 * @date 2020-08-28 14:40
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
#ifndef ANALYSISURL_H
#define ANALYSISURL_H

//#include "urlthread.h"
#include <QObject>
#include <QMap>
class UrlThread;
struct LinkInfo {
    enum UrlState {
        Start = 1,
        Analysising,
        Finished
    };
    QString isChecked;
    int index; //url序号
    QString url; //url链接
    UrlState state; //url状态
    QString urlName; //url名字
    QString type; //url类型
    QString urlSize; //url大小
    QString urlTrueLink; //url真实链接
    long length;
    LinkInfo()
    {
        isChecked = "0";
        index = -1;
        length = 0;
        url.clear();
        state = UrlState::Start;
        urlName.clear();
        urlSize.clear();
        urlTrueLink.clear();
    }
    LinkInfo &operator=(const LinkInfo &l)
    {
        isChecked = l.isChecked;
        index = l.index;
        state = l.state;
        url = l.url;
        type = l.type;
        urlSize = l.urlSize;
        urlName = l.urlName;
        urlTrueLink = l.urlTrueLink;
        length = l.length;
        return *this;
    }
};

class AnalysisUrl : public QObject
{
    Q_OBJECT

public:
    explicit AnalysisUrl(QObject *parent = nullptr);
    ~AnalysisUrl();

    /**
     * @brief setUrlList 设置需要解析的url列表
     * @param list url信息
     */
    void setUrlList(QMap<QString, LinkInfo> list);


public slots:
    /**
     * @brief getLinkInfo 获取到解析后的url信息
     * @param list url信息
     */
    void getLinkInfo(LinkInfo);

    void getTrueLinkInfo(LinkInfo);

private:
    /**
     * @brief stopWork 停止当前线程工作
     * @param index 当前线程索引
     */
    void stopWork(int index);

signals:
    /**
     * @brief sendFinishedUrl 将解析后的信息，发送至新建任务窗口
     * @param *linkInfo 解析后信息
     */
    void sendFinishedUrl(LinkInfo *linkInfo);

private:
    QMap<QString, LinkInfo> m_curAllUrl;
    QMap<int, QThread *> m_workThread;
    QMap<int, UrlThread*> m_urlThread;


};

#endif // ANALYSISURL_H
