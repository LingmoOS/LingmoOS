// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utils.h"
#include "../common/commondefine.h"

#include <QFile>
#include <QDir>
#include <QUrl>
#include <QStandardPaths>
#include <QDebug>
#include <QJsonDocument>

#include <pwd.h>
#include <unistd.h>

const static QString wallpaperJsonPath = QString("%1/dde-appearance/").arg(utils::GetUserConfigDir());

utils::utils()
{
}

void utils::writeWallpaperConfig(const QVariant &wallpaper)
{
    QString value = QJsonDocument::fromVariant(wallpaper).toJson();
    QDir dir;
    if (!dir.exists(wallpaperJsonPath)) {
        bool res = dir.mkpath(wallpaperJsonPath);
        if (!res) {
            qWarning() << QString("mkdir %1 failed.").arg(wallpaperJsonPath);
            return;
        }
    }

    QFile file(wallpaperJsonPath + "config.json");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream textStream(&file);
        textStream << value;
        textStream.flush();
        file.close();
    } else {
        qWarning() << QString("%1 error.").arg(wallpaperJsonPath);
    }
}

bool utils::WriteStringToFile(QString filename, QString content)
{
    if (filename.length() == 0) {
        return false;
    }

    QString swapFile = filename + "/.swap";
    QDir dir(swapFile);
    if (!dir.mkpath(swapFile)) {
        return false;
    }

    QFile file(swapFile);
    if (!file.open(QIODevice::WriteOnly))
        return false;

    file.write(content.toLatin1(), content.length());
    file.close();

    return file.rename(filename);
}

bool utils::isURI(QString uri)
{
    if (uri.indexOf("://") != -1)
        return true;

    return false;
}

bool utils::isDir(QString path)
{
    QFileInfo fileInfo(path);
    return fileInfo.isDir();
}

bool utils::isFilesInDir(QVector<QString> files, QString dir)
{
    if (!isDir(dir)) {
        return false;
    }

    for (auto file : files) {
        QString tmp = dir + "/" + file;
        QFile qfile(tmp);
        if (!qfile.exists()) {
            return false;
        }
    }

    return true;
}

bool utils::isFileExists(QString filename)
{
    QString path = utils::deCodeURI(filename);
    if (QFile::exists(filename)) {
        return true;
    }

    return false;
}

QString utils::deCodeURI(QString uri)
{
    QString path;
    if (isURI(uri)) {
        QUrl Url(uri);
        path = Url.path();
    } else {
        path = uri;
    }

    return path;
}

QString utils::enCodeURI(QString content, QString scheme)
{
    QString path;
    if (isURI(content)) {
        path = deCodeURI(content);
    } else {
        path = content;
    }
    return scheme + path;
}

QString utils::GetUserHomeDir()
{
    return QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
}

QString utils::GetUserDataDir()
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
}

QString utils::GetUserConfigDir()
{
    return QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
}

QString utils::GetUserCacheDir()
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation);
}

QString utils::GetUserRuntimeDir()
{
    return QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation);
}
