// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_vnoteforlder.h"
#include "vnoteforlder.h"
#include "vnotedatamanager.h"
#include <QtDebug>

UT_VNoteFolder::UT_VNoteFolder()
{
}

void UT_VNoteFolder::SetUp()
{
    m_vnoteforlder = VNoteDataManager::instance()->getNoteFolders()->folders[0];
}

void UT_VNoteFolder::TearDown()
{
    ;
}

TEST_F(UT_VNoteFolder, UT_VNoteFolder_isValid_001)
{
    EXPECT_EQ(true, m_vnoteforlder->isValid());
}

TEST_F(UT_VNoteFolder, UT_VNoteFolder_maxNoteIdRef_001)
{
    EXPECT_EQ(2, m_vnoteforlder->maxNoteIdRef());
}

TEST_F(UT_VNoteFolder, UT_VNoteFolder_getNotesCount_001)
{
    EXPECT_EQ(2, m_vnoteforlder->getNotesCount());
}
