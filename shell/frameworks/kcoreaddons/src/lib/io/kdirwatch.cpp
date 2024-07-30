/* This file is part of the KDE libraries
   SPDX-FileCopyrightText: 1998 Sven Radej <sven@lisa.exp.univie.ac.at>
   SPDX-FileCopyrightText: 2006 Dirk Mueller <mueller@kde.org>
   SPDX-FileCopyrightText: 2007 Flavio Castelli <flavio.castelli@gmail.com>
   SPDX-FileCopyrightText: 2008 Rafal Rzepecki <divided.mind@gmail.com>
   SPDX-FileCopyrightText: 2010 David Faure <faure@kde.org>
   SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

   SPDX-License-Identifier: LGPL-2.0-only
*/

// CHANGES:
// Jul 30, 2008 - Don't follow symlinks when recursing to avoid loops (Rafal)
// Aug 6,  2007 - KDirWatch::WatchModes support complete, flags work fine also
// when using FAMD (Flavio Castelli)
// Aug 3,  2007 - Handled KDirWatch::WatchModes flags when using inotify, now
// recursive and file monitoring modes are implemented (Flavio Castelli)
// Jul 30, 2007 - Substituted addEntry boolean params with KDirWatch::WatchModes
// flag (Flavio Castelli)
// Oct 4,  2005 - Inotify support (Dirk Mueller)
// February 2002 - Add file watching and remote mount check for STAT
// Mar 30, 2001 - Native support for Linux dir change notification.
// Jan 28, 2000 - Usage of FAM service on IRIX (Josef.Weidendorfer@in.tum.de)
// May 24. 1998 - List of times introduced, and some bugs are fixed. (sven)
// May 23. 1998 - Removed static pointer - you can have more instances.
// It was Needed for KRegistry. KDirWatch now emits signals and doesn't
// call (or need) KFM. No more URL's - just plain paths. (sven)
// Mar 29. 1998 - added docs, stop/restart for particular Dirs and
// deep copies for list of dirs. (sven)
// Mar 28. 1998 - Created.  (sven)

#include "kdirwatch.h"
#include "kcoreaddons_debug.h"
#include "kdirwatch_p.h"
#include "kfilesystemtype.h"
#include "knetworkmounts.h"

#include <io/config-kdirwatch.h>

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QLoggingCategory>
#include <QSocketNotifier>
#include <QThread>
#include <QThreadStorage>
#include <QTimer>
#include <assert.h>
#include <cerrno>
#include <sys/stat.h>

#include <qplatformdefs.h> // QT_LSTAT, QT_STAT, QT_STATBUF

#include <stdlib.h>
#include <string.h>

#if HAVE_SYS_INOTIFY_H
#include <fcntl.h>
#include <sys/inotify.h>
#include <unistd.h>

#ifndef IN_DONT_FOLLOW
#define IN_DONT_FOLLOW 0x02000000
#endif

#ifndef IN_ONLYDIR
#define IN_ONLYDIR 0x01000000
#endif

// debug
#include <sys/ioctl.h>

#include <sys/utsname.h>

#endif // HAVE_SYS_INOTIFY_H

Q_DECLARE_LOGGING_CATEGORY(KDIRWATCH)
// logging category for this framework, default: log stuff >= warning
Q_LOGGING_CATEGORY(KDIRWATCH, "kf.coreaddons.kdirwatch", QtWarningMsg)

// set this to true for much more verbose debug output
static bool s_verboseDebug = false;

static QThreadStorage<KDirWatchPrivate *> dwp_self;
static KDirWatchPrivate *createPrivate()
{
    if (!dwp_self.hasLocalData()) {
        dwp_self.setLocalData(new KDirWatchPrivate);
    }
    return dwp_self.localData();
}
static void destroyPrivate()
{
    dwp_self.localData()->deleteLater();
    dwp_self.setLocalData(nullptr);
}

// Convert a string into a watch Method
static KDirWatch::Method methodFromString(const QByteArray &method)
{
    if (method == "Stat") {
        return KDirWatch::Stat;
    } else if (method == "QFSWatch") {
        return KDirWatch::QFSWatch;
    } else {
#if HAVE_SYS_INOTIFY_H
        // inotify supports delete+recreate+modify, which QFSWatch doesn't support
        return KDirWatch::INotify;
#else
        return KDirWatch::QFSWatch;
#endif
    }
}

static const char *methodToString(KDirWatch::Method method)
{
    switch (method) {
    case KDirWatch::INotify:
        return "INotify";
    case KDirWatch::Stat:
        return "Stat";
    case KDirWatch::QFSWatch:
        return "QFSWatch";
    }
    // not reached
    return nullptr;
}

static const char s_envNfsPoll[] = "KDIRWATCH_NFSPOLLINTERVAL";
static const char s_envPoll[] = "KDIRWATCH_POLLINTERVAL";
static const char s_envMethod[] = "KDIRWATCH_METHOD";
static const char s_envNfsMethod[] = "KDIRWATCH_NFSMETHOD";

//
// Class KDirWatchPrivate (singleton)
//

/* All entries (files/directories) to be watched in the
 * application (coming from multiple KDirWatch instances)
 * are registered in a single KDirWatchPrivate instance.
 *
 * At the moment, the following methods for file watching
 * are supported:
 * - Polling: All files to be watched are polled regularly
 *   using stat (more precise: QFileInfo.lastModified()).
 *   The polling frequency is determined from global kconfig
 *   settings, defaulting to 500 ms for local directories
 *   and 5000 ms for remote mounts
 * - FAM (File Alternation Monitor): first used on IRIX, SGI
 *   has ported this method to LINUX. It uses a kernel part
 *   (IMON, sending change events to /dev/imon) and a user
 *   level daemon (fam), to which applications connect for
 *   notification of file changes. For NFS, the fam daemon
 *   on the NFS server machine is used; if IMON is not built
 *   into the kernel, fam uses polling for local files.
 * - INOTIFY: In LINUX 2.6.13, inode change notification was
 *   introduced. You're now able to watch arbitrary inode's
 *   for changes, and even get notification when they're
 *   unmounted.
 */

KDirWatchPrivate::KDirWatchPrivate()
    : m_statRescanTimer()
    , freq(3600000)
    , // 1 hour as upper bound
    statEntries(0)
    , delayRemove(false)
    , rescan_all(false)
    , rescan_timer()
    ,
#if HAVE_SYS_INOTIFY_H
    mSn(nullptr)
    ,
#endif
    _isStopped(false)
{
    // Debug unittest on CI
    if (qAppName() == QLatin1String("kservicetest") || qAppName() == QLatin1String("filetypestest")) {
        s_verboseDebug = true;
    }
    m_statRescanTimer.setObjectName(QStringLiteral("KDirWatchPrivate::timer"));
    connect(&m_statRescanTimer, &QTimer::timeout, this, &KDirWatchPrivate::slotRescan);

    m_nfsPollInterval = qEnvironmentVariableIsSet(s_envNfsPoll) ? qEnvironmentVariableIntValue(s_envNfsPoll) : 5000;
    m_PollInterval = qEnvironmentVariableIsSet(s_envPoll) ? qEnvironmentVariableIntValue(s_envPoll) : 500;

    m_preferredMethod = methodFromString(qEnvironmentVariableIsSet(s_envMethod) ? qgetenv(s_envMethod) : "inotify");
    // The nfs method defaults to the normal (local) method
    m_nfsPreferredMethod = methodFromString(qEnvironmentVariableIsSet(s_envNfsMethod) ? qgetenv(s_envNfsMethod) : "Stat");

    QList<QByteArray> availableMethods;

    availableMethods << "Stat";

    // used for inotify
    rescan_timer.setObjectName(QStringLiteral("KDirWatchPrivate::rescan_timer"));
    rescan_timer.setSingleShot(true);
    connect(&rescan_timer, &QTimer::timeout, this, &KDirWatchPrivate::slotRescan);

#if HAVE_SYS_INOTIFY_H
    m_inotify_fd = inotify_init();
    supports_inotify = m_inotify_fd > 0;

    if (!supports_inotify) {
        qCDebug(KDIRWATCH) << "Can't use Inotify, kernel doesn't support it:" << strerror(errno);
    } else {
        availableMethods << "INotify";
        (void)fcntl(m_inotify_fd, F_SETFD, FD_CLOEXEC);

        mSn = new QSocketNotifier(m_inotify_fd, QSocketNotifier::Read, this);
        connect(mSn, &QSocketNotifier::activated, this, &KDirWatchPrivate::inotifyEventReceived);
    }
#endif
#if HAVE_QFILESYSTEMWATCHER
    availableMethods << "QFileSystemWatcher";
    fsWatcher = nullptr;
#endif

    qCDebug(KDIRWATCH) << "Available methods: " << availableMethods << "preferred=" << methodToString(m_preferredMethod);
}

// This is called on app exit (deleted by QThreadStorage)
KDirWatchPrivate::~KDirWatchPrivate()
{
    m_statRescanTimer.stop();

    // Unset us as d pointer. This indicates to the KDirWatch that the private has already been destroyed and therefore
    // needs no additional cleanup from its destructor.
    for (auto it = m_mapEntries.begin(); it != m_mapEntries.end(); it++) {
        auto &entry = it.value();
        for (auto &client : entry.m_clients) {
            client.instance->d = nullptr;
        }
    }
    // Also cover all instance that hold a reference to us (even when they don't have an entry)
    for (auto &referenceObject : m_referencesObjects) {
        referenceObject->d = nullptr;
    }

#if HAVE_SYS_INOTIFY_H
    if (supports_inotify) {
        QT_CLOSE(m_inotify_fd);
    }
#endif
#if HAVE_QFILESYSTEMWATCHER
    delete fsWatcher;
#endif
}

void KDirWatchPrivate::inotifyEventReceived()
{
#if HAVE_SYS_INOTIFY_H
    if (!supports_inotify) {
        return;
    }

    int pending = -1;
    int offsetStartRead = 0; // where we read into buffer
    char buf[8192];
    assert(m_inotify_fd > -1);
    ioctl(m_inotify_fd, FIONREAD, &pending);

    while (pending > 0) {
        const int bytesToRead = qMin<int>(pending, sizeof(buf) - offsetStartRead);

        int bytesAvailable = read(m_inotify_fd, &buf[offsetStartRead], bytesToRead);
        pending -= bytesAvailable;
        bytesAvailable += offsetStartRead;
        offsetStartRead = 0;

        int offsetCurrent = 0;
        while (bytesAvailable >= int(sizeof(struct inotify_event))) {
            const struct inotify_event *const event = reinterpret_cast<inotify_event *>(&buf[offsetCurrent]);

            if (event->mask & IN_Q_OVERFLOW) {
                qCWarning(KDIRWATCH) << "Inotify Event queue overflowed, check max_queued_events value";
                return;
            }

            const int eventSize = sizeof(struct inotify_event) + event->len;
            if (bytesAvailable < eventSize) {
                break;
            }

            bytesAvailable -= eventSize;
            offsetCurrent += eventSize;

            QString path;
            // strip trailing null chars, see inotify_event documentation
            // these must not end up in the final QString version of path
            int len = event->len;
            while (len > 1 && !event->name[len - 1]) {
                --len;
            }
            QByteArray cpath(event->name, len);
            if (len) {
                path = QFile::decodeName(cpath);
            }

            if (!path.isEmpty() && isNoisyFile(cpath.data())) {
                continue;
            }

            // Is set to true if the new event is a directory, false otherwise. This prevents a stat call in clientsForFileOrDir
            const bool isDir = (event->mask & (IN_ISDIR));

            Entry *e = m_inotify_wd_to_entry.value(event->wd);
            if (!e) {
                continue;
            }
            const bool wasDirty = e->dirty;
            e->dirty = true;

            const QString tpath = e->path + QLatin1Char('/') + path;

            qCDebug(KDIRWATCH).nospace() << "got event " << inotifyEventName(event) << " for entry " << e->path
                                         << (event->mask & IN_ISDIR ? " [directory] " : " [file] ") << path;

            if (event->mask & IN_DELETE_SELF) {
                e->m_status = NonExistent;
                m_inotify_wd_to_entry.remove(e->wd);
                e->wd = -1;
                e->m_ctime = invalid_ctime;
                emitEvent(e, Deleted);
                // If the parent dir was already watched, tell it something changed
                Entry *parentEntry = entry(e->parentDirectory());
                if (parentEntry) {
                    parentEntry->dirty = true;
                }
                // Add entry to parent dir to notice if the entry gets recreated
                addEntry(nullptr, e->parentDirectory(), e, true /*isDir*/);
            }
            if (event->mask & IN_IGNORED) {
                // Causes bug #207361 with kernels 2.6.31 and 2.6.32!
                // e->wd = -1;
            }
            if (event->mask & (IN_CREATE | IN_MOVED_TO)) {
                Entry *sub_entry = e->findSubEntry(tpath);

                qCDebug(KDIRWATCH) << "-->got CREATE signal for" << (tpath) << "sub_entry=" << sub_entry;

                if (sub_entry) {
                    // We were waiting for this new file/dir to be created
                    sub_entry->dirty = true;
                    rescan_timer.start(0); // process this asap, to start watching that dir
                } else if (e->isDir && !e->m_clients.empty()) {
                    const QList<const Client *> clients = e->inotifyClientsForFileOrDir(isDir);
                    // See discussion in addEntry for why we don't addEntry for individual
                    // files in WatchFiles mode with inotify.
                    if (isDir) {
                        for (const Client *client : clients) {
                            addEntry(client->instance, tpath, nullptr, isDir, isDir ? client->m_watchModes : KDirWatch::WatchDirOnly);
                        }
                    }
                    if (!clients.isEmpty()) {
                        emitEvent(e, Created, tpath);
                        qCDebug(KDIRWATCH).nospace() << clients.count() << " instance(s) monitoring the new " << (isDir ? "dir " : "file ") << tpath;
                    }
                    e->m_pendingFileChanges.append(e->path);
                    if (!rescan_timer.isActive()) {
                        rescan_timer.start(m_PollInterval); // singleshot
                    }
                }
            }
            if (event->mask & (IN_DELETE | IN_MOVED_FROM)) {
                if ((e->isDir) && (!e->m_clients.empty())) {
                    // A file in this directory has been removed.  It wasn't an explicitly
                    // watched file as it would have its own watch descriptor, so
                    // no addEntry/ removeEntry bookkeeping should be required.  Emit
                    // the event immediately if any clients are interested.
                    const KDirWatch::WatchModes flag = isDir ? KDirWatch::WatchSubDirs : KDirWatch::WatchFiles;
                    int counter = std::count_if(e->m_clients.cbegin(), e->m_clients.cend(), [flag](const Client &client) {
                        return client.m_watchModes & flag;
                    });

                    if (counter != 0) {
                        emitEvent(e, Deleted, tpath);
                    }
                }
            }
            if (event->mask & (IN_MODIFY | IN_ATTRIB)) {
                if ((e->isDir) && (!e->m_clients.empty())) {
                    // A file in this directory has been changed.  No
                    // addEntry/ removeEntry bookkeeping should be required.
                    // Add the path to the list of pending file changes if
                    // there are any interested clients.
                    // QT_STATBUF stat_buf;
                    // QByteArray tpath = QFile::encodeName(e->path+'/'+path);
                    // QT_STAT(tpath, &stat_buf);
                    // bool isDir = S_ISDIR(stat_buf.st_mode);

                    // The API doc is somewhat vague as to whether we should emit
                    // dirty() for implicitly watched files when WatchFiles has
                    // not been specified - we'll assume they are always interested,
                    // regardless.
                    // Don't worry about duplicates for the time
                    // being; this is handled in slotRescan.
                    e->m_pendingFileChanges.append(tpath);
                    // Avoid stat'ing the directory if only an entry inside it changed.
                    e->dirty = (wasDirty || (path.isEmpty() && (event->mask & IN_ATTRIB)));
                }
            }

            if (!rescan_timer.isActive()) {
                rescan_timer.start(m_PollInterval); // singleshot
            }
        }
        if (bytesAvailable > 0) {
            // copy partial event to beginning of buffer
            memmove(buf, &buf[offsetCurrent], bytesAvailable);
            offsetStartRead = bytesAvailable;
        }
    }
#endif
}

KDirWatchPrivate::Entry::~Entry()
{
}

/* In inotify mode, only entries which are marked dirty are scanned.
 * We first need to mark all yet nonexistent, but possible created
 * entries as dirty...
 */
void KDirWatchPrivate::Entry::propagate_dirty()
{
    for (Entry *sub_entry : std::as_const(m_entries)) {
        if (!sub_entry->dirty) {
            sub_entry->dirty = true;
            sub_entry->propagate_dirty();
        }
    }
}

/* A KDirWatch instance is interested in getting events for
 * this file/Dir entry.
 */
void KDirWatchPrivate::Entry::addClient(KDirWatch *instance, KDirWatch::WatchModes watchModes)
{
    if (instance == nullptr) {
        return;
    }

    auto it = findInstance(instance);
    if (it != m_clients.end()) {
        Client &client = *it;
        ++client.count;
        client.m_watchModes = watchModes;
        return;
    }

    m_clients.emplace_back(instance, watchModes);
}

void KDirWatchPrivate::Entry::removeClient(KDirWatch *instance)
{
    auto it = findInstance(instance);
    if (it != m_clients.end()) {
        Client &client = *it;
        --client.count;
        if (client.count == 0) {
            m_clients.erase(it);
        }
    }
}

/* get number of clients */
int KDirWatchPrivate::Entry::clientCount() const
{
    int clients = 0;
    for (const Client &client : m_clients) {
        clients += client.count;
    }

    return clients;
}

QString KDirWatchPrivate::Entry::parentDirectory() const
{
    return QDir::cleanPath(path + QLatin1String("/.."));
}

QList<const KDirWatchPrivate::Client *> KDirWatchPrivate::Entry::clientsForFileOrDir(const QString &tpath, bool *isDir) const
{
    QList<const Client *> ret;
    QFileInfo fi(tpath);
    if (fi.exists()) {
        *isDir = fi.isDir();
        const KDirWatch::WatchModes flag = *isDir ? KDirWatch::WatchSubDirs : KDirWatch::WatchFiles;
        for (const Client &client : m_clients) {
            if (client.m_watchModes & flag) {
                ret.append(&client);
            }
        }
    } else {
        // Happens frequently, e.g. ERROR: couldn't stat "/home/dfaure/.viminfo.tmp"
        // qCDebug(KDIRWATCH) << "ERROR: couldn't stat" << tpath;
        // In this case isDir is not set, but ret is empty anyway
        // so isDir won't be used.
    }
    return ret;
}

// inotify specific function that doesn't call KDE::stat to figure out if we have a file or folder.
// isDir is determined through inotify's "IN_ISDIR" flag in KDirWatchPrivate::inotifyEventReceived
QList<const KDirWatchPrivate::Client *> KDirWatchPrivate::Entry::inotifyClientsForFileOrDir(bool isDir) const
{
    QList<const Client *> ret;
    const KDirWatch::WatchModes flag = isDir ? KDirWatch::WatchSubDirs : KDirWatch::WatchFiles;
    for (const Client &client : m_clients) {
        if (client.m_watchModes & flag) {
            ret.append(&client);
        }
    }
    return ret;
}

QDebug operator<<(QDebug debug, const KDirWatch & /* watch */)
{
    if (!dwp_self.hasLocalData()) {
        debug << "KDirWatch not used";
        return debug;
    }
    debug << dwp_self.localData();
    return debug;
}

QDebug operator<<(QDebug debug, const KDirWatchPrivate &dwp)
{
    debug << "Entries watched:";
    if (dwp.m_mapEntries.count() == 0) {
        debug << "  None.";
    } else {
        auto it = dwp.m_mapEntries.cbegin();
        for (; it != dwp.m_mapEntries.cend(); ++it) {
            const KDirWatchPrivate::Entry &e = it.value();
            debug << "  " << e;

            for (const KDirWatchPrivate::Client &c : e.m_clients) {
                QByteArray pending;
                if (c.watchingStopped) {
                    if (c.pending & KDirWatchPrivate::Deleted) {
                        pending += "deleted ";
                    }
                    if (c.pending & KDirWatchPrivate::Created) {
                        pending += "created ";
                    }
                    if (c.pending & KDirWatchPrivate::Changed) {
                        pending += "changed ";
                    }
                    if (!pending.isEmpty()) {
                        pending = " (pending: " + pending + ')';
                    }
                    pending = ", stopped" + pending;
                }
                debug << "    by " << c.instance->objectName() << " (" << c.count << " times)" << pending;
            }
            if (!e.m_entries.isEmpty()) {
                debug << "    dependent entries:";
                for (KDirWatchPrivate::Entry *d : e.m_entries) {
                    debug << "      " << d << d->path << (d->m_status == KDirWatchPrivate::NonExistent ? "NonExistent" : "EXISTS this is an ERROR!");
                    if (s_verboseDebug) {
                        Q_ASSERT(d->m_status == KDirWatchPrivate::NonExistent); // it doesn't belong here otherwise
                    }
                }
            }
        }
    }
    return debug;
}

QDebug operator<<(QDebug debug, const KDirWatchPrivate::Entry &entry)
{
    debug.nospace() << "[ Entry for " << entry.path << ", " << (entry.isDir ? "dir" : "file");
    if (entry.m_status == KDirWatchPrivate::NonExistent) {
        debug << ", non-existent";
    }
    debug << ", using "
          << ((entry.m_mode == KDirWatchPrivate::INotifyMode)        ? "INotify"
                  : (entry.m_mode == KDirWatchPrivate::QFSWatchMode) ? "QFSWatch"
                  : (entry.m_mode == KDirWatchPrivate::StatMode)     ? "Stat"
                                                                     : "Unknown Method");
#if HAVE_SYS_INOTIFY_H
    if (entry.m_mode == KDirWatchPrivate::INotifyMode) {
        debug << " inotify_wd=" << entry.wd;
    }
#endif
    debug << ", has " << entry.m_clients.size() << " clients";
    debug.space();
    if (!entry.m_entries.isEmpty()) {
        debug << ", nonexistent subentries:";
        for (KDirWatchPrivate::Entry *subEntry : std::as_const(entry.m_entries)) {
            debug << subEntry << subEntry->path;
        }
    }
    debug << ']';
    return debug;
}

KDirWatchPrivate::Entry *KDirWatchPrivate::entry(const QString &_path)
{
    if (_path.isEmpty()) {
        return nullptr;
    }

    QString path(_path);

    if (path.length() > 1 && path.endsWith(QLatin1Char('/'))) {
        path.chop(1);
    }

    auto it = m_mapEntries.find(path);
    return it != m_mapEntries.end() ? &it.value() : nullptr;
}

// set polling frequency for a entry and adjust global freq if needed
void KDirWatchPrivate::useFreq(Entry *e, int newFreq)
{
    e->freq = newFreq;

    // a reasonable frequency for the global polling timer
    if (e->freq < freq) {
        freq = e->freq;
        if (m_statRescanTimer.isActive()) {
            m_statRescanTimer.start(freq);
        }
        qCDebug(KDIRWATCH) << "Global Poll Freq is now" << freq << "msec";
    }
}

#if HAVE_SYS_INOTIFY_H
// setup INotify notification, returns false if not possible
bool KDirWatchPrivate::useINotify(Entry *e)
{
    e->wd = -1;
    e->dirty = false;

    if (!supports_inotify) {
        return false;
    }

    e->m_mode = INotifyMode;

    if (e->m_status == NonExistent) {
        addEntry(nullptr, e->parentDirectory(), e, true);
        return true;
    }

    // May as well register for almost everything - it's free!
    int mask = IN_DELETE | IN_DELETE_SELF | IN_CREATE | IN_MOVE | IN_MOVE_SELF | IN_DONT_FOLLOW | IN_MOVED_FROM | IN_MODIFY | IN_ATTRIB;

    if ((e->wd = inotify_add_watch(m_inotify_fd, QFile::encodeName(e->path).data(), mask)) != -1) {
        m_inotify_wd_to_entry.insert(e->wd, e);
        if (s_verboseDebug) {
            qCDebug(KDIRWATCH) << "inotify successfully used for monitoring" << e->path << "wd=" << e->wd;
        }
        return true;
    }

    if (errno == ENOSPC) {
        // Inotify max_user_watches was reached (/proc/sys/fs/inotify/max_user_watches)
        // See man inotify_add_watch, https://github.com/guard/listen/wiki/Increasing-the-amount-of-inotify-watchers
        qCWarning(KDIRWATCH) << "inotify failed for monitoring" << e->path << "\n"
                             << "Because it reached its max_user_watches,\n"
                             << "you can increase the maximum number of file watches per user,\n"
                             << "by setting an appropriate fs.inotify.max_user_watches parameter in your /etc/sysctl.conf";
    } else {
        qCDebug(KDIRWATCH) << "inotify failed for monitoring" << e->path << ":" << strerror(errno) << " (errno:" << errno << ")";
    }
    return false;
}
#endif
#if HAVE_QFILESYSTEMWATCHER
bool KDirWatchPrivate::useQFSWatch(Entry *e)
{
    e->m_mode = QFSWatchMode;
    e->dirty = false;

    if (e->m_status == NonExistent) {
        addEntry(nullptr, e->parentDirectory(), e, true /*isDir*/);
        return true;
    }

    // qCDebug(KDIRWATCH) << "fsWatcher->addPath" << e->path;
    if (!fsWatcher) {
        fsWatcher = new QFileSystemWatcher();
        connect(fsWatcher, &QFileSystemWatcher::directoryChanged, this, &KDirWatchPrivate::fswEventReceived);
        connect(fsWatcher, &QFileSystemWatcher::fileChanged, this, &KDirWatchPrivate::fswEventReceived);
    }
    fsWatcher->addPath(e->path);
    return true;
}
#endif

bool KDirWatchPrivate::useStat(Entry *e)
{
    if (KFileSystemType::fileSystemType(e->path) == KFileSystemType::Nfs) { // TODO: or Smbfs?
        useFreq(e, m_nfsPollInterval);
    } else {
        useFreq(e, m_PollInterval);
    }

    if (e->m_mode != StatMode) {
        e->m_mode = StatMode;
        statEntries++;

        if (statEntries == 1) {
            // if this was first STAT entry (=timer was stopped)
            m_statRescanTimer.start(freq); // then start the timer
            qCDebug(KDIRWATCH) << " Started Polling Timer, freq " << freq;
        }
    }

    qCDebug(KDIRWATCH) << " Setup Stat (freq " << e->freq << ") for " << e->path;

    return true;
}

/* If <instance> !=0, this KDirWatch instance wants to watch at <_path>,
 * providing in <isDir> the type of the entry to be watched.
 * Sometimes, entries are dependent on each other: if <sub_entry> !=0,
 * this entry needs another entry to watch itself (when notExistent).
 */
void KDirWatchPrivate::addEntry(KDirWatch *instance, const QString &_path, Entry *sub_entry, bool isDir, KDirWatch::WatchModes watchModes)
{
    QString path(_path);
    if (path.startsWith(QLatin1String(":/"))) {
        qCWarning(KDIRWATCH) << "Cannot watch QRC-like path" << path;
        return;
    }
    if (path.isEmpty()
#ifndef Q_OS_WIN
        || path == QLatin1String("/dev")
        || (path.startsWith(QLatin1String("/dev/")) && !path.startsWith(QLatin1String("/dev/.")) && !path.startsWith(QLatin1String("/dev/shm")))
#endif
    ) {
        return; // Don't even go there.
    }

    if (path.length() > 1 && path.endsWith(QLatin1Char('/'))) {
        path.chop(1);
    }

    auto it = m_mapEntries.find(path);
    if (it != m_mapEntries.end()) {
        Entry &entry = it.value();
        if (sub_entry) {
            entry.m_entries.append(sub_entry);
            if (s_verboseDebug) {
                qCDebug(KDIRWATCH) << "Added already watched Entry" << path << "(for" << sub_entry->path << ")";
            }
        } else {
            entry.addClient(instance, watchModes);
            if (s_verboseDebug) {
                qCDebug(KDIRWATCH) << "Added already watched Entry" << path << "(now" << entry.clientCount() << "clients)"
                                   << QStringLiteral("[%1]").arg(instance->objectName());
            }
        }
        return;
    }

    // we have a new path to watch

    QT_STATBUF stat_buf;
    bool exists = (QT_STAT(QFile::encodeName(path).constData(), &stat_buf) == 0);

    auto newIt = m_mapEntries.insert(path, Entry());
    // the insert does a copy, so we have to use <e> now
    Entry *e = &(*newIt);

    if (exists) {
        e->isDir = (stat_buf.st_mode & QT_STAT_MASK) == QT_STAT_DIR;

#ifndef Q_OS_WIN
        if (e->isDir && !isDir) {
            if (QT_LSTAT(QFile::encodeName(path).constData(), &stat_buf) == 0) {
                if ((stat_buf.st_mode & QT_STAT_MASK) == QT_STAT_LNK) {
                    // if it's a symlink, don't follow it
                    e->isDir = false;
                }
            }
        }
#endif

        if (e->isDir && !isDir) {
            qCWarning(KCOREADDONS_DEBUG) << "KDirWatch:" << path << "is a directory. Use addDir!";
        } else if (!e->isDir && isDir) {
            qCWarning(KCOREADDONS_DEBUG) << "KDirWatch:" << path << "is a file. Use addFile!";
        }

        if (!e->isDir && (watchModes != KDirWatch::WatchDirOnly)) {
            qCWarning(KCOREADDONS_DEBUG) << "KDirWatch:" << path
                                         << "is a file. You can't use recursive or "
                                            "watchFiles options";
            watchModes = KDirWatch::WatchDirOnly;
        }

#ifdef Q_OS_WIN
        // ctime is the 'creation time' on windows - use mtime instead
        e->m_ctime = stat_buf.st_mtime;
#else
        e->m_ctime = stat_buf.st_ctime;
#endif
        e->m_status = Normal;
        e->m_nlink = stat_buf.st_nlink;
        e->m_ino = stat_buf.st_ino;
    } else {
        e->isDir = isDir;
        e->m_ctime = invalid_ctime;
        e->m_status = NonExistent;
        e->m_nlink = 0;
        e->m_ino = 0;
    }

    e->path = path;
    if (sub_entry) {
        e->m_entries.append(sub_entry);
    } else {
        e->addClient(instance, watchModes);
    }

    if (s_verboseDebug) {
        qCDebug(KDIRWATCH).nospace() << "Added " << (e->isDir ? "Dir " : "File ") << path << (e->m_status == NonExistent ? " NotExisting" : "") << " for "
                                     << (sub_entry ? sub_entry->path : QString()) << " [" << (instance ? instance->objectName() : QString()) << "]";
    }

    // now setup the notification method
    e->m_mode = UnknownMode;
    e->msecLeft = 0;

    if (isNoisyFile(QFile::encodeName(path).data())) {
        return;
    }

    if (exists && e->isDir && (watchModes != KDirWatch::WatchDirOnly)) {
        // recursive watch for folders
        QFlags<QDir::Filter> filters = QDir::NoDotAndDotDot;

        if ((watchModes & KDirWatch::WatchSubDirs) && (watchModes & KDirWatch::WatchFiles)) {
            filters |= (QDir::Dirs | QDir::Files);
        } else if (watchModes & KDirWatch::WatchSubDirs) {
            filters |= QDir::Dirs;
        } else if (watchModes & KDirWatch::WatchFiles) {
            filters |= QDir::Files;
        }

#if HAVE_SYS_INOTIFY_H
        if (m_preferredMethod == KDirWatch::INotify) {
            // qCDebug(KDIRWATCH) << "Ignoring WatchFiles directive - this is implicit with inotify";
            // Placing a watch on individual files is redundant with inotify
            // (inotify gives us WatchFiles functionality "for free") and indeed
            // actively harmful, so prevent it.  WatchSubDirs is necessary, though.
            filters &= ~QDir::Files;
        }
#endif

        QDir basedir(e->path);
        const QFileInfoList contents = basedir.entryInfoList(filters);
        for (const QFileInfo &fileInfo : contents) {
            // treat symlinks as files--don't follow them.
            bool isDir = fileInfo.isDir() && !fileInfo.isSymLink();

            addEntry(instance, fileInfo.absoluteFilePath(), nullptr, isDir, isDir ? watchModes : KDirWatch::WatchDirOnly);
        }
    }

    addWatch(e);
}

void KDirWatchPrivate::addWatch(Entry *e)
{
    // If the watch is on a network filesystem use the nfsPreferredMethod as the
    // default, otherwise use preferredMethod as the default, if the methods are
    // the same we can skip the mountpoint check

    // This allows to configure a different method for NFS mounts, since inotify
    // cannot detect changes made by other machines. However as a default inotify
    // is fine, since the most common case is a NFS-mounted home, where all changes
    // are made locally. #177892.

    KDirWatch::Method preferredMethod = m_preferredMethod;
    if (m_nfsPreferredMethod != m_preferredMethod) {
        if (KFileSystemType::fileSystemType(e->path) == KFileSystemType::Nfs) {
            preferredMethod = m_nfsPreferredMethod;
        }
    }

    // Try the appropriate preferred method from the config first
    bool entryAdded = false;
    switch (preferredMethod) {
#if HAVE_SYS_INOTIFY_H
    case KDirWatch::INotify:
        entryAdded = useINotify(e);
        break;
#else
    case KDirWatch::INotify:
        entryAdded = false;
        break;
#endif
#if HAVE_QFILESYSTEMWATCHER
    case KDirWatch::QFSWatch:
        entryAdded = useQFSWatch(e);
        break;
#else
    case KDirWatch::QFSWatch:
        entryAdded = false;
        break;
#endif
    case KDirWatch::Stat:
        entryAdded = useStat(e);
        break;
    }

    // Failing that try in order INotify, QFSWatch, Stat
    if (!entryAdded) {
#if HAVE_SYS_INOTIFY_H
        if (preferredMethod != KDirWatch::INotify && useINotify(e)) {
            return;
        }
#endif
#if HAVE_QFILESYSTEMWATCHER
        if (preferredMethod != KDirWatch::QFSWatch && useQFSWatch(e)) {
            return;
        }
#endif
        if (preferredMethod != KDirWatch::Stat) {
            useStat(e);
        }
    }
}

void KDirWatchPrivate::removeWatch(Entry *e)
{
#if HAVE_SYS_INOTIFY_H
    if (e->m_mode == INotifyMode) {
        m_inotify_wd_to_entry.remove(e->wd);
        (void)inotify_rm_watch(m_inotify_fd, e->wd);
        if (s_verboseDebug) {
            qCDebug(KDIRWATCH).nospace() << "Cancelled INotify (fd " << m_inotify_fd << ", " << e->wd << ") for " << e->path;
        }
    }
#endif
#if HAVE_QFILESYSTEMWATCHER
    if (e->m_mode == QFSWatchMode && fsWatcher) {
        if (s_verboseDebug) {
            qCDebug(KDIRWATCH) << "fsWatcher->removePath" << e->path;
        }
        fsWatcher->removePath(e->path);
    }
#endif
}

void KDirWatchPrivate::removeEntry(KDirWatch *instance, const QString &_path, Entry *sub_entry)
{
    qCDebug(KDIRWATCH) << "path=" << _path << "sub_entry:" << sub_entry;

    Entry *e = entry(_path);
    if (e) {
        removeEntry(instance, e, sub_entry);
    }
}

void KDirWatchPrivate::removeEntry(KDirWatch *instance, Entry *e, Entry *sub_entry)
{
    removeList.remove(e);

    if (sub_entry) {
        e->m_entries.removeAll(sub_entry);
    } else {
        e->removeClient(instance);
    }

    if (!e->m_clients.empty() || !e->m_entries.empty()) {
        return;
    }

    if (delayRemove) {
        removeList.insert(e);
        // now e->isValid() is false
        return;
    }

    if (e->m_status == Normal) {
        removeWatch(e);
    } else {
        // Removed a NonExistent entry - we just remove it from the parent
        if (e->isDir) {
            removeEntry(nullptr, e->parentDirectory(), e);
        } else {
            removeEntry(nullptr, QFileInfo(e->path).absolutePath(), e);
        }
    }

    if (e->m_mode == StatMode) {
        statEntries--;
        if (statEntries == 0) {
            m_statRescanTimer.stop(); // stop timer if lists are empty
            qCDebug(KDIRWATCH) << " Stopped Polling Timer";
        }
    }

    if (s_verboseDebug) {
        qCDebug(KDIRWATCH).nospace() << "Removed " << (e->isDir ? "Dir " : "File ") << e->path << " for " << (sub_entry ? sub_entry->path : QString()) << " ["
                                     << (instance ? instance->objectName() : QString()) << "]";
    }
    QString p = e->path; // take a copy, QMap::remove takes a reference and deletes, since e points into the map
#if HAVE_SYS_INOTIFY_H
    m_inotify_wd_to_entry.remove(e->wd);
#endif
    m_mapEntries.remove(p); // <e> not valid any more
}

/* Called from KDirWatch destructor:
 * remove <instance> as client from all entries
 */
void KDirWatchPrivate::removeEntries(KDirWatch *instance)
{
    int minfreq = 3600000;

    QStringList pathList;
    // put all entries where instance is a client in list
    for (auto it = m_mapEntries.begin(); it != m_mapEntries.end(); ++it) {
        Entry &entry = it.value();
        auto clientIt = entry.findInstance(instance);
        if (clientIt != entry.m_clients.end()) {
            clientIt->count = 1; // forces deletion of instance as client
            pathList.append(entry.path);
        } else if (entry.m_mode == StatMode && entry.freq < minfreq) {
            minfreq = entry.freq;
        }
    }

    for (const QString &path : std::as_const(pathList)) {
        removeEntry(instance, path, nullptr);
    }

    if (minfreq > freq) {
        // we can decrease the global polling frequency
        freq = minfreq;
        if (m_statRescanTimer.isActive()) {
            m_statRescanTimer.start(freq);
        }
        qCDebug(KDIRWATCH) << "Poll Freq now" << freq << "msec";
    }
}

// instance ==0: stop scanning for all instances
bool KDirWatchPrivate::stopEntryScan(KDirWatch *instance, Entry *e)
{
    int stillWatching = 0;
    for (Client &client : e->m_clients) {
        if (!instance || instance == client.instance) {
            client.watchingStopped = true;
        } else if (!client.watchingStopped) {
            stillWatching += client.count;
        }
    }

    qCDebug(KDIRWATCH) << (instance ? instance->objectName() : QStringLiteral("all")) << "stopped scanning" << e->path << "(now" << stillWatching
                       << "watchers)";

    if (stillWatching == 0) {
        // if nobody is interested, we don't watch, and we don't report
        // changes that happened while not watching
        e->m_ctime = invalid_ctime; // invalid

        // Changing m_status like this would create wrong "created" events in stat mode.
        // To really "stop watching" we would need to determine 'stillWatching==0' in scanEntry...
        // e->m_status = NonExistent;
    }
    return true;
}

// instance ==0: start scanning for all instances
bool KDirWatchPrivate::restartEntryScan(KDirWatch *instance, Entry *e, bool notify)
{
    int wasWatching = 0;
    int newWatching = 0;
    for (Client &client : e->m_clients) {
        if (!client.watchingStopped) {
            wasWatching += client.count;
        } else if (!instance || instance == client.instance) {
            client.watchingStopped = false;
            newWatching += client.count;
        }
    }
    if (newWatching == 0) {
        return false;
    }

    qCDebug(KDIRWATCH) << (instance ? instance->objectName() : QStringLiteral("all")) << "restarted scanning" << e->path << "(now" << wasWatching + newWatching
                       << "watchers)";

    // restart watching and emit pending events

    int ev = NoChange;
    if (wasWatching == 0) {
        if (!notify) {
            QT_STATBUF stat_buf;
            bool exists = (QT_STAT(QFile::encodeName(e->path).constData(), &stat_buf) == 0);
            if (exists) {
                // ctime is the 'creation time' on windows, but with qMax
                // we get the latest change of any kind, on any platform.
                e->m_ctime = qMax(stat_buf.st_ctime, stat_buf.st_mtime);
                e->m_status = Normal;
                if (s_verboseDebug) {
                    qCDebug(KDIRWATCH) << "Setting status to Normal for" << e << e->path;
                }
                e->m_nlink = stat_buf.st_nlink;
                e->m_ino = stat_buf.st_ino;

                // Same as in scanEntry: ensure no subentry in parent dir
                removeEntry(nullptr, e->parentDirectory(), e);
            } else {
                e->m_ctime = invalid_ctime;
                e->m_status = NonExistent;
                e->m_nlink = 0;
                if (s_verboseDebug) {
                    qCDebug(KDIRWATCH) << "Setting status to NonExistent for" << e << e->path;
                }
            }
        }
        e->msecLeft = 0;
        ev = scanEntry(e);
    }
    emitEvent(e, ev);

    return true;
}

// instance ==0: stop scanning for all instances
void KDirWatchPrivate::stopScan(KDirWatch *instance)
{
    for (auto it = m_mapEntries.begin(); it != m_mapEntries.end(); ++it) {
        stopEntryScan(instance, &it.value());
    }
}

void KDirWatchPrivate::startScan(KDirWatch *instance, bool notify, bool skippedToo)
{
    if (!notify) {
        resetList(instance, skippedToo);
    }

    for (auto it = m_mapEntries.begin(); it != m_mapEntries.end(); ++it) {
        restartEntryScan(instance, &it.value(), notify);
    }

    // timer should still be running when in polling mode
}

// clear all pending events, also from stopped
void KDirWatchPrivate::resetList(KDirWatch *instance, bool skippedToo)
{
    Q_UNUSED(instance);

    for (auto it = m_mapEntries.begin(); it != m_mapEntries.end(); ++it) {
        for (Client &client : it.value().m_clients) {
            if (!client.watchingStopped || skippedToo) {
                client.pending = NoChange;
            }
        }
    }
}

// Return event happened on <e>
//
int KDirWatchPrivate::scanEntry(Entry *e)
{
    // Shouldn't happen: Ignore "unknown" notification method
    if (e->m_mode == UnknownMode) {
        return NoChange;
    }

    if (e->m_mode == INotifyMode) {
        // we know nothing has changed, no need to stat
        if (!e->dirty) {
            return NoChange;
        }
        e->dirty = false;
    }

    if (e->m_mode == StatMode) {
        // only scan if timeout on entry timer happens;
        // e.g. when using 500msec global timer, a entry
        // with freq=5000 is only watched every 10th time

        e->msecLeft -= freq;
        if (e->msecLeft > 0) {
            return NoChange;
        }
        e->msecLeft += e->freq;
    }

    QT_STATBUF stat_buf;
    const bool exists = (QT_STAT(QFile::encodeName(e->path).constData(), &stat_buf) == 0);
    if (exists) {
        if (e->m_status == NonExistent) {
            // ctime is the 'creation time' on windows, but with qMax
            // we get the latest change of any kind, on any platform.
            e->m_ctime = qMax(stat_buf.st_ctime, stat_buf.st_mtime);
            e->m_status = Normal;
            e->m_ino = stat_buf.st_ino;
            if (s_verboseDebug) {
                qCDebug(KDIRWATCH) << "Setting status to Normal for just created" << e << e->path;
            }
            // We need to make sure the entry isn't listed in its parent's subentries... (#222974, testMoveTo)
            removeEntry(nullptr, e->parentDirectory(), e);

            return Created;
        }

#if 1 // for debugging the if() below
        if (s_verboseDebug) {
            struct tm *tmp = localtime(&e->m_ctime);
            char outstr[200];
            strftime(outstr, sizeof(outstr), "%H:%M:%S", tmp);
            qCDebug(KDIRWATCH) << e->path << "e->m_ctime=" << e->m_ctime << outstr << "stat_buf.st_ctime=" << stat_buf.st_ctime
                               << "stat_buf.st_mtime=" << stat_buf.st_mtime << "e->m_nlink=" << e->m_nlink << "stat_buf.st_nlink=" << stat_buf.st_nlink
                               << "e->m_ino=" << e->m_ino << "stat_buf.st_ino=" << stat_buf.st_ino;
        }
#endif

        if ((e->m_ctime != invalid_ctime)
            && (qMax(stat_buf.st_ctime, stat_buf.st_mtime) != e->m_ctime || stat_buf.st_ino != e->m_ino
                || int(stat_buf.st_nlink) != int(e->m_nlink)
#ifdef Q_OS_WIN
                // on Windows, we trust QFSW to get it right, the ctime comparisons above
                // fail for example when adding files to directories on Windows
                // which doesn't change the mtime of the directory
                || e->m_mode == QFSWatchMode
#endif
                )) {
            e->m_ctime = qMax(stat_buf.st_ctime, stat_buf.st_mtime);
            e->m_nlink = stat_buf.st_nlink;
            if (e->m_ino != stat_buf.st_ino) {
                // The file got deleted and recreated. We need to watch it again.
                removeWatch(e);
                addWatch(e);
                e->m_ino = stat_buf.st_ino;
                return (Deleted | Created);
            } else {
                return Changed;
            }
        }

        return NoChange;
    }

    // dir/file doesn't exist

    e->m_nlink = 0;
    e->m_ino = 0;
    e->m_status = NonExistent;

    if (e->m_ctime == invalid_ctime) {
        return NoChange;
    }

    e->m_ctime = invalid_ctime;
    return Deleted;
}

/* Notify all interested KDirWatch instances about a given event on an entry
 * and stored pending events. When watching is stopped, the event is
 * added to the pending events.
 */
void KDirWatchPrivate::emitEvent(Entry *e, int event, const QString &fileName)
{
    QString path(e->path);
    if (!fileName.isEmpty()) {
        if (!QDir::isRelativePath(fileName)) {
            path = fileName;
        } else {
#ifdef Q_OS_UNIX
            path += QLatin1Char('/') + fileName;
#elif defined(Q_OS_WIN)
            // current drive is passed instead of /
            path += QStringView(QDir::currentPath()).left(2) + QLatin1Char('/') + fileName;
#endif
        }
    }

    if (s_verboseDebug) {
        qCDebug(KDIRWATCH) << event << path << e->m_clients.size() << "clients";
    }

    for (Client &c : e->m_clients) {
        if (c.instance == nullptr || c.count == 0) {
            continue;
        }

        if (c.watchingStopped) {
            // Do not add event to a list of pending events, the docs say restartDirScan won't emit!
            continue;
        }
        // not stopped
        if (event == NoChange || event == Changed) {
            event |= c.pending;
        }
        c.pending = NoChange;
        if (event == NoChange) {
            continue;
        }

        // Emit the signals delayed, to avoid unexpected re-entrance from the slots (#220153)

        if (event & Deleted) {
            QMetaObject::invokeMethod(
                c.instance,
                [c, path]() {
                    c.instance->setDeleted(path);
                },
                Qt::QueuedConnection);
        }

        if (event & Created) {
            QMetaObject::invokeMethod(
                c.instance,
                [c, path]() {
                    c.instance->setCreated(path);
                },
                Qt::QueuedConnection);
            // possible emit Change event after creation
        }

        if (event & Changed) {
            QMetaObject::invokeMethod(
                c.instance,
                [c, path]() {
                    c.instance->setDirty(path);
                },
                Qt::QueuedConnection);
        }
    }
}

// Remove entries which were marked to be removed
void KDirWatchPrivate::slotRemoveDelayed()
{
    delayRemove = false;
    // Removing an entry could also take care of removing its parent
    // (e.g. in inotify mode), which would remove other entries in removeList,
    // so don't use Q_FOREACH or iterators here...
    while (!removeList.isEmpty()) {
        Entry *entry = *removeList.begin();
        removeEntry(nullptr, entry, nullptr); // this will remove entry from removeList
    }
}

/* Scan all entries to be watched for changes. This is done regularly
 * when polling. inotify uses a single-shot timer to call this slot delayed.
 */
void KDirWatchPrivate::slotRescan()
{
    if (s_verboseDebug) {
        qCDebug(KDIRWATCH);
    }

    EntryMap::Iterator it;

    // People can do very long things in the slot connected to dirty(),
    // like showing a message box. We don't want to keep polling during
    // that time, otherwise the value of 'delayRemove' will be reset.
    // ### TODO: now the emitEvent delays emission, this can be cleaned up
    bool timerRunning = m_statRescanTimer.isActive();
    if (timerRunning) {
        m_statRescanTimer.stop();
    }

    // We delay deletions of entries this way.
    // removeDir(), when called in slotDirty(), can cause a crash otherwise
    // ### TODO: now the emitEvent delays emission, this can be cleaned up
    delayRemove = true;

    if (rescan_all) {
        // mark all as dirty
        it = m_mapEntries.begin();
        for (; it != m_mapEntries.end(); ++it) {
            (*it).dirty = true;
        }
        rescan_all = false;
    } else {
        // propagate dirty flag to dependent entries (e.g. file watches)
        it = m_mapEntries.begin();
        for (; it != m_mapEntries.end(); ++it) {
            if (((*it).m_mode == INotifyMode || (*it).m_mode == QFSWatchMode) && (*it).dirty) {
                (*it).propagate_dirty();
            }
        }
    }

#if HAVE_SYS_INOTIFY_H
    QList<Entry *> cList;
#endif

    it = m_mapEntries.begin();
    for (; it != m_mapEntries.end(); ++it) {
        // we don't check invalid entries (i.e. remove delayed)
        Entry *entry = &(*it);
        if (!entry->isValid()) {
            continue;
        }

        const int ev = scanEntry(entry);
        if (s_verboseDebug) {
            qCDebug(KDIRWATCH) << "scanEntry for" << entry->path << "says" << ev;
        }

        switch (entry->m_mode) {
#if HAVE_SYS_INOTIFY_H
        case INotifyMode:
            if (ev == Deleted) {
                if (s_verboseDebug) {
                    qCDebug(KDIRWATCH) << "scanEntry says" << entry->path << "was deleted";
                }
                addEntry(nullptr, entry->parentDirectory(), entry, true);
            } else if (ev == Created) {
                if (s_verboseDebug) {
                    qCDebug(KDIRWATCH) << "scanEntry says" << entry->path << "was created. wd=" << entry->wd;
                }
                if (entry->wd < 0) {
                    cList.append(entry);
                    addWatch(entry);
                }
            }
            break;
#endif
        case QFSWatchMode:
            if (ev == Created) {
                addWatch(entry);
            }
            break;
        default:
            // dunno about StatMode...
            break;
        }

#if HAVE_SYS_INOTIFY_H
        if (entry->isDir) {
            // Report and clear the list of files that have changed in this directory.
            // Remove duplicates by changing to set and back again:
            // we don't really care about preserving the order of the
            // original changes.
            QStringList pendingFileChanges = entry->m_pendingFileChanges;
            pendingFileChanges.removeDuplicates();
            for (const QString &changedFilename : std::as_const(pendingFileChanges)) {
                if (s_verboseDebug) {
                    qCDebug(KDIRWATCH) << "processing pending file change for" << changedFilename;
                }
                emitEvent(entry, Changed, changedFilename);
            }
            entry->m_pendingFileChanges.clear();
        }
#endif

        if (ev != NoChange) {
            emitEvent(entry, ev);
        }
    }

    if (timerRunning) {
        m_statRescanTimer.start(freq);
    }

#if HAVE_SYS_INOTIFY_H
    // Remove watch of parent of new created directories
    for (Entry *e : std::as_const(cList)) {
        removeEntry(nullptr, e->parentDirectory(), e);
    }
#endif

    QTimer::singleShot(0, this, &KDirWatchPrivate::slotRemoveDelayed);
}

bool KDirWatchPrivate::isNoisyFile(const char *filename)
{
    // $HOME/.X.err grows with debug output, so don't notify change
    if (*filename == '.') {
        if (strncmp(filename, ".X.err", 6) == 0) {
            return true;
        }
        if (strncmp(filename, ".xsession-errors", 16) == 0) {
            return true;
        }
        // fontconfig updates the cache on every KDE app start
        // as well as during kio_thumbnail worker execution
        // TODO:; check which fontconfig version this file was deprecated and the check can be removed
        if (strncmp(filename, ".fonts.cache", 12) == 0) {
            return true;
        }
    }

    return false;
}

void KDirWatchPrivate::ref(KDirWatch *watch)
{
    m_referencesObjects.push_back(watch);
}

void KDirWatchPrivate::unref(KDirWatch *watch)
{
    m_referencesObjects.removeOne(watch);
    if (m_referencesObjects.isEmpty()) {
        destroyPrivate();
    }
}

#if HAVE_SYS_INOTIFY_H
QString KDirWatchPrivate::inotifyEventName(const inotify_event *event) const
{
    if (event->mask & IN_OPEN)
        return QStringLiteral("OPEN");
    else if (event->mask & IN_CLOSE_NOWRITE)
        return QStringLiteral("CLOSE_NOWRITE");
    else if (event->mask & IN_CLOSE_WRITE)
        return QStringLiteral("CLOSE_WRITE");
    else if (event->mask & IN_MOVED_TO)
        return QStringLiteral("MOVED_TO");
    else if (event->mask & IN_MOVED_FROM)
        return QStringLiteral("MOVED_FROM");
    else if (event->mask & IN_MOVE)
        return QStringLiteral("MOVE");
    else if (event->mask & IN_CREATE)
        return QStringLiteral("CREATE");
    else if (event->mask & IN_DELETE)
        return QStringLiteral("DELETE");
    else if (event->mask & IN_DELETE_SELF)
        return QStringLiteral("DELETE_SELF");
    else if (event->mask & IN_MOVE_SELF)
        return QStringLiteral("MOVE_SELF");
    else if (event->mask & IN_ATTRIB)
        return QStringLiteral("ATTRIB");
    else if (event->mask & IN_MODIFY)
        return QStringLiteral("MODIFY");
    if (event->mask & IN_ACCESS)
        return QStringLiteral("ACCESS");
    if (event->mask & IN_IGNORED)
        return QStringLiteral("IGNORED");
    if (event->mask & IN_UNMOUNT)
        return QStringLiteral("IN_UNMOUNT");
    else
        return QStringLiteral("UNKWOWN");
}
#endif

#if HAVE_QFILESYSTEMWATCHER
// Slot for QFileSystemWatcher
void KDirWatchPrivate::fswEventReceived(const QString &path)
{
    if (s_verboseDebug) {
        qCDebug(KDIRWATCH) << path;
    }

    auto it = m_mapEntries.find(path);
    if (it != m_mapEntries.end()) {
        Entry *entry = &it.value();
        entry->dirty = true;
        const int ev = scanEntry(entry);
        if (s_verboseDebug) {
            qCDebug(KDIRWATCH) << "scanEntry for" << entry->path << "says" << ev;
        }
        if (ev != NoChange) {
            emitEvent(entry, ev);
        }
        if (ev == Deleted) {
            if (entry->isDir) {
                addEntry(nullptr, entry->parentDirectory(), entry, true);
            } else {
                addEntry(nullptr, QFileInfo(entry->path).absolutePath(), entry, true);
            }
        } else if (ev == Created) {
            // We were waiting for it to appear; now watch it
            addWatch(entry);
        } else if (entry->isDir) {
            // Check if any file or dir was created under this directory, that we were waiting for
            for (Entry *sub_entry : std::as_const(entry->m_entries)) {
                fswEventReceived(sub_entry->path); // recurse, to call scanEntry and see if something changed
            }
        } else {
            /* Even though QFileSystemWatcher only reported the file as modified, it is possible that the file
             * was in fact just deleted and then immediately recreated.  If the file was deleted, QFileSystemWatcher
             * will delete the watch, and will ignore the file, even after it is recreated.  Since it is impossible
             * to reliably detect this case, always re-request the watch on a dirty signal, to avoid losing the
             * underlying OS monitor.
             */
            fsWatcher->addPath(entry->path);
        }
    }
}
#else
void KDirWatchPrivate::fswEventReceived(const QString &path)
{
    Q_UNUSED(path);
    qCWarning(KCOREADDONS_DEBUG) << "QFileSystemWatcher event received but QFileSystemWatcher is not supported";
}
#endif // HAVE_QFILESYSTEMWATCHER

//
// Class KDirWatch
//

Q_GLOBAL_STATIC(KDirWatch, s_pKDirWatchSelf)
KDirWatch *KDirWatch::self()
{
    return s_pKDirWatchSelf();
}

// <steve> is this used anywhere?
// <dfaure> yes, see kio/src/core/kcoredirlister_p.h:328
bool KDirWatch::exists()
{
    return s_pKDirWatchSelf.exists() && dwp_self.hasLocalData();
}

KDirWatch::KDirWatch(QObject *parent)
    : QObject(parent)
    , d(createPrivate())
{
    d->ref(this);
    static QBasicAtomicInt nameCounter = Q_BASIC_ATOMIC_INITIALIZER(1);
    const int counter = nameCounter.fetchAndAddRelaxed(1); // returns the old value
    setObjectName(QStringLiteral("KDirWatch-%1").arg(counter));
}

KDirWatch::~KDirWatch()
{
    if (d) {
        d->removeEntries(this);
        d->unref(this);
    }
}

void KDirWatch::addDir(const QString &_path, WatchModes watchModes)
{
    if (KNetworkMounts::self()->isOptionEnabledForPath(_path, KNetworkMounts::KDirWatchDontAddWatches)) {
        return;
    }

    if (d) {
        d->addEntry(this, _path, nullptr, true, watchModes);
    }
}

void KDirWatch::addFile(const QString &_path)
{
    if (KNetworkMounts::self()->isOptionEnabledForPath(_path, KNetworkMounts::KDirWatchDontAddWatches)) {
        return;
    }

    if (!d) {
        return;
    }

    d->addEntry(this, _path, nullptr, false);
}

QDateTime KDirWatch::ctime(const QString &_path) const
{
    KDirWatchPrivate::Entry *e = d->entry(_path);

    if (!e) {
        return QDateTime();
    }

    return QDateTime::fromSecsSinceEpoch(e->m_ctime);
}

void KDirWatch::removeDir(const QString &_path)
{
    if (d) {
        d->removeEntry(this, _path, nullptr);
    }
}

void KDirWatch::removeFile(const QString &_path)
{
    if (d) {
        d->removeEntry(this, _path, nullptr);
    }
}

bool KDirWatch::stopDirScan(const QString &_path)
{
    if (d) {
        KDirWatchPrivate::Entry *e = d->entry(_path);
        if (e && e->isDir) {
            return d->stopEntryScan(this, e);
        }
    }
    return false;
}

bool KDirWatch::restartDirScan(const QString &_path)
{
    if (d) {
        KDirWatchPrivate::Entry *e = d->entry(_path);
        if (e && e->isDir)
        // restart without notifying pending events
        {
            return d->restartEntryScan(this, e, false);
        }
    }
    return false;
}

void KDirWatch::stopScan()
{
    if (d) {
        d->stopScan(this);
        d->_isStopped = true;
    }
}

bool KDirWatch::isStopped()
{
    return d->_isStopped;
}

void KDirWatch::startScan(bool notify, bool skippedToo)
{
    if (d) {
        d->_isStopped = false;
        d->startScan(this, notify, skippedToo);
    }
}

bool KDirWatch::contains(const QString &_path) const
{
    KDirWatchPrivate::Entry *e = d->entry(_path);
    if (!e) {
        return false;
    }

    for (const KDirWatchPrivate::Client &client : e->m_clients) {
        if (client.instance == this) {
            return true;
        }
    }

    return false;
}

void KDirWatch::setCreated(const QString &_file)
{
    qCDebug(KDIRWATCH) << objectName() << "emitting created" << _file;
    Q_EMIT created(_file);
}

void KDirWatch::setDirty(const QString &_file)
{
    Q_EMIT dirty(_file);
}

void KDirWatch::setDeleted(const QString &_file)
{
    qCDebug(KDIRWATCH) << objectName() << "emitting deleted" << _file;
    Q_EMIT deleted(_file);
}

KDirWatch::Method KDirWatch::internalMethod() const
{
    // This reproduces the logic in KDirWatchPrivate::addWatch
    switch (d->m_preferredMethod) {
    case KDirWatch::INotify:
#if HAVE_SYS_INOTIFY_H
        if (d->supports_inotify) {
            return KDirWatch::INotify;
        }
#endif
        break;
    case KDirWatch::QFSWatch:
#if HAVE_QFILESYSTEMWATCHER
        return KDirWatch::QFSWatch;
#else
        break;
#endif
    case KDirWatch::Stat:
        return KDirWatch::Stat;
    }

#if HAVE_SYS_INOTIFY_H
    if (d->supports_inotify) {
        return KDirWatch::INotify;
    }
#endif
#if HAVE_QFILESYSTEMWATCHER
    return KDirWatch::QFSWatch;
#else
    return KDirWatch::Stat;
#endif
}

bool KDirWatch::event(QEvent *event)
{
    if (Q_LIKELY(event->type() != QEvent::ThreadChange)) {
        return QObject::event(event);
    }

    qCCritical(KDIRWATCH) << "KDirwatch is moving its thread. This is not supported at this time; your watch will not watch anything anymore!"
                          << "Create and use watches on the correct thread"
                          << "Watch:" << this;

    // We are still in the old thread when the event runs, so this is safe.
    Q_ASSERT(thread() == d->thread());
    d->removeEntries(this);
    d->unref(this);
    d = nullptr;

    // Schedule the creation of the new private in the new thread.
    QMetaObject::invokeMethod(
        this,
        [this] {
            d = createPrivate();
        },
        Qt::QueuedConnection);

    // NOTE: to actually support moving watches across threads we'd have to make Entry copyable and schedule a complete
    // re-installation of watches on the new thread after createPrivate.

    return QObject::event(event);
}

#include "moc_kdirwatch.cpp"
#include "moc_kdirwatch_p.cpp"

// sven
