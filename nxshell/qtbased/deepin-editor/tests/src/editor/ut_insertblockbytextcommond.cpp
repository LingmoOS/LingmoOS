// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_insertblockbytextcommond.h"
#include "../src/editor/insertblockbytextcommond.h"
#include "../src/editor/dtextedit.h"
#include "../src/editor/editwrapper.h"
#include "../src/widgets/window.h"
#include "qtextcursor.h"
#include "src/stub.h"


namespace insertblockstub {
int intvalue = 1;
int retintstub()
{
    return intvalue;
}
}

using namespace insertblockstub;

test_insertblockbytextcommond::test_insertblockbytextcommond()
{

}

TEST_F(test_insertblockbytextcommond, InsertBlockByTextCommand)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world."));

    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    textCursor.movePosition(QTextCursor::Start, QTextCursor::KeepAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    InsertBlockByTextCommand *pInsertBlockByTextCommand = new InsertBlockByTextCommand(QString("Hei man"),
                                                                                       pWindow->currentWrapper()->textEditor(),
                                                                                       pWindow->currentWrapper());
    QString strRet(pInsertBlockByTextCommand->m_selected);
    ASSERT_TRUE(!strRet.compare(QString("Holle world.")));

    pWindow->deleteLater();
}

TEST_F(test_insertblockbytextcommond, redo)
{
    QString text = "tt";
    for(int i=0;i<1024*1024;i++)
        text += "tt";

    QTextCursor cursor;
    cursor.insertText(text);
    cursor.movePosition(QTextCursor::Start,QTextCursor::KeepAnchor);
    TextEdit* edit = new TextEdit;
    edit->setTextCursor(cursor);
    InsertBlockByTextCommand* com = new InsertBlockByTextCommand(text,edit,nullptr);
    com->redo();

    ASSERT_TRUE(!edit->textCursor().hasSelection());

    delete com;
    com=nullptr;
    edit->deleteLater();
}


TEST_F(test_insertblockbytextcommond, undo)
{
    QString text = "tt";
    for(int i=0;i<1024*1024;i++)
        text += "tt";

    QTextCursor cursor;
    cursor.insertText(text);
    cursor.movePosition(QTextCursor::Start,QTextCursor::KeepAnchor);
    TextEdit* edit = new TextEdit;
    edit->setTextCursor(cursor);
    InsertBlockByTextCommand* com = new InsertBlockByTextCommand(text,edit,nullptr);
    com->undo();

    ASSERT_TRUE(!edit->textCursor().hasSelection());

    delete com;
    com=nullptr;
    edit->deleteLater();
}


TEST_F(test_insertblockbytextcommond, treat)
{
//    Window* window = new Window;
//    EditWrapper* wrapper = new EditWrapper(window);
//    QString text = "tt";
//    for(int i=0;i<1024*1024;i++)
//        text += "tt";

//    QTextCursor cursor;
//    cursor.insertText(text);
//    cursor.movePosition(QTextCursor::Start, QTextCursor::KeepAnchor);
//    TextEdit* edit = new TextEdit(window);
//    edit->m_wrapper = wrapper;
//    edit->setTextCursor(cursor);
//    InsertBlockByTextCommand* com = new InsertBlockByTextCommand(text,edit,wrapper);

//    Stub s1;
//    s1.set(ADDR(Window,setPrintEnabled),retintstub);

//    com->treat();

//    EXPECT_NE(edit,nullptr);
//    delete com;
//    com=nullptr;
//    edit->deleteLater();
//    wrapper->deleteLater();
//    window->deleteLater();
}


TEST_F(test_insertblockbytextcommond, insertByBlock)
{
    QString text = "tt";
    for(int i=0;i<1024*1024;i++)
        text += "tt";

    QTextCursor cursor;
    cursor.insertText(text);
    cursor.movePosition(QTextCursor::Start,QTextCursor::KeepAnchor);
    TextEdit* edit = new TextEdit;
    edit->setTextCursor(cursor);
    InsertBlockByTextCommand* com = new InsertBlockByTextCommand(text,edit,nullptr);
    com->insertByBlock();

    ASSERT_TRUE(!edit->textCursor().hasSelection());

    delete com;
    com=nullptr;
    edit->deleteLater(); 
}
