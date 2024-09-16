/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef ADJUNCTAIDE_H
#define ADJUNCTAIDE_H

#include <QObject>
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include "dataconverter.h"

class AdjunctAide : public QObject
{
    Q_OBJECT
public:
    explicit AdjunctAide(QObject *parent = 0);
    ~AdjunctAide();

    void getScreenShot(const QString &target);
    void insertToUploadedList(const QString &filePath, const QString &bucketUrl);
    void deleteFromUploadedList(const QString &filePath);
    QString getBucketUrl(const QString &filePath);
    bool isInUploadedList(const QString &filePath);

    static bool removeDirWidthContent(const QString &dirName);
    static void removeSysAdjuncts(const QString &dirName);

signals:
    void getScreenshotFinish(QString fileName);

private slots:
    void finishGetScreenShot();

private:

    QJsonObject getJsonObjFromFile();

private:
    QProcess * screenShotProcess;
    QString tmpFileName = "";
    const QString DRAFT_SAVE_PATH = QDir::homePath() + "/.cache/deepin-feedback/draft/";
    const QString UPLOAD_RECORD_FILE = DRAFT_SAVE_PATH + "uploadrecord.json";
};

#endif // ADJUNCTAIDE_H
