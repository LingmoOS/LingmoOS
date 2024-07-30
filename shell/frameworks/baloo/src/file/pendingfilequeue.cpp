/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2011 Sebastian Trueg <trueg@kde.org>
    SPDX-FileCopyrightText: 2013-2014 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "pendingfilequeue.h"

#include <memory>

#include <QDateTime>

using namespace Baloo;

PendingFileQueue::PendingFileQueue(QObject* parent)
    : QObject(parent)
{
    m_cacheTimer.setInterval(10);
    m_cacheTimer.setSingleShot(true);
    connect(&m_cacheTimer, &QTimer::timeout, [this] {
            PendingFileQueue::processCache(QTime::currentTime());
    });

    m_trackingTime = 120 * 1000;

    m_clearRecentlyEmittedTimer.setInterval(m_trackingTime);
    m_clearRecentlyEmittedTimer.setSingleShot(true);
    connect(&m_clearRecentlyEmittedTimer, &QTimer::timeout, [this] {
            PendingFileQueue::clearRecentlyEmitted(QTime::currentTime());
    });

    m_minTimeout = 5 * 1000;
    m_maxTimeout = 60 * 1000;
    m_pendingFilesTimer.setInterval(m_minTimeout);
    m_pendingFilesTimer.setSingleShot(true);
    connect(&m_pendingFilesTimer, &QTimer::timeout, [this] {
            PendingFileQueue::processPendingFiles(QTime::currentTime());
    });
}

PendingFileQueue::~PendingFileQueue()
{
}

void PendingFileQueue::enqueue(const PendingFile& file)
{
    // If we get an event to remove /home/A, remove all events for everything under /home/A/

    if (file.shouldRemoveIndex() && file.path().endsWith(QLatin1Char('/'))) {
        const auto keepFile = [&file](const PendingFile& pending) {
            return !pending.path().startsWith(file.path());
        };
        const auto end = m_cache.end();
        // std::partition moves all matching entries to the first partition
        const auto droppedFilesBegin = std::partition(m_cache.begin(), end, keepFile);
        for (auto it = droppedFilesBegin; it != end; it++) {
            m_pendingFiles.remove(it->path());
            m_recentlyEmitted.remove(it->path());
        }
        m_cache.erase(droppedFilesBegin, end);
    }

    if (file.shouldRemoveIndex()) {
        m_cache.removeOne(file);
        m_pendingFiles.remove(file.path());
        Q_EMIT removeFileIndex(file.path());
        return;
    }

    int i = m_cache.indexOf(file);
    if (i == -1) {
        m_cache << file;
    } else {
        m_cache[i].merge(file);
    }

    m_cacheTimer.start();
}

void PendingFileQueue::processCache(const QTime& currentTime)
{
    for (const PendingFile& file : std::as_const(m_cache)) {
        if (file.shouldIndexXAttrOnly()) {
            Q_EMIT indexXAttr(file.path());
        }
        else if (file.shouldIndexContents()) {
            if (m_pendingFiles.contains(file.path())) {
                QTime time = m_pendingFiles[file.path()];

                int msecondsLeft = currentTime.msecsTo(time);
                msecondsLeft = qBound(m_minTimeout, msecondsLeft * 2, m_maxTimeout);

                time = currentTime.addMSecs(msecondsLeft);
                m_pendingFiles[file.path()] = time;
            }
            else if (m_recentlyEmitted.contains(file.path())) {
                QTime time = currentTime.addMSecs(m_minTimeout);
                m_pendingFiles[file.path()] = time;
            }
            else {
                if (file.isNewFile()) {
                    Q_EMIT indexNewFile(file.path());
                } else {
                    Q_EMIT indexModifiedFile(file.path());
                }
                m_recentlyEmitted.insert(file.path(), currentTime);
            }
        } else {
            Q_ASSERT_X(false, "FileWatch", "The PendingFile should always have some flags set");
        }
    }

    m_cache.clear();

    if (!m_pendingFiles.isEmpty() && !m_pendingFilesTimer.isActive()) {
        m_pendingFilesTimer.setInterval(m_minTimeout);
        m_pendingFilesTimer.start();
    }

    if (!m_recentlyEmitted.isEmpty() && !m_clearRecentlyEmittedTimer.isActive()) {
        m_clearRecentlyEmittedTimer.setInterval(m_trackingTime);
        m_clearRecentlyEmittedTimer.start();
    }
}

void PendingFileQueue::clearRecentlyEmitted(const QTime& time)
{
    int nextUpdate = m_trackingTime;

    QMutableHashIterator<QString, QTime> it(m_recentlyEmitted);
    while (it.hasNext()) {
        it.next();

        int msecondsSinceEmitted = it.value().msecsTo(time);
        if (msecondsSinceEmitted >= m_trackingTime) {
            it.remove();
        } else {
            int timeLeft = m_trackingTime - msecondsSinceEmitted;
            nextUpdate = qMin(nextUpdate, timeLeft);
        }
    }

    if (!m_recentlyEmitted.isEmpty()) {
        m_clearRecentlyEmittedTimer.setInterval(nextUpdate);
        m_clearRecentlyEmittedTimer.start();
    }
}

void PendingFileQueue::processPendingFiles(const QTime& currentTime)
{
    int nextUpdate = m_maxTimeout;

    QMutableHashIterator<QString, QTime> it(m_pendingFiles);
    while (it.hasNext()) {
        it.next();

        int mSecondsLeft = currentTime.msecsTo(it.value());
        if (mSecondsLeft <= 0) {
            Q_EMIT indexModifiedFile(it.key());
            m_recentlyEmitted.insert(it.key(), currentTime);

            it.remove();
        }
        else {
            nextUpdate = qMin(mSecondsLeft, nextUpdate);
        }
    }

    if (!m_pendingFiles.isEmpty()) {
        m_pendingFilesTimer.setInterval(nextUpdate);
        m_pendingFilesTimer.start();
    }

    if (!m_recentlyEmitted.isEmpty() && !m_clearRecentlyEmittedTimer.isActive()) {
        m_clearRecentlyEmittedTimer.setInterval(m_trackingTime);
        m_clearRecentlyEmittedTimer.start();
    }
}

void PendingFileQueue::setTrackingTime(int seconds)
{
    m_trackingTime = seconds * 1000;
}

void PendingFileQueue::setMinimumTimeout(int seconds)
{
    m_minTimeout = seconds * 1000;
}

void PendingFileQueue::setMaximumTimeout(int seconds)
{
    m_maxTimeout = seconds * 1000;
}

#include "moc_pendingfilequeue.cpp"
