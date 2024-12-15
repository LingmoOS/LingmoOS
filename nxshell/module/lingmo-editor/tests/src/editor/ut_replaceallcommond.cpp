// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_replaceallcommond.h"
#include "../../src/editor/replaceallcommond.h"
#include "QTextCursor"
test_replaceallcommond::test_replaceallcommond()
{

}

TEST_F(test_replaceallcommond, ReplaceAllCommand)
{
    QString text = "test";
    QTextCursor cursor;
    ReplaceAllCommand* com = new ReplaceAllCommand(text,text,cursor);
    ASSERT_TRUE(!text.compare(com->m_newText));

    delete com;
    com=nullptr;
}

TEST_F(test_replaceallcommond, redo)
{
    QString text = "test";
    QTextCursor cursor;
    ReplaceAllCommand* com = new ReplaceAllCommand(text,text,cursor);
    com->redo();
    ASSERT_TRUE(com->m_cursor.position() != 0);

    delete com;
    com=nullptr;
}

TEST_F(test_replaceallcommond, undo)
{
    QString text = "test";
    QTextCursor cursor;
    ReplaceAllCommand* com = new ReplaceAllCommand(text,text,cursor);
    com->undo();
    ASSERT_TRUE(com->m_cursor.position() != 0);

    delete com;
    com=nullptr;

    
}
