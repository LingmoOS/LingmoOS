/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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
 *
 */

#include "file-system-watcher-private.h"
#include <sys/inotify.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>
#include <QDir>
#include <QQueue>
#include <QtConcurrentRun>

#include "lingmo-search-qdbus.h"
#include "file-utils.h"
using namespace LingmoUISearch;
FileSystemWatcherPrivate::FileSystemWatcherPrivate(FileSystemWatcher *parent) : q(parent)
{
    qDebug() << "setInotifyMaxUserWatches start";
    LingmoUISearchQDBus usQDBus;
    usQDBus.setInotifyMaxUserWatches();
    qDebug() << "setInotifyMaxUserWatches end";

    init();
}

FileSystemWatcherPrivate::~FileSystemWatcherPrivate()
{
    close(m_inotifyFd);
    if(m_notifier) {
        delete m_notifier;
        m_notifier = nullptr;
    }
}

void FileSystemWatcherPrivate::addWatch(const QString &path)
{
    int wd = inotify_add_watch(m_inotifyFd, path.toStdString().c_str(), m_watchEvents | m_watchFlags);
    if(wd > 0) {
        m_watchPathHash[wd] = path;
    } else {
        qWarning() << "AddWatch error:" << path << strerror(errno);
        if (errno == ENOSPC) {
            qWarning() << "User limit reached. Count: " << m_watchPathHash.count();
        }
    }
}

void FileSystemWatcherPrivate::addWatchWithBlackList(const QStringList &pathList, const QStringList &blackList)
{
    QQueue<QString> bfs;
    QStringList tmpPathList = pathList;
    for(QString blackPath : blackList) {
        for(QString path : pathList) {
            if(FileUtils::isOrUnder(path, blackPath)) {
                tmpPathList.removeOne(path);
            }
        }
    }
    for(QString path : tmpPathList) {
        if(!m_watchedRootPaths.contains(path)) {
            addWatch(path);
            bfs.enqueue(path);
            m_watchedRootPaths.append(path);
        }
    }
    if(!m_recursive) {
        return;
    }
    QFileInfoList list;
    QDir dir;
    QStringList tmpList = blackList;
    dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    while(!bfs.empty()) {
        dir.setPath(bfs.dequeue());
        list = dir.entryInfoList();
        for(auto i : list) {
            bool isBlocked = false;
            for(QString path : tmpList) {
                if(i.absoluteFilePath() == path) {
                    isBlocked = true;
                    tmpList.removeOne(path);
                    break;
                }
            }
            if(isBlocked)
                continue;

            if(!(i.isSymLink())) {
                addWatch(i.absoluteFilePath());
                bfs.enqueue(i.absoluteFilePath());
            }
        }
    }
}

QStringList FileSystemWatcherPrivate::removeWatch(const QString &path)
{
    QStringList paths;
    QHash<int, QString>::iterator iter = m_watchPathHash.begin();
    while(iter != m_watchPathHash.end()) {
        if(iter.value().length() >= path.length() && FileUtils::isOrUnder(iter.value(), path)) {
            paths.append(iter.value());
            inotify_rm_watch(m_inotifyFd, iter.key());
            iter = m_watchPathHash.erase(iter);
        } else {
            ++iter;
        }
    }
    m_watchPathHash.squeeze();
    return paths;
}

QString FileSystemWatcherPrivate::removeWatch(int wd)
{
    inotify_rm_watch(m_inotifyFd, wd);
    return m_watchPathHash.take(wd);
}

void FileSystemWatcherPrivate::clearAll()
{
    if(m_inotifyFd > 0) {
        close(m_inotifyFd);
        m_inotifyFd = -1;
    }
    if(m_notifier) {
        delete m_notifier;
        m_notifier = nullptr;
    }
    m_watchPathHash.clear();
    m_watchedRootPaths.clear();
    init();
}

void FileSystemWatcherPrivate::init()
{
    if(m_inotifyFd < 0) {
        m_inotifyFd = inotify_init();

        if (m_inotifyFd > 0) {
            qDebug()<<"Inotify init success!";
        } else {
            qWarning() << "Inotify init fail! Now try add inotify_user_instances.";
            LingmoUISearchQDBus usQDBus;
            usQDBus.addInotifyUserInstances(128);
            m_inotifyFd = inotify_init();
            if (m_inotifyFd > 0) {
                qDebug()<<"Inotify init success!";
            } else {
                printf("errno=%d\n",errno);
                printf("Mesg:%s\n",strerror(errno));
                Q_ASSERT_X(0, "InotifyWatch", "Failed to initialize inotify");
                return;
            }
        }
        fcntl(m_inotifyFd, F_SETFD, FD_CLOEXEC);
        m_notifier = new QSocketNotifier(m_inotifyFd, QSocketNotifier::Read);
        QObject::connect(m_notifier, &QSocketNotifier::activated, q, &FileSystemWatcher::eventProcess);
    }
}

FileSystemWatcher::FileSystemWatcher(bool recursive, WatchEvents events, WatchFlags flags, QObject *parent)
                  : QObject(parent)
                  , d(new FileSystemWatcherPrivate(this))
{
    d->m_watchEvents = events;
    d->m_watchFlags = flags;
    d->m_recursive = recursive;
}

FileSystemWatcher::~FileSystemWatcher()
{
    if(d) {
        delete d;
        d = nullptr;
    }
}

void FileSystemWatcher::addWatch(const QStringList &pathList)
{
    d->addWatchWithBlackList(pathList, QStringList());
}

void FileSystemWatcher::addWatch(const QString &path)
{
    d->addWatchWithBlackList(QStringList(path), QStringList());
}

void FileSystemWatcher::addWatchWithBlackList(const QStringList &pathList, const QStringList &blackList)
{
    d->addWatchWithBlackList(pathList, blackList);
}

QStringList FileSystemWatcher::removeWatch(const QString &path)
{
    for(QString watchedPath : d->m_watchedRootPaths) {
        if(FileUtils::isOrUnder(watchedPath, path)) {
            d->m_watchedRootPaths.removeAll(watchedPath);
        }
    }
    return d->removeWatch(path);
}

void FileSystemWatcher::clearAll()
{
    d->clearAll();
}

void FileSystemWatcher::eventProcess(int socket)
{
//    qDebug() << "-----begin event process-----";
    int avail;
    if (ioctl(socket, FIONREAD, &avail) == EINVAL) {
        qWarning() << "Did not receive an entire inotify event.";
        return;
    }

    char* buf = (char*)malloc(avail);
    memset(buf, 0x00, avail);

    const ssize_t len = read(socket, buf, avail);

    if(len != avail) {
        qWarning()<<"read event error";
    }

    int i = 0;
    while (i < len) {
        const struct inotify_event* event = (struct inotify_event*)&buf[i];

        if(event->name[0] == '.') {
            i += sizeof(struct inotify_event) + event->len;
            continue;
        }
        if (event->wd < 0 && (event->mask & EventQueueOverflow)) {
            qWarning() << "Inotify Event queued overflowed";
            free(buf);
            return;
        }

//        qDebug() << "event mask:" << event->mask
//                 << "isDir:" << (event->mask & IN_ISDIR)
//                 << "event->wd:" << event->wd
//                 << "event->name" << QString(QByteArray::fromRawData(event->name, qstrnlen(event->name, event->len)))
//                 << "event->len" << event->len
//                 << "event->cookie" << event->cookie
//                 << "path:" << d->m_watchPathHash.value(event->wd);
        QString path;

        if (event->mask & (EventDeleteSelf | EventMoveSelf | EventUnmount)) {
            path = d->m_watchPathHash.value(event->wd);
            if(path.isEmpty()) {
                i += sizeof(struct inotify_event) + event->len;
                continue;
            }
        } else {
            QByteArray name = QByteArray::fromRawData(event->name, qstrnlen(event->name, event->len));
            path = d->m_watchPathHash[event->wd];
            if(name.isEmpty() || path.isEmpty()) {
                i += sizeof(struct inotify_event) + event->len;
                continue;
            }
            path.append("/").append(name);
        }

        if(event->mask & EventCreate) {
//            qDebug() << path << "--EventCreate";
            Q_EMIT created(path, event->mask & IN_ISDIR);
            if(event->mask & IN_ISDIR) {
                for(const QString &createdPath : traverse(path)) {
                    Q_EMIT created(createdPath, QFileInfo(createdPath).isDir());
                }
            }
        }
        if (event->mask & EventDeleteSelf) {
//            qDebug() << path << "--EventDeleteSelf";
            if(d->m_watchedRootPaths.contains(path)) {
//                qDebug() << "EventDeleteSelf send" << path;
                //All folders under this one should be deleted.
                for(const QString &removedPath : d->removeWatch(path)) {
                    Q_EMIT deleted(removedPath, true);
                }
                d->m_watchedRootPaths.removeAll(path);
            }
        }
        if (event->mask & EventDelete) {
//            qDebug() << path << "--EventDelete";
            if (event->mask & IN_ISDIR) {
                for(const QString &removedPath : d->removeWatch(path)) {
                    Q_EMIT deleted(removedPath, true);
                }
            } else {
                Q_EMIT deleted(path, false);
            }
        }
        if (event->mask & EventModify) {
//            qDebug() << path << "--EventModify";
            if(!(event->mask & IN_ISDIR)) {
                Q_EMIT modified(path);
            }
        }
        if (event->mask & EventMoveSelf) {
//             qDebug() << path << "--EventMoveSelf";
            if(d->m_watchedRootPaths.contains(path)) {
                for(const QString &removedPath : d->removeWatch(path)) {
                    Q_EMIT moved(removedPath, true);
                }
                d->m_watchedRootPaths.removeAll(path);
            }
        }
        if (event->mask & EventMoveFrom) {
//            qDebug() << path << "--EventMoveFrom";
            if (event->mask & IN_ISDIR) {
                for(const QString &removedPath : d->removeWatch(path)) {
                    Q_EMIT moved(removedPath, true);
                }
            } else {
                Q_EMIT moved(path, false);
            }
        }
        if (event->mask & EventMoveTo) {
//            qDebug() << path << "--EventMoveTo";
            Q_EMIT moveTo(path, event->mask & IN_ISDIR);
            if (event->mask & IN_ISDIR) {
                for(const QString &createdPath : traverse(path)) {
                    Q_EMIT moveTo(createdPath, QFileInfo(createdPath).isDir());
                }
            }
        }
        if (event->mask & EventOpen) {
//            qDebug() << path << "--EventOpen";
            Q_EMIT opened(path);
        }
        if (event->mask & EventUnmount) {
//            qDebug() << path << "--EventUnmount";
            // This is present because a unmount event is sent by inotify after unmounting, by
            // which time the watches have already been removed.
            if (path != "/") {
                Q_EMIT unmounted(path, event->mask & IN_ISDIR);
            }
            d->m_watchedRootPaths.removeAll(path);
        }
        if (event->mask & EventAttributeChange) {
//            qDebug() << path << "--EventAttributeChange";
            Q_EMIT attributeChanged(path);
            if (event->mask & IN_ISDIR) {
                QFileInfo info(path);
                if(!info.isReadable() || !info.isExecutable()) {
                    for(const QString &removedPath : d->removeWatch(path)) {
                        Q_EMIT accessibleChanged(removedPath, true, false);
                    }
                } else {
                    if (!d->m_watchPathHash.values().contains(path)) {
                        for(const QString &createdPath : traverse(path)) {
                            Q_EMIT accessibleChanged(createdPath, QFileInfo(createdPath).isDir(), true);
                        }
                    }
                }
            } else {
                Q_EMIT accessibleChanged(path, false, QFileInfo(path).isReadable());
            }
        }
        if (event->mask & EventAccess) {
//            qDebug() << path << "--EventAccess";
            Q_EMIT accessed(path);
        }
        if (event->mask & EventCloseWrite) {
//            qDebug() << path << "--EventCloseWrite";
            Q_EMIT closedWrite(path);
        }
        if (event->mask & EventCloseRead) {
//            qDebug() << path << "--EventCloseRead";
            Q_EMIT closedRead(path);
        }
        if (event->mask & EventIgnored) {
//            qDebug() << path << "--EventIgnored";
        }
        i += sizeof(struct inotify_event) + event->len;
    }
    if (len < 0) {
        qWarning() << "Failed to read event.";
    }

    free(buf);
}
QStringList FileSystemWatcher::traverse(QString &path)
{
    QStringList paths;
    d->addWatch(path);
    if(!d->m_recursive || QFileInfo(path).isSymLink()) {
        return paths;
    }
    QQueue<QString> queue;
    queue.enqueue(path);

    QFileInfoList list;
    QDir dir;
    dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
    dir.setSorting(QDir::DirsFirst);
    while(!queue.empty()) {
        dir.setPath(queue.dequeue());
        list = dir.entryInfoList();
        for(auto i : list) {
            if(i.isDir() && !(i.isSymLink())) {
                queue.enqueue(i.absoluteFilePath());
                d->addWatch(i.absoluteFilePath());
            }
            paths.append(i.absoluteFilePath());
        }
    }
    return paths;
}
