// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>
#include <QSignalSpy>

#include "ut_Head.h"
#include "stub.h"

#define private public
#include "PageDriverManager.h"
#include "PageListView.h"

void ut_installNextDriver()
{
    return;
}

void ut_backupNextDriver()
{
    return;
}


class PageDriverManager_UT : public UT_HEAD
{
public:
    void SetUp()
    {
        stub.set(ADDR(PageDriverManager, installNextDriver), ut_installNextDriver);
        stub.set(ADDR(PageDriverManager, backupNextDriver), ut_backupNextDriver);
        m_dPageDriverManager = new PageDriverManager;
    }
    void TearDown()
    {
    }
    PageDriverManager *m_dPageDriverManager;
    Stub stub;
};

TEST_F(PageDriverManager_UT, ut_updateListView)
{
    QList<QPair<QString, QString> > list;
    list.append(QPair<QString, QString>("Driver Install", "driverinstall##Overview"));
    list.append(QPair<QString, QString>("Driver Backup", "driverbackup##Overview"));
    list.append(QPair<QString, QString>("Driver Restore", "driverrestore##Overview"));
    m_dPageDriverManager->updateListView(list);

    m_dPageDriverManager->mp_ListView->setCurType("Driver Install");

    EXPECT_EQ("Driver Install", m_dPageDriverManager->mp_ListView->currentType());
}

TEST_F(PageDriverManager_UT, ut_addDriverInfo)
{
    DriverInfo *info = new DriverInfo();
    info->m_Name = "Sharp MX-C2622R PS, 1.1";
    info->m_DebVersion = "1.0.0";
    info->m_Type = DR_Tablet;
    info->m_Packages = "com.sharp.griffin2light";
    info->m_Status = ST_CAN_UPDATE;

    m_dPageDriverManager->addDriverInfo(info);
    EXPECT_EQ(1, m_dPageDriverManager->m_ListDriverInfo.size());
}

TEST_F(PageDriverManager_UT, ut_scanDriverInfo)
{
    m_dPageDriverManager->clearAllData();
    m_dPageDriverManager->scanDriverInfo();
    EXPECT_TRUE(m_dPageDriverManager->isScanning());
}

TEST_F(PageDriverManager_UT, ut_slotScanFinished)
{
    m_dPageDriverManager->clearAllData();
    m_dPageDriverManager->testDevices();
    m_dPageDriverManager->slotScanFinished(SR_SUCESS);
    EXPECT_EQ(4, m_dPageDriverManager->m_ListDriverInfo.size());
}

TEST_F(PageDriverManager_UT, ut_slotDriverOperationClicked)
{
    m_dPageDriverManager->testDevices();
    m_dPageDriverManager->slotScanFinished(SR_SUCESS);

    m_dPageDriverManager->slotDriverOperationClicked(0, 0, DriverOperationItem::INSTALL);
    EXPECT_EQ(1, m_dPageDriverManager->m_ListDriverIndex.size());

    m_dPageDriverManager->slotDriverOperationClicked(0, 0, DriverOperationItem::BACKUP);
    EXPECT_EQ(1, m_dPageDriverManager->m_ListBackupIndex.size());
}

TEST_F(PageDriverManager_UT, ut_slotItemCheckedClicked)
{
    m_dPageDriverManager->slotItemCheckedClicked(0, true);
    EXPECT_EQ(1, m_dPageDriverManager->m_ListDriverIndex.size());

    m_dPageDriverManager->slotItemCheckedClicked(0, false);
    EXPECT_EQ(0, m_dPageDriverManager->m_ListDriverIndex.size());
}

TEST_F(PageDriverManager_UT, ut_slotBackupItemCheckedClicked)
{
    m_dPageDriverManager->slotBackupItemCheckedClicked(0, true);
    EXPECT_EQ(1, m_dPageDriverManager->m_ListBackupIndex.size());

    m_dPageDriverManager->slotBackupItemCheckedClicked(0, false);
    EXPECT_EQ(0, m_dPageDriverManager->m_ListBackupIndex.size());
}

TEST_F(PageDriverManager_UT, ut_slotDownloadProgressChanged)
{
    QStringList msg;
    msg.append("20");
    msg.append("test");
    msg.append("test-info");
    m_dPageDriverManager->slotDownloadProgressChanged(msg);
    m_dPageDriverManager->slotDownloadFinished();
}

TEST_F(PageDriverManager_UT, ut_slotInstallProgressChanged)
{
    m_dPageDriverManager->slotInstallProgressChanged(30);
    m_dPageDriverManager->slotInstallProgressFinished(false, 4);
}

TEST_F(PageDriverManager_UT, ut_slotBackupFinished)
{
    m_dPageDriverManager->slotBackupFinished(true);
    EXPECT_EQ(0, m_dPageDriverManager->m_ListBackupIndex.size());
}

TEST_F(PageDriverManager_UT, ut_slotRestoreProgress)
{
    DriverInfo *info = new DriverInfo();
    info->m_Name = "Sharp MX-C2622R PS, 1.1";
    info->m_DebVersion = "1.0.1";
    info->m_DebBackupVersion = "1.0.0";
    info->m_Type = DR_Tablet;
    info->m_Packages = "com.sharp.griffin2light";
    info->m_Status = ST_CAN_UPDATE;

    m_dPageDriverManager->mp_CurRestoreDriverInfo = info;
    m_dPageDriverManager->slotRestoreProgress(50, "test");
}

TEST_F(PageDriverManager_UT, ut_slotRestoreFinished)
{
    m_dPageDriverManager->slotRestoreFinished(true, "test");
    EXPECT_EQ(0, m_dPageDriverManager->m_ListBackedupeIndex.size());
}

