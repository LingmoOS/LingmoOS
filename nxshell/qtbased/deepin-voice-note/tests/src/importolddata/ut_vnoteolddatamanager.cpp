// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_vnoteolddatamanager.h"
#include "vnoteolddatamanager.h"
#include <QThreadPool>
#include <stub.h>

static void stub_void()
{
}

UT_VNoteOldDataManager::UT_VNoteOldDataManager()
{
}

void UT_VNoteOldDataManager::SetUp()
{
    m_vnoteolddatamanager = new VNoteOldDataManager;
}

void UT_VNoteOldDataManager::TearDown()
{
    delete m_vnoteolddatamanager;
}

TEST_F(UT_VNoteOldDataManager, UT_VNoteOldDataManager_folders_001)
{
    EXPECT_EQ(nullptr, m_vnoteolddatamanager->folders());
}

TEST_F(UT_VNoteOldDataManager, UT_VNoteOldDataManager_initOldDb_001)
{
    m_vnoteolddatamanager->initOldDb();
    EXPECT_TRUE(nullptr != m_vnoteolddatamanager->m_oldDbManger);
}

TEST_F(UT_VNoteOldDataManager, UT_VNoteOldDataManager_onFinishLoad_001)
{
    m_vnoteolddatamanager->onFinishLoad();
}

TEST_F(UT_VNoteOldDataManager, UT_VNoteOldDataManager_onFinishUpgrade_001)
{
    m_vnoteolddatamanager->onFinishUpgrade();
}

TEST_F(UT_VNoteOldDataManager, UT_VNoteOldDataManager_onProgress_001)
{
    m_vnoteolddatamanager->onProgress(1);
}
