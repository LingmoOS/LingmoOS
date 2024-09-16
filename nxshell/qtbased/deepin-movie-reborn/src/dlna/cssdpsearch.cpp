// Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cssdpsearch.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>

const char* urlAddrPro = "urlAddr";
const char* replayShowNum = "ShowNum";
const char* controlURLPro = "controlURL";
const char* friendlyNamePro = "friendlyName";

CSSDPSearch::CSSDPSearch(QObject *parent) : QObject(parent)
{
    m_networkManager = new QNetworkAccessManager(this);
    m_HostAddr = QHostAddress("239.255.255.250");
    m_udpSocket = new QUdpSocket (this);
    m_udpSocket->bind(QHostAddress::Any, 56123, QUdpSocket::ShareAddress);
    connect(m_udpSocket, SIGNAL(readyRead()), this, SLOT(readMsg()));
    connect(this, SIGNAL(updataAvAddr(QByteArray)), this, SLOT(showDlnaCastAddr(QByteArray)));
}

CSSDPSearch::~CSSDPSearch()
{
    if(m_networkManager) {
        m_networkManager->deleteLater();
        m_networkManager = NULL;
    }
    if(m_udpSocket) {
        m_udpSocket->deleteLater();
        m_udpSocket = NULL;
    }
}
/**
 * @brief readMsg 读取设备的单播消息
 */
void CSSDPSearch::readMsg()
{
    while(m_udpSocket->hasPendingDatagrams()) {
        QByteArray reply;
        reply.resize(m_udpSocket->pendingDatagramSize());
        m_udpSocket->readDatagram(reply.data(),reply.size());
        emit updataAvAddr(reply);
    }

}
/**
 * @brief SsdpSearch 广播请求发现dlna设备
 */
void CSSDPSearch::SsdpSearch()
{
    m_lstStrLocationUrlAddr.clear();
    m_nFindReplyCount = 0;
    //UNPN 广播发现投屏设备请求消息
    QByteArray msg("M-SEARCH * HTTP /1.1\r\n" \
                   "Host:239.255.255.250:1900\r\n" \
                   "ST: ssdp:all\r\n" \
                   "Man:\"ssdp:discover\"\r\n" \
                   "MX:3\r\n" \
                   "\r\n");
    qint64 ret = m_udpSocket->writeDatagram(msg.data(), m_HostAddr, 1900);
    if(ret == -1) {
        qInfo() << "writeDatagram failed";
    }
}
/**
 * @brief showDlnaCastAddr 识别投屏设备消息
 * @param replyData 应答数据
 */
void CSSDPSearch::showDlnaCastAddr(QByteArray replyData)
{
    if(replyData.contains("AVTransport")) {
        QList<QByteArray>  sList = replyData.split('\n');
        foreach(QByteArray data, sList) {
            if(data.contains("LOCATION")) {
                qInfo()<<"replyData: " << data;
                QList<QByteArray>  tmpList = data.split(' ');
                if(tmpList.size() >= 2)
                {
                    QString url = tmpList.at(1).trimmed();
                    m_lstStrLocationUrlAddr.append(url);
                    QString urlAddr = "http://" + QUrl(url).host() + ":"+ QString::number(QUrl(url).port());
                    QNetworkRequest request;
                    request.setUrl(QUrl(url));
                    QNetworkReply *reply = m_networkManager->get(request);
                    reply->setProperty(urlAddrPro, urlAddr);
                    reply->setProperty(replayShowNum, m_nFindReplyCount++);
                    connect(reply, SIGNAL(readChannelFinished()), parent(), SLOT(slotReadyRead()));
                    QEventLoop loop;
                    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
                    loop.exec();
                }
            }
        }
    }
}

