// Copyright (C) 2019 ~ 2019 UnionTech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_loadnoteitemsworker.h"
#include "task/loadnoteitemsworker.h"

UT_LoadNoteItemsWorker::UT_LoadNoteItemsWorker(QObject *parent)
    : QObject(parent)
{
}

void UT_LoadNoteItemsWorker::onNoteLoad(VNOTE_ALL_NOTES_MAP *notesMap)
{
    EXPECT_TRUE(nullptr != notesMap);
    if (notesMap) {
        delete notesMap;
        notesMap = nullptr;
    }
}

TEST_F(UT_LoadNoteItemsWorker, UT_LoadNoteItemsWorker_run_001)
{
    LoadNoteItemsWorker work;
    connect(&work, &LoadNoteItemsWorker::onAllNotesLoaded, this, &UT_LoadNoteItemsWorker::onNoteLoad);
    work.run();
}
