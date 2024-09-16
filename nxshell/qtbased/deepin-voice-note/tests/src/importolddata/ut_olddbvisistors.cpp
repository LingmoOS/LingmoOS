// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_olddbvisistors.h"
#include "olddbvisistors.h"
#include "stub.h"

static bool ret = true;

bool stub_sql()
{
    if (ret == true) {
        ret = false;
        return true;
    }
    return ret;
}

UT_OldDBVisistors::UT_OldDBVisistors()
{
}

TEST_F(UT_OldDBVisistors, UT_OldDBVisistors_visitorData_001)
{
    Stub stub1;
    stub1.set(ADDR(QSqlQuery, next), stub_sql);
    QSqlDatabase db;
    VNOTE_FOLDERS_MAP folders;
    folders.autoRelease = true;

    OldFolderQryDbVisitor oldfolderqrydbvisitor(db, nullptr, &folders);
    oldfolderqrydbvisitor.prepareSqls();
    EXPECT_FALSE(oldfolderqrydbvisitor.m_dbvSqls.isEmpty());
    EXPECT_TRUE(oldfolderqrydbvisitor.visitorData());
    VNOTE_ALL_NOTES_MAP notes;
    notes.autoRelease = true;

    ret = true;
    OldNoteQryDbVisitor oldnoteqrydbvisitor(db, nullptr, &notes);
    oldnoteqrydbvisitor.prepareSqls();
    EXPECT_FALSE(oldnoteqrydbvisitor.m_dbvSqls.isEmpty());
    EXPECT_TRUE(oldnoteqrydbvisitor.visitorData());
}
