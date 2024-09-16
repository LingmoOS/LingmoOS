/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef ADJUNCTUPLOADTHREAD_H
#define ADJUNCTUPLOADTHREAD_H

#include <QObject>
#include <QThread>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QUrlQuery>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QFile>
#include <QMimeDatabase>
#include <QMimeType>
#include <QDebug>
#include <QPointer>
#include <QTimer>

struct ResponeData{
    QString id;
    QString resourceUrl;
    QString postUrl;
    QJsonObject postHeader;
    QJsonObject postBody;
};

class AdjunctUploadThread : public QThread
{
    Q_OBJECT
public:
    explicit AdjunctUploadThread(const QString &filePath);
    ~AdjunctUploadThread();

    void startUpload();
    void stopUpload();
    void run();


signals:
    void uploadProgress(QString filePath, int progress);//progress: 0 ~ 100
    void uploadFailed(QString filePath, QString message);
    void uploadFinish(QString filePath, QString resourceUrl);

public slots:
    void getServerAccessResult(QNetworkReply * reply);
    void parseJsonData(const QByteArray &jsonData, ResponeData * resData);
    void slotUploadFinish(QNetworkReply * reply);
    void slotUploadProgress(qint64 value1, qint64 value2);
    void slotGotError(QNetworkReply::NetworkError code);

private:
    QString getSuffixe(const QString &filePath);

private:
    QNetworkAccessManager * networkAccessManager;
    QPointer<QNetworkReply> gUploadReply;
    QPointer<QNetworkReply> tmpUploadReply;
    ResponeData gResponeData;
    QFile * gUploadFile;
    QString gFilePath;
    QString gResourceUrl;
    QTimer uploadTimeoutTimer;

    const QString REST_TYPE = "report";
    const QString BUCKET_HOST = "https://api.deepin.org/";
    const QString BUCKET_API = "https://api.deepin.org/bucket/";

private slots:
    void uploadTimeout();
};

#endif // ADJUNCTUPLOADTHREAD_H
