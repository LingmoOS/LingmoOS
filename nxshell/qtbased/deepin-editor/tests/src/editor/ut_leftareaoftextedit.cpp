// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_leftareaoftextedit.h"
#include "../../src/editor/leftareaoftextedit.h"
#include "../../src/editor/dtextedit.h"

test_leftareaoftextedit::test_leftareaoftextedit()
{

}

//LeftAreaTextEdit(TextEdit *textEdit);
TEST_F(test_leftareaoftextedit, LeftAreaTextEdit)
{
    TextEdit *textEdit = new TextEdit;
    LeftAreaTextEdit *leftArea = new LeftAreaTextEdit(textEdit);
    
    ASSERT_TRUE(leftArea->m_pFlodArea != nullptr);
    leftArea->deleteLater();
    textEdit->deleteLater();
}

//void lineNumberAreaPaintEvent(QPaintEvent *event);
TEST_F(test_leftareaoftextedit, lineNumberAreaPaintEvent)
{
    TextEdit *textEdit = new TextEdit;
    LeftAreaTextEdit *leftArea = new LeftAreaTextEdit(textEdit);
    QPaintEvent *event = new QPaintEvent(QRegion());
    leftArea->lineNumberAreaPaintEvent(event);
    
    ASSERT_TRUE(leftArea->m_pFlodArea != nullptr);
    delete event;
    event = nullptr;
    leftArea->deleteLater();
    textEdit->deleteLater();
}

//int lineNumberAreaWidth();
TEST_F(test_leftareaoftextedit, lineNumberAreaWidth)
{
    TextEdit *textEdit = new TextEdit;
    LeftAreaTextEdit *leftArea = new LeftAreaTextEdit(textEdit);
    leftArea->lineNumberAreaWidth();
    
    ASSERT_TRUE(leftArea->m_pFlodArea != nullptr);
    leftArea->deleteLater();
    textEdit->deleteLater();
}

//void bookMarkAreaPaintEvent(QPaintEvent *event);
TEST_F(test_leftareaoftextedit, bookMarkAreaPaintEvent)
{
    TextEdit *textEdit = new TextEdit;
    LeftAreaTextEdit *leftArea = new LeftAreaTextEdit(textEdit);
    QPaintEvent *event = new QPaintEvent(QRegion());
    leftArea->bookMarkAreaPaintEvent(event);
    
    ASSERT_TRUE(leftArea->m_pFlodArea != nullptr);
    delete event;
    event = nullptr;
    leftArea->deleteLater();
    textEdit->deleteLater();
}

//void codeFlodAreaPaintEvent(QPaintEvent *event);
TEST_F(test_leftareaoftextedit, codeFlodAreaPaintEvent)
{
    TextEdit *textEdit = new TextEdit;
    LeftAreaTextEdit *leftArea = new LeftAreaTextEdit(textEdit);
    QPaintEvent *event = new QPaintEvent(QRegion());
    leftArea->codeFlodAreaPaintEvent(event);
    
    ASSERT_TRUE(leftArea->m_pFlodArea != nullptr);
    delete event;
    event = nullptr;
    leftArea->deleteLater();
    textEdit->deleteLater();
}
