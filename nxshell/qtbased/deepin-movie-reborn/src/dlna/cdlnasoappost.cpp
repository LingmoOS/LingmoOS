// Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cdlnasoappost.h"
#include <QString>
#include <QHostAddress>
#include <QAbstractSocket>
#include <QNetworkInterface>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFileInfo>
#include "getdlnaxmlvalue.h"
#include <QEventLoop>
#include <QTimer>
static QString dlnaPlay(
        "<?xml version='1.0' encoding='utf-8'?>\r\n"
        "<s:Envelope s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\">\r\n"
        "   <s:Body>\r\n"
        "       <u:Play xmlns:u=\"urn:schemas-upnp-org:service:AVTransport:1\">\r\n"
        "           <InstanceID>0</InstanceID>\r\n"
        "           <Speed>1</Speed>\r\n"
        "       </u:Play>\r\n"
        "   </s:Body>"
        "</s:Envelope>"
    );
static QString dlnaStop(
        "<?xml version='1.0' encoding='utf-8'?>\r\n"
        "<s:Envelope s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\">\r\n"
        "   <s:Body>\r\n"
        "       <u:Stop xmlns:u=\"urn:schemas-upnp-org:service:AVTransport:1\">\r\n"
        "           <InstanceID>0</InstanceID>\r\n"
        "       </u:Stop>\r\n"
        "   </s:Body>"
        "</s:Envelope>"
    );

static QString dlnaPause(
        "<?xml version='1.0' encoding='utf-8'?>\r\n"
        "<s:Envelope s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\">\r\n"
        "   <s:Body>\r\n"
        "       <u:Pause xmlns:u=\"urn:schemas-upnp-org:service:AVTransport:1\">\r\n"
        "           <InstanceID>0</InstanceID>\r\n"
        "       </u:Pause>\r\n"
        "   </s:Body>"
        "</s:Envelope>"
    );
static QString dlnaSeek(
        "<?xml version='1.0' encoding='utf-8'?>\r\n"
        "<s:Envelope s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\">\r\n"
        "   <s:Body>\r\n"
        "       <u:Seek xmlns:u=\"urn:schemas-upnp-org:service:AVTransport:1\">\r\n"
        "           <InstanceID>0</InstanceID>\r\n"
        "           <Unit>ABS_TIME</Unit>\r\n"
        "           <Target>%1</Target>\r\n"
        "       </u:Seek>\r\n"
        "   </s:Body>"
        "</s:Envelope>"
    );
static QString dlnaGetPositionInfo(
        "<?xml version='1.0' encoding='utf-8'?>\r\n"
        "<s:Envelope s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\">\r\n"
        "   <s:Body>\r\n"
        "       <u:GetPositionInfo xmlns:u=\"urn:schemas-upnp-org:service:AVTransport:1\">\r\n"
        "           <InstanceID>0</InstanceID>\r\n"
        "       </u:GetPositionInfo>\r\n"
        "   </s:Body>"
        "</s:Envelope>"
    );
//TRACK_NR ABS_COUNT
static QString dlnaSetAVTransportURI(
        "<?xml version='1.0' encoding='utf-8'?>\r\n"
        "<s:Envelope s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\">\r\n"
        "   <s:Body>\r\n"
        "       <u:SetAVTransportURI xmlns:u=\"urn:schemas-upnp-org:service:AVTransport:1\">\r\n"
        "           <InstanceID>0</InstanceID>\r\n"
        "           <CurrentURI>%1</CurrentURI>\r\n"
        "           <CurrentURIMetaData></CurrentURIMetaData>\r\n"
        "       </u:SetAVTransportURI>\r\n"
        "   </s:Body>"
        "</s:Envelope>"
    );
static QString dlnaSetNextAVTransportURI(
        "<?xml version='1.0' encoding='utf-8'?>\r\n"
        "<s:Envelope s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\">\r\n"
        "   <s:Body>\r\n"
        "       <u:SetNextAVTransportURI xmlns:u=\"urn:schemas-upnp-org:service:AVTransport:1\">\r\n"
        "           <InstanceID>0</InstanceID>\r\n"
        "           <NextURI>%1</NextURI>\r\n"
        "           <NextURIMetaData></NextURIMetaData>\r\n"
        "       </u:SetNextAVTransportURI>\r\n"
        "   </s:Body>"
        "</s:Envelope>"
    );
CDlnaSoapPost::CDlnaSoapPost(QObject *parent) : QObject(parent)
{
    m_pNetWorkManager = new QNetworkAccessManager(this);
}

CDlnaSoapPost::~CDlnaSoapPost()
{
    if(m_pNetWorkManager) {
        m_pNetWorkManager->deleteLater();
        m_pNetWorkManager = nullptr;
    }
}
/**
 * @brief getTimeStr 时间转换
 * @param pos 当前播放位置
 */
QString CDlnaSoapPost::getTimeStr(qint64 pos)
{
    QTime time(0, 0, 0);
    QString strTime = time.addSecs(static_cast<int>(pos)).toString("hh:mm:ss");
    return strTime;
}
/**
 * @brief SoapOperPost 操作投屏
 * @param oper 操作投屏命令
 * @param ControlURLPro 投屏控制地址
 * @param sHostUrl Http请求地址
 * @param sLocalUrl Http视频地址
 * @param nSeek seek值
 */
void CDlnaSoapPost::SoapOperPost(DlnaOper oper,
                             QString ControlURLPro, QString sHostUrl, QString sLocalUrl, int nSeek)
{
    QByteArray reqData;
    QString sOperName;
    qDebug() <<"sLocalUrl: " << sLocalUrl;
    if(oper == DLNA_SetAVTransportURI) {
        sOperName = "SetAVTransportURI";
        reqData = dlnaSetAVTransportURI.arg(sLocalUrl).toUtf8();
    }
    else if(oper == DLNA_Stop) {
        sOperName = "Stop";
        reqData = dlnaStop.toUtf8();
    }
    else if(oper == DLNA_Pause) {
        sOperName = "Pause";
        reqData = dlnaPause.toUtf8();
    }
    else if(oper == DLNA_Play) {
        sOperName = "Play";
        reqData = dlnaPlay.toUtf8();
    }
    else if(oper == DLNA_Seek) {
        sOperName = "Seek";
        reqData = dlnaSeek.arg(getTimeStr(nSeek)).toUtf8();
    }
    else if(oper == DLNA_GetPositionInfo) {
        sOperName = "GetPositionInfo";
        reqData = dlnaGetPositionInfo.toUtf8();
    }

    QNetworkRequest request;
     request.setUrl(QUrl(ControlURLPro));
     request.setRawHeader("Accept-Encoding", "identity");
     QString sHost = sHostUrl.split("//").last();
     request.setRawHeader("Host", sHost.toUtf8());
     request.setRawHeader("Content-Type", "text/xml; charset=\"utf-8\"");
     request.setRawHeader("Content-Length", QString::number( reqData.length()).toUtf8());
     request.setRawHeader("Soapaction", QString("\"urn:schemas-upnp-org:service:AVTransport:1#%1\"").arg(sOperName).toUtf8());
     request.setRawHeader("Connection", "close");
     QNetworkReply *reply = m_pNetWorkManager->post(request, reqData);
     QEventLoop loop;
     connect(reply, &QNetworkReply::finished, [=]() {
         QByteArray data = reply->readAll();
         qDebug() <<"reply:" << data;
         if(data.contains("SetAVTransportURIResponse")) {
            SoapOperPost(DLNA_Play, ControlURLPro, sHostUrl, sLocalUrl);
         }
         if(data.contains("GetPositionInfoResponse")) {
            GetDlnaXmlValue xmldata(data);
            DlnaPositionInfo posInfo;
            posInfo.nTrack  = xmldata.getValueByPath("s:Body/u:GetPositionInfoResponse/Track").toInt();
            posInfo.sTrackDuration  = xmldata.getValueByPath("s:Body/u:GetPositionInfoResponse/TrackDuration");
            posInfo.sTrackMetaData  = xmldata.getValueByPath("s:Body/u:GetPositionInfoResponse/TrackMetaData");
            posInfo.sTrackURI  = xmldata.getValueByPath("s:Body/u:GetPositionInfoResponse/TrackURI");
            posInfo.sRelTime  = xmldata.getValueByPath("s:Body/u:GetPositionInfoResponse/RelTime");
            posInfo.sAbsTime  = xmldata.getValueByPath("s:Body/u:GetPositionInfoResponse/AbsTime");
            posInfo.nRelCount  = xmldata.getValueByPath("s:Body/u:GetPositionInfoResponse/RelCount").toLongLong();
            posInfo.nAbsCount  = xmldata.getValueByPath("s:Body/u:GetPositionInfoResponse/AbsCount").toLongLong();
            emit sigGetPostionInfo(posInfo);
         }
     });
     connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
     QTimer::singleShot(1500, &loop, &QEventLoop::quit);
     loop.exec();
}
