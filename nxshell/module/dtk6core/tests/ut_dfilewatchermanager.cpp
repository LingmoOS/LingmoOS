// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include <QObject>
#include <QDir>
#include <QFile>
#include <QUrl>
#include <QSignalSpy>
#include <QTest>
#include <QTemporaryFile>
#include "filesystem/dfilewatcher.h"
#include "filesystem/dfilewatchermanager.h"

DCORE_USE_NAMESPACE

class ut_DFileWatcherManager : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    DFileWatcherManager *fileWatcherManager = nullptr;
};

void ut_DFileWatcherManager::SetUp()
{
    fileWatcherManager = new DFileWatcherManager(nullptr);
}

void ut_DFileWatcherManager::TearDown()
{
    if (fileWatcherManager) {
        delete fileWatcherManager;
        fileWatcherManager = nullptr;
    }
}

TEST_F(ut_DFileWatcherManager, testDFileWatcherManagerAdd)
{
    QTemporaryFile tmpfile;
    if (!tmpfile.open())
        return;

    auto watcher = fileWatcherManager->add(tmpfile.fileName());
    if (!watcher->startWatcher())
        return;

    // test fileDeleted signal
    QSignalSpy spy(watcher, &DBaseFileWatcher::fileDeleted);

    if (tmpfile.exists())
        tmpfile.remove();
    ASSERT_TRUE(QTest::qWaitFor([&spy]() { return spy.count() >= 1; }, 1000));
    ASSERT_TRUE(spy.count() >= 1);

    watcher->stopWatcher();
}

TEST_F(ut_DFileWatcherManager, testDFileWatcherManagerRemove)
{
    fileWatcherManager->add("/tmp/test");
    fileWatcherManager->remove("/tmp/test");
    ASSERT_EQ(fileWatcherManager->watchedFiles().count(), 0);
}

TEST_F(ut_DFileWatcherManager, testDFileWatcherManagerRemoveAll)
{
    fileWatcherManager->add("/tmp/test");
    fileWatcherManager->add("/tmp/test1");
    fileWatcherManager->removeAll();
    ASSERT_EQ(fileWatcherManager->watchedFiles().count(), 0);
}

TEST_F(ut_DFileWatcherManager, testDFileSystemWatcherwatchedFiles)
{
    fileWatcherManager->add("/tmp/test");
    fileWatcherManager->add("/tmp/test1");
    fileWatcherManager->watchedFiles();
    ASSERT_EQ(fileWatcherManager->watchedFiles().count(), 2);
}
