// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filewatcher.h"

#include <QGuiApplication>
#include <QDebug>
#include <QString>
#include <QFileInfo>

#ifdef Q_OS_LINUX
#include <sys/inotify.h>
#endif

SystemFileWatcher::SystemFileWatcher(QObject *parent)
#ifdef Q_OS_LINUX
    : QThread(parent)
#else
    : QFileSystemWatcher(parent)
#endif
{
#ifdef Q_OS_LINUX
    _handleId = inotify_init();
#endif
}

SystemFileWatcher::~SystemFileWatcher()
{
    clear();
}

bool SystemFileWatcher::isValid()
{
#ifdef Q_OS_LINUX
    return (_handleId != -1);
#else
    return true;
#endif
}

void SystemFileWatcher::addWather(const QString &path)
{
#ifdef Q_OS_LINUX
    QMutexLocker loker(&_mutex);
    if (!isValid())
        return;

    QFileInfo info(path);
    if (!info.exists() || !info.isFile()) {
        return;
    }

    if (watchedFiles.find(path) != watchedFiles.end()) {
        return;
    }

    std::string sfile = path.toStdString();
    int fileId = inotify_add_watch(_handleId, sfile.c_str(), IN_MODIFY | IN_DELETE_SELF | IN_MOVE_SELF);

    watchedFiles.insert(path, fileId);
    watchedFilesId.insert(fileId, path);

    if (!_running) {
        _running = true;
        start();
    }
#else
    if (addPath(path)) {
        watchedFiles.insert(path);
    }
#endif
}

void SystemFileWatcher::removePath(const QString &path)
{
#ifdef Q_OS_LINUX
    QMutexLocker loker(&_mutex);

    if (!isValid())
        return;

    auto itf = watchedFiles.find(path);
    if (itf != watchedFiles.end()) {
        inotify_rm_watch(_handleId, itf.value());

        watchedFilesId.remove(itf.value());
        watchedFiles.erase(itf);
    }
#else
    if (QFileSystemWatcher::removePath(path)) {
        watchedFiles.remove(path);
    }
#endif
}

void SystemFileWatcher::clear()
{
#ifdef Q_OS_LINUX
    QMutexLocker loker(&_mutex);

    foreach (auto it, watchedFiles) {
        inotify_rm_watch(_handleId, it);
    }
    watchedFilesId.clear();
    watchedFiles.clear();
#else
    foreach (auto path, watchedFiles) {
        removePath(path);
    }
    watchedFiles.clear();
#endif
}

#ifdef Q_OS_LINUX
void SystemFileWatcher::run()
{
    doRun();
}

void SystemFileWatcher::doRun()
{

    if (!isValid())
        return;

    char name[1024];
    auto freadsome = [ = ](void *dest, size_t remain, FILE * file) {
        char *offset = reinterpret_cast<char *>(dest);
        while (remain) {
            size_t n = fread(offset, 1, remain, file);
            if (n == 0) {
                return -1;
            }

            remain -= n;
            offset += n;
        }
        return 0;
    };

    FILE *watcher_file = fdopen(_handleId, "r");

    while (true) {
        inotify_event event;
        if (-1 == freadsome(&event, sizeof(event), watcher_file)) {
            qWarning() << "------------- freadsome error !!!!!---------- ";
        }
        if (event.len) {
            freadsome(name, event.len, watcher_file);
        } else {
            QMutexLocker loker(&_mutex);
            auto itf = watchedFilesId.find(event.wd);
            if (itf != watchedFilesId.end()) {
                qDebug() << "file = " << itf.value() << " event.wd = " << event.wd << "event.mask = " << event.mask;

                if (event.mask & IN_MODIFY) {
                    emit fileChanged(itf.value(), EFileModified);
                } else if (event.mask & IN_MOVE_SELF) {
                    emit fileChanged(itf.value(), EFileMoved);
                } else if (event.mask & IN_DELETE_SELF) {
                    emit fileChanged(itf.value(), EFileMoved);
                }
            }
        }
    }

}
#endif

