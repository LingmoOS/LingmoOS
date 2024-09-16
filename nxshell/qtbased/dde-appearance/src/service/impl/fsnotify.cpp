// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fsnotify.h"

#include "../modules/common/commondefine.h"

#include <QDebug>
#include <QStandardPaths>
#include <QTimer>

Fsnotify::Fsnotify(QObject *parent)
    : QObject(parent)
    , fileWatcher(new QFileSystemWatcher())
    , backgrounds(new Backgrounds())
    , prevTimestamp(0)
    , timer(new QTimer(this))
{
    watchGtkDirs();
    watchIconDirs();
    watchGlobalDirs();
    watchBgDirs();

    connect(timer, &QTimer::timeout, this, &Fsnotify::onTimeOut);
    timer->setSingleShot(true);
    timer->setInterval(10000);
    connect(fileWatcher.data(), &QFileSystemWatcher::directoryChanged, this, &Fsnotify::onFileChanged);
    connect(fileWatcher.data(), &QFileSystemWatcher::fileChanged, this, &Fsnotify::onFileChanged);
}

Fsnotify::~Fsnotify() { }

void Fsnotify::watchGtkDirs()
{
    auto home = qgetenv("HOME");
    gtkDirs.append(home + "/.local/share/themes");
    gtkDirs.append(home + "/.themes");
    for (const QString &basedir : QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation)) {
        const QString path = QDir(basedir).filePath("themes");
        if (QFile::exists(path))
            gtkDirs.append(path);
    }
    watchDirs(gtkDirs);
}

void Fsnotify::watchIconDirs()
{
    auto home = qgetenv("HOME");
    iconDirs.append(home + "/.local/share/icons");
    iconDirs.append(home + "/.icons");
    for (const QString &basedir : QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation)) {
        const QString path = QDir(basedir).filePath("icons");
        if (QFile::exists(path))
            iconDirs.append(path);
    }
    watchDirs(iconDirs);
}

void Fsnotify::watchBgDirs()
{
    bgDirs = backgrounds->listDirs();
    watchDirs(bgDirs);
}

void Fsnotify::watchGlobalDirs()
{
    QStringList globalDirs;
    QDir home = QDir::home();
    globalDirs.append(home.absoluteFilePath(
            QString("%1/dde-appearance/deepin-themes/").arg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation))));
    globalDirs.append(home.absoluteFilePath(".local/share/deepin-themes"));
    globalDirs.append(home.absoluteFilePath(".deepin-themes"));
    for (const QString &basedir : QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation)) {
        const QString path = QDir(basedir).filePath("deepin-themes");
        if (QFile::exists(path))
            globalDirs.append(path);
    }
    watchDirs(globalDirs);
}

void Fsnotify::watchDirs(QStringList dirs)
{
    QDir qdir;
    // TODO 文件创建失败 监听失败
    for (auto dir : dirs) {
        if (!qdir.exists(dir)) {
            qdir.mkpath(dir);
            qInfo() << "mkpath:" << dir;
        }
        if (!fileWatcher->addPath(dir)) {
            qInfo() << "filewatcher add path failed" << dir << __FUNCTION__;
        }
    }
}

bool Fsnotify::hasEventOccurred(QString name, QStringList lists)
{
    for (auto list : lists) {
        if (list.contains(name))
            return true;
    }
    return false;
}

void Fsnotify::onFileChanged(const QString &path)
{
    QString tmpFilePrefix = backgrounds->getCustomWallpapersConfigDir() + "/temp-";
    if (path.startsWith(tmpFilePrefix)) {
        return;
    }

    if (hasEventOccurred(path, bgDirs)) {
        changedThemes.insert(TYPEBACKGROUND);
    } else if (hasEventOccurred(path, gtkDirs)) {
        changedThemes.insert(TYPEGTK);
    } else if (hasEventOccurred(path, iconDirs)) {
        changedThemes.insert(TYPEICON);
    } else if (path.contains(".config") || path.contains(".cache")) { // 自定义不延时
        // TODO: if config path is set to other place and not contain .cache
        // This will not work
        Q_EMIT themeFileChange(TYPEGLOBALTHEME);
    } else if (path.contains("deepin-themes")) {
        changedThemes.insert(TYPEGLOBALTHEME);
    }
    if (!timer->isActive()) {
        timer->start();
    }
}

void Fsnotify::onTimeOut()
{
    qInfo() << __LINE__ << __FUNCTION__ << changedThemes;
    for (auto &&theme : changedThemes) {
        Q_EMIT themeFileChange(theme);
    }
    changedThemes.clear();
}
