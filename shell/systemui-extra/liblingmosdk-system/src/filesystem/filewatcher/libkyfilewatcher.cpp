/*
 * liblingmosdk-system's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yunhe Liu <liuyunhe@kylinos.cn>
 *
 */

#include "libkyfilewatcher.hpp"
#include <libkylog.h>
#include <errno.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <pthread.h>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <sys/stat.h>
using namespace LINGMOSDK_FILEWATCHER;

#define unlikely(x)	__builtin_expect(!!(x) , 0)

void* LINGMOSDK_FILEWATCHER::fileWatcherThreadRunner(void *inst)
{
    FileWatcher *core   = static_cast<FileWatcher*>(inst);
#define EVENTBUFLEN sizeof(struct inotify_event) + NAME_MAX + 1
    char events_buf[EVENTBUFLEN] __attribute__((aligned(8)));
    char *events_pos;
    int rd;
    struct inotify_event *event;
    while (! core->quit)
    {
        memset(events_buf, 0, EVENTBUFLEN);
        rd = read(core->watcherFd, events_buf, EVENTBUFLEN);
        if (rd < (int)sizeof(struct inotify_event))
        {
            continue;
        }

        for (events_pos = events_buf; events_pos < events_buf + rd;)
        {
            if (! core->isActived)
                break;
            event = (struct inotify_event *)events_pos;

            char pathName[NAME_MAX];
            snprintf(pathName, event->len, "%s", event->name);

            core->sendSignal(event->wd, pathName, event->mask);

            events_pos += sizeof(struct inotify_event) + event->len;
        }
    }

    return nullptr;
}

FileWatcher::FileWatcher()
{
    this->isActived = true;
    this->quit  = false;
    this->watcherFd = inotify_init1(IN_CLOEXEC);
    if (this->watcherFd <= 0)
    {
        klog_err("文件监听启动失败，错误码：%d\n", errno);
        throw 2;
    }
    this->threadRunner = (pthread_t *)malloc(sizeof(pthread_t));
    if (! this->threadRunner)
    {
        klog_err("文件监听启动失败，错误码：%d\n", errno);
        throw 1;
    }
    pthread_create(this->threadRunner, nullptr, fileWatcherThreadRunner, this);
}

FileWatcher::~FileWatcher()
{
    this->quit  = true;
    if (this->threadRunner)
    {
        pthread_cancel(*this->threadRunner);
        pthread_join(*this->threadRunner, nullptr);
        free(this->threadRunner);
    }

    this->isActived = false;
    if (this->watcherFd > 0)
        close(this->watcherFd);
}

int FileWatcher::addWatchTarget(QString url, FileWatcherType type, int attr)
{
    FileDescription node = {.url = url, .type = type, .attr = attr, .wfd = -1};
    return this->addWatchFile(node);
}

QStringList FileWatcher::addWatchTargetRecursive(QString url, FileWatcherType type, int attr, int maxdepth, int recurType)
{
    QStringList failedList;
    QStringList list;
    QFileInfo finfo(url);

    // 处理传入的不是目录的情况
    if (! finfo.isDir())
    {
        if (this->addWatchTarget(url, type, attr))
            failedList.append(url);
        return failedList;
    }

    // 递归获取目录下的所有文件
    if (recurType & DIR)
        list    = this->getChildDir(url, 1, maxdepth, recurType);
    else
        list    = this->getChildFile(url, 1, maxdepth, recurType);

    if (finfo.isDir() && (recurType & DIR))
        list.append(url);


    QString tmpURL;
    foreach (tmpURL, list)
    {
        if (this->addWatchTarget(tmpURL, type, attr))
            failedList.append(tmpURL);
    }

    return failedList;
}

QStringList FileWatcher::addWatchTargetList(QStringList urlList, FileWatcherType type, int attr)
{
    QString url;
    QStringList failedList;
    foreach (url, urlList) {
        if (this->addWatchTarget(url, type, attr) != 0)
        {
            // errlog
            failedList.append(url);
        }
    }

    return failedList;
}

QStringList FileWatcher::addWatchTargetListRecursive(QStringList urlList, FileWatcherType type, int attr, int maxdepth, int recurType)
{
    QString url;
    QStringList failedList;
    foreach (url, urlList)
    {
        failedList.append(this->addWatchTargetRecursive(url, type, attr, maxdepth, recurType));
    }

    return failedList;
}

int FileWatcher::updateWatchTargetAttribute(QString url, int newattr)
{
    if (this->watchList.contains(url))
        return updateWatchFileAttribute(url, newattr);
    return -1;
}

int FileWatcher::updateWatchTargetType(QString url, FileWatcherType newtype)
{
    if (this->watchList.contains(url))
        return this->updateWatchFileType(url, newtype);
    return -1;
}

int FileWatcher::removeWatchTarget(QString url)
{
    return this->removeWatchFile(url);
}

QStringList FileWatcher::removeWatchTargetRecursive(QString url, int maxdepth, int recurType)
{
    QStringList failedList;
    QStringList list;
    list.append(url);
    QFileInfo finfo(url);

    // 处理传入的不是目录的情况
    if (! finfo.isDir())
    {
        if (this->removeWatchTarget(url))
            failedList.append(url);
        return failedList;
    }

    // 递归获取目录下的所有文件
    if (recurType & DIR)
        list    = this->getChildDir(url, 1, maxdepth, recurType);
    else
        list    = this->getChildFile(url, 1, maxdepth, recurType);

    list.append(url);

    QString tmpURL;
    foreach (tmpURL, list)
    {
        if (this->removeWatchTarget(tmpURL))
            failedList.append(tmpURL);
    }

    return failedList;
}

QStringList FileWatcher::removeWatchTargetList(QString urlList)
{
    QString url;
    QStringList failedList;
    foreach(url, urlList)
    {
        if (this->removeWatchTarget(url))
            failedList.append(url);
    }
    return failedList;
}

QStringList FileWatcher::removeWatchTargetListRecursive(QString urlList, int maxdepth, int recurType)
{
    QString url;
    QStringList failedList;
    foreach(url, urlList)
    {
        failedList.append(this->removeWatchTargetRecursive(url, maxdepth, recurType));
    }
    return failedList;
}

void FileWatcher::clearWatchList()
{
     std::lock_guard<std::mutex> lock(this->listLocker);

     QHash<QString, FileDescription>::iterator iter =  this->watchList.begin();
     for (; iter != this->watchList.end(); iter ++)
     {
        inotify_rm_watch(iter.value().wfd, this->watcherFd);
     }

     this->watchList.clear();
     this->fdCacheMap.clear();

    klog_info("文件监听列表已清空\n");     
}

void FileWatcher::pauseWatcher()
{
    klog_info("文件监听已暂停\n");
    this->isActived = false;
}

void FileWatcher::restartWatcher()
{
    klog_info("文件监听已恢复\n");
    this->isActived = true;
}

QStringList FileWatcher::getWatchList()
{
    QStringList res;
    QHash<QString, FileDescription>::iterator iter =  this->watchList.begin();
    for (; iter != this->watchList.end(); iter ++)
        res.append(iter.value().url);

    return res;
}

QStringList FileWatcher::getWatchList(FileWatcherType type)
{
    QStringList res;
    QHash<QString, FileDescription>::iterator iter =  this->watchList.begin();
    for (; iter != this->watchList.end(); iter ++)
        if (iter.value().type == type)
            res.append(iter.value().url);

    return res;
}

QStringList FileWatcher::getWatchList(int attr)
{
    QStringList res;
    QHash<QString, FileDescription>::iterator iter =  this->watchList.begin();
    for (; iter != this->watchList.end(); iter ++)
        if (iter.value().attr == attr)
            res.append(iter.value().url);

    return res;
}

void FileWatcher::sendSignal(int wfd, QString name, int mask)
{
    std::unique_lock<std::mutex> lock(this->listLocker);
    QString url = this->fdCacheMap[wfd];
    QString path = this->fdCacheMap[wfd];

    // 丢弃那些已经被移除的或者设定为不触发的监视器的事件
    if (! this->watchList.contains(this->fdCacheMap[wfd]) || this->watchList[this->fdCacheMap[wfd]].type == NEVER)
        return;

    bool isSubFile  = name.length() ? true : false;
    if (isSubFile)
        url += "/" + name;

    // qDebug() << url << " event "<< mask;

    if (mask & IN_ACCESS)
    {
        emit this->fileAccessed(isSubFile ? name : this->fdCacheMap[wfd], isSubFile ? this->fdCacheMap[wfd] : QString());
        qDebug() << "File: "<<url<<", is accessed.";
    }
    if (mask & IN_CREATE)
    {
        emit this->fileCreated(isSubFile ? name : this->fdCacheMap[wfd], isSubFile ? this->fdCacheMap[wfd] : QString());
        qDebug() << "File: "<<url<<" is created.";
    }
    if (mask & IN_MODIFY)
    {
        emit this->fileModified(isSubFile ? name : this->fdCacheMap[wfd], isSubFile ? this->fdCacheMap[wfd] : QString());
        qDebug() << "File: "<<url<<" is modified.";
    }
    if (mask & IN_ATTRIB)
    {
        emit this->fileAttrChanged(isSubFile ? name : this->fdCacheMap[wfd], isSubFile ? this->fdCacheMap[wfd] : QString());
        qDebug() << "File: "<<url<<" is metadata changed.";
    }
    if (mask & IN_DELETE)
    {
        emit this->fileDeleted(isSubFile ? name : this->fdCacheMap[wfd], isSubFile ? this->fdCacheMap[wfd] : QString());
        qDebug() << "File: "<<url<<" is deleted.";
    }
    if (mask & IN_MOVED_FROM)
    {
        emit this->fileMoveOut(isSubFile ? name : this->fdCacheMap[wfd], isSubFile ? this->fdCacheMap[wfd] : QString());
        qDebug() << "File: "<<url<<" is moved in.";
    }
    if (mask & IN_MOVED_TO)
    {
        emit this->fileMoveIn(isSubFile ? name : this->fdCacheMap[wfd], isSubFile ? this->fdCacheMap[wfd] : QString());
        qDebug() << "File: "<<url<<" is moved out.";
    }
    if (mask & IN_OPEN)
    {
        emit this->fileOpened(isSubFile ? name : this->fdCacheMap[wfd], isSubFile ? this->fdCacheMap[wfd] : QString());
        qDebug() << "File: "<<url<<" is opened.";
    }
    if (mask & IN_CLOSE)
    {
        emit this->fileClosed(isSubFile ? name : this->fdCacheMap[wfd], isSubFile ? this->fdCacheMap[wfd] : QString());
        qDebug() << "File: "<<url<<" is closed.";
    }
    if (mask & IN_IGNORED)
    {
        // 请注意：发现Vim、gedit等编辑器会在保存已修改的文件时，用swap替换原有文件，导致触发ignore信号。
        // ignore信号发生后，监听符号会自动被内核移除（由于inode变化），因此建议对于所有普通文件监听这个
        // 信号，在发生这个事件后，重新将其加入监听列表中
        emit this->fileIgnoredAutomatic(isSubFile ? name : this->fdCacheMap[wfd], isSubFile ? this->fdCacheMap[wfd] : QString());
        qDebug() << "File: "<<url<<" is ignored by kernel automatic.";

        lock.release();
        this->listLocker.unlock();
        int result = this->removeWatchFile(path);
        if (result) {
            // 即使失败也需要清wfd空缓存，否则下一次添加会继续失败
            this->fdCacheMap.remove(watchList[path].wfd);
            this->watchList.remove(path);
            this->listLocker.unlock();
        }
    }
    if (mask & IN_UNMOUNT)
    {
        emit this->fileUnmount(isSubFile ? name : this->fdCacheMap[wfd], isSubFile ? this->fdCacheMap[wfd] : QString());
        qDebug() << "File: "<<url<<" base fs is umount.";
        lock.release();
        this->listLocker.unlock();
        this->removeWatchFile(path);
    }

    // 处理ONESHOT情况；做一个contains判断，避免之前被autoremove的文件索引出错
    if (this->fdCacheMap.contains(wfd) && this->watchList[this->fdCacheMap[wfd]].type == ONESHOT) {
        lock.release();
        this->listLocker.unlock();
        this->removeWatchFile(path);
    }
}

int FileWatcher::addWatchFile(FileWatcher::FileDescription node)
{
    std::unique_lock<std::mutex> lock(this->listLocker);
    if (! watchList.contains(node.url))
    {
        int attr    = node.attr;
        if (node.type == ONESHOT)
            attr    |= IN_ONESHOT;

        node.wfd    = inotify_add_watch(this->watcherFd, node.url.toStdString().c_str(), attr);
        if (node.wfd < 0)
        {
            // log
            qDebug() << node.url <<"监听符添加失败："<<errno;
            klog_err("文件[%s]监听添加失败，错误码%d\n", node.url.toStdString().c_str(), errno);
            return -1;
        }
        this->fdCacheMap.insert(node.wfd, node.url);
        this->watchList.insert(node.url, node);
    }
    else
    {
        lock.release();
        this->listLocker.unlock();
        // 文件删除再添加存在bug，errno=22
        this->updateWatchFileType(node.url, node.type);
        this->updateWatchFileAttribute(node.url, node.attr);
    }

    klog_debug("文件[%s]已加入监听\n", node.url.toStdString().c_str());
    qDebug() << "Watcher for "<<node.url<<" has been added.";

    return 0;
}

int FileWatcher::removeWatchFile(QString url)
{
    std::lock_guard<std::mutex> lock(this->listLocker);
    int ret = 0;
    if (watchList.contains(url))
    {
        ret = inotify_rm_watch(this->watcherFd, this->watchList[url].wfd);
        if (ret == 0)
        {
            fdCacheMap.remove(watchList[url].wfd);
            watchList.remove(url);
            klog_debug("文件[%s]监听已移除\n", url.toStdString().c_str());
            qDebug() << "Watcher for "<<url<<" has been removed.";
        }
        else
        {
            klog_err("文件[%s]监听移除失败，错误码：%d\n", url.toStdString().c_str(), errno);
            qDebug() << "remove watcher for "<<url<<" with wfd" <<this->watchList[url].wfd<<" failed. "<<errno;
        }
    }
    return ret;
}

int FileWatcher::updateWatchFileAttribute(QString url, int attr)
{
    std::unique_lock<std::mutex> lock(this->listLocker);
    if (this->watchList[url].attr == attr)
        return 0;

    int ret = inotify_rm_watch(this->watcherFd, this->watchList[url].wfd);
    if (ret)
    {
        qDebug() << "unable to remove watcher for "<<url<<", errno = "<<errno;
        return ret;
    }
    this->fdCacheMap.remove(this->watchList[url].wfd);
    ret = inotify_add_watch(this->watcherFd, url.toStdString().c_str(), attr);
    if (ret >= 0)
    {
        // 因为监听符句柄变化，更新对应的缓存内容
        this->watchList[url].attr  = attr;
        this->watchList[url].wfd    = ret;
        this->fdCacheMap.insert(this->watchList[url].wfd, url);
    }
    return ret;
}

int FileWatcher::updateWatchFileType(QString url, FileWatcherType type)
{
    std::lock_guard<std::mutex> lock(this->listLocker);
    int ret = inotify_rm_watch(this->watcherFd, this->watchList[url].wfd);
    if (ret)
    {
        qDebug() << "unable to remove watcher for "<<url<<", errno = "<<errno;
        return ret;
    }
    this->fdCacheMap.remove(this->watchList[url].wfd);
    if (type == ONESHOT)
        ret = inotify_add_watch(this->watcherFd, url.toStdString().c_str(), this->watchList[url].attr | IN_ONESHOT);
    else if (type == PERIODIC)
        ret = inotify_add_watch(this->watcherFd, url.toStdString().c_str(), (this->watchList[url].attr & ~IN_ONESHOT));
    if (ret >= 0)
    {
        this->watchList[url].type  = type;
        this->watchList[url].wfd    = ret;
        this->fdCacheMap.insert(this->watchList[url].wfd, url);
    }
    return ret;
}

QStringList FileWatcher::getChildFile(QString parent, int depth, int maxdepth, int recurType)
{
    QStringList res;
    if (depth > maxdepth && maxdepth >= 0)
        return res;

    QDir dir(parent);
    if (! dir.exists())
        return res;

    if (recurType & HIDDEN)
            dir.setFilter(QDir::Dirs|QDir::Files|QDir::NoDotAndDotDot|QDir::Hidden);
    else
            dir.setFilter(QDir::Dirs|QDir::Files|QDir::NoDotAndDotDot);

    dir.setSorting(QDir::DirsFirst);

    QFileInfoList finfolist = dir.entryInfoList();
    if (finfolist.size() < 1)
        return res;

    int index = 0;
    do {
        QFileInfo finfo = finfolist.at(index);

        if ((recurType & TMPFILE) == 0 && finfo.fileName().startsWith("~"))
        {
            klog_info("文件[%s]是临时文件，略过\n", finfo.absoluteFilePath().toStdString().c_str());
            index ++;
            continue;
        }
        if ((recurType & HIDDEN) == 0 && finfo.fileName().startsWith("."))
        {
            klog_info("文件[%s]是隐藏文件，略过\n", finfo.absoluteFilePath().toStdString().c_str());
            index ++;
            continue;
        }

        if (finfo.isDir())
        {
            klog_info("文件[%s]是一个目录，略过\n", finfo.absoluteFilePath().toStdString().c_str());
            res.append(this->getChildFile(finfo.absoluteFilePath(), depth + 1, maxdepth));
            if (recurType & DIR)
                res.append(finfo.absoluteFilePath());
        }
        else
        {
            res.append(finfo.absoluteFilePath());
        }
        index ++;
    }while (index < finfolist.size());

    return res;
}

QStringList FileWatcher::getChildDir(QString parent, int depth, int maxdepth, int recurType)
{
    QStringList res;
    if (depth > maxdepth && maxdepth >= 0)
        return res;

    res.append(parent);

    QDir dir(parent);
    if (! dir.exists())
        return res;

    if (recurType & HIDDEN)
        dir.setFilter(QDir::Dirs|QDir::NoDotAndDotDot|QDir::Hidden);
    else
        dir.setFilter(QDir::Dirs|QDir::NoDotAndDotDot);

    dir.setSorting(QDir::DirsFirst);

    QFileInfoList finfolist = dir.entryInfoList();
    if (finfolist.size() < 1)
        return res;

    int index = 0;
    do {
        QFileInfo finfo = finfolist.at(index);

        if (finfo.isDir())
        {
            if ((recurType & TMPFILE) == 0 && finfo.fileName().startsWith("~"))
            {
                klog_info("文件[%s]是临时文件，略过\n", finfo.absoluteFilePath().toStdString().c_str());
                index ++;
                continue;
            }
            if ((recurType & HIDDEN) == 0 && finfo.fileName().startsWith("."))
            {
                klog_info("文件[%s]是隐藏文件，略过\n", finfo.absoluteFilePath().toStdString().c_str());
                index ++;
                continue;
            }
            res.append(this->getChildDir(finfo.absoluteFilePath(), depth + 1, maxdepth));
            res.append(finfo.absoluteFilePath());
        }
        else
            break;
        index ++;
    }while (index < finfolist.size());

    return res;
}
