/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2011 Sebastian Trueg <trueg@kde.org>
    SPDX-FileCopyrightText: 2013-2014 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef PENDINGFILEQUEUE_H
#define PENDINGFILEQUEUE_H

#include "pendingfile.h"

#include <QObject>
#include <QString>
#include <QHash>
#include <QTimer>
#include <QVector>

namespace Baloo {

class PendingFileQueueTest;

/**
 *
 */
class PendingFileQueue : public QObject
{
    Q_OBJECT

public:
    explicit PendingFileQueue(QObject* parent = nullptr);
    ~PendingFileQueue() override;

Q_SIGNALS:
    void indexNewFile(const QString& fileUrl);
    void indexModifiedFile(const QString& fileUrl);
    void indexXAttr(const QString& fileUrl);
    void removeFileIndex(const QString& fileUrl);

public Q_SLOTS:
    void enqueue(const PendingFile& file);

    /**
     * The number of seconds the file should be tracked after it has
     * been emitted. This defaults to 2 minutes
     */
    void setTrackingTime(int seconds);

    /**
     * Set the minimum amount of seconds a file should be kept in the queue
     * on receiving successive modifications events.
     */
    void setMinimumTimeout(int seconds);
    void setMaximumTimeout(int seconds);

private Q_SLOTS:
    void processCache(const QTime& currentTime);
    void processPendingFiles(const QTime& currentTime);
    void clearRecentlyEmitted(const QTime& currentTime);

private:
    QVector<PendingFile> m_cache;

    QTimer m_cacheTimer;
    QTimer m_clearRecentlyEmittedTimer;
    QTimer m_pendingFilesTimer;

    /**
     * Holds the list of files that were recently emitted along with
     * the time they were last emitted.
     */
    QHash<QString, QTime> m_recentlyEmitted;

    /**
     * The QTime contains the time when these file events should be processed.
     */
    QHash<QString, QTime> m_pendingFiles;

    int m_minTimeout;
    int m_maxTimeout;
    int m_trackingTime;

    friend class PendingFileQueueTest;
};

}

#endif
