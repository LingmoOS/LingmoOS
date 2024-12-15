// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_EXPORTNOTEWORKER_H
#define UT_EXPORTNOTEWORKER_H

#include "gtest/gtest.h"
#include <QList>

struct VNoteItem;
struct VNoteBlock;

class UT_ExportNoteWorker : public ::testing::Test
{
public:
    UT_ExportNoteWorker();

protected:
    virtual void SetUp() override;
    virtual void TearDown() override;
 public:
     QList<VNoteItem *> m_noteList;
     VNoteBlock *m_block = nullptr;
     VNoteItem *note {nullptr};
};

#endif // UT_EXPORTNOTEWORKER_H
