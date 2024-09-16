/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "adjunctuploader.h"

AdjunctUploader::AdjunctUploader(QObject *parent) : QObject(parent)
{    
    //import时使用AdjunctUploader 1.0
    qmlRegisterSingletonType<AdjunctUploader>("AdjunctUploader", 1, 0, "AdjunctUploader", uploaderObj);
}

AdjunctUploader * AdjunctUploader::adjunctUploader = NULL;
AdjunctUploader * AdjunctUploader::getInstance()
{
    if (adjunctUploader == NULL)
        adjunctUploader = new AdjunctUploader();
    return adjunctUploader;
}

void AdjunctUploader::uploadAdjunct(const QString &filePath)
{
    AdjunctUploadThread * tmpThread = new AdjunctUploadThread(filePath);
    connect(tmpThread, SIGNAL(uploadProgress(QString,int)), this , SIGNAL(uploadProgress(QString,int)));
    connect(tmpThread, SIGNAL(uploadFinish(QString,QString)), this ,SLOT(slotUploadFinish(QString,QString)));
    connect(tmpThread, SIGNAL(uploadFailed(QString,QString)), this, SIGNAL(uploadFailed(QString,QString)));
    connect(tmpThread, SIGNAL(uploadFailed(QString,QString)), this, SLOT(slotUploadFailed(QString)));
    threadMap.insert(filePath,tmpThread);
    tmpThread->startUpload();
}

void AdjunctUploader::cancelUpload(const QString &filePath)
{
    if (threadMap.keys().indexOf(filePath) != -1)
    {
        threadMap.take(filePath)->stopUpload();
    }
    AdjunctAide tmpAide;
    tmpAide.deleteFromUploadedList(filePath);
}

void AdjunctUploader::cancelAllUpload()
{
    QStringList tmpKeys = threadMap.keys();
    for (int i = 0; i < tmpKeys.length(); i ++)
    {
        threadMap.take(tmpKeys.at(i))->stopUpload();
    }
}

bool AdjunctUploader::isInUploadedList(const QString &filePath)
{
    AdjunctAide tmpAide;
    return tmpAide.isInUploadedList(filePath);
}

QString AdjunctUploader::getBucketUrl(const QString &filePath)
{
    AdjunctAide tmpAide;
    return tmpAide.getBucketUrl(filePath);
}

QString AdjunctUploader::getFileNameByPath(const QString &filePath)
{
    return filePath.mid(filePath.lastIndexOf("/") + 1);
}

QString AdjunctUploader::getMimeType(const QString &filePath)
{
    QMimeDatabase db;
    QMimeType mime = db.mimeTypeForFile(filePath);
    return mime.name();
}

void AdjunctUploader::slotUploadFinish(QString filePath, QString resourceUrl)
{
    threadMap.take(filePath);

    AdjunctAide tmpAide;
    tmpAide.insertToUploadedList(filePath,resourceUrl);
    emit uploadFinish(filePath, resourceUrl);
}

void AdjunctUploader::slotUploadFailed(const QString &filePath)
{
    cancelUpload(filePath);
}

QObject *AdjunctUploader::uploaderObj(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)

    return AdjunctUploader::getInstance();
}

AdjunctUploader::~AdjunctUploader()
{

}

