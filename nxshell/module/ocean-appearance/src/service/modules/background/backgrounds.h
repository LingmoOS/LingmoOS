// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BACKGROUNDPRIVATE_H
#define BACKGROUNDPRIVATE_H
#include <QDir>
#include <QList>
#include <QDebug>
#include <QDateTime>
#include <QMutex>
#include <QVector>

#include "../subthemes/scanner.h"
#include "format.h"
#include "background.h"

class Backgrounds: public QObject
{
    Q_OBJECT

public:
    Backgrounds(QObject *parent = nullptr);
    ~Backgrounds();

    bool deleteBackground(const QString& uri);
    QStringList listDirs();
    void refreshBackground();
    void sortByTime(QFileInfoList listFileInfo);
    QStringList getCustomBgFilesInDir(QString dir);
    QStringList getBgFilesInDir(QString dir);
    bool isFileInDirs(QString file, QStringList dirs);
    bool isBackgroundFile(QString file);
    QVector<Background> listBackground();
    void notifyChanged();
    QString prepare(QString file);
    QString getCustomWallpapersConfigDir() { return customWallpapersConfigDir; }

private:
    void init();
    QStringList getSysBgFIles();
    QStringList getCustomBgFiles();
    QString resizeImage(QString fileName, QString cacheDir);
    QString onPrepare(QString fileName);

private:
    QVector<Background> backgrounds;
    static QStringList systemWallpapersDir;
    static QStringList uiSupportedFormats;
    QSharedPointer<QMutex> backgroundsMu;
    QString customWallpapersConfigDir;
    bool fsChanged;
};

#endif // BACKGROUNDPRIVATE_H
