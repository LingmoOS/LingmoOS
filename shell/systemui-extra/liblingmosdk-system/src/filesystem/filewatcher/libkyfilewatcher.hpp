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

#ifndef KDK_SYSTEM_FILEWATCHER_H__
#define KDK_SYSTEM_FILEWATCHER_H__

/**
 * @file libkyfilewatcher.hpp
 * @author liuyunhe (liuyunhe@kylinos.cn)
 * @brief 文件管理
 * @version 0.1
 * @date 2021-11-17
 * 
 * @copyright Copyright (c) 2021
 * @defgroup liblingmosdk-filesystem
 * @{
 * 
 */

#include "libkyfilewatcher_global.hpp"
#include <QObject>
#include <QHash>
#include <QThread>
#include <QFile>
#include <mutex>
#include <sys/inotify.h>

namespace LINGMOSDK_FILEWATCHER {

/*
 * 文件监听类型
 */
enum FileWatcherType{
    ONESHOT = 0,    // 仅报送一次事件。请注意事件发生后，监听符号会从监听列表中移除，下次再用需要重新add
    PERIODIC,
    NEVER
};

/*
 * 文件监听属性
 */
enum FileWatcherAttribute{
    ACCESS = IN_ACCESS,
    CREATE = IN_CREATE,
    MODIFY = IN_MODIFY,
    DELETE = IN_DELETE | IN_DELETE_SELF,
    ATTRIB = IN_ATTRIB,
    MOVE   = IN_MOVE | IN_MOVE_SELF,
    OPEN   = IN_OPEN,
    CLOSE  = IN_CLOSE,
    ALL = IN_ALL_EVENTS
};

/*
 * 递归类型
 */
enum RecursiveType{
    DIR = 1 << 0,
    REGULAR = 1 << 1,
    HIDDEN = 1 << 2,
    TMPFILE = 1 << 3,    //认为那些以~开头的是临时文件
    ALLFILE = HIDDEN | TMPFILE | DIR | REGULAR
};

class FileWatcher;
extern void* fileWatcherThreadRunner(void *inst);

/**
 * @brief 文件管理类
 * 主要用来增加需要监听的文件路径，增加需要监听的文件列表，从监听列表中移除指定的文件，清空监听列表，暂停文件监听，恢复文件监听
 */
class FILEWATCHER_EXPORT FileWatcher : public QObject
{
    Q_OBJECT
public:
    FileWatcher();
    ~FileWatcher();

    
    int addWatchTarget(QString url, FileWatcherType type = PERIODIC, int attr = OPEN|CLOSE|MODIFY|DELETE);

    /**
     * @brief 增加需要监听的文件路径，对于目录，默认启用递归监听子目录
     * @param url 文件路径
     * @param type 文件监听类型
     * @param attr 文件监听属性
     * @param maxdepth 指定遍历搜索的最大深度
     * @param recurType 递归类型
     */
    QStringList addWatchTargetRecursive(QString url, FileWatcherType type = PERIODIC, int attr = OPEN|CLOSE|MODIFY|DELETE, int maxdepth = 5, int recurType = DIR|REGULAR);

    
    QStringList addWatchTargetList(QStringList urlList, FileWatcherType type = PERIODIC, int attr = OPEN|CLOSE|MODIFY|DELETE);

    /**
     * @brief 增加需要监听的文件列表
     * @param urlList 需要监听的文件列表
     * @param type 文件监听类型
     * @param attr 文件监听属性
     * @param maxdepth 指定遍历搜索的最大深度
     * @param recurType 递归类型
     */
    QStringList addWatchTargetListRecursive(QStringList urlList, FileWatcherType type = PERIODIC, int attr = OPEN|CLOSE|MODIFY|DELETE, int maxdepth = 5, int recurType = DIR|REGULAR);

    int updateWatchTargetAttribute(QString url, int newattr);

    int updateWatchTargetType(QString url, FileWatcherType newtype);

    
    int removeWatchTarget(QString url);

    /**
     * @brief 从监听列表中移除指定的文件
     * @param url 指定的文件的路径
     * @param maxdepth 指定遍历搜索的最大深度
     * @param recurType 递归类型
     */
    QStringList removeWatchTargetRecursive(QString url, int maxdepth = 5, int recurType = ALLFILE);

    QStringList removeWatchTargetList(QString urlList);
    QStringList removeWatchTargetListRecursive(QString urlList, int maxdepth = 5, int recurType = ALLFILE);

    /**
     * @brief 清空监听列表
     */
    void clearWatchList();

    /**
     * @brief 暂停文件监听
     */
    void pauseWatcher();

    /**
     * @brief 恢复文件监听
     */
    void restartWatcher();

    /**
     * @brief 获取文件监听列表
     * @return QStringList
     */
    QStringList getWatchList();
    QStringList getWatchList(FileWatcherType type);
    QStringList getWatchList(int attr);

Q_SIGNALS:
    void fileCreated(QString url, QString parent);
    void fileModified(QString url, QString parent);
    void fileDeleted(QString url, QString parent);
    void fileMoveOut(QString url, QString parent);
    void fileMoveIn(QString url, QString parent);
    void fileAccessed(QString url, QString parent);
    void fileOpened(QString url, QString parent);
    void fileClosed(QString url, QString parent);
    void fileAttrChanged(QString url, QString parent);
    void fileIgnoredAutomatic(QString url, QString parent);
    void fileUnmount(QString url, QString parent);

protected:
    void sendSignal(int wfd, QString name, int mask);

private:
    Q_DISABLE_COPY(FileWatcher)

    typedef struct {
        QString url;
        FileWatcherType type;
        int attr;
        int wfd;
    }FileDescription;
    std::mutex listLocker;
    std::mutex runningLocker;       // 控制暂停与继续监听变动
    QHash<QString, FileDescription> watchList;
    QHash<int, QString> fdCacheMap;
    int watcherFd;
    bool isActived;
    bool quit;
    pthread_t *threadRunner;

    friend void* fileWatcherThreadRunner(void *inst);
    int addWatchFile(FileDescription node);
    int removeWatchFile(QString url);
    int updateWatchFileAttribute(QString url, int attr);
    int updateWatchFileType(QString url, FileWatcherType type);
    QStringList getChildFile(QString parent, int depth, int maxdepth = 20, int recurType = REGULAR);
    QStringList getChildDir(QString parent, int depth, int maxdepth = 20, int recurType = DIR);
};

}

#endif // FILEWATCHER_H

/**
 * \example lingmosdk-system/src/filesystem/filewatcher/test/kyfilewatcher-test.cpp
 * 
 */
