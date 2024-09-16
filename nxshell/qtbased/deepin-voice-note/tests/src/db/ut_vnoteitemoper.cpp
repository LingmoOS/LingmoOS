// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_vnoteitemoper.h"
#include "vnotedbmanager.h"
#include "vnotedatamanager.h"
#include "vnotefolderoper.h"
#include "vnoteforlder.h"
#include "vnoteitemoper.h"
#include "vnoteitem.h"
#include <stub.h>

static bool stub_true()
{
    return true;
}

static bool stub_false()
{
    return false;
}

static VNoteItem *stub_null()
{
    return nullptr;
}

UT_VNoteItemOper::UT_VNoteItemOper()
{
}

void UT_VNoteItemOper::SetUp()
{
    VNOTE_ALL_NOTES_MAP *notes = VNoteDataManager::instance()->getAllNotesInFolder();
    if (notes && !notes->notes.isEmpty()) {
        VNOTE_ITEMS_MAP *tmp = notes->notes.first();
        if (tmp && !tmp->folderNotes.isEmpty()) {
            m_note = tmp->folderNotes.first();
        }
    }

    m_vnoteitemoper = new VNoteItemOper(m_note);
}

void UT_VNoteItemOper::TearDown()
{
    delete m_vnoteitemoper;
}

TEST_F(UT_VNoteItemOper, UT_VNoteItemOper_loadAllVNotes_001)
{
    VNOTE_ALL_NOTES_MAP *notes = m_vnoteitemoper->loadAllVNotes();
    EXPECT_FALSE(nullptr == notes);
    delete notes;
}

TEST_F(UT_VNoteItemOper, UT_VNoteItemOper_modifyNoteTitle_001)
{
    Stub stub;
    stub.set(ADDR(VNoteDbManager, updateData), stub_false);
    EXPECT_FALSE(m_vnoteitemoper->modifyNoteTitle("test"));
}

TEST_F(UT_VNoteItemOper, UT_VNoteItemOper_modifyNoteTitle_002)
{
    Stub stub;
    stub.set(ADDR(VNoteDbManager, updateData), stub_true);
    EXPECT_TRUE(m_vnoteitemoper->modifyNoteTitle("test"));
}

TEST_F(UT_VNoteItemOper, UT_VNoteItemOper_updateNote_001)
{
    Stub stub;
    stub.set(ADDR(VNoteDbManager, updateData), stub_false);
    EXPECT_FALSE(m_vnoteitemoper->updateNote());
}

TEST_F(UT_VNoteItemOper, UT_VNoteItemOper_updateNote_002)
{
    Stub stub;
    stub.set(ADDR(VNoteDbManager, updateData), stub_true);
    EXPECT_TRUE(m_vnoteitemoper->updateNote());
}

TEST_F(UT_VNoteItemOper, UT_VNoteItemOper_addNote_001)
{
    Stub stub;
    stub.set(ADDR(VNoteDbManager, insertData), stub_false);
    VNoteItem tmpNote;
    tmpNote.folderId = m_note->folderId;
    tmpNote.noteType = VNoteItem::VNT_Text;
    tmpNote.noteTitle = m_vnoteitemoper->getDefaultNoteName(tmpNote.folderId);
    VNoteBlock *ptrBlock1 = tmpNote.newBlock(VNoteBlock::Text);
    tmpNote.addBlock(ptrBlock1);
    EXPECT_FALSE(m_vnoteitemoper->addNote(tmpNote));
}

TEST_F(UT_VNoteItemOper, UT_VNoteItemOper_addNote_002)
{
    Stub stub;
    stub.set(ADDR(VNoteDbManager, insertData), stub_true);
    stub.set(ADDR(VNoteDataManager, addNote), stub_null);
    VNoteItem tmpNote;
    tmpNote.folderId = m_note->folderId;
    tmpNote.noteType = VNoteItem::VNT_Text;
    tmpNote.noteTitle = m_vnoteitemoper->getDefaultNoteName(tmpNote.folderId);
    VNoteBlock *ptrBlock1 = tmpNote.newBlock(VNoteBlock::Text);
    tmpNote.addBlock(ptrBlock1);
    EXPECT_FALSE(m_vnoteitemoper->addNote(tmpNote));
}

TEST_F(UT_VNoteItemOper, UT_VNoteItemOper_addNote_003)
{
    Stub stub;
    stub.set(ADDR(VNoteDbManager, insertData), stub_true);
    stub.set(ADDR(VNoteDataManager, addNote), stub_true);
    VNoteItem tmpNote;
    tmpNote.folderId = m_note->folderId;
    tmpNote.noteType = VNoteItem::VNT_Text;
    tmpNote.noteTitle = m_vnoteitemoper->getDefaultNoteName(tmpNote.folderId);
    VNoteBlock *ptrBlock1 = tmpNote.newBlock(VNoteBlock::Text);
    tmpNote.addBlock(ptrBlock1);
    VNoteItem *itemNote = m_vnoteitemoper->addNote(tmpNote);
    EXPECT_TRUE(itemNote);
    delete itemNote;
}

TEST_F(UT_VNoteItemOper, UT_VNoteItemOper_deleteNote_001)
{
    Stub stub;
    stub.set(ADDR(VNoteDbManager, deleteData), stub_false);
    VNoteItem tmpNote;
    tmpNote.folderId = m_note->folderId;
    tmpNote.noteType = VNoteItem::VNT_Text;
    tmpNote.noteTitle = m_vnoteitemoper->getDefaultNoteName(tmpNote.folderId);
    VNoteItemOper op(&tmpNote);
    EXPECT_FALSE(op.deleteNote());
}

TEST_F(UT_VNoteItemOper, UT_VNoteItemOper_deleteNote_002)
{
    Stub stub;
    stub.set(ADDR(VNoteDbManager, deleteData), stub_true);
    VNoteItem tmpNote;
    tmpNote.folderId = m_note->folderId;
    tmpNote.noteType = VNoteItem::VNT_Text;
    tmpNote.noteTitle = m_vnoteitemoper->getDefaultNoteName(tmpNote.folderId);
    VNoteItemOper op(&tmpNote);
    EXPECT_TRUE(op.deleteNote());
}

TEST_F(UT_VNoteItemOper, UT_VNoteItemOper_getDefaultVoiceName_001)
{
    m_vnoteitemoper->getDefaultVoiceName();
}

TEST_F(UT_VNoteItemOper, UT_VNoteItemOper_updateTop_001)
{
    EXPECT_FALSE(m_vnoteitemoper->updateTop(m_note->isTop));
}

TEST_F(UT_VNoteItemOper, UT_VNoteItemOper_updateTop_002)
{
    Stub stub;
    stub.set(ADDR(VNoteDbManager, updateData), stub_false);
    EXPECT_FALSE(m_vnoteitemoper->updateTop(1));
    EXPECT_EQ(0, m_vnoteitemoper->m_note->isTop);
}

TEST_F(UT_VNoteItemOper, UT_VNoteItemOper_updateTop_003)
{
    Stub stub;
    stub.set(ADDR(VNoteDbManager, updateData), stub_true);
    EXPECT_TRUE(m_vnoteitemoper->updateTop(1));
    EXPECT_EQ(1, m_vnoteitemoper->m_note->isTop);
}

TEST_F(UT_VNoteItemOper, UT_VNoteItemOper_updateFolderId_001)
{
    Stub stub;
    stub.set(ADDR(VNoteDbManager, updateData), stub_false);
    EXPECT_FALSE(m_vnoteitemoper->updateFolderId(m_note));
}

TEST_F(UT_VNoteItemOper, UT_VNoteItemOper_updateFolderId_002)
{
    Stub stub;
    stub.set(ADDR(VNoteDbManager, updateData), stub_true);
    EXPECT_TRUE(m_vnoteitemoper->updateFolderId(m_note));
}

TEST_F(UT_VNoteItemOper, UT_VNoteItemOper_getNote_001)
{
    EXPECT_TRUE(m_vnoteitemoper->getNote(m_note->folderId, m_note->noteId));
}
