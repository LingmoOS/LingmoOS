/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 1998 Sven Radej <sven@lisa.exp.univie.ac.at>
    SPDX-FileCopyrightText: 2006 Dirk Mueller <mueller@kde.org>
    SPDX-FileCopyrightText: 2007 Flavio Castelli <flavio.castelli@gmail.com>
    SPDX-FileCopyrightText: 2008 Jarosław Staniek <staniek@kde.org>
    SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only

    Private Header for class of KDirWatchPrivate
    this separate header file is needed for MOC processing
    because KDirWatchPrivate has signals and slots
*/

#ifndef KDIRWATCH_P_H
#define KDIRWATCH_P_H

#include "kdirwatch.h"
#include <io/config-kdirwatch.h>

#ifndef QT_NO_FILESYSTEMWATCHER
#define HAVE_QFILESYSTEMWATCHER 1
#else
#define HAVE_QFILESYSTEMWATCHER 0
#endif

#include <QList>
#include <QMap>
#include <QObject>
#include <QSet>
#include <QString>
#include <QTimer>
class QSocketNotifier;

#include <ctime>
#include <sys/types.h> // time_t, ino_t

#define invalid_ctime (static_cast<time_t>(-1))

#if HAVE_QFILESYSTEMWATCHER
#include <QFileSystemWatcher>
#endif // HAVE_QFILESYSTEMWATCHER

#if HAVE_SYS_INOTIFY_H
struct inotify_event;
#endif

/* KDirWatchPrivate is a singleton and does the watching
 * for every KDirWatch instance in the application.
 */
class KDirWatchPrivate : public QObject
{
    Q_OBJECT
public:
    enum entryStatus {
        Normal = 0,
        NonExistent,
    };
    enum entryMode {
        UnknownMode = 0,
        StatMode,
        INotifyMode,
        QFSWatchMode,
    };
    enum {
        NoChange = 0,
        Changed = 1,
        Created = 2,
        Deleted = 4,
    };

    struct Client {
        Client(KDirWatch *inst, KDirWatch::WatchModes watchModes)
            : instance(inst)
            , count(1)
            , watchingStopped(inst->isStopped())
            , pending(NoChange)
            , m_watchModes(watchModes)
        {
        }

        // The compiler needs a copy ctor for Client when Entry is inserted into m_mapEntries
        // (even though the vector of clients is empty at that point, so no performance penalty there)
        // Client(const Client &) = delete;
        // Client &operator=(const Client &) = delete;
        // Client(Client &&) = default;
        // Client &operator=(Client &&) = default;

        KDirWatch *instance;
        int count;
        // did the instance stop watching
        bool watchingStopped;
        // events blocked when stopped
        int pending;
        KDirWatch::WatchModes m_watchModes;
    };

    class Entry
    {
    public:
        ~Entry();
        // instances interested in events
        std::vector<Client> m_clients;
        // nonexistent entries of this directory
        QList<Entry *> m_entries;
        QString path;

        // the last observed modification time
        time_t m_ctime;
        // last observed inode
        ino_t m_ino;
        // the last observed link count
        int m_nlink;
        entryStatus m_status;
        entryMode m_mode;
        int msecLeft, freq;
        bool isDir;

        QString parentDirectory() const;
        void addClient(KDirWatch *, KDirWatch::WatchModes);
        void removeClient(KDirWatch *);
        int clientCount() const;
        bool isValid()
        {
            return !m_clients.empty() || !m_entries.empty();
        }

        Entry *findSubEntry(const QString &path) const
        {
            for (Entry *sub_entry : std::as_const(m_entries)) {
                if (sub_entry->path == path) {
                    return sub_entry;
                }
            }
            return nullptr;
        }

        bool dirty;
        void propagate_dirty();

        QList<const Client *> clientsForFileOrDir(const QString &tpath, bool *isDir) const;
        QList<const Client *> inotifyClientsForFileOrDir(bool isDir) const;

        std::vector<Client>::iterator findInstance(KDirWatch *other)
        {
            return std::find_if(m_clients.begin(), m_clients.end(), [other](const Client &client) {
                return client.instance == other;
            });
        }

#if HAVE_SYS_INOTIFY_H
        int wd;
        // Creation and Deletion of files happens infrequently, so
        // can safely be reported as they occur.  File changes i.e. those that emit "dirty()" can
        // happen many times per second, though, so maintain a list of files in this directory
        // that can be emitted and flushed at the next slotRescan(...).
        // This will be unused if the Entry is not a directory.
        QList<QString> m_pendingFileChanges;
#endif
    };

    typedef QMap<QString, Entry> EntryMap;

    KDirWatchPrivate();
    ~KDirWatchPrivate() override;

    void resetList(KDirWatch *instance, bool skippedToo);
    void useFreq(Entry *e, int newFreq);
    void addEntry(KDirWatch *instance, const QString &_path, Entry *sub_entry, bool isDir, KDirWatch::WatchModes watchModes = KDirWatch::WatchDirOnly);
    void removeEntry(KDirWatch *instance, const QString &path, Entry *sub_entry);
    void removeEntry(KDirWatch *instance, Entry *e, Entry *sub_entry);
    bool stopEntryScan(KDirWatch *instance, Entry *e);
    bool restartEntryScan(KDirWatch *instance, Entry *e, bool notify);
    void stopScan(KDirWatch *instance);
    void startScan(KDirWatch *instance, bool notify, bool skippedToo);

    void removeEntries(KDirWatch *instance);

    void addWatch(Entry *entry);
    void removeWatch(Entry *entry);
    Entry *entry(const QString &_path);
    int scanEntry(Entry *e);
    void emitEvent(Entry *e, int event, const QString &fileName = QString());

    static bool isNoisyFile(const char *filename);

    void ref(KDirWatch *watch);
    void unref(KDirWatch *watch);

#if HAVE_SYS_INOTIFY_H
    QString inotifyEventName(const inotify_event *event) const;
#endif

public Q_SLOTS:
    void slotRescan();
    void inotifyEventReceived(); // for inotify
    void slotRemoveDelayed();
    void fswEventReceived(const QString &path); // for QFileSystemWatcher

public:
    QTimer m_statRescanTimer;
    EntryMap m_mapEntries;

    KDirWatch::Method m_preferredMethod, m_nfsPreferredMethod;
    int freq;
    int statEntries;
    int m_nfsPollInterval, m_PollInterval;
    bool useStat(Entry *e);

    // removeList is allowed to contain any entry at most once
    QSet<Entry *> removeList;
    bool delayRemove;

    bool rescan_all;
    QTimer rescan_timer;

#if HAVE_SYS_INOTIFY_H
    QSocketNotifier *mSn;
    bool supports_inotify;
    int m_inotify_fd;
    QHash<int, Entry *> m_inotify_wd_to_entry;

    bool useINotify(Entry *e);
#endif
#if HAVE_QFILESYSTEMWATCHER
    QFileSystemWatcher *fsWatcher;
    bool useQFSWatch(Entry *e);
#endif

    bool _isStopped;

private:
    // Public objects that reference this thread-local private instance.
    QList<KDirWatch *> m_referencesObjects;
};

QDebug operator<<(QDebug debug, const KDirWatchPrivate &dwp);
QDebug operator<<(QDebug debug, const KDirWatchPrivate::Entry &entry);

#endif // KDIRWATCH_P_H
