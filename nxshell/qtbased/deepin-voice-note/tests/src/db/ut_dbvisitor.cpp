// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_dbvisitor.h"
#include "dbvisitor.h"
#include "vnotedbmanager.h"
#include "common/vnoteitem.h"
#include "common/vnoteforlder.h"

UT_DbVisitor::UT_DbVisitor()
{
}

TEST_F(UT_DbVisitor, UT_DbVisitor_visitorData_001)
{
    DbVisitor *dbvisitor;
    QSqlDatabase db = VNoteDbManager::instance()->getVNoteDb();
    dbvisitor = new FolderQryDbVisitor(db, nullptr, nullptr);
    EXPECT_TRUE(dbvisitor->prepareSqls());
    EXPECT_FALSE(dbvisitor->visitorData());
    EXPECT_FALSE(nullptr == dbvisitor->sqlQuery());
    EXPECT_FALSE(dbvisitor->dbvSqls().isEmpty());
    EXPECT_FALSE(dbvisitor->extraData().data.flag);
    delete dbvisitor;
}

TEST_F(UT_DbVisitor, UT_DbVisitor_AddNoteDbVisitor_001)
{
    DbVisitor *dbvisitor;
    VNoteFolder *folder = new VNoteFolder;
    VNoteItem *note = new VNoteItem();
    note->setFolder(folder);
    QSqlDatabase db = VNoteDbManager::instance()->getVNoteDb();
    dbvisitor = new AddNoteDbVisitor(db, nullptr, nullptr);
    dbvisitor->param.newNote = note;
    EXPECT_FALSE(dbvisitor->visitorData());
    EXPECT_TRUE(dbvisitor->prepareSqls());
    EXPECT_FALSE(nullptr == dbvisitor->sqlQuery());
    EXPECT_FALSE(dbvisitor->dbvSqls().isEmpty());
    EXPECT_FALSE(dbvisitor->extraData().data.flag);
    delete folder;
    delete note;
    delete dbvisitor;
}

TEST_F(UT_DbVisitor, UT_DbVisitor_MaxIdFolderDbVisitor_001)
{
    DbVisitor *dbvisitor;
    QSqlDatabase db = VNoteDbManager::instance()->getVNoteDb();
    dbvisitor = new MaxIdFolderDbVisitor(db, nullptr, nullptr);
    EXPECT_TRUE(dbvisitor->prepareSqls());
    EXPECT_FALSE(dbvisitor->visitorData());
    delete dbvisitor;
}

TEST_F(UT_DbVisitor, UT_DbVisitor_AddFolderDbVisitor_001)
{
    DbVisitor *dbvisitor;
    QSqlDatabase db = VNoteDbManager::instance()->getVNoteDb();
    dbvisitor = new AddFolderDbVisitor(db, nullptr, nullptr);
    EXPECT_FALSE(dbvisitor->prepareSqls());
    EXPECT_FALSE(dbvisitor->visitorData());
    delete dbvisitor;
}

TEST_F(UT_DbVisitor, UT_DbVisitor_RenameFolderDbVisitor_001)
{
    DbVisitor *dbvisitor;
    QSqlDatabase db = VNoteDbManager::instance()->getVNoteDb();
    dbvisitor = new RenameFolderDbVisitor(db, nullptr, nullptr);
    EXPECT_FALSE(dbvisitor->prepareSqls());
    EXPECT_TRUE(dbvisitor->visitorData());
    delete dbvisitor;
}

TEST_F(UT_DbVisitor, UT_DbVisitor_DelFolderDbVisitor_001)
{
    DbVisitor *dbvisitor;
    QSqlDatabase db = VNoteDbManager::instance()->getVNoteDb();
    dbvisitor = new DelFolderDbVisitor(db, nullptr, nullptr);
    EXPECT_FALSE(dbvisitor->prepareSqls());
    EXPECT_TRUE(dbvisitor->visitorData());
    delete dbvisitor;
}

TEST_F(UT_DbVisitor, UT_DbVisitor_DelFolderDbVisitor_002)
{
    VNoteFolder *folder = new VNoteFolder;
    VNoteItem *note = new VNoteItem();
    note->setFolder(folder);
    DbVisitor *dbvisitor;
    QSqlDatabase db = VNoteDbManager::instance()->getVNoteDb();
    dbvisitor = new DelFolderDbVisitor(db, nullptr, nullptr);
    dbvisitor->param.id = &note->folderId;
    EXPECT_TRUE(dbvisitor->prepareSqls());
    EXPECT_TRUE(dbvisitor->visitorData());
    delete folder;
    delete note;
    delete dbvisitor;
}

TEST_F(UT_DbVisitor, UT_DbVisitor_NoteQryDbVisitor_001)
{
    DbVisitor *dbvisitor;
    QSqlDatabase db = VNoteDbManager::instance()->getVNoteDb();
    dbvisitor = new NoteQryDbVisitor(db, nullptr, nullptr);
    EXPECT_TRUE(dbvisitor->prepareSqls());
    EXPECT_FALSE(dbvisitor->visitorData());
    delete dbvisitor;
}

TEST_F(UT_DbVisitor, UT_DbVisitor_RenameNoteDbVisitor_001)
{
    DbVisitor *dbvisitor;
    QSqlDatabase db = VNoteDbManager::instance()->getVNoteDb();
    dbvisitor = new RenameNoteDbVisitor(db, nullptr, nullptr);
    EXPECT_FALSE(dbvisitor->prepareSqls());
    EXPECT_TRUE(dbvisitor->visitorData());
    delete dbvisitor;
}

TEST_F(UT_DbVisitor, UT_DbVisitor_UpdateNoteDbVisitor_001)
{
    VNoteItem *note = new VNoteItem();
    DbVisitor *dbvisitor;
    QSqlDatabase db = VNoteDbManager::instance()->getVNoteDb();
    dbvisitor = new UpdateNoteDbVisitor(db, nullptr, nullptr);
    dbvisitor->param.newNote = note;
    EXPECT_TRUE(dbvisitor->prepareSqls());
    EXPECT_TRUE(dbvisitor->visitorData());
    delete note;
    delete dbvisitor;
}

TEST_F(UT_DbVisitor, UT_DbVisitor_UpdateNoteFolderIdDbVisitor_001)
{
    VNoteItem *note = new VNoteItem();
    DbVisitor *dbvisitor;
    QSqlDatabase db = VNoteDbManager::instance()->getVNoteDb();
    dbvisitor = new UpdateNoteFolderIdDbVisitor(db, nullptr, nullptr);
    dbvisitor->param.newNote = note;
    EXPECT_TRUE(dbvisitor->prepareSqls());
    EXPECT_TRUE(dbvisitor->visitorData());
    delete note;
    delete dbvisitor;
}

TEST_F(UT_DbVisitor, UT_DbVisitor_UpdateNoteTopDbVisitor_001)
{
    VNoteItem *note = new VNoteItem();
    DbVisitor *dbvisitor;
    QSqlDatabase db = VNoteDbManager::instance()->getVNoteDb();
    dbvisitor = new UpdateNoteTopDbVisitor(db, nullptr, nullptr);
    dbvisitor->param.newNote = note;
    EXPECT_TRUE(dbvisitor->prepareSqls());
    EXPECT_TRUE(dbvisitor->visitorData());
    delete note;
    delete dbvisitor;
}

TEST_F(UT_DbVisitor, UT_DbVisitor_DelNoteDbVisitor_001)
{
    DbVisitor *dbvisitor;
    QSqlDatabase db = VNoteDbManager::instance()->getVNoteDb();
    dbvisitor = new DelNoteDbVisitor(db, nullptr, nullptr);
    EXPECT_FALSE(dbvisitor->prepareSqls());
    EXPECT_TRUE(dbvisitor->visitorData());
    delete dbvisitor;
}

TEST_F(UT_DbVisitor, UT_DbVisitor_DelNoteDbVisitor_002)
{
    VNoteFolder *folder = new VNoteFolder;
    VNoteItem *note = new VNoteItem();
    note->setFolder(folder);
    DbVisitor *dbvisitor;
    QSqlDatabase db = VNoteDbManager::instance()->getVNoteDb();
    dbvisitor = new DelNoteDbVisitor(db, nullptr, nullptr);
    dbvisitor->param.newNote = note;
    EXPECT_TRUE(dbvisitor->prepareSqls());
    EXPECT_TRUE(dbvisitor->visitorData());
    delete folder;
    delete note;
    delete dbvisitor;
}
