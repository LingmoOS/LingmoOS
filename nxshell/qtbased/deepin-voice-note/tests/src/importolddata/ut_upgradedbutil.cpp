// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_upgradedbutil.h"
#include "upgradedbutil.h"
#include "vnoteforlder.h"
#include "db/vnotedbmanager.h"
#include <stub.h>

static bool stub_false()
{
    return false;
}

static bool stub_true()
{
    return true;
}

UT_UpgradeDbUtil::UT_UpgradeDbUtil()
{
}

void UT_UpgradeDbUtil::SetUp()
{
    m_upgradedbutil = new UpgradeDbUtil;
}

void UT_UpgradeDbUtil::TearDown()
{
    delete m_upgradedbutil;
}

TEST_F(UT_UpgradeDbUtil, UT_UpgradeDbUtil_saveUpgradeState_001)
{
    m_upgradedbutil->saveUpgradeState(m_upgradedbutil->Loading);
    EXPECT_EQ(UpgradeDbUtil::Loading, m_upgradedbutil->readUpgradeState());
}

TEST_F(UT_UpgradeDbUtil, UT_UpgradeDbUtil_readUpgradeState_001)
{
    m_upgradedbutil->saveUpgradeState(m_upgradedbutil->Loading);
    EXPECT_EQ(UpgradeDbUtil::Loading, m_upgradedbutil->readUpgradeState());
}

TEST_F(UT_UpgradeDbUtil, UT_UpgradeDbUtil_needUpdateOldDb_001)
{
    Stub stub;
    stub.set(ADDR(VNoteDbManager, hasOldDataBase), stub_true);
    EXPECT_TRUE(m_upgradedbutil->needUpdateOldDb(m_upgradedbutil->Loading));
}

TEST_F(UT_UpgradeDbUtil, UT_UpgradeDbUtil_needUpdateOldDb_002)
{
    Stub stub;
    stub.set(ADDR(VNoteDbManager, hasOldDataBase), stub_false);
    EXPECT_FALSE(m_upgradedbutil->needUpdateOldDb(m_upgradedbutil->Loading));
}

TEST_F(UT_UpgradeDbUtil, UT_UpgradeDbUtil_checkUpdateState_001)
{
    m_upgradedbutil->checkUpdateState(m_upgradedbutil->Processing);
}

TEST_F(UT_UpgradeDbUtil, UT_UpgradeDbUtil_backUpOldDb_001)
{
    m_upgradedbutil->backUpOldDb();
}

TEST_F(UT_UpgradeDbUtil, UT_UpgradeDbUtil_clearVoices_001)
{
    m_upgradedbutil->clearVoices();
}

TEST_F(UT_UpgradeDbUtil, UT_UpgradeDbUtil_doFolderUpgrade_001)
{
    VNoteFolder folder;
    folder.notesCount = 1;
    folder.defaultIcon = 1;
    folder.name = "test";
    folder.iconPath = "test1";
    folder.createTime = QDateTime::currentDateTime();
    folder.modifyTime = QDateTime::currentDateTime();
    folder.deleteTime = QDateTime::currentDateTime();
    m_upgradedbutil->doFolderUpgrade(&folder);
}

TEST_F(UT_UpgradeDbUtil, UT_UpgradeDbUtil_doFolderNoteUpgrade_001)
{
    m_upgradedbutil->doFolderNoteUpgrade(1, 1);
}
