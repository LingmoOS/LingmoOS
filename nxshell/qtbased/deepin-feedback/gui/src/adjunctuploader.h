/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef ADJUNCTUPLOADER_H
#define ADJUNCTUPLOADER_H

#include <QObject>
#include <QMap>
#include <QtQml>
#include <QQmlEngine>
#include <QJSEngine>
#include <QMimeDatabase>
#include <QMimeType>
#include <QDebug>
#include "adjunctuploadthread.h"
#include "adjunctaide.h"

class AdjunctUploader : public QObject
{
    Q_OBJECT
public:
    static AdjunctUploader * getInstance();
    ~AdjunctUploader();

    Q_INVOKABLE void uploadAdjunct(const QString &filePath);
    Q_INVOKABLE void cancelUpload(const QString &filePath);
    Q_INVOKABLE void cancelAllUpload();
    Q_INVOKABLE bool isInUploadedList(const QString &filePath);
    Q_INVOKABLE QString getBucketUrl(const QString &filePath);
    Q_INVOKABLE QString getFileNameByPath(const QString &filePath);
    Q_INVOKABLE QString getMimeType(const QString &filePath);
    static QObject * uploaderObj(QQmlEngine *engine, QJSEngine *scriptEngine);

signals:
    void uploadProgress(QString filePath, int progress);//progress: 0 ~ 100
    void uploadFailed(QString filePath, QString message);
    void uploadFinish(QString filePath, QString bucketUrl);

private slots:
    void slotUploadFinish(QString filePath, QString resourceUrl);
    void slotUploadFailed(const QString &filePath);

private:
    explicit AdjunctUploader(QObject *parent = 0);

private:
    static AdjunctUploader * adjunctUploader;
    QMap<QString, AdjunctUploadThread *> threadMap;
};

#endif // ADJUNCTUPLOADER_H
