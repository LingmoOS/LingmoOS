/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2009 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <kdirwatch.h>

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <QThread>
#include <sys/stat.h>
#ifdef Q_OS_UNIX
#include <unistd.h> // ::link()
#endif

#include "config-tests.h"
#include "kcoreaddons_debug.h"

#include "kdirwatch_test_utils.h"

using namespace KDirWatchTestUtils;

class KDirWatch_UnitTest : public QObject
{
    Q_OBJECT
public:
    KDirWatch_UnitTest()
    {
        // Speed up the test by making the kdirwatch timer (to compress changes) faster
        qputenv("KDIRWATCH_POLLINTERVAL", "50");
        qputenv("KDIRWATCH_METHOD", KDIRWATCH_TEST_METHOD);
        s_staticObjectUsingSelf();

        m_path = m_tempDir.path() + QLatin1Char('/');
        KDirWatch *dirW = &s_staticObject()->m_dirWatch;
        qCDebug(KCOREADDONS_DEBUG) << "Using method" << methodToString(dirW->internalMethod());
    }

private Q_SLOTS: // test methods
    void initTestCase()
    {
        QFileInfo pathInfo(m_path);
        QVERIFY(pathInfo.isDir() && pathInfo.isWritable());

        // By creating the files upfront, we save waiting a full second for an mtime change
        createFile(m_path + QLatin1String("ExistingFile"));
        createFile(m_path + QLatin1String("TestFile"));
        createFile(m_path + QLatin1String("nested_0"));
        createFile(m_path + QLatin1String("nested_1"));

        s_staticObject()->m_dirWatch.addFile(m_path + QLatin1String("ExistingFile"));
    }
    void benchCreateTree();
    void benchCreateWatcher();
    void benchNotifyWatcher();

private:
    QTemporaryDir m_tempDir;
    QString m_path;
};

QTEST_MAIN(KDirWatch_UnitTest)

void KDirWatch_UnitTest::benchCreateTree()
{
#if !ENABLE_BENCHMARKS
    QSKIP("Benchmarks are disabled in debug mode");
#endif
    QTemporaryDir dir;

    QBENCHMARK {
        createDirectoryTree(dir.path());
    }
}

void KDirWatch_UnitTest::benchCreateWatcher()
{
#if !ENABLE_BENCHMARKS
    QSKIP("Benchmarks are disabled in debug mode");
#endif
    QTemporaryDir dir;
    createDirectoryTree(dir.path());

    QBENCHMARK {
        KDirWatch watch;
        watch.addDir(dir.path(), KDirWatch::WatchSubDirs | KDirWatch::WatchFiles);
    }
}

void KDirWatch_UnitTest::benchNotifyWatcher()
{
#if !ENABLE_BENCHMARKS
    QSKIP("Benchmarks are disabled in debug mode");
#endif
    QTemporaryDir dir;
    // create the dir once upfront
    auto numFiles = createDirectoryTree(dir.path());
    waitUntilMTimeChange(dir.path());

    KDirWatch watch;
    watch.addDir(dir.path(), KDirWatch::WatchSubDirs | KDirWatch::WatchFiles);

    // now touch all the files repeatedly and wait for the dirty updates to come in
    QSignalSpy spy(&watch, &KDirWatch::dirty);
    QBENCHMARK {
        createDirectoryTree(dir.path());
        QTRY_COMPARE_WITH_TIMEOUT(spy.count(), numFiles, 30000);
        spy.clear();
    }
}

#include "kdirwatch_benchmarktest.moc"
