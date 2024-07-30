/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2007-2010 Sebastian Trueg <trueg@kde.org>
    SPDX-FileCopyrightText: 2012-2014 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kinotify.h"
#include "fileindexerconfig.h"
#include "filtereddiriterator.h"
#include "baloodebug.h"

#include <QSocketNotifier>
#include <QHash>
#include <QFile>
#include <QTimer>
#include <QDeadlineTimer>
#include <QPair>

#include <sys/inotify.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>
#include <dirent.h>

namespace
{
QByteArray normalizeTrailingSlash(QByteArray&& path)
{
    if (!path.endsWith('/')) {
        path.append('/');
    }
    return path;
}

QByteArray concatPath(const QByteArray& p1, const QByteArray& p2)
{
    QByteArray p(p1);
    if (p.isEmpty() || (!p2.isEmpty() && p[p.length() - 1] != '/')) {
        p.append('/');
    }
    p.append(p2);
    return p;
}
}

class KInotify::Private
{
public:
    Private(KInotify* parent)
        : userLimitReachedSignaled(false)
        , config(nullptr)
        , m_inotifyFd(-1)
        , m_notifier(nullptr)
        , q(parent) {
    }

    ~Private() {
        close();
    }

    struct MovedFileCookie {
        QDeadlineTimer deadline;
        QByteArray path;
        WatchFlags flags;
    };

    QHash<int, MovedFileCookie> cookies;
    QTimer cookieExpireTimer;
    // This variable is set to true if the watch limit is reached, and reset when it is raised
    bool userLimitReachedSignaled;

    // url <-> wd mappings
    QHash<int, QByteArray> watchPathHash;
    QHash<QByteArray, int> pathWatchHash;

    bool parentWatched(const QByteArray& path)
    {
        auto parent = path.chopped(1);
        if (auto index = parent.lastIndexOf('/'); index > 0) {
            parent.truncate(index + 1);
            return pathWatchHash.contains(parent);
        }
        return false;
    }

    Baloo::FileIndexerConfig* config;
    QStringList m_paths;
    std::unique_ptr<Baloo::FilteredDirIterator> m_dirIter;

    // FIXME: only stored from the last addWatch call
    WatchEvents mode;
    WatchFlags flags;

    int inotify() {
        if (m_inotifyFd < 0) {
            open();
        }
        return m_inotifyFd;
    }

    void close() {
        delete m_notifier;
        m_notifier = nullptr;

        ::close(m_inotifyFd);
        m_inotifyFd = -1;
    }

    bool addWatch(const QString& path) {
        WatchEvents newMode = mode;
        WatchFlags newFlags = flags;

        // we always need the unmount event to maintain our path hash
        const int mask = newMode | newFlags | EventUnmount | FlagExclUnlink;

        const QByteArray encpath = normalizeTrailingSlash(QFile::encodeName(path));
        int wd = inotify_add_watch(inotify(), encpath.data(), mask);
        if (wd > 0) {
//             qCDebug(BALOO) << "Successfully added watch for" << path << watchPathHash.count();
            watchPathHash.insert(wd, encpath);
            pathWatchHash.insert(encpath, wd);
            return true;
        } else {
            qCDebug(BALOO) << "Failed to create watch for" << path << strerror(errno);
            //If we could not create the watch because we have hit the limit, try raising it.
            if (errno == ENOSPC) {
                //If we can't, fall back to signalling
                qCDebug(BALOO) << "User limit reached. Count: " << watchPathHash.count();
                userLimitReachedSignaled = true;
                Q_EMIT q->watchUserLimitReached(path);
            }
            return false;
        }
    }

    void removeWatch(int wd) {
        pathWatchHash.remove(watchPathHash.take(wd));
        inotify_rm_watch(inotify(), wd);
    }

    /**
     * Add one watch and call oneself asynchronously
     */
    void _k_addWatches()
    {
        // It is much faster to add watches in batches instead of adding each one
        // asynchronously. Try out the inotify test to compare results.
        for (int i = 0; i < 1000; i++) {
            if (userLimitReachedSignaled) {
                return;
            }
            if (!m_dirIter || m_dirIter->next().isEmpty()) {
                if (!m_paths.isEmpty()) {
                    m_dirIter = std::make_unique<Baloo::FilteredDirIterator>(config, m_paths.takeFirst(), Baloo::FilteredDirIterator::DirsOnly);
                } else {
                    m_dirIter = nullptr;
                    break;
                }
            } else {
                addWatch(m_dirIter->filePath());
            }
        }

        // asynchronously add the next batch
        if (m_dirIter) {
            QMetaObject::invokeMethod(q, [this]() {
                this->_k_addWatches();
            }, Qt::QueuedConnection);
        }
        else {
            Q_EMIT q->installedWatches();
        }
    }

private:
    void open() {
        m_inotifyFd = inotify_init();
        delete m_notifier;
        if (m_inotifyFd > 0) {
            fcntl(m_inotifyFd, F_SETFD, FD_CLOEXEC);
            m_notifier = new QSocketNotifier(m_inotifyFd, QSocketNotifier::Read);
            connect(m_notifier, &QSocketNotifier::activated, q, &KInotify::slotEvent);
        } else {
            Q_ASSERT_X(0, "kinotify", "Failed to initialize inotify");
        }
    }

    int m_inotifyFd;
    QSocketNotifier* m_notifier;

    KInotify* q;
};

KInotify::KInotify(Baloo::FileIndexerConfig* config, QObject* parent)
    : QObject(parent)
    , d(new Private(this))
{
    d->config = config;
    // 1 second is more than enough time for the EventMoveTo event to occur
    // after the EventMoveFrom event has occurred
    d->cookieExpireTimer.setInterval(1000);
    d->cookieExpireTimer.setSingleShot(true);
    connect(&d->cookieExpireTimer, &QTimer::timeout, this, &KInotify::slotClearCookies);
}

KInotify::~KInotify()
{
    delete d;
}

bool KInotify::watchingPath(const QString& path) const
{
    const QByteArray p = normalizeTrailingSlash(QFile::encodeName(path));
    return d->pathWatchHash.contains(p);
}

void KInotify::resetUserLimit()
{
    d->userLimitReachedSignaled = false;
}

bool KInotify::addWatch(const QString& path, WatchEvents mode, WatchFlags flags)
{
//    qCDebug(BALOO) << path;

    d->mode = mode;
    d->flags = flags;
    d->m_paths << path;
    // If the inotify user limit has been signaled,
    // there will be no watchInstalled signal
    if (d->userLimitReachedSignaled) {
        return false;
    }

    QMetaObject::invokeMethod(this, [this]() {
        this->d->_k_addWatches();
    }, Qt::QueuedConnection);
    return true;
}

bool KInotify::removeWatch(const QString& path)
{
    // Stop all of the iterators which contain path
    QMutableListIterator<QString> iter(d->m_paths);
    while (iter.hasNext()) {
        if (iter.next().startsWith(path)) {
            iter.remove();
        }
    }
    if (d->m_dirIter) {
        if (d->m_dirIter->filePath().startsWith(path)) {
            d->m_dirIter = nullptr;
        }
    }

    // Remove all the watches
    QByteArray encodedPath(QFile::encodeName(path));
    auto it = d->watchPathHash.begin();
    while (it != d->watchPathHash.end()) {
        if (it.value().startsWith(encodedPath)) {
            inotify_rm_watch(d->inotify(), it.key());
            d->pathWatchHash.remove(it.value());
            it = d->watchPathHash.erase(it);
        } else {
            ++it;
        }
    }
    return true;
}

void KInotify::handleDirCreated(const QString& path)
{
    Baloo::FilteredDirIterator it(d->config, path);
    // First entry is the directory itself (if not excluded)
    if (!it.next().isEmpty()) {
        d->addWatch(it.filePath());
    }
    while (!it.next().isEmpty()) {
        Q_EMIT created(it.filePath(), it.fileInfo().isDir());
        if (it.fileInfo().isDir()) {
            d->addWatch(it.filePath());
        }
    }
}

void KInotify::slotEvent(int socket)
{
    int avail;
    if (ioctl(socket, FIONREAD, &avail) == EINVAL) {
        qCDebug(BALOO) << "Did not receive an entire inotify event.";
        return;
    }

    char* buffer = (char*)malloc(avail);

    const int len = read(socket, buffer, avail);
    Q_ASSERT(len == avail);

    // deadline for MoveFrom events without matching MoveTo event
    QDeadlineTimer deadline(QDeadlineTimer::Forever);

    int i = 0;
    while (i < len) {
        const struct inotify_event* event = (struct inotify_event*)&buffer[i];

        QByteArray path;

        // Overflow happens sometimes if we process the events too slowly
        if (event->wd < 0 && (event->mask & EventQueueOverflow)) {
            qCWarning(BALOO) << "Inotify - too many event - Overflowed";
            free(buffer);
            return;
        }

        // the event name only contains an interesting value if we get an event for a file/folder inside
        // a watched folder. Otherwise we should ignore it
        if (event->mask & (EventDeleteSelf | EventMoveSelf)) {
            path = d->watchPathHash.value(event->wd);
        } else {
            // we cannot use event->len here since it contains the size of the buffer and not the length of the string
            const QByteArray eventName = QByteArray::fromRawData(event->name, qstrnlen(event->name, event->len));
            const QByteArray hashedPath = d->watchPathHash.value(event->wd);
            path = concatPath(hashedPath, eventName);
            if (event->mask & IN_ISDIR) {
                path = normalizeTrailingSlash(std::move(path));
            }
        }

        Q_ASSERT(!path.isEmpty() || event->mask & EventIgnored);
        Q_ASSERT(path != "/" || event->mask & EventIgnored  || event->mask & EventUnmount);

        // All events which need a decoded path, i.e. everything
        // but EventMoveFrom | EventQueueOverflow | EventIgnored
        uint32_t fileEvents = EventAll & ~(EventMoveFrom | EventQueueOverflow | EventIgnored);
        const QString fname = (event->mask & fileEvents) ? QFile::decodeName(path) : QString();

        // now signal the event
        if (event->mask & EventAccess) {
//            qCDebug(BALOO) << path << "EventAccess";
            Q_EMIT accessed(fname);
        }
        if (event->mask & EventAttributeChange) {
//            qCDebug(BALOO) << path << "EventAttributeChange";
            Q_EMIT attributeChanged(fname);
        }
        if (event->mask & EventCloseWrite) {
//            qCDebug(BALOO) << path << "EventCloseWrite";
            Q_EMIT closedWrite(fname);
        }
        if (event->mask & EventCloseRead) {
//            qCDebug(BALOO) << path << "EventCloseRead";
            Q_EMIT closedRead(fname);
        }
        if (event->mask & EventCreate) {
//            qCDebug(BALOO) << path << "EventCreate";
            Q_EMIT created(fname, event->mask & IN_ISDIR);
            if (event->mask & IN_ISDIR) {
                // Files/directories inside the new directory may be created before the watch
                // is installed. Ensure created events for all children are issued at least once
                handleDirCreated(fname);
            }
        }
        if (event->mask & EventDeleteSelf) {
//            qCDebug(BALOO) << path << "EventDeleteSelf";
            d->removeWatch(event->wd);
            Q_EMIT deleted(fname, true);
        }
        if (event->mask & EventDelete) {
//            qCDebug(BALOO) << path << "EventDelete";
            // we watch all folders recursively. Thus, folder removing is reported in DeleteSelf.
            if (!(event->mask & IN_ISDIR)) {
                Q_EMIT deleted(fname, false);
            }
        }
        if (event->mask & EventModify) {
//            qCDebug(BALOO) << path << "EventModify";
            Q_EMIT modified(fname);
        }
        if (event->mask & EventMoveSelf) {
//            qCDebug(BALOO) << path << "EventMoveSelf";
            // Problematic if the parent is not watched, otherwise
            // handled by MoveFrom/MoveTo from the parent
            if (!d->parentWatched(path))
                qCWarning(BALOO) << path << "EventMoveSelf: THIS CASE MAY NOT BE HANDLED PROPERLY!";
        }
        if (event->mask & EventMoveFrom) {
//            qCDebug(BALOO) << path << "EventMoveFrom";
            if (deadline.isForever()) {
                deadline = QDeadlineTimer(1000); // 1 second
            }
            d->cookies[event->cookie] = Private::MovedFileCookie{ deadline, path, WatchFlags(event->mask) };
        }
        if (event->mask & EventMoveTo) {
            // check if we have a cookie for this one
            if (d->cookies.contains(event->cookie)) {
                const QByteArray oldPath = d->cookies.take(event->cookie).path;

                // update the path cache
                if (event->mask & IN_ISDIR) {
                    auto it = d->pathWatchHash.find(oldPath);
                    if (it != d->pathWatchHash.end()) {
//                        qCDebug(BALOO) << oldPath << path;
                        const int wd = it.value();
                        d->watchPathHash[wd] = path;
                        d->pathWatchHash.erase(it);
                        d->pathWatchHash.insert(path, wd);
                    }
                }
//                qCDebug(BALOO) << oldPath << "EventMoveTo" << path;
                Q_EMIT moved(QFile::decodeName(oldPath), fname);
            } else {
//                qCDebug(BALOO) << "No cookie for move information of" << path << "simulating new file event";
                Q_EMIT created(fname, event->mask & IN_ISDIR);
                if (event->mask & IN_ISDIR) {
                    handleDirCreated(fname);
                }
            }
        }
        if (event->mask & EventOpen) {
//            qCDebug(BALOO) << path << "EventOpen";
            Q_EMIT opened(fname);
        }
        if (event->mask & EventUnmount) {
//            qCDebug(BALOO) << path << "EventUnmount. removing from path hash";
            if (event->mask & IN_ISDIR) {
                d->removeWatch(event->wd);
            }
            // This is present because a unmount event is sent by inotify after unmounting, by
            // which time the watches have already been removed.
            if (path != "/") {
                Q_EMIT unmounted(fname);
            }
        }
        if (event->mask & EventIgnored) {
//             qCDebug(BALOO) << path << "EventIgnored";
        }

        i += sizeof(struct inotify_event) + event->len;
    }

    if (d->cookies.empty()) {
        d->cookieExpireTimer.stop();
    } else {
        if (!d->cookieExpireTimer.isActive()) {
            d->cookieExpireTimer.start();
        }
    }

    if (len < 0) {
        qCDebug(BALOO) << "Failed to read event.";
    }

    free(buffer);
}

void KInotify::slotClearCookies()
{
    auto now = QDeadlineTimer::current();

    auto it = d->cookies.begin();
    while (it != d->cookies.end()) {
        if (now > (*it).deadline) {
            const QString fname = QFile::decodeName((*it).path);
            removeWatch(fname);
            Q_EMIT deleted(fname, (*it).flags & IN_ISDIR);
            it = d->cookies.erase(it);
        } else {
            ++it;
        }
    }

    if (!d->cookies.empty()) {
        d->cookieExpireTimer.start();
    }
}

#include "moc_kinotify.cpp"
