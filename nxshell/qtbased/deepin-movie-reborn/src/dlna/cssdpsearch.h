#ifndef CSSDPSEARCH_H

// Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#define CSSDPSEARCH_H
#include <QObject>
#include <QHostAddress>
#include <QUdpSocket>
#include <QNetworkAccessManager>
#include <QByteArray>

extern const char* urlAddrPro;
extern const char* replayShowNum;
extern const char* controlURLPro;
extern const char* friendlyNamePro;


class CSSDPSearch : public QObject
{
    Q_OBJECT
public:
    explicit CSSDPSearch(QObject *parent = nullptr);
    ~CSSDPSearch();
public:
    /**
     * @brief SsdpSearch 广播请求发现dlna设备
     */
    void SsdpSearch(); //广播请求发现dlna设备

signals:
    void updataAvAddr(QByteArray);
public slots:
    /**
     * @brief showDlnaCastAddr 识别投屏设备消息
     * @param replyData 应答数据
     */
    void showDlnaCastAddr(QByteArray replyData);//识别投屏设备消息
    /**
     * @brief readMsg 读取设备的单播消息
     */
    void readMsg(); //读取设备的单播消息

private:
    QHostAddress m_HostAddr; //建立发现服务
    QUdpSocket *m_udpSocket; //发现请求udp sock
    QList<QString> m_lstStrLocationUrlAddr; //发现的投屏设备LOCATION地址
    QNetworkAccessManager *m_networkManager; //网络请求
    int m_nFindReplyCount; //发现应答数
};

#endif // CSSDPSEARCH_H
