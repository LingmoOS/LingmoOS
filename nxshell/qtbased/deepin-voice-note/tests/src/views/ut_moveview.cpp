// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_moveview.h"
#include "vnoteforlder.h"
#include "moveview.h"
#include "vnoteitem.h"
#include "middleview.h"
#include "common/vnotedatamanager.h"

#include <QRect>
#include <QPaintEvent>
#include <standarditemcommon.h>

UT_MoveView::UT_MoveView()
{
}

TEST_F(UT_MoveView, UT_MoveView_setFolder_001)
{
    MoveView moveView;
    VNoteFolder *folder = VNoteDataManager::instance()->getNoteFolders()->folders[0];
    moveView.setFolder(folder);
    EXPECT_EQ(moveView.m_folder, folder);
    moveView.grab();
    moveView.m_hasComposite = !moveView.m_hasComposite;
    moveView.grab();
}

TEST_F(UT_MoveView, UT_MoveView_setNoteList_001)
{
    MoveView moveView;
    VNoteFolder *folder = VNoteDataManager::instance()->getNoteFolders()->folders[0];
    VNoteItem *note = folder->getNotes()->folderNotes[0];
    QList<VNoteItem *> list;
    list.append(note);
    moveView.setNote(note);
    EXPECT_EQ(moveView.m_note, note);
    moveView.setNoteList(list);
    EXPECT_EQ(moveView.m_noteList, list);
    moveView.setNotesNumber(1);
    EXPECT_EQ(moveView.m_notesNumber, 1);
    moveView.grab();
    moveView.setNotesNumber(2);
    EXPECT_EQ(moveView.m_notesNumber, 2);
    moveView.grab();
    moveView.m_hasComposite = !moveView.m_hasComposite;
    moveView.setNotesNumber(1);
    EXPECT_EQ(moveView.m_notesNumber, 1);
    moveView.grab();
    moveView.setNotesNumber(2);
    EXPECT_EQ(moveView.m_notesNumber, 2);
    moveView.grab();
}
