/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef DATASENDER_H
#define DATASENDER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QDBusInterface>
#include <QDBusReply>
#include <QVariantMap>

class DataSender : public QObject
{
    Q_OBJECT
public:
    explicit DataSender(QObject *parent = 0);
    Q_INVOKABLE void postFeedbackData(const QString &jsonData);
    Q_INVOKABLE void showSuccessNotification(const QString &title, const QString &msg);
    Q_INVOKABLE quint32 showErrorNotification(const QString &title, const QString &msg, const QString &action);
    Q_INVOKABLE void closeNotification(quint32 id);

signals:
    void postError(QString message);
    void postFinish();
    void retryPost();

private slots:
    void slotGotError(QNetworkReply::NetworkError error);
    void slotPostFinish(QNetworkReply * reply);
    void slotRetry(uint code,QString id);

private:
    quint32 showNotification(const QString &title, const QString &msg, const QStringList &actions);

private:
    QDBusInterface * notifyInterface;
    const QString PUT_FEEDBACK_METHOD = "Deepin.Feedback.putFeedback";
    const QString JSONRPC_HOST = "https://bugzilla.deepin.io/jsonrpc.psgi";
};

#endif // DATASENDER_H
