// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_upgradeview.h"
#include "upgradeview.h"
#include "importolddata/vnoteolddatamanager.h"
#include "common/vnoteforlder.h"

#include <stub.h>
#include <QThreadPool>

static void stub_void()
{
}

UT_UpgradeView::UT_UpgradeView()
{
}

TEST_F(UT_UpgradeView, UT_UpgradeView_onUpgradeFinish_001)
{
    UpgradeView upgradeview;
    upgradeview.onUpgradeFinish();
    EXPECT_EQ(100, upgradeview.m_waterProgress->value());
}

TEST_F(UT_UpgradeView, UT_UpgradeView_setProgress_001)
{
    UpgradeView upgradeview;
    upgradeview.setProgress(1);
    EXPECT_EQ(1, upgradeview.m_waterProgress->value());
}

TEST_F(UT_UpgradeView, UT_UpgradeView_startUpgrade_001)
{
    Stub stub;
    stub.set((void (QThreadPool::*)(QRunnable *, int))ADDR(QThreadPool, start), stub_void);
    UpgradeView upgradeview;
    upgradeview.startUpgrade();
}

TEST_F(UT_UpgradeView, UT_UpgradeView_onDataReady_001)
{
    VNOTE_FOLDERS_MAP *m_qspNoteFoldersMap = new VNOTE_FOLDERS_MAP;
    VNoteOldDataManager::instance()->m_qspNoteFoldersMap.reset(m_qspNoteFoldersMap);
    UpgradeView upgradeview;
    upgradeview.onDataReady();
}

TEST_F(UT_UpgradeView, UT_UpgradeView_onDataReady_002)
{
    Stub stub;
    stub.set((void (QThreadPool::*)(QRunnable *, int))ADDR(QThreadPool, start), stub_void);
    VNOTE_FOLDERS_MAP *qspNoteFoldersMap = new VNOTE_FOLDERS_MAP;
    VNoteFolder *folder = new VNoteFolder();
    qspNoteFoldersMap->folders.insert(1, folder);
    VNoteOldDataManager::instance()->m_qspNoteFoldersMap.reset(qspNoteFoldersMap);
    UpgradeView upgradeview;
    upgradeview.onDataReady();
    delete folder;
}
