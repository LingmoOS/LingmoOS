// Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DLNACONTENTSERVER_H
#define DLNACONTENTSERVER_H

#include <QObject>
#include <QFile>
#include <dlna/dlnaHttpServer/qhttpserver.h>
#include <dlna/dlnaHttpServer/qhttpresponse.h>
#include <dlna/dlnaHttpServer/qhttprequest.h>
#include <memory>
#include <optional>
#include <QThread>

class QHttpServer;
class QHttpRequest;
class QHttpResponse;
class DlnaContentServer : public QObject
{
    Q_OBJECT
public:
    struct Range {
        qint64 start;
        qint64 end;
        qint64 length;
        inline bool full() const {
            return start == 0 && (end == -1 || end == length - 1);
        }
        inline qint64 rangeLength() const { return end - start + 1; }
        inline bool operator==(const Range &rv) const {
            return start == rv.start && end == rv.end;
        }
        static std::optional<DlnaContentServer::Range> fromRange(const QString &rangeHeader,
                                              qint64 length = -1);
    };
    enum DLNA_ORG_FLAGS {
        DLNA_ORG_FLAG_NONE = 0U,
        DLNA_ORG_FLAG_SENDER_PACED = (1U << 31),
        DLNA_ORG_FLAG_TIME_BASED_SEEK = (1U << 30),
        DLNA_ORG_FLAG_BYTE_BASED_SEEK = (1U << 29),
        DLNA_ORG_FLAG_PLAY_CONTAINER = (1U << 28),
        DLNA_ORG_FLAG_S0_INCREASE = (1U << 27),
        DLNA_ORG_FLAG_SN_INCREASE = (1U << 26),
        DLNA_ORG_FLAG_RTSP_PAUSE = (1U << 25),
        DLNA_ORG_FLAG_STREAMING_TRANSFER_MODE = (1U << 24),
        DLNA_ORG_FLAG_INTERACTIVE_TRANSFERT_MODE = (1U << 23),
        DLNA_ORG_FLAG_BACKGROUND_TRANSFER_MODE = (1U << 22),
        DLNA_ORG_FLAG_CONNECTION_STALL = (1U << 21),
        DLNA_ORG_FLAG_DLNA_V15 = (1U << 20)
    };
public:

    DlnaContentServer(QObject *parent = nullptr, int nPort = 8080);
    ~DlnaContentServer();
    /**
     * @brief slotBaseMuteChanged 请求传输文件数据
     * @param req Http请求
     * @param resp Http应答
     */
    void requestHandler(QHttpRequest *req, QHttpResponse *resp);
    /**
     * @brief streamFile 传输文件流数据
     * @param file Http传输文件
     * @param mime 视频格式
     * @param req Http请求
     * @param resp Http应答
     */
    void streamFile(const QString &path, const QString &mime,
                                         QHttpRequest *req, QHttpResponse *resp);
    /**
     * @brief streamFileRange 断点续传流
     * @param file Http传输文件
     * @param req Http请求
     * @param resp Http应答
     */
    void streamFileRange(std::shared_ptr<QFile> file, QHttpRequest *req,
                         QHttpResponse *resp);
    /**
     * @brief streamFileNoRange 全部流
     * @param file Http传输文件
     * @param req Http请求
     * @param resp Http应答
     */
    void streamFileNoRange(std::shared_ptr<QFile> file, QHttpRequest *req,
                           QHttpResponse *resp);
    /**
     * @brief sendEmptyResponse 发送空应答
     * @param resp Http应答
     * @param code Http应答码
     */
    void sendEmptyResponse(QHttpResponse *resp, int code);
    /**
     * @brief dlnaContentFeaturesHeader 填充dlna传输头
     * @param seek 是否seek传输
     * @param flags 文件或流标志
     */
    QString dlnaContentFeaturesHeader(const QString& mime,
                                      bool seek = true,
                                      bool flags = true);
    /**
     * @brief dlnaOrgPnFlags 视频格式转换为upnp标准
     * @param mime 视频格式
     */
    QString dlnaOrgPnFlags(const QString &mime);
    /**
     * @brief dlnaOrgFlagsForFile dlna文件传输协议
     */
    QString dlnaOrgFlagsForFile();
    /**
     * @brief dlnaOrgFlagsForStreaming dlna流传输协议
     */
    QString dlnaOrgFlagsForStreaming();
    /**
     * @brief setBaseUrl 设置Http视频连接地址
     * @param baseUrl Http视频连接地址
     */
    void setBaseUrl(const QString &baseUrl);
    /**
     * @brief setDlnaFileName 设置传输文件名
     * @param fileName 传输文件名
     */
    void setDlnaFileName(const QString &fileName);
    /**
     * @brief getBaseUrl 获取Http视频连接地址
     */
    QString getBaseUrl() const;
    /**
     * @brief getIsStartHttpServer Http服务是否启动
     */
    bool getIsStartHttpServer();
private:
    QString m_sDlnaFileName; // http传输文件
    QHttpServer *m_httpServer; // http服务
    QString m_sBaseUrl; // http url
    bool m_bStartHttpServer = false; // http 服务是否启动
    QThread *m_pThread; // http 服务线程
signals:
    void contSeqWriteData(std::shared_ptr<QFile> file, qint64 size,
                          QHttpResponse *resp);
    void closeServer();
public slots:
    /**
     * @brief seqWriteData 请求传输文件数据
     * @param file Http请求文件
     * @param size Http请求文件大小
     * @param resp Http应答
     */
    void seqWriteData(std::shared_ptr<QFile> file, qint64 size,
                      QHttpResponse *resp);
    /**
     * @brief initializeHttpServer 初始化HttpServer
     * @param port Http服务端口
     */
    bool initializeHttpServer(int port);
};

#endif // DLNACONTENTSERVER_H
