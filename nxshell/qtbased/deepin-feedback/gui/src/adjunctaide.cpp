/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "adjunctaide.h"

AdjunctAide::AdjunctAide(QObject *parent) : QObject(parent)
{

}

AdjunctAide::~AdjunctAide()
{

}

void AdjunctAide::getScreenShot(const QString &target)
{
    tmpFileName = "/tmp/" + QDateTime::currentDateTime().toString("yyyyMMddhhmmss") + ".png";
    screenShotProcess = new QProcess(this);
    connect(screenShotProcess, SIGNAL(finished(int)), this, SLOT(finishGetScreenShot()));

    QStringList arguments;

    arguments << "-s" << tmpFileName;
    screenShotProcess->start("deepin-screenshot", arguments);
}

bool AdjunctAide::removeDirWidthContent(const QString &dirName)
{
    QStringList dirNames;
    QDir tmpDir;
    QFileInfoList infoList;
    QFileInfoList::iterator currentFile;

    dirNames.clear();
    if(tmpDir.exists())
        dirNames<<dirName;
    else
        return true;


    for(int i=0;i<dirNames.size();++i)
    {
        tmpDir.setPath(dirNames[i]);
        infoList = tmpDir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot ,QDir::Name);
        if(infoList.size()>0)
        {
            currentFile = infoList.begin();
            while(currentFile != infoList.end())
            {
                //dir, appent to dirNames
                if(currentFile->isDir())
                {
                    dirNames.append(currentFile->filePath());
                }
                else if(currentFile->isFile())
                {
                    AdjunctAide tmpAide;
                    tmpAide.deleteFromUploadedList(currentFile->filePath());
                    if(!tmpDir.remove(currentFile->fileName()))
                    {
                        return false;
                    }
                }
                currentFile++;
            }//end of while
        }
    }
    //delete dir
    for(int i = dirNames.size()-1; i >= 0; --i)
    {
        if(!tmpDir.rmdir(dirNames[i]))
        {
            return false;
        }
    }
    return true;
}

void AdjunctAide::removeSysAdjuncts(const QString &dirName)
{
    QDir tmpDir(dirName);
    QFileInfoList infoList;

    infoList = tmpDir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot ,QDir::Name);
    for (int i = 0; i < infoList.count(); i++)
    {
        if (infoList.at(i).fileName().startsWith("deepin-feedback-results"))
        {
            QString tmpPath = infoList.at(i).filePath();
            QFile::remove(tmpPath);
            AdjunctAide tmpAide;
            tmpAide.deleteFromUploadedList(tmpPath);
        }
    }
}

void AdjunctAide::finishGetScreenShot()
{
    if (screenShotProcess->exitCode() == 0)
        emit getScreenshotFinish(tmpFileName);
    else
        emit getScreenshotFinish("");

    qDebug() << "Get screenshot process finish!" << screenShotProcess->exitCode();
    screenShotProcess->deleteLater();
    tmpFileName = "";
}

void AdjunctAide::insertToUploadedList(const QString &filePath, const QString &bucketUrl)
{
    QDir tmpDir;
    if (!tmpDir.exists(DRAFT_SAVE_PATH))
        tmpDir.mkpath(DRAFT_SAVE_PATH);

    QJsonDocument jsonDocument;
    QJsonObject jsonObj = getJsonObjFromFile();
    jsonObj.insert(filePath,QJsonValue(bucketUrl));

    jsonDocument.setObject(jsonObj);

    QFile uploadRecordFile(UPLOAD_RECORD_FILE);
    if (uploadRecordFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        uploadRecordFile.write(jsonDocument.toJson(QJsonDocument::Compact));
        uploadRecordFile.close();
    }
}

void AdjunctAide::deleteFromUploadedList(const QString &filePath)
{
    if (!QFile::exists(UPLOAD_RECORD_FILE))
        return;

    QJsonDocument parseDoc;
    QJsonObject tmpObj = getJsonObjFromFile();

    int tmpIndex = tmpObj.keys().indexOf(filePath);
    if (tmpIndex != -1)
        tmpObj.remove(filePath);

    parseDoc.setObject(tmpObj);

    QFile uploadRecordFile(UPLOAD_RECORD_FILE);
    if (uploadRecordFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        uploadRecordFile.write(parseDoc.toJson(QJsonDocument::Compact));
        uploadRecordFile.close();
    }
}

QJsonObject AdjunctAide::getJsonObjFromFile()
{
    QJsonObject jsonObj;
    if (!QFile::exists(UPLOAD_RECORD_FILE))
        return jsonObj;

    QFile recordFile(UPLOAD_RECORD_FILE);
    if (!recordFile.open(QIODevice::ReadOnly))
    {
        qWarning() << "Open upload record file to read error: " << UPLOAD_RECORD_FILE;
        return jsonObj;
    }

    QByteArray tmpByteArray = recordFile.readAll();
    recordFile.close();
\
    QJsonDocument parseDoc = QJsonDocument::fromJson(tmpByteArray);
    if (parseDoc.isObject())
        jsonObj = parseDoc.object();

    return jsonObj;
}

QString AdjunctAide::getBucketUrl(const QString &filePath)
{
    return getJsonObjFromFile().value(filePath).toString();
}

bool AdjunctAide::isInUploadedList(const QString &filePath)
{
    if (!QFile::exists(UPLOAD_RECORD_FILE))
        return false;

    QJsonObject tmpObj = getJsonObjFromFile();
    if (tmpObj.isEmpty())
        return false;

    int tmpIndex = tmpObj.keys().indexOf(filePath);
    return tmpIndex != -1;
}
