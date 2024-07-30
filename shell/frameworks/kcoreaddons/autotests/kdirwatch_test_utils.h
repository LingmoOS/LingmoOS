/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2009 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <kdirwatch.h>

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QTest>
#include <QThread>

#include "kcoreaddons_debug.h"

class StaticObject
{
public:
    KDirWatch m_dirWatch;
};
Q_GLOBAL_STATIC(StaticObject, s_staticObject)

class StaticObjectUsingSelf // like KSambaShare does, bug 353080
{
public:
    StaticObjectUsingSelf()
    {
        KDirWatch::self();
    }
    ~StaticObjectUsingSelf()
    {
        if (KDirWatch::exists() && KDirWatch::self()->contains(QDir::homePath())) {
            KDirWatch::self()->removeDir(QDir::homePath());
        }
    }
};
Q_GLOBAL_STATIC(StaticObjectUsingSelf, s_staticObjectUsingSelf)

namespace KDirWatchTestUtils
{
// Just to make the inotify packets bigger
static const char s_filePrefix[] = "This_is_a_test_file_";

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
    return "ERROR!";
}

// helper method: create a file
inline void createFile(const QString &path, bool slow = false)
{
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
#ifdef Q_OS_FREEBSD
    // FreeBSD has inotify implemented as user-space library over native kevent API.
    // When using it, one has to open() a file to start watching it, so workaround
    // test breakage by giving inotify time to react to file creation.
    // Full context: https://github.com/libinotify-kqueue/libinotify-kqueue/issues/10
    if (!slow) {
        QThread::msleep(1);
    }
#else
    Q_UNUSED(slow)
#endif
    file.write(QByteArray("foo"));
    file.close();
}

inline int createDirectoryTree(const QString &basePath, int depth = 4)
{
    int filesCreated = 0;

    const int numFiles = 10;
    for (int i = 0; i < numFiles; ++i) {
        createFile(basePath + QLatin1Char('/') + QLatin1String(s_filePrefix) + QString::number(i));
        ++filesCreated;
    }

    if (depth <= 0) {
        return filesCreated;
    }

    const int numFolders = 5;
    for (int i = 0; i < numFolders; ++i) {
        const QString childPath = basePath + QLatin1String("/subdir") + QString::number(i);
        QDir().mkdir(childPath);
        filesCreated += createDirectoryTree(childPath, depth - 1);
    }

    return filesCreated;
}

inline void waitUntilAfter(const QDateTime &ctime)
{
    int totalWait = 0;
    QDateTime now;
    Q_FOREVER {
        now = QDateTime::currentDateTime();
        if (now.toMSecsSinceEpoch() / 1000 == ctime.toMSecsSinceEpoch() / 1000) // truncate milliseconds
        {
            totalWait += 50;
            QTest::qWait(50);
        } else {
            QVERIFY(now > ctime); // can't go back in time ;)
            QTest::qWait(50); // be safe
            break;
        }
    }
    // if (totalWait > 0)
    qCDebug(KCOREADDONS_DEBUG) << "Waited" << totalWait << "ms so that now" << now.toString(Qt::ISODate) << "is >" << ctime.toString(Qt::ISODate);
}
inline void waitUntilMTimeChange(const QString &path)
{
    // Wait until the current second is more than the file's mtime
    // otherwise this change will go unnoticed

    QFileInfo fi(path);
    QVERIFY(fi.exists());
    const QDateTime ctime = fi.lastModified();
    waitUntilAfter(ctime);
}

inline void waitUntilNewSecond()
{
    QDateTime now = QDateTime::currentDateTime();
    waitUntilAfter(now);
}

}
