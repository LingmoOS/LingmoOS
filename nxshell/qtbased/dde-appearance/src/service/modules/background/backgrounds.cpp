// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "backgrounds.h"
#include "../api/utils.h"
#include "../common/commondefine.h"
#include "dbus/appearancedbusproxy.h"

#include <QDBusInterface>
#include <pwd.h>
#include <QDBusReply>

QStringList Backgrounds::systemWallpapersDir = { "/usr/share/wallpapers/deepin" };
QStringList Backgrounds::uiSupportedFormats = { "jpeg", "png", "bmp", "tiff", "gif" };

Backgrounds::Backgrounds(QObject *parent)
    : QObject(parent)
    , backgroundsMu(new QMutex())
    , fsChanged(false)
{
    init();
}

Backgrounds::~Backgrounds()
{
}

bool Backgrounds::deleteBackground(const QString &uri)
{
    QVector<Background>::iterator iter = backgrounds.begin();
    QString url = utils::enCodeURI(uri, "file://");
    while (iter != backgrounds.end()) {
        if ((*iter).getId() == url) {
            (*iter).Delete();
            iter = backgrounds.erase(iter);
            return true;
        } else {
            iter++;
        }
    }

    return false;
}

void Backgrounds::init()
{
    QString configPath = utils::GetUserConfigDir();
    customWallpapersConfigDir = configPath + "/deepin/dde-daemon/appearance/custom-wallpapers";
    QDir qdir;
    if (!qdir.exists(customWallpapersConfigDir)) {
        qdir.mkdir(customWallpapersConfigDir);
        qInfo() << "mkdir: " << customWallpapersConfigDir;
    }

    refreshBackground();
}

QStringList Backgrounds::listDirs()
{
    QStringList result;
    result.append(systemWallpapersDir);
    result.append(customWallpapersConfigDir);
    return result;
}

void Backgrounds::refreshBackground()
{
    QStringList files = getCustomBgFiles();
    for (auto file : files) {
        Background bg;
        bg.setId(utils::enCodeURI(file, SCHEME_FILE));
        bg.setDeletable(true);
        backgrounds.push_back(bg);
    }

    files = getSysBgFIles();
    for (auto file : files) {
        Background bg;
        bg.setId(utils::enCodeURI(file, SCHEME_FILE));
        bg.setDeletable(false);
        backgrounds.push_back(bg);
    }
    fsChanged = false;
}

void Backgrounds::sortByTime(QFileInfoList listFileInfo)
{
    std::sort(listFileInfo.begin(), listFileInfo.end(), [=](const QFileInfo &f1, QFileInfo &f2) {
        return f1.lastModified().toTime_t() < f2.lastModified().toTime_t();
    });
}

QStringList Backgrounds::getSysBgFIles()
{
    QStringList files;
    for (auto dir : systemWallpapersDir) {
        files.append(getBgFilesInDir(dir));
    }
    return files;
}

QStringList Backgrounds::getCustomBgFiles()
{
    struct passwd *user = getpwuid(getuid());
    if (user == nullptr) {
        return QStringList();
    }
    return AppearanceDBusProxy::GetCustomWallPapers(user->pw_name);
}

QStringList Backgrounds::getCustomBgFilesInDir(QString dir)
{
    QStringList wallpapers;
    QDir qdir(dir);
    if (!qdir.exists())
        return wallpapers;

    QFileInfoList fileInfoList = qdir.entryInfoList(QDir::NoSymLinks);
    sortByTime(fileInfoList);

    for (auto info : fileInfoList) {
        if (info.isDir()) {
            continue;
        }
        if (!isBackgroundFile(info.path())) {
            continue;
        }
        wallpapers.append(info.path());
    }

    return wallpapers;
}

QStringList Backgrounds::getBgFilesInDir(QString dir)
{
    QStringList walls;

    QDir qdir(dir);
    if (!qdir.exists())
        return walls;

    QFileInfoList fileInfoList = qdir.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);
    for (auto file : fileInfoList) {
        if (!isBackgroundFile(file.filePath())) {
            continue;
        }
        walls.append(file.filePath());
    }

    return walls;
}

bool Backgrounds::isFileInDirs(QString file, QStringList dirs)
{
    for (auto dir : dirs) {
        QFileInfo qfile(file);
        if (qfile.absolutePath() == dir)
            return true;
    }

    return false;
}

bool Backgrounds::isBackgroundFile(QString file)
{
    file = utils::deCodeURI(file);

    QString format = FormatPicture::getPictureType(file);
    if (format == "") {
        return false;
    }

    if (uiSupportedFormats.contains(format)) {
        return true;
    }

    return false;
}

QVector<Background> Backgrounds::listBackground()
{
    backgroundsMu->lock();

    if (backgrounds.length() == 0 || fsChanged)
        refreshBackground();

    backgroundsMu->unlock();
    return backgrounds;
}

void Backgrounds::notifyChanged()
{
    backgroundsMu->lock();
    fsChanged = true;
    backgroundsMu->unlock();
}

QString Backgrounds::resizeImage(QString fileName, QString cacheDir)
{
    const int stdWidth = 3840;
    const int stdHeight = 2400;
    // TODO: 对图片过大的做处理
    return fileName;
}

QString Backgrounds::onPrepare(QString fileName)
{
    struct passwd *user = getpwuid(getuid());
    if (user == nullptr) {
        return "";
    }
    QString file = resizeImage(fileName, customWallpapersConfigDir);
    notifyChanged();

    return AppearanceDBusProxy::SaveCustomWallPaper(user->pw_name, file);
}

QString Backgrounds::prepare(QString file)
{
    QString tempFile = utils::deCodeURI(file);
    if (isFileInDirs(tempFile, systemWallpapersDir)) {
        return tempFile;
    }

    return onPrepare(tempFile);
}
