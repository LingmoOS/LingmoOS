/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2007-2010 Sebastian Trueg <trueg@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KINOTIFY_H_
#define KINOTIFY_H_

#include <QObject>

namespace Baloo {
    class FileIndexerConfig;
}

/**
 * A simple wrapper around inotify which only allows
 * to add folders recursively.
 *
 * Warning: moving of top-level folders is not supported and
 * results in undefined behaviour.
 */
class KInotify : public QObject
{
    Q_OBJECT

public:
    explicit KInotify(Baloo::FileIndexerConfig* config, QObject* parent = nullptr);
    ~KInotify() override;

    /**
     * Inotify events that can occur. Use with addWatch
     * to define the events that should be watched.
     *
     * These flags correspond to the native Linux inotify flags.
     */
    enum WatchEvent {
        EventAccess = 0x00000001, /**< File was accessed (read, compare inotify's IN_ACCESS) */
        EventAttributeChange = 0x00000004, /**< Metadata changed (permissions, timestamps, extended attributes, etc., compare inotify's IN_ATTRIB) */
        EventCloseWrite = 0x00000008, /**< File opened for writing was closed (compare inotify's IN_CLOSE_WRITE) */
        EventCloseRead = 0x00000010, /**< File not opened for writing was closed (compare inotify's IN_CLOSE_NOWRITE) */
        EventCreate = 0x00000100, /** File/directory created in watched directory (compare inotify's IN_CREATE) */
        EventDelete = 0x00000200, /**< File/directory deleted from watched directory (compare inotify's IN_DELETE) */
        EventDeleteSelf = 0x00000400, /**< Watched file/directory was itself deleted (compare inotify's IN_DELETE_SELF) */
        EventModify = 0x00000002, /**< File was modified (compare inotify's IN_MODIFY) */
        EventMoveSelf = 0x00000800, /**< Watched file/directory was itself moved (compare inotify's IN_MOVE_SELF) */
        EventMoveFrom = 0x00000040, /**< File moved out of watched directory (compare inotify's IN_MOVED_FROM) */
        EventMoveTo = 0x00000080, /**< File moved into watched directory (compare inotify's IN_MOVED_TO) */
        EventOpen = 0x00000020, /**< File was opened (compare inotify's IN_OPEN) */
        EventUnmount = 0x00002000, /**< Backing fs was unmounted (compare inotify's IN_UNMOUNT) */
        EventQueueOverflow = 0x00004000, /**< Event queued overflowed (compare inotify's IN_Q_OVERFLOW) */
        EventIgnored = 0x00008000, /**< File was ignored (compare inotify's IN_IGNORED) */
        EventMove = (EventMoveFrom | EventMoveTo),
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

    bool watchingPath(const QString& path) const;

    /**
     * Call this when the inotify limit has been increased.
     */
    void resetUserLimit();

public Q_SLOTS:
    bool addWatch(const QString& path, WatchEvents modes, WatchFlags flags = WatchFlags());
    bool removeWatch(const QString& path);

Q_SIGNALS:
    /**
     * Emitted if a file is accessed (KInotify::EventAccess)
     */
    void accessed(const QString& file);

    /**
     * Emitted if file attributes are changed (KInotify::EventAttributeChange)
     */
    void attributeChanged(const QString& file);

    /**
     * Emitted if FIXME (KInotify::EventCloseWrite)
     */
    void closedWrite(const QString& file);

    /**
     * Emitted if FIXME (KInotify::EventCloseRead)
     */
    void closedRead(const QString& file);

    /**
     * Emitted if a new file has been created in one of the watched
     * folders (KInotify::EventCreate)
     */
    void created(const QString& file, bool isDir);

    /**
     * Emitted if a watched file or folder has been deleted.
     * This includes files in watched folders (KInotify::EventDelete and KInotify::EventDeleteSelf)
     */
    void deleted(const QString& file, bool isDir);

    /**
     * Emitted if a watched file is modified (KInotify::EventModify)
     */
    void modified(const QString& file);

    /**
     * Emitted if a file or folder has been moved or renamed.
     *
     * \warning The moved signal will only be emitted if both the source and target folder
     * are being watched.
     */
    void moved(const QString& oldName, const QString& newName);

    /**
     * Emitted if a file is opened (KInotify::EventOpen)
     */
    void opened(const QString& file);

    /**
     * Emitted if a watched path has been unmounted (KInotify::EventUnmount)
     */
    void unmounted(const QString& file);

    /**
     * Emitted if during updating the internal watch structures (recursive watches)
     * the inotify user watch limit was reached.
     *
     * This means that not all requested paths can be watched until the user watch
     * limit is increased.
     *
     * The argument is the path being added when the limit was reached.
     *
     * This signal will only be emitted once until resetUserLimit is called.
     */
    void watchUserLimitReached(const QString& path);

    /**
     * This is emitted once watches have been installed in all the directories
     * indicated by addWatch
     */
    void installedWatches();

private Q_SLOTS:
    void slotEvent(int);
    void slotClearCookies();

private:
    class Private;
    Private* const d;

    /**
     * Recursively iterates over all files/folders inside @param path
     * (that are not excluded by the config);
     * emits created() signal for each entry (excluding @param path)
     * and installs watches for all subdirectories (including @param path)
     */
    void handleDirCreated(const QString& path);
};

#endif
