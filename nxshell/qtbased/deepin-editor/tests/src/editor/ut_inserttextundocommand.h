// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEST_INSERTTEXTUNDOCOMMAND_H
#define TEST_INSERTTEXTUNDOCOMMAND_H

#include "gtest/gtest.h"
#include <QTest>

class InsertTextUndoCommand;
class test_InsertTextUndoCommand :public testing::Test
{
public:
    test_InsertTextUndoCommand();
    virtual void SetUp() override;
    virtual void TearDown() override;
    InsertTextUndoCommand *ituc;
};

#endif // TEST_INSERTTEXTUNDOCOMMAND_H
