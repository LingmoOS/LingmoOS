/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "adjunctuploadthread.h"

#define UPLOAD_TIMEOUT 60*1000

AdjunctUploadThread::AdjunctUploadThread(const QString &filePath) :
    gFilePath(filePath)
{
    uploadTimeoutTimer.setSingleShot(true);
    uploadTimeoutTimer.setInterval(UPLOAD_TIMEOUT);
    connect(&uploadTimeoutTimer, &QTimer::timeout, this, &AdjunctUploadThread::uploadTimeout);
}

void AdjunctUploadThread::startUpload()
{
    QString url = BUCKET_API + REST_TYPE;
    QNetworkRequest request;
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");
    request.setUrl(QUrl(url));

    QUrlQuery postData;
    postData.addQueryItem("file-type", getSuffixe(gFilePath));

    QNetworkAccessManager * tmpManager = new QNetworkAccessManager();
    tmpUploadReply = tmpManager->post(request, postData.toString(QUrl::FullyEncoded).toUtf8());

    uploadTimeoutTimer.start();

    connect(tmpManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(getServerAccessResult(QNetworkReply*)));
    connect(tmpManager, SIGNAL(finished(QNetworkReply*)), tmpManager, SLOT(deleteLater()));
    connect(tmpManager, SIGNAL(finished(QNetworkReply*)), &uploadTimeoutTimer, SLOT(stop()));
    connect(tmpUploadReply.data(), SIGNAL(uploadProgress(qint64,qint64)), &uploadTimeoutTimer, SLOT(start()));
}

void AdjunctUploadThread::stopUpload()
{
    if (this->isRunning())
    {
        qDebug() << "Stop upload file:" << gFilePath;
        this->quit();
        this->wait();
        this->deleteLater();
    }
}

void AdjunctUploadThread::run()
{
    gUploadFile = new QFile(gFilePath);
    if (!gUploadFile->open(QIODevice::ReadOnly))
    {
        qWarning() << "Open upload-file faileed:" << gUploadFile->errorString();
        emit uploadFailed(gFilePath, "Open upload-file faileed");
        this->terminate();
        this->wait();
    }
    else
    {
        qDebug() << "Start upload thread...";

        QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

        QString signature = gResponeData.postBody.take("signature").toString();
        QString policy = gResponeData.postBody.take("policy").toString();

        QHttpPart signaturePart;
        signaturePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"signature\""));
        signaturePart.setBody(signature.toUtf8());

        QHttpPart policyPart;
        policyPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"policy\""));
        policyPart.setBody(policy.toUtf8());

        QHttpPart filePart;
//        filePart.setHeader(QNetworkRequest::ContentTypeHeader,"multipart/form-data");
        QString fileFlag = "form-data; name=\"file\";filename=\"" + gFilePath + "\"";
        filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(fileFlag));
        filePart.setBodyDevice(gUploadFile);

        multiPart->append(signaturePart);
        multiPart->append(policyPart);
        multiPart->append(filePart);

        QNetworkRequest request;
        request.setUrl(QUrl(gResponeData.postUrl));

        QNetworkAccessManager * tmpManager = new QNetworkAccessManager();

        gUploadReply = tmpManager->post(request, multiPart);
        connect(tmpManager, SIGNAL(finished(QNetworkReply*)), tmpManager, SLOT(deleteLater()));
        connect(gUploadReply, SIGNAL(finished()), gUploadReply, SLOT(deleteLater()));
        connect(gUploadReply, SIGNAL(finished()), gUploadFile, SLOT(deleteLater()));
        connect(gUploadReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotGotError(QNetworkReply::NetworkError)), Qt::DirectConnection);
        connect(tmpManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(slotUploadFinish(QNetworkReply*)), Qt::DirectConnection);
        connect(gUploadReply, SIGNAL(uploadProgress(qint64,qint64)), this, SLOT(slotUploadProgress(qint64,qint64)), Qt::DirectConnection);
        connect(tmpManager, SIGNAL(finished(QNetworkReply*)), &uploadTimeoutTimer, SLOT(stop()));
        connect(gUploadReply.data(), SIGNAL(uploadProgress(qint64,qint64)), &uploadTimeoutTimer, SLOT(start()));

        exec();
    }
}

void AdjunctUploadThread::getServerAccessResult(QNetworkReply * reply)
{
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QByteArray tmpArray = reply->readAll();

    if (statusCode != 200)
    {
        qWarning() << "Attachment Server Create Resource Error: " << tmpArray;
        emit uploadFailed(gFilePath,"Requrest error!");
        return;
    }
    else
    {
        qDebug() << "Attachment Server Create Resource Success!";
    }

    parseJsonData(tmpArray, &gResponeData);
    gResourceUrl = BUCKET_HOST +gResponeData.resourceUrl;
    reply->deleteLater();
    this->start();
}

void AdjunctUploadThread::parseJsonData(const QByteArray &jsonData, ResponeData *resData)
{
    QJsonParseError jsonError;
    QJsonDocument parseDoucment = QJsonDocument::fromJson(jsonData, &jsonError);
    if(jsonError.error == QJsonParseError::NoError)
    {
        if(parseDoucment.isObject())
        {
            QJsonObject obj = parseDoucment.object();
            if(obj.contains("ID"))
            {
                QJsonValue idValue = obj.take("ID");
                if(idValue.isString())
                    resData->id = idValue.toVariant().toString();
            }
            if(obj.contains("ResourceUrl"))
            {
                QJsonValue urlValue = obj.take("ResourceUrl");
                if(urlValue.isString())
                    resData->resourceUrl = urlValue.toString();
            }
            if(obj.contains("PostUrl"))
            {
                QJsonValue urlValue = obj.take("PostUrl");
                if(urlValue.isString())
                    resData->postUrl = urlValue.toString();
            }
            if(obj.contains("PostHeader"))
            {
                QJsonValue headerValue = obj.take("PostHeader");
                if (headerValue.isObject())
                    resData->postHeader = headerValue.toObject();
            }
            if (obj.contains("PostBody"))
            {
                QJsonValue bodyValue = obj.take("PostBody");
                if (bodyValue.isObject())
                    resData->postBody = bodyValue.toObject();
            }
        }
    }
}

void AdjunctUploadThread::slotUploadFinish(QNetworkReply * reply)
{
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    qDebug() << "Upload finish:" << statusCode << gResourceUrl;

    if ((statusCode == 302 || statusCode == 301) && gResourceUrl != BUCKET_HOST)
        emit uploadFinish(gFilePath, gResourceUrl);
    else
    {
        emit uploadProgress(gFilePath, 0);
        emit uploadFailed(gFilePath, "");
    }

    gUploadFile->close();
}

void AdjunctUploadThread::slotUploadProgress(qint64 value1, qint64 value2)
{
    if (value2 != 0)
    {
        emit uploadProgress(gFilePath, value1 * 100 / value2);
    }
}

void AdjunctUploadThread::slotGotError(QNetworkReply::NetworkError code)
{
    qWarning() << "Upload failed:" << gFilePath;
    emit uploadFailed(gFilePath, QString::number(code));
}

QString AdjunctUploadThread::getSuffixe(const QString &filePath)
{
    QMimeDatabase db;
    QMimeType mime = db.mimeTypeForFile(filePath);
    QStringList suffixesList = mime.suffixes();
    if (suffixesList.count() > 0)
        return suffixesList.at(0);
    else
        return "";
}

void AdjunctUploadThread::uploadTimeout()
{
    if(tmpUploadReply&&!tmpUploadReply->isFinished()){
        tmpUploadReply->abort();
    }

    if(gUploadReply&&!gUploadReply->isFinished()){
        gUploadReply->abort();
    }

    qWarning()<<"Upload timeout";
}

AdjunctUploadThread::~AdjunctUploadThread()
{

}
