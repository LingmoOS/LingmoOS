/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "datasender.h"

DataSender::DataSender(QObject *parent) :
    QObject(parent)
{
    notifyInterface = new QDBusInterface("org.freedesktop.Notifications", "/org/freedesktop/Notifications",
                                         "org.freedesktop.Notifications",QDBusConnection::sessionBus(),this);
    connect(notifyInterface,SIGNAL(ActionInvoked(uint,QString)),this,SLOT(slotRetry(uint,QString)));
}

void DataSender::postFeedbackData(const QString &jsonData)
{
    qDebug()<< "Sending feedback data...{JSON--" << jsonData << "--JSON}";

    QNetworkRequest request;
    request.setUrl(QUrl(JSONRPC_HOST));
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json-rpc");

    QNetworkAccessManager * tmpManager = new QNetworkAccessManager();

    QNetworkReply * gUploadReply = tmpManager->post(request, jsonData.toUtf8());
    connect(tmpManager, SIGNAL(finished(QNetworkReply*)), tmpManager, SLOT(deleteLater()));
    connect(gUploadReply, SIGNAL(finished()), gUploadReply, SLOT(deleteLater()));
    connect(gUploadReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotGotError(QNetworkReply::NetworkError)), Qt::DirectConnection);
    connect(tmpManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(slotPostFinish(QNetworkReply*)), Qt::DirectConnection);
}

void DataSender::showSuccessNotification(const QString &title, const QString &msg)
{
    showNotification(title,msg,QStringList());
}

quint32 DataSender::showErrorNotification(const QString &title, const QString &msg, const QString &action)
{
    QStringList actions;
    actions << "deepin_feedback_retry";
    actions << action;
    return showNotification(title,msg,actions);
}

void DataSender::closeNotification(quint32 id)
{
    notifyInterface->call(QDBus::AutoDetect, "CloseNotification", 
		    id);
}

void DataSender::slotGotError(QNetworkReply::NetworkError error)
{
    qWarning() << "Send feedback failed! QNetworkReply::NetworkError:" << error;
    emit postError(QString::number(error));
}

void DataSender::slotPostFinish(QNetworkReply *reply)
{
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    QString replyStr(reply->readAll());
    if (statusCode == 200)
    {
        if (replyStr.indexOf("error") != -1)
        {
            emit postError(QString("Error"));
            qWarning() << "Sending feedback data failed: " << replyStr;
        }
        else
        {
            emit postFinish();
            qWarning() << "Sending feedback data success!" << statusCode << replyStr;
        }
    }
    else {
        emit postError("Error Status Code:" + QString::number(statusCode));
        qWarning() << "Sending feedback data failed,Reply status code: " << statusCode;
    }

}

void DataSender::slotRetry(uint code, QString id)
{
    if (id == "deepin_feedback_retry")
    {
        emit retryPost();
    }
}

quint32 DataSender::showNotification(const QString &title, const QString &msg, const QStringList &actions)
{
    QDBusMessage ret = notifyInterface->call(QDBus::AutoDetect,"Notify",
                      "Deepin Feedback",
                      uint(0),
                      "deepin-feedback",
                      title,
                      msg,
                      actions,
                      QVariantMap(),
                      -1);
    return ret.arguments().at(0).toUInt();
}
