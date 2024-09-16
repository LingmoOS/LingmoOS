// SPDX-FileCopyrightText: 2022-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_deletebackcommond.h"
#include "../src/editor/deletebackcommond.h"
#include"../../src/widgets/window.h"
#include "qplaintextedit.h"
#include "qtextcursor.h"
UT_Deletebackcommond::UT_Deletebackcommond()
{

}

TEST(UT_Deletebackcommond_DeleteBackCommand, UT_Deletebackcommond_DeleteBackCommand)
{
    QTextCursor cursor;
    QPlainTextEdit *pEdit = new QPlainTextEdit;
    DeleteBackCommand *pCom = new DeleteBackCommand(cursor, pEdit);
    ASSERT_TRUE(pCom->m_insertPos != 0);

    delete pCom;
    pCom = nullptr;
    delete pEdit;
    pEdit = nullptr;
}

TEST(UT_Deletebackcommond_redo, UT_Deletebackcommond_redo)
{
    QString text = "test";
    Window *pWindow = new Window;
    pWindow->addBlankTab(QString());
    QTextCursor cursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertPlainText(QString("12345"));
    cursor.setPosition(10, QTextCursor::MoveMode::KeepAnchor);
    DeleteBackCommand *pCom = new DeleteBackCommand(cursor, pWindow->currentWrapper()->textEditor());
    pCom->m_delText = text;
    pCom->redo();

    ASSERT_EQ(cursor.position(), pWindow->currentWrapper()->textEditor()->textCursor().position());

    delete pCom;
    pCom = nullptr;
    delete pWindow;
    pWindow = nullptr;
}

TEST(UT_Deletebackcommond_undo, UT_Deletebackcommond_undo)
{
    QString text = "test";
    Window *pWindow = new Window;
    pWindow->addBlankTab(QString());
    QTextCursor cursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertPlainText(QString("12345"));
    cursor.setPosition(5, QTextCursor::MoveMode::KeepAnchor);
    DeleteBackCommand *pCom = new DeleteBackCommand(cursor, pWindow->currentWrapper()->textEditor());
    pCom->m_delText = text;
    pCom->undo();

    EXPECT_EQ(cursor.position(), pWindow->currentWrapper()->textEditor()->textCursor().position());

    delete pCom;
    pCom = nullptr;
    pWindow->deleteLater();
}

UT_Deletebackaltcommond::UT_Deletebackaltcommond()
{

}

TEST(UT_Deletebackaltcommond_DeleteBackAltCommand, UT_Deletebackaltcommond_DeleteBackAltCommand)
{
    QString text = "test";
    QList<QTextEdit::ExtraSelection> list;
    QTextEdit::ExtraSelection sel;
    QTextCursor cursor;
    cursor.insertText(text);
    cursor.movePosition(QTextCursor::Start, QTextCursor::KeepAnchor);
    sel.cursor = cursor;
    list.push_back(sel);
    list.push_back(sel);

    QPlainTextEdit* edit = new QPlainTextEdit;
    DeleteBackAltCommand* com = new DeleteBackAltCommand(list, edit);

    delete com;
    com = nullptr;
    delete edit;
    edit = nullptr;
}

TEST(UT_Deletebackaltcommond_redo, UT_Deletebackaltcommond_redo)
{

    Window* window = new Window;
    EditWrapper *wrapper = window->createEditor();
    TextEdit * edit = wrapper->textEditor();
    QString text = "test";
    QList<QTextEdit::ExtraSelection> list;
    QTextEdit::ExtraSelection sel;
    QTextCursor cursor;
    cursor.insertText(text);
    cursor.movePosition(QTextCursor::Start,QTextCursor::KeepAnchor);
    sel.cursor = cursor;
    list.push_back(sel);
    list.push_back(sel);
    DeleteBackAltCommand * commond = new DeleteBackAltCommand(list,edit);
    commond->m_deletions = {{"123",1,1,1,cursor}};
    commond->redo();

    window->deleteLater();
    wrapper->deleteLater();
    edit->deleteLater();

    delete commond;commond=nullptr;
}


TEST(UT_Deletebackaltcommond_undo, UT_Deletebackaltcommond_undo)
{
    Window* window = new Window;
    EditWrapper *wrapper = window->createEditor();
    TextEdit * edit = wrapper->textEditor();
    QString text = "test";
    QList<QTextEdit::ExtraSelection> list;
    QTextEdit::ExtraSelection sel;
    QTextCursor cursor;
    cursor.insertText(text);
    cursor.movePosition(QTextCursor::Start,QTextCursor::KeepAnchor);
    sel.cursor = cursor;
    list.push_back(sel);
    list.push_back(sel);

    DeleteBackAltCommand* com = new DeleteBackAltCommand(list,edit);
    com->m_deletions = {{"123",1,1,1,cursor}};
    com->undo();

    window->deleteLater();
    wrapper->deleteLater();
    edit->deleteLater();
    delete com;
    com = nullptr;

}
