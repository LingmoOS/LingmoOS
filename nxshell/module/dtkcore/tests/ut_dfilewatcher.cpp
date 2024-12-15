// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include <QDir>
#include <QSignalSpy>
#include <QTest>
#include <QUrl>
#include "filesystem/dfilewatcher.h"

DCORE_USE_NAMESPACE


class ut_DFileWatcher : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    DFileWatcher *fileWatcher = nullptr;

};

void ut_DFileWatcher::SetUp()
{
    QDir dir("/tmp/etc/");
    if (!dir.exists())
        dir.mkdir("/tmp/etc/");
    QFile file("/tmp/etc/test");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;
    file.close();
    fileWatcher = new DFileWatcher("/tmp/etc/test");

}

void ut_DFileWatcher::TearDown()
{
    if (fileWatcher) {
        delete fileWatcher;
        fileWatcher = nullptr;
    }
    QDir dir("/tmp/etc/");
    if (dir.exists())
        dir.remove("/tmp/etc/");
    QFile file("/tmp/etc/test");
    if (file.exists())
        file.remove();
    QFile file1("/tmp/etc/test1");
    if (file1.exists())
        file1.remove();
}

TEST_F(ut_DFileWatcher, testDFileWatcherFileUrl)
{
    QUrl url = fileWatcher->fileUrl();
    ASSERT_TRUE(url.toString() == "file:///tmp/etc/test");
}

TEST_F(ut_DFileWatcher, testDFileWatcherStartWatcher)
{
    if (!fileWatcher->startWatcher()) return;

    fileWatcher->setEnabledSubfileWatcher(QUrl());
    ASSERT_TRUE(fileWatcher->startWatcher());
}

TEST_F(ut_DFileWatcher, testDFileWatcherStopWatcher)
{
    if (!fileWatcher->startWatcher()) return;

    ASSERT_TRUE(fileWatcher->startWatcher());
    ASSERT_TRUE(fileWatcher->stopWatcher());
}

TEST_F(ut_DFileWatcher, testDFileWatcherRestartWatcher)
{
    if (!fileWatcher->startWatcher()) return;

    ASSERT_TRUE(fileWatcher->startWatcher());
    ASSERT_TRUE(fileWatcher->restartWatcher());
}

TEST_F(ut_DFileWatcher, testDFileSystemWatcherFileDeleted)
{
    if (!fileWatcher->startWatcher()) return;

    ASSERT_TRUE(fileWatcher->startWatcher());
    QSignalSpy spy(fileWatcher, &DBaseFileWatcher::fileDeleted);
    QFile file("/tmp/etc/test");
    if (file.exists())
        file.remove();
    ASSERT_TRUE(QTest::qWaitFor([&spy](){
        return spy.count() >= 1;
    }, 1000));
    ASSERT_TRUE(spy.count() >= 1);
}

TEST_F(ut_DFileWatcher, testDFileSystemWatcherFileAttributeChanged)
{
    if (!fileWatcher->startWatcher()) return;

    ASSERT_TRUE(fileWatcher->startWatcher());
    QSignalSpy spy(fileWatcher, &DBaseFileWatcher::fileAttributeChanged);
    QFile file("/tmp/etc/test");
    if (file.exists()) {
        file.remove();
    }
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;
    file.close();

    ASSERT_TRUE(QTest::qWaitFor([&spy](){
        return spy.count() >= 1;
    }, 1000));
    ASSERT_TRUE(spy.count() >= 1);
}


TEST_F(ut_DFileWatcher, testDFileSystemWatcherFileMoved)
{
    if (!fileWatcher->startWatcher()) return;

    ASSERT_TRUE(fileWatcher->startWatcher());
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QSignalSpy spy(fileWatcher, &DBaseFileWatcher::fileMoved);
#else
    QSignalSpy spy(fileWatcher, &DBaseFileWatcher::fileDeleted);
#endif
    QString oldFile("/tmp/etc/test");
    QString newFile("/tmp/etc/test1");
    QFile::rename(oldFile, newFile);

    ASSERT_TRUE(QTest::qWaitFor([&spy](){
        return spy.count() >= 1;
    }, 1000));
    ASSERT_TRUE(spy.count() >= 1);
}

TEST_F(ut_DFileWatcher, testDFileSystemWatcherSubfileCreated)
{
    if (!fileWatcher->startWatcher()) return;

    ASSERT_TRUE(fileWatcher->startWatcher());
    QSignalSpy spy(fileWatcher, &DBaseFileWatcher::subfileCreated);
    QFile file("/tmp/etc/test");
    if (file.exists()) {
        file.remove();
    }
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    ASSERT_TRUE(QTest::qWaitFor([&spy](){
        return spy.count() >= 1;
    }, 1000));
    ASSERT_TRUE(spy.count() >= 1);
}

TEST_F(ut_DFileWatcher, testDFileSystemWatcherFileModified)
{
    if (!fileWatcher->startWatcher()) return;

    ASSERT_TRUE(fileWatcher->startWatcher());
    QSignalSpy spy(fileWatcher, &DBaseFileWatcher::fileModified);
    QFile file("/tmp/etc/test");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }
    QTextStream out(&file);
    out << "hello";
    file.close();
    ASSERT_TRUE(QTest::qWaitFor([&spy](){
        return spy.count() >= 1;
    }, 1000));
    ASSERT_TRUE(spy.count() >= 1);
}

TEST_F(ut_DFileWatcher, testDFileSystemWatcherFileClosed)
{
    if (!fileWatcher->startWatcher()) return;

    ASSERT_TRUE(fileWatcher->startWatcher());
    QSignalSpy spy(fileWatcher, &DBaseFileWatcher::fileClosed);
    QFile file("/tmp/etc/test");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }
    file.close();
    ASSERT_TRUE(QTest::qWaitFor([&spy](){
        return spy.count() >= 1;
    }, 1000));
    ASSERT_TRUE(spy.count() >= 1);
}
