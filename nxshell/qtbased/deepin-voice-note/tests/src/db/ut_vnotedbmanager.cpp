// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_vnotedbmanager.h"
#include "vnotedbmanager.h"
#include "vnotefolderoper.h"
#include "vnoteforlder.h"
#include "vnoteitemoper.h"
#include "vnoteitem.h"
#include "db/dbvisitor.h"
#include <stub.h>

static bool stub_true()
{
    return true;
}

UT_VNoteDbManager::UT_VNoteDbManager()
{
}

TEST_F(UT_VNoteDbManager, UT_VNoteDbManager_initVNoteDb_001)
{
    VNoteDbManager::instance()->initVNoteDb();
    EXPECT_TRUE(VNoteDbManager::instance()->m_vnoteDB.isValid());
}

TEST_F(UT_VNoteDbManager, UT_VNoteDbManager_getVNoteDb_001)
{
    VNoteDbManager *instance = VNoteDbManager::instance();
    EXPECT_TRUE(instance->getVNoteDb().isValid());
}

TEST_F(UT_VNoteDbManager, UT_VNoteDbManager_insertData_001)
{
    VNoteDbManager *instance = VNoteDbManager::instance();
    EXPECT_FALSE(instance->insertData(nullptr));
    EXPECT_FALSE(instance->updateData(nullptr));
    EXPECT_FALSE(instance->queryData(nullptr));
    EXPECT_FALSE(instance->deleteData(nullptr));
}

TEST_F(UT_VNoteDbManager, UT_VNoteDbManager_insertData_002)
{
    VNoteDbManager *instance = VNoteDbManager::instance();
    VNoteItem newNote;
    VNoteItem note;
    AddNoteDbVisitor noteVisitor(instance->getVNoteDb(), &note, &newNote);
    EXPECT_FALSE(instance->insertData(&noteVisitor));
    EXPECT_FALSE(instance->updateData(&noteVisitor));
    EXPECT_FALSE(instance->queryData(&noteVisitor));
    EXPECT_FALSE(instance->deleteData(&noteVisitor));
}

TEST_F(UT_VNoteDbManager, UT_VNoteDbManager_insertData_003)
{
    VNoteFolder *folder = new VNoteFolder;
    VNoteItem *note = new VNoteItem();
    note->setFolder(folder);
    VNoteDbManager *instance = VNoteDbManager::instance();
    VNoteItem newNote;
    DbVisitor *noteVisitor = new AddNoteDbVisitor(instance->getVNoteDb(), note, &newNote);
    instance->insertData(noteVisitor);
    instance->updateData(noteVisitor);
    instance->queryData(noteVisitor);
    instance->deleteData(noteVisitor);
    delete folder;
    delete note;
    delete noteVisitor;
}

TEST_F(UT_VNoteDbManager, UT_VNoteDbManager_createTablesIfNeed_001)
{
    VNoteDbManager *instance = VNoteDbManager::instance();
    instance->createTablesIfNeed();
}
