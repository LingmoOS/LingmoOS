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
#ifndef FILESYSTEMWATCHER_H
#define FILESYSTEMWATCHER_H

#include <QObject>
namespace LingmoUISearch {
class FileSystemWatcherPrivate;
class FileSystemWatcher : public QObject
{
    friend class FileSystemWatcherPrivate;
    Q_OBJECT
public:
    enum WatchEvent {
            EventAccess          = 0x00000001, /**< File was accessed (read, compare inotify's IN_ACCESS) */
            EventModify          = 0x00000002, /**< File was modified (compare inotify's IN_MODIFY) */
            EventAttributeChange = 0x00000004, /**< Metadata changed (permissions, timestamps, extended attributes, etc., compare inotify's IN_ATTRIB) */
            EventCloseWrite      = 0x00000008, /**< File opened for writing was closed (compare inotify's IN_CLOSE_WRITE) */
            EventCloseRead       = 0x00000010, /**< File not opened for writing was closed (compare inotify's IN_CLOSE_NOWRITE) */
            EventClose = (EventCloseWrite | EventCloseRead),
            EventOpen            = 0x00000020, /**< File was opened (compare inotify's IN_OPEN) */
            EventMoveFrom        = 0x00000040, /**< File moved out of watched directory (compare inotify's IN_MOVED_FROM) */
            EventMoveTo          = 0x00000080, /**< File moved into watched directory (compare inotify's IN_MOVED_TO) */
            EventMove = (EventMoveFrom | EventMoveTo),
            EventCreate          = 0x00000100, /** File/directory created in watched directory (compare inotify's IN_CREATE) */
            EventDelete          = 0x00000200, /**< File/directory deleted from watched directory (compare inotify's IN_DELETE) */
            EventDeleteSelf      = 0x00000400, /**< Watched file/directory was itself deleted (compare inotify's IN_DELETE_SELF) */
            EventMoveSelf        = 0x00000800, /**< Watched file/directory was itself moved (compare inotify's IN_MOVE_SELF) */
            EventUnmount         = 0x00002000, /**< Backing fs was unmounted (compare inotify's IN_UNMOUNT) */
            EventQueueOverflow   = 0x00004000, /**< Event queued overflowed (compare inotify's IN_Q_OVERFLOW) */
            EventIgnored         = 0x00008000, /**< File was ignored (compare inotify's IN_IGNORED) */
            EventAll = (EventAccess |
                        EventAttributeChange |
                        EventCloseWrite |
                        EventCloseRead |
                        EventCreate |
                        EventDelete |
                        EventDeleteSelf |
                        EventModify |
                        EventMoveSelf |
                        EventMoveFrom |
                        EventMoveTo |
                        EventOpen),
        };
        Q_DECLARE_FLAGS(WatchEvents, WatchEvent)
    /**
     * Watch flags
     *
     * These flags correspond to the native Linux inotify flags.
     */
    enum WatchFlag {
        FlagOnlyDir = 0x01000000, /**< Only watch the path if it is a directory (IN_ONLYDIR) */
        FlagDoNotFollow = 0x02000000, /**< Don't follow a sym link (IN_DONT_FOLLOW) */
        FlagOneShot = 0x80000000, /**< Only send event once (IN_ONESHOT) */
        FlagExclUnlink = 0x04000000, /**< Do not generate events for unlinked files (IN_EXCL_UNLINK) */
    };
    Q_DECLARE_FLAGS(WatchFlags, WatchFlag)

    /**
     * @brief FileSystemWatcher
     * @param recursive if the watcher should watch the paths recursively. .
     * @param events Events that shoude be concerned,default: FileSystemWatcher::EventMove | FileSystemWatcher::EventMoveSelf |
                                                              FileSystemWatcher::EventCreate | FileSystemWatcher::EventDelete |
                                                              FileSystemWatcher::EventDeleteSelf | FileSystemWatcher::EventUnmount |
                                                              FileSystemWatcher::EventModify | FileSystemWatcher::EventAttributeChange
     * @param flags Watch flags.
     * @param parent
     */
    explicit FileSystemWatcher(bool recursive = true,
                               WatchEvents events = WatchEvents(FileSystemWatcher::EventMove | FileSystemWatcher::EventMoveSelf |
                                                                FileSystemWatcher::EventCreate | FileSystemWatcher::EventDelete |
                                                                FileSystemWatcher::EventDeleteSelf | FileSystemWatcher::EventUnmount |
                                                                FileSystemWatcher::EventModify),
                               WatchFlags flags = WatchFlags(),
                               QObject *parent = nullptr);

    ~FileSystemWatcher();

    FileSystemWatcher(FileSystemWatcher &) = delete;
    FileSystemWatcher &operator =(const FileSystemWatcher &) = delete;

//    bool isWatchingPath(const QString& path) const;

public Q_SLOTS:
    void addWatch(const QStringList &pathList);
    void addWatch(const QString &path);
    void addWatchWithBlackList(const QStringList &pathList, const QStringList &blackList);
    QStringList removeWatch(const QString &path);
    void clearAll();

Q_SIGNALS:

    /**
     * Emitted if a file is accessed (FileSystemWatcher::EventAccess)
     */
    void accessed(const QString& path);

    /**
     * Emitted if a watched file is modified (FileSystemWatcher::EventModify)
     */
    void modified(const QString& path);

    /**
     * Emitted if file attributes are changed (FileSystemWatcher::EventAttributeChange)
     */
    void attributeChanged(const QString& path);

    void closedWrite(const QString& path);

    void closedRead(const QString& path);

    /**
     * Emitted if a new file has been created in one of the watched
     * folders (FileSystemWatcher::EventCreate)
     */
    void created(const QString& path, bool isDir);

    /**
     * Emitted if a watched file or folder has been deleted.
     * This includes files in watched folders (FileSystemWatcher::EventDelete and FileSystemWatcher::EventDeleteSelf)
     */
    void deleted(const QString& path, bool isDir);

    /**
     * Emitted if a file or folder has been moved or renamed.
     */
    void moved(const QString& path, bool isDir);

    /**
     * Emitted if a file is opened (FileSystemWatcher::EventOpen)
     */
    void opened(const QString& path);

    /**
     * Emitted if a watched path has been unmounted (FileSystemWatcher::EventUnmount)
     */
    void unmounted(const QString& path, bool isDir);

    /**
     * Emitted if a file is moved to one of the watched folders.
     * Note:if the new file moved here overwrited a file already existed, there will not be a 'deleted' signal.
     */
    void moveTo(const QString& path, bool isDir);
    /**
     * Emitted when the current user's file permissions changed(FileSystemWatcher::EventAttributeChange)
     */
    void accessibleChanged(const QString& path, bool isDir, bool accessable);
private Q_SLOTS:
    void eventProcess(int socket);
    QStringList traverse(QString &path);

private:

    FileSystemWatcherPrivate* d = nullptr;

};
}
#endif // FILESYSTEMWATCHER_H
