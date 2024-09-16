// Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CDLNASOAPPOST_H
#define CDLNASOAPPOST_H
#include <QObject>
// DLNA 投屏操作
typedef enum {
    DLNA_SetAVTransportURI = 0,
    DLNA_Play,
    DLNA_Pause,
    DLNA_Seek,
    DLNA_Stop,
    DLNA_GetPositionInfo
} DlnaOper;
// DLNA 投屏状态进度信息
typedef struct {
    int nTrack;
    QString sTrackDuration;
    QString sTrackMetaData;
    QString sTrackURI;
    QString sRelTime;
    QString sAbsTime;
    qint64 nRelCount;
    qint64 nAbsCount;
} DlnaPositionInfo;
class QNetworkAccessManager;
class CDlnaSoapPost: public QObject
{
    Q_OBJECT
public:
    explicit CDlnaSoapPost(QObject *parent = nullptr);
    ~CDlnaSoapPost();
    /**
     * @brief SoapOperPost 操作投屏
     * @param oper 操作投屏命令
     * @param ControlURLPro 投屏控制地址
     * @param sHostUrl Http请求地址
     * @param sLocalUrl Http视频地址
     * @param nSeek seek值
     */
    void SoapOperPost(DlnaOper oper,
                  QString ControlURLPro, QString sHostUrl, QString sLocalUrl, int nSeek = 0);
private:
    /**
     * @brief getTimeStr 时间转换
     * @param pos 当前播放位置
     */
    QString getTimeStr(qint64 pos);
private:
    QNetworkAccessManager *m_pNetWorkManager; //网络传输管理
signals:
    void sigGetPostionInfo(DlnaPositionInfo);
};

#endif // CDLNASOAPPOST_H
