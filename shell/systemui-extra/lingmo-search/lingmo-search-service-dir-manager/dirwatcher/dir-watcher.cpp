/*
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: iaom <zhangpengfei@kylinos.cn>
 */
#include "dir-watcher.h"

#include <QDebug>
#include <QDir>
#include <QDBusConnection>
#include <QDBusArgument>
#include <QThread>
#include <fstab.h>
#include <QMutexLocker>
#include "config.h"
#include "fileindexserviceadaptor.h"

#define CURRENT_INDEXABLE_DIR_SETTINGS QDir::homePath() + "/.config/org.lingmo/lingmo-search/lingmo-search-current-indexable-dir.conf"
#define INDEXABLE_DIR_VALUE "IndexableDir"
#define DEFAULT_INDEXABLE_DIR QDir::homePath()

static std::once_flag flag;
static DirWatcher *global_intance = nullptr;

DirWatcher::DirWatcher(QObject *parent) : QObject(parent)
{
    //兼容旧版配置
    Config::self()->processCompatibleCache();

    new FileindexAdaptor(this);
}

DirWatcher::~DirWatcher()
{

}

DirWatcher *DirWatcher::getDirWatcher()
{
    std::call_once(flag, [ & ] {
        global_intance = new DirWatcher();
    });
    return global_intance;
}

QStringList DirWatcher::currentIndexableDir()
{
    return currentSearchDirs();
}


QStringList DirWatcher::currentBlackListOfIndex()
{
    return currentBlackList();
}

/**
 * @brief DirWatcher::blackListOfDir 根据传入目录返回当前目录下的所有不可搜索目录，没有则返回空列表
 * @param dirPath 要搜索的目录
 */
QStringList DirWatcher::blackListOfDir(const QString &dirPath)
{
    return SearchDir::blackListOfDir(dirPath);
}

void DirWatcher::initDbusService()
{
    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    if (!sessionBus.registerObject("/org/lingmo/search/fileindex", this, QDBusConnection::ExportAdaptors)){
        qWarning() << "lingmo-search-fileindex dbus register object failed reason:" << sessionBus.lastError();
    }
}

QStringList DirWatcher::currentSearchableDir()
{
    return currentSearchDirs();
}

QStringList DirWatcher::searchableDirForSearchApplication()
{
    return currentSearchDirs();
}

int DirWatcher::appendIndexableListItem(const QString &path)
{
    return appendSearchDir(path);
    /* code:
     * 0: successful
     * 1: path or its parent dir has been added
     * 2: path is or under blacklist
     * 3: path is in repeat mounted devices and another path which is in the same device has been indexed
     * 4: another path which is in the same device has been indexed
     * 5: path is not exists
     */
}

bool DirWatcher::removeIndexableListItem(const QString &path)
{
    removeSearchDir(path);
    return true;
}

int DirWatcher::appendSearchDir(const QString &path)
{
    SearchDir dir(path);
    if (dir.error() == SearchDir::Successful) {
        Q_EMIT this->appendIndexItem(path, dir.getBlackList());
        qDebug() << "Add search dir:" << path << "blacklist:" << dir.getBlackList();
        //要添加已索引目录的父目录，先添加索引，再同步配置文件，从而使得下次读取配置文件时不会将子目录排除掉
        for (const QString &searchDir : Config::self()->searchDirs()) {
            if (searchDir.startsWith(path + "/") || path == "/") {
                SearchDir subDir(searchDir);
                Config::self()->removeDir(subDir);
            }
        }
        dir.generateBlackList();
        Config::self()->addDir(dir);
    }
    qWarning() << dir.getPath() << dir.errorString();
    return dir.error();
}

void DirWatcher::removeSearchDir(const QString &path)
{
    SearchDir dir(path, false);
    Config::self()->removeDir(dir);
    Q_EMIT this->removeIndexItem(path);
    return;
}

QStringList DirWatcher::currentSearchDirs()
{
    return Config::self()->searchDirs();
}

QStringList DirWatcher::currentBlackList()
{
    return Config::self()->blackDirs();
}

QStringList DirWatcher::blockDirsForUser()
{
    return Config::self()->blockDirsForUser();
}

int DirWatcher::addBlockDirOfUser(const QString &dir)
{
    return Config::self()->addBlockDirOfUser(dir);
}

void DirWatcher::removeBlockDirOfUser(const QString &dir)
{
    Config::self()->removeBlockDirOfUser(dir);
}
