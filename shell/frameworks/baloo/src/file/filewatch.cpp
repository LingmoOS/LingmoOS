/*
    This file is part of the KDE Project
    SPDX-FileCopyrightText: 2007-2011 Sebastian Trueg <trueg@kde.org>
    SPDX-FileCopyrightText: 2012-2014 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "filewatch.h"
#include "metadatamover.h"
#include "fileindexerconfig.h"
#include "pendingfilequeue.h"
#include "regexpcache.h"
#include "database.h"
#include "baloodebug.h"

#include "kinotify.h"

#include <QDateTime>
#include <QFileInfo>
#include <QDir>

using namespace Baloo;

FileWatch::FileWatch(Database* db, FileIndexerConfig* config, QObject* parent)
    : QObject(parent)
    , m_db(db)
    , m_config(config)
    , m_dirWatch(nullptr)
{
    Q_ASSERT(db);
    Q_ASSERT(config);

    m_metadataMover = new MetadataMover(m_db, this);
    connect(m_metadataMover, &MetadataMover::movedWithoutData, this, &FileWatch::indexNewFile);
    connect(m_metadataMover, &MetadataMover::fileRemoved, this, &FileWatch::fileRemoved);

    m_pendingFileQueue = new PendingFileQueue(this);
    connect(m_pendingFileQueue, &PendingFileQueue::indexNewFile, this, &FileWatch::indexNewFile);
    connect(m_pendingFileQueue, &PendingFileQueue::indexModifiedFile, this, &FileWatch::indexModifiedFile);
    connect(m_pendingFileQueue, &PendingFileQueue::indexXAttr, this, &FileWatch::indexXAttr);
    connect(m_pendingFileQueue, &PendingFileQueue::removeFileIndex, m_metadataMover, &MetadataMover::removeFileMetadata);

    // monitor the file system for changes (restricted by the inotify limit)
    m_dirWatch = new KInotify(m_config, this);

    connect(m_dirWatch, &KInotify::moved, this, &FileWatch::slotFileMoved);
    connect(m_dirWatch, &KInotify::deleted, this, &FileWatch::slotFileDeleted);
    connect(m_dirWatch, &KInotify::created, this, &FileWatch::slotFileCreated);
    connect(m_dirWatch, &KInotify::modified, this, &FileWatch::slotFileModified);
    connect(m_dirWatch, &KInotify::closedWrite, this, &FileWatch::slotFileClosedAfterWrite);
    connect(m_dirWatch, &KInotify::attributeChanged, this, &FileWatch::slotAttributeChanged);
    connect(m_dirWatch, &KInotify::watchUserLimitReached, this, &FileWatch::slotInotifyWatchUserLimitReached);
    connect(m_dirWatch, &KInotify::installedWatches, this, &FileWatch::installedWatches);
}

FileWatch::~FileWatch()
{
}

// FIXME: listen to Create for folders!
void FileWatch::watchFolder(const QString& path)
{
    if (m_dirWatch && !m_dirWatch->watchingPath(path)) {
        KInotify::WatchEvents flags(KInotify::EventMove | KInotify::EventDelete | KInotify::EventDeleteSelf
                                    | KInotify::EventCloseWrite | KInotify::EventCreate
                                    | KInotify::EventAttributeChange | KInotify::EventModify);

        m_dirWatch->addWatch(path, flags, KInotify::FlagOnlyDir);
    }
}

void FileWatch::slotFileMoved(const QString& urlFrom, const QString& urlTo)
{
    if (m_config->shouldBeIndexed(urlTo)) {
        m_metadataMover->moveFileMetadata(urlFrom, urlTo);
    } else {
        QFileInfo src(urlFrom);
        QString url = urlFrom;

        if (src.isDir() && !url.endsWith(QLatin1Char('/'))) {
            url.append(QLatin1Char('/'));
        }

        PendingFile file(url);
        file.setDeleted();

        m_pendingFileQueue->enqueue(file);
    }
}

void FileWatch::slotFileDeleted(const QString& urlString, bool isDir)
{
    // Directories must always end with a trailing slash '/'
    QString url = urlString;
    if (isDir && !url.endsWith(QLatin1Char('/'))) {
        url.append(QLatin1Char('/'));
    }

    PendingFile file(url);
    file.setDeleted();

    m_pendingFileQueue->enqueue(file);
}

void FileWatch::slotFileCreated(const QString& path, bool isDir)
{
    Q_UNUSED(isDir);
    PendingFile file(path);
    file.setCreated();

    m_pendingFileQueue->enqueue(file);
}

void FileWatch::slotFileModified(const QString& path)
{
    PendingFile file(path);
    file.setModified();

    //qCDebug(BALOO) << "MOD" << path;
    m_pendingFileQueue->enqueue(file);
}

void FileWatch::slotFileClosedAfterWrite(const QString& path)
{
    QDateTime current = QDateTime::currentDateTime();
    QDateTime fileModification = QFileInfo(path).lastModified();
    QDateTime dirModification = QFileInfo(QFileInfo(path).absoluteDir().absolutePath()).lastModified();

    // If we have received a FileClosedAfterWrite event, then the file must have been
    // closed within the last minute.
    // This is being done cause many applications open the file under write mode, do not
    // make any modifications and then close the file. This results in us getting
    // the FileClosedAfterWrite event
    if (fileModification.secsTo(current) <= 1000 * 60 || dirModification.secsTo(current) <= 1000 * 60) {
        PendingFile file(path);
        file.setClosedOnWrite();
        //qCDebug(BALOO) << "CLOSE" << path;
        m_pendingFileQueue->enqueue(file);
    }
}

void FileWatch::slotAttributeChanged(const QString& path)
{
    PendingFile file(path);
    file.setAttributeChanged();

    m_pendingFileQueue->enqueue(file);
}

// This slot is connected to a signal emitted in KInotify when
// inotify_add_watch fails with ENOSPC.
void FileWatch::slotInotifyWatchUserLimitReached(const QString&)
{
    // If we got here, we hit the limit and are not allowed to raise it,
    // so put something in the log.
    qCWarning(BALOO) << "KDE Baloo File Indexer has reached the inotify folder watch limit. File changes will be ignored.";
    // we do it the brutal way for now hoping with new kernels and defaults this will never happen
    // Delete the KInotify
    // FIXME: Maybe we should be aborting?
    if (m_dirWatch) {
        m_dirWatch->deleteLater();
        m_dirWatch = nullptr;
    }
    Q_EMIT installedWatches();
}

void FileWatch::updateIndexedFoldersWatches()
{
    if (m_dirWatch) {
        const QStringList excludedFolders = m_config->excludeFolders();
        const QStringList includedFolders = m_config->includeFolders();

        for (const QString& folder : excludedFolders) {
            // Remove watch for new excluded folders
            if (!m_excludedFolders.contains(folder)) {
                m_dirWatch->removeWatch(folder);
            }
        }
        for (const QString& folder : m_excludedFolders) {
            // Add no longer excluded folders
            if (!excludedFolders.contains(folder)) {
                watchFolder(folder);
            }
        }

        for (const QString& folder : m_includedFolders) {
            // Remove no longer included folders
            if (!includedFolders.contains(folder)) {
                m_dirWatch->removeWatch(folder);
            }
        }
        for (const QString& folder : includedFolders) {
            if (!m_includedFolders.contains(folder)) {
                watchFolder(folder);
            }
        }

        m_excludedFolders = excludedFolders;
        m_includedFolders = includedFolders;
    }
}

#include "moc_filewatch.cpp"
