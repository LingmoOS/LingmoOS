/*
    This file is part of the KDE Project
    SPDX-FileCopyrightText: 2007-2011 Sebastian Trueg <trueg@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef BALOO_FILE_WATCH_H_
#define BALOO_FILE_WATCH_H_

#include <QObject>
#include "pendingfile.h"

class KInotify;

namespace Baloo
{
class Database;
class MetadataMover;
class FileIndexerConfig;
class PendingFileQueue;
class FileWatchTest;

class FileWatch : public QObject
{
    Q_OBJECT

public:
    FileWatch(Database* db, FileIndexerConfig* config, QObject* parent = nullptr);
    ~FileWatch() override;

public Q_SLOTS:
    /**
     * To be called whenever the list of indexed/excluded folders in the config
     * changes.
     */
    void updateIndexedFoldersWatches();

Q_SIGNALS:
    void indexNewFile(const QString& string);
    void indexModifiedFile(const QString& string);
    void indexXAttr(const QString& path);

    /**
     * This signal is emitted when a file has been removed, and everyone else
     * should update their caches
     */
    void fileRemoved(const QString& path);

    void installedWatches();

private Q_SLOTS:
    void slotFileMoved(const QString& from, const QString& to);
    void slotFileDeleted(const QString& urlString, bool isDir);
    void slotFileCreated(const QString& path, bool isDir);
    void slotFileClosedAfterWrite(const QString&);
    void slotAttributeChanged(const QString& path);
    void slotFileModified(const QString& path);
    void slotInotifyWatchUserLimitReached(const QString&);

private:
    /** Watch a folder, provided it is not already watched*/
    void watchFolder(const QString& path);

    Database* m_db;

    MetadataMover* m_metadataMover;
    FileIndexerConfig* m_config;

    KInotify* m_dirWatch;

    /// queue used to "compress" multiple file events like downloads
    PendingFileQueue* m_pendingFileQueue;

    QStringList m_includedFolders;
    QStringList m_excludedFolders;

    friend class FileWatchTest;
};
}

#endif
