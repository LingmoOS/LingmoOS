// SPDX-FileCopyrightText: 2022-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_deletebackcommond.h"

#include <QPlainTextEdit>
#include <QTextCursor>

#include "stub.h"

#include "../src/editor/deletebackcommond.h"
#include "../../src/widgets/window.h"

UT_Deletebackcommond::UT_Deletebackcommond() {}

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

UT_Deletebackaltcommond::UT_Deletebackaltcommond() {}

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

    TextEdit *edit = new TextEdit;
    DeleteBackAltCommand *com = new DeleteBackAltCommand(list, edit);

    delete com;
    com = nullptr;
    delete edit;
    edit = nullptr;
}

TEST(UT_Deletebackaltcommond_redo, UT_Deletebackaltcommond_redo)
{
    Window *window = new Window;
    EditWrapper *wrapper = window->createEditor();
    TextEdit *edit = wrapper->textEditor();
    QString text = "test";
    QList<QTextEdit::ExtraSelection> list;
    QTextEdit::ExtraSelection sel;
    QTextCursor cursor;
    cursor.insertText(text);
    cursor.movePosition(QTextCursor::Start, QTextCursor::KeepAnchor);
    sel.cursor = cursor;
    list.push_back(sel);
    list.push_back(sel);
    DeleteBackAltCommand *commond = new DeleteBackAltCommand(list, edit);
    commond->m_deletions = {{"123", 1, 1, 1, cursor, true}};
    commond->redo();

    window->deleteLater();
    wrapper->deleteLater();
    edit->deleteLater();

    delete commond;
    commond = nullptr;
}

TEST(UT_Deletebackaltcommond_undo, UT_Deletebackaltcommond_undo)
{
    Window *window = new Window;
    EditWrapper *wrapper = window->createEditor();
    TextEdit *edit = wrapper->textEditor();
    QString text = "test";
    QList<QTextEdit::ExtraSelection> list;
    QTextEdit::ExtraSelection sel;
    QTextCursor cursor;
    cursor.insertText(text);
    cursor.movePosition(QTextCursor::Start, QTextCursor::KeepAnchor);
    sel.cursor = cursor;
    list.push_back(sel);
    list.push_back(sel);

    DeleteBackAltCommand *com = new DeleteBackAltCommand(list, edit);
    com->m_deletions = {{"123", 1, 1, 1, cursor}};
    com->undo();

    window->deleteLater();
    wrapper->deleteLater();
    edit->deleteLater();
    delete com;
    com = nullptr;
}

TEST(UT_Deletebackaltcommond_MoveCursor, MoveCursor_Undo)
{
    Window *window = new Window;
    EditWrapper *wrapper = window->createEditor();
    TextEdit *edit = wrapper->textEditor();
    QString text = "123456\nabcdef";
    edit->setPlainText(text);

    QList<QTextEdit::ExtraSelection> list;
    QTextEdit::ExtraSelection sel;
    QTextCursor cursor = edit->textCursor();
    // select the left 3 colums: 123|456
    //                           abc|def
    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, 3);
    sel.cursor = cursor;
    list.append(sel);

    cursor.movePosition(QTextCursor::NextBlock);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, 3);
    sel.cursor = cursor;
    list.append(sel);

    DeleteBackAltCommand *com = new DeleteBackAltCommand(list, edit);
    com->redo();
    EXPECT_EQ(edit->toPlainText(), QString("456\ndef"));

    list.clear();
    cursor.movePosition(QTextCursor::End);
    edit->setTextCursor(cursor);

    com->undo();
    EXPECT_EQ(edit->toPlainText(), text);

    window->deleteLater();
    wrapper->deleteLater();
    edit->deleteLater();
    delete com;
    com = nullptr;
}

void stub_slotCanUndoRedoChanged(bool)
{
    // do nothing
}

void stub_command_updateModifyStatus(const QString &, bool)
{
    // do nothing
}

TEST(UT_Deletebackaltcommond_Selection, Selection_ColumnStatus_Restore)
{
    Stub s;
    s.set(ADDR(TextEdit, slotCanUndoChanged), stub_slotCanUndoRedoChanged);
    s.set(ADDR(TextEdit, slotCanRedoChanged), stub_slotCanUndoRedoChanged);
    s.set(ADDR(Window, updateModifyStatus), stub_command_updateModifyStatus);

    Window *window = new Window;
    EditWrapper *wrapper = window->createEditor();
    TextEdit *edit = wrapper->textEditor();
    QString text = "123456\nabcdef";
    edit->setPlainText(text);

    QList<QTextEdit::ExtraSelection> list;
    QTextEdit::ExtraSelection sel;
    QTextCursor cursor = edit->textCursor();
    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, 3);
    sel.cursor = cursor;
    list.append(sel);
    cursor.movePosition(QTextCursor::NextBlock);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, 3);
    sel.cursor = cursor;
    list.append(sel);

    DeleteBackCommand *emptyCommand = new DeleteBackCommand({}, edit);
    edit->m_pUndoStack->push(emptyCommand);

    DeleteBackAltCommand *com = new DeleteBackAltCommand(list, edit);
    edit->m_pUndoStack->push(com);

    edit->undo_();
    // temp command
    edit->undo_();

    EXPECT_TRUE(edit->m_altModSelections.isEmpty());
    EXPECT_FALSE(edit->m_bIsAltMod);

    // temp command
    edit->redo_();

    EXPECT_TRUE(edit->m_altModSelections.isEmpty());
    EXPECT_FALSE(edit->m_bIsAltMod);

    // delete command
    edit->redo_();

    EXPECT_EQ(edit->m_altModSelections.size(), list.size());
    EXPECT_TRUE(edit->m_bIsAltMod);

    window->deleteLater();
    wrapper->deleteLater();
    edit->deleteLater();
}

TEST(UT_Deletebackaltcommond_DelStart, DelStart_TextUpdate_Success)
{
    Stub s;
    s.set(ADDR(TextEdit, slotCanUndoChanged), stub_slotCanUndoRedoChanged);
    s.set(ADDR(TextEdit, slotCanRedoChanged), stub_slotCanUndoRedoChanged);
    s.set(ADDR(Window, updateModifyStatus), stub_command_updateModifyStatus);

    Window *window = new Window;
    EditWrapper *wrapper = window->createEditor();
    TextEdit *edit = wrapper->textEditor();
    QString text = "123456\nabcdef";
    edit->setPlainText(text);

    QList<QTextEdit::ExtraSelection> list;
    QTextEdit::ExtraSelection sel;
    QTextCursor cursor = edit->textCursor();
    // at start of block: |123456
    //                    |abcdef
    cursor.movePosition(QTextCursor::Start);
    sel.cursor = cursor;
    list.append(sel);

    cursor.movePosition(QTextCursor::NextBlock);
    sel.cursor = cursor;
    list.append(sel);

    // backward delete
    DeleteBackAltCommand *comBackward = new DeleteBackAltCommand(list, edit, true);
    edit->m_pUndoStack->push(comBackward);
    EXPECT_EQ(edit->toPlainText(), QString("23456\nbcdef"));

    edit->m_pUndoStack->undo();
    EXPECT_EQ(edit->toPlainText(), text);

    // forward delete
    list[0].cursor.setPosition(0);
    list[1].cursor.setPosition(7);
    DeleteBackAltCommand *comForward = new DeleteBackAltCommand(list, edit);
    edit->m_pUndoStack->push(comForward);
    EXPECT_EQ(edit->toPlainText(), QString("123456abcdef"));

    edit->m_pUndoStack->undo();
    EXPECT_EQ(edit->toPlainText(), text);

    window->deleteLater();
    wrapper->deleteLater();
    edit->deleteLater();
}

TEST(UT_Deletebackaltcommond_DelEnd, DelEnd_TextUpdate_Success)
{
    Stub s;
    s.set(ADDR(TextEdit, slotCanUndoChanged), stub_slotCanUndoRedoChanged);
    s.set(ADDR(TextEdit, slotCanRedoChanged), stub_slotCanUndoRedoChanged);
    s.set(ADDR(Window, updateModifyStatus), stub_command_updateModifyStatus);

    Window *window = new Window;
    EditWrapper *wrapper = window->createEditor();
    TextEdit *edit = wrapper->textEditor();
    QString text = "123456\nabcdef";
    edit->setPlainText(text);

    QList<QTextEdit::ExtraSelection> list;
    QTextEdit::ExtraSelection sel;
    QTextCursor cursor = edit->textCursor();
    // at end of block: 123456|
    //                  abcdef|
    cursor.movePosition(QTextCursor::EndOfBlock);
    sel.cursor = cursor;
    list.append(sel);

    cursor.movePosition(QTextCursor::End);
    sel.cursor = cursor;
    list.append(sel);

    // forward delete
    DeleteBackAltCommand *comForward = new DeleteBackAltCommand(list, edit);
    edit->m_pUndoStack->push(comForward);
    EXPECT_EQ(edit->toPlainText(), QString("12345\nabcde"));

    edit->m_pUndoStack->undo();
    EXPECT_EQ(edit->toPlainText(), text);

    // backward delete
    list[0].cursor.setPosition(6);
    list[1].cursor.setPosition(13);
    DeleteBackAltCommand *comBackward = new DeleteBackAltCommand(list, edit, true);
    edit->m_pUndoStack->push(comBackward);
    EXPECT_EQ(edit->toPlainText(), QString("123456abcdef"));

    edit->m_pUndoStack->undo();
    EXPECT_EQ(edit->toPlainText(), text);

    window->deleteLater();
    wrapper->deleteLater();
    edit->deleteLater();
}
